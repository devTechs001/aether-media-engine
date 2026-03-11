// ═══════════════════════════════════════════════════════════════════════════════
// FILE: src/render/video/renderer_vulkan.cpp
// ═══════════════════════════════════════════════════════════════════════════════

#include "aether/video/video_renderer.hpp"
#include "aether/core/types.hpp"
#include "aether/utils/logging.hpp"

#ifdef AETHER_HAS_VULKAN
#include <vulkan/vulkan.h>
#endif

#include <vector>
#include <array>
#include <set>
#include <algorithm>

namespace aether {

#ifdef AETHER_HAS_VULKAN

class VulkanRenderer : public VideoRenderer {
public:
    VulkanRenderer() = default;
    ~VulkanRenderer() override { Shutdown(); }

    Result<void> Initialize(const VideoRendererConfig& config) override {
        config_ = config;

        if (auto result = CreateInstance(); !result) return result;
        if (auto result = CreateSurface(config.window_handle); !result) return result;
        if (auto result = PickPhysicalDevice(); !result) return result;
        if (auto result = CreateLogicalDevice(); !result) return result;
        if (auto result = CreateSwapChain(); !result) return result;
        if (auto result = CreateImageViews(); !result) return result;
        if (auto result = CreateRenderPass(); !result) return result;
        if (auto result = CreateGraphicsPipeline(); !result) return result;
        if (auto result = CreateFramebuffers(); !result) return result;
        if (auto result = CreateCommandBuffers(); !result) return result;
        if (auto result = CreateSyncObjects(); !result) return result;

        initialized_ = true;
        GetLogger().Info("Vulkan renderer initialized");
        return {};
    }

    void Shutdown() override {
        if (!initialized_) return;

        vkDeviceWaitIdle(device_);
        CleanupSwapChain();

        for (usize i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
            vkDestroySemaphore(device_, render_finished_semaphores_[i], nullptr);
            vkDestroySemaphore(device_, image_available_semaphores_[i], nullptr);
            vkDestroyFence(device_, in_flight_fences_[i], nullptr);
        }

        vkDestroyCommandPool(device_, command_pool_, nullptr);
        vkDestroyPipeline(device_, graphics_pipeline_, nullptr);
        vkDestroyPipelineLayout(device_, pipeline_layout_, nullptr);
        vkDestroyRenderPass(device_, render_pass_, nullptr);
        vkDestroyDevice(device_, nullptr);
        vkDestroySurfaceKHR(instance_, surface_, nullptr);
        vkDestroyInstance(instance_, nullptr);

        initialized_ = false;
    }

    Result<void> Render(const VideoFrame& frame) override {
        return RenderFrame(frame);
    }

    Result<void> Present() override {
        return {};
    }

    Result<void> Resize(const SizeU& size) override {
        framebuffer_resized_ = true;
        config_.output_size = size;
        return RecreateSwapChain();
    }

    void SetViewport(const Rectangle& viewport) override {
        viewport_ = viewport;
    }

    [[nodiscard]] RendererType GetType() const override {
        return RendererType::Vulkan;
    }

    [[nodiscard]] bool IsHDREnabled() const override {
        return config_.hdr_output;
    }

private:
    static constexpr usize MAX_FRAMES_IN_FLIGHT = 2;

    Result<void> CreateInstance() {
        VkApplicationInfo app_info{};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = "AETHER Media Engine";
        app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.pEngineName = "AETHER";
        app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.apiVersion = VK_API_VERSION_1_3;

        VkInstanceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pApplicationInfo = &app_info;

        std::vector<const char*> extensions = GetRequiredExtensions();
        create_info.enabledExtensionCount = static_cast<u32>(extensions.size());
        create_info.ppEnabledExtensionNames = extensions.data();

        if (vkCreateInstance(&create_info, nullptr, &instance_) != VK_SUCCESS) {
            return Error::Make(ErrorCode::RenderError, "Failed to create Vulkan instance");
        }

        return {};
    }

    std::vector<const char*> GetRequiredExtensions() {
        std::vector<const char*> extensions;

#ifdef AETHER_PLATFORM_WINDOWS
        extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(AETHER_PLATFORM_LINUX)
        extensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#elif defined(AETHER_PLATFORM_MACOS)
        extensions.push_back(VK_EXT_METAL_SURFACE_EXTENSION_NAME);
#endif

        extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
        return extensions;
    }

    Result<void> CreateSurface(void* window_handle) {
#ifdef AETHER_PLATFORM_WINDOWS
        VkWin32SurfaceCreateInfoKHR create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        create_info.hwnd = static_cast<HWND>(window_handle);
        create_info.hinstance = GetModuleHandle(nullptr);

        if (vkCreateWin32SurfaceKHR(instance_, &create_info, nullptr, &surface_) != VK_SUCCESS) {
            return Error::Make(ErrorCode::RenderError, "Failed to create window surface");
        }
#endif
        (void)window_handle;
        return {};
    }

    Result<void> PickPhysicalDevice() {
        u32 device_count = 0;
        vkEnumeratePhysicalDevices(instance_, &device_count, nullptr);

        if (device_count == 0) {
            return Error::Make(ErrorCode::RenderError, "No Vulkan-capable GPU found");
        }

        std::vector<VkPhysicalDevice> devices(device_count);
        vkEnumeratePhysicalDevices(instance_, &device_count, devices.data());

        for (const auto& device : devices) {
            if (IsDeviceSuitable(device)) {
                physical_device_ = device;
                break;
            }
        }

        if (physical_device_ == VK_NULL_HANDLE) {
            return Error::Make(ErrorCode::RenderError, "No suitable GPU found");
        }

        return {};
    }

    bool IsDeviceSuitable(VkPhysicalDevice device) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(device, &props);

        // Prefer discrete GPU
        if (props.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            return false;
        }

        // Check extensions
        u32 extension_count = 0;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);
        std::vector<VkExtensionProperties> available(extension_count);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available.data());

        std::set<std::string> required = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
        for (const auto& ext : available) {
            required.erase(ext.extensionName);
        }

        return required.empty();
    }

    Result<void> CreateLogicalDevice() {
        u32 queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device_, &queue_family_count, nullptr);

        std::vector<VkQueueFamilyProperties> families(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device_, &queue_family_count, families.data());

        // Find graphics queue
        for (u32 i = 0; i < queue_family_count; ++i) {
            if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                graphics_queue_family_ = i;
                break;
            }
        }

        if (graphics_queue_family_ == UINT32_MAX) {
            return Error::Make(ErrorCode::RenderError, "No graphics queue family found");
        }

        float queue_priority = 1.0f;
        VkDeviceQueueCreateInfo queue_create_info{};
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = graphics_queue_family_;
        queue_create_info.queueCount = 1;
        queue_create_info.pQueuePriorities = &queue_priority;

        VkDeviceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        create_info.queueCreateInfoCount = 1;
        create_info.pQueueCreateInfos = &queue_create_info;

        const char* extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
        create_info.enabledExtensionCount = 1;
        create_info.ppEnabledExtensionNames = extensions;

        if (vkCreateDevice(physical_device_, &create_info, nullptr, &device_) != VK_SUCCESS) {
            return Error::Make(ErrorCode::RenderError, "Failed to create logical device");
        }

        vkGetDeviceQueue(device_, graphics_queue_family_, 0, &graphics_queue_);
        return {};
    }

    Result<void> CreateSwapChain() {
        // Get surface capabilities
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device_, surface_, &capabilities);

        // Choose format
        VkSurfaceFormatKHR format = ChooseSwapSurfaceFormat();
        VkPresentModeKHR present_mode = ChooseSwapPresentMode();
        VkExtent2D extent = ChooseSwapExtent(capabilities);

        VkSwapchainCreateInfoKHR create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        create_info.surface = surface_;
        create_info.minImageCount = capabilities.minImageCount + 1;
        create_info.imageFormat = format.format;
        create_info.imageColorSpace = format.colorSpace;
        create_info.imageExtent = extent;
        create_info.imageArrayLayers = 1;
        create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        create_info.preTransform = capabilities.currentTransform;
        create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        create_info.presentMode = present_mode;
        create_info.clipped = VK_TRUE;
        create_info.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(device_, &create_info, nullptr, &swap_chain_) != VK_SUCCESS) {
            return Error::Make(ErrorCode::RenderError, "Failed to create swap chain");
        }

        swap_chain_format_ = format.format;
        swap_chain_extent_ = extent;

        return {};
    }

    VkSurfaceFormatKHR ChooseSwapSurfaceFormat() {
        u32 format_count = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_, surface_, &format_count, nullptr);

        std::vector<VkSurfaceFormatKHR> formats(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_, surface_, &format_count, formats.data());

        for (const auto& fmt : formats) {
            if (fmt.format == VK_FORMAT_B8G8R8A8_SRGB &&
                fmt.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return fmt;
            }
        }

        return formats[0];
    }

    VkPresentModeKHR ChooseSwapPresentMode() {
        u32 mode_count = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device_, surface_, &mode_count, nullptr);

        std::vector<VkPresentModeKHR> modes(mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device_, surface_, &mode_count, modes.data());

        // Prefer mailbox (low latency) or FIFO (vsync)
        for (const auto& mode : modes) {
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return mode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
        if (capabilities.currentExtent.width != UINT32_MAX) {
            return capabilities.currentExtent;
        }

        VkExtent2D extent = {
            std::clamp(config_.output_size.width,
                      capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
            std::clamp(config_.output_size.height,
                      capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
        };

        return extent;
    }

    Result<void> RecreateSwapChain() {
        vkDeviceWaitIdle(device_);
        CleanupSwapChain();
        return CreateSwapChain();
    }

    void CleanupSwapChain() {
        vkDestroySwapchainKHR(device_, swap_chain_, nullptr);
    }

    Result<void> CreateImageViews() {
        u32 image_count = 0;
        vkGetSwapchainImagesKHR(device_, swap_chain_, &image_count, nullptr);

        swap_chain_images_.resize(image_count);
        vkGetSwapchainImagesKHR(device_, swap_chain_, &image_count, swap_chain_images_.data());

        swap_chain_image_views_.resize(image_count);
        for (u32 i = 0; i < image_count; ++i) {
            VkImageViewCreateInfo create_info{};
            create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            create_info.image = swap_chain_images_[i];
            create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            create_info.format = swap_chain_format_;
            create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            create_info.subresourceRange.baseMipLevel = 0;
            create_info.subresourceRange.levelCount = 1;
            create_info.subresourceRange.baseArrayLayer = 0;
            create_info.subresourceRange.layerCount = 1;

            if (vkCreateImageView(device_, &create_info, nullptr, &swap_chain_image_views_[i]) != VK_SUCCESS) {
                return Error::Make(ErrorCode::RenderError, "Failed to create image view");
            }
        }

        return {};
    }

    Result<void> CreateRenderPass() {
        VkAttachmentDescription color_attachment{};
        color_attachment.format = swap_chain_format_;
        color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference color_ref{};
        color_ref.attachment = 0;
        color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_ref;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        create_info.attachmentCount = 1;
        create_info.pAttachments = &color_attachment;
        create_info.subpassCount = 1;
        create_info.pSubpasses = &subpass;
        create_info.dependencyCount = 1;
        create_info.pDependencies = &dependency;

        if (vkCreateRenderPass(device_, &create_info, nullptr, &render_pass_) != VK_SUCCESS) {
            return Error::Make(ErrorCode::RenderError, "Failed to create render pass");
        }

        return {};
    }

    Result<void> CreateGraphicsPipeline() {
        // Shader stages would be loaded from compiled SPIR-V files
        // For now, create placeholder

        VkPipelineShaderStageCreateInfo vert_stage{};
        vert_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vert_stage.stage = VK_SHADER_STAGE_VERTEX_BIT;

        VkPipelineShaderStageCreateInfo frag_stage{};
        frag_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        frag_stage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkPipelineShaderStageCreateInfo stages[] = {vert_stage, frag_stage};

        VkPipelineVertexInputStateCreateInfo vertex_info{};
        vertex_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        VkPipelineInputAssemblyStateCreateInfo input_assembly{};
        input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<f32>(swap_chain_extent_.width);
        viewport.height = static_cast<f32>(swap_chain_extent_.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = swap_chain_extent_;

        VkPipelineViewportStateCreateInfo viewport_state{};
        viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state.viewportCount = 1;
        viewport_state.pViewports = &viewport;
        viewport_state.scissorCount = 1;
        viewport_state.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState color_blend_attachment{};
        color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                                VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        color_blend_attachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo color_blending{};
        color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blending.logicOpEnable = VK_FALSE;
        color_blending.attachmentCount = 1;
        color_blending.pAttachments = &color_blend_attachment;

        VkPipelineLayoutCreateInfo layout_info{};
        layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

        if (vkCreatePipelineLayout(device_, &layout_info, nullptr, &pipeline_layout_) != VK_SUCCESS) {
            return Error::Make(ErrorCode::RenderError, "Failed to create pipeline layout");
        }

        VkGraphicsPipelineCreateInfo pipeline_info{};
        pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipeline_info.stageCount = 2;
        pipeline_info.pStages = stages;
        pipeline_info.pVertexInputState = &vertex_info;
        pipeline_info.pInputAssemblyState = &input_assembly;
        pipeline_info.pViewportState = &viewport_state;
        pipeline_info.pRasterizationState = &rasterizer;
        pipeline_info.pMultisampleState = &multisampling;
        pipeline_info.pColorBlendState = &color_blending;
        pipeline_info.layout = pipeline_layout_;
        pipeline_info.renderPass = render_pass_;
        pipeline_info.subpass = 0;

        if (vkCreateGraphicsPipelines(device_, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &graphics_pipeline_) != VK_SUCCESS) {
            return Error::Make(ErrorCode::RenderError, "Failed to create graphics pipeline");
        }

        return {};
    }

    Result<void> CreateFramebuffers() {
        u32 image_count = static_cast<u32>(swap_chain_image_views_.size());
        framebuffers_.resize(image_count);

        for (u32 i = 0; i < image_count; ++i) {
            VkImageView attachments[] = {swap_chain_image_views_[i]};

            VkFramebufferCreateInfo create_info{};
            create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            create_info.renderPass = render_pass_;
            create_info.attachmentCount = 1;
            create_info.pAttachments = attachments;
            create_info.width = swap_chain_extent_.width;
            create_info.height = swap_chain_extent_.height;
            create_info.layers = 1;

            if (vkCreateFramebuffer(device_, &create_info, nullptr, &framebuffers_[i]) != VK_SUCCESS) {
                return Error::Make(ErrorCode::RenderError, "Failed to create framebuffer");
            }
        }

        return {};
    }

    Result<void> CreateCommandBuffers() {
        VkCommandPoolCreateInfo pool_info{};
        pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pool_info.queueFamilyIndex = graphics_queue_family_;

        if (vkCreateCommandPool(device_, &pool_info, nullptr, &command_pool_) != VK_SUCCESS) {
            return Error::Make(ErrorCode::RenderError, "Failed to create command pool");
        }

        command_buffers_.resize(MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.commandPool = command_pool_;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandBufferCount = static_cast<u32>(command_buffers_.size());

        if (vkAllocateCommandBuffers(device_, &alloc_info, command_buffers_.data()) != VK_SUCCESS) {
            return Error::Make(ErrorCode::RenderError, "Failed to allocate command buffers");
        }

        return {};
    }

    Result<void> CreateSyncObjects() {
        image_available_semaphores_.resize(MAX_FRAMES_IN_FLIGHT);
        render_finished_semaphores_.resize(MAX_FRAMES_IN_FLIGHT);
        in_flight_fences_.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphore_info{};
        semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fence_info{};
        fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (usize i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
            if (vkCreateSemaphore(device_, &semaphore_info, nullptr, &image_available_semaphores_[i]) != VK_SUCCESS ||
                vkCreateSemaphore(device_, &semaphore_info, nullptr, &render_finished_semaphores_[i]) != VK_SUCCESS ||
                vkCreateFence(device_, &fence_info, nullptr, &in_flight_fences_[i]) != VK_SUCCESS) {
                return Error::Make(ErrorCode::RenderError, "Failed to create sync objects");
            }
        }

        return {};
    }

    Result<void> RenderFrame(const VideoFrame& frame) {
        (void)frame;
        // Simplified - full implementation would upload texture and render
        return {};
    }

    void RecordCommandBuffer(VkCommandBuffer cmd_buf, u32 image_index) {
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        vkBeginCommandBuffer(cmd_buf, &begin_info);

        VkRenderPassBeginInfo render_pass_info{};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_info.renderPass = render_pass_;
        render_pass_info.framebuffer = framebuffers_[image_index];
        render_pass_info.renderArea.offset = {0, 0};
        render_pass_info.renderArea.extent = swap_chain_extent_;

        VkClearValue clear_color{{0.0f, 0.0f, 0.0f, 1.0f}};
        render_pass_info.clearValueCount = 1;
        render_pass_info.pClearValues = &clear_color;

        vkCmdBeginRenderPass(cmd_buf, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_);
        vkCmdDraw(cmd_buf, 3, 1, 0, 0);
        vkCmdEndRenderPass(cmd_buf);

        vkEndCommandBuffer(cmd_buf);
    }

    // Member variables
    VkInstance instance_ = VK_NULL_HANDLE;
    VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
    VkSurfaceKHR surface_ = VK_NULL_HANDLE;
    VkQueue graphics_queue_ = VK_NULL_HANDLE;
    u32 graphics_queue_family_ = UINT32_MAX;

    VkSwapchainKHR swap_chain_ = VK_NULL_HANDLE;
    VkFormat swap_chain_format_;
    VkExtent2D swap_chain_extent_;
    std::vector<VkImage> swap_chain_images_;
    std::vector<VkImageView> swap_chain_image_views_;

    VkRenderPass render_pass_ = VK_NULL_HANDLE;
    VkPipelineLayout pipeline_layout_ = VK_NULL_HANDLE;
    VkPipeline graphics_pipeline_ = VK_NULL_HANDLE;

    std::vector<VkFramebuffer> framebuffers_;
    VkCommandPool command_pool_ = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> command_buffers_;

    std::vector<VkSemaphore> image_available_semaphores_;
    std::vector<VkSemaphore> render_finished_semaphores_;
    std::vector<VkFence> in_flight_fences_;
    u32 current_frame_ = 0;

    VideoRendererConfig config_;
    Rectangle viewport_;
    bool initialized_ = false;
    bool framebuffer_resized_ = false;
    u32 frame_count_ = 0;
};

std::unique_ptr<VideoRenderer> CreateVulkanRenderer() {
    return std::make_unique<VulkanRenderer>();
}

#else // AETHER_HAS_VULKAN

std::unique_ptr<VideoRenderer> CreateVulkanRenderer() {
    return nullptr;
}

#endif // AETHER_HAS_VULKAN

} // namespace aether
