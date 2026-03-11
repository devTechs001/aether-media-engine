// ═══════════════════════════════════════════════════════════════════════════════
// FILE: src/pipeline/pipeline_impl.cpp
// ═══════════════════════════════════════════════════════════════════════════════

#include "aether/pipeline/pipeline.hpp"
#include "aether/utils/logging.hpp"

#include <mutex>
#include <atomic>
#include <thread>
#include <queue>
#include <condition_variable>

namespace aether {

class Pipeline::Impl {
public:
    std::mutex mutex;
    PipelineState state = PipelineState::Created;
    PipelineConfig config;
    
    std::unique_ptr<Demuxer> demuxer;
    std::unique_ptr<Decoder> video_decoder;
    std::unique_ptr<Decoder> audio_decoder;
    std::vector<std::unique_ptr<Filter>> video_filters;
    std::vector<std::unique_ptr<Filter>> audio_filters;
    std::unique_ptr<Renderer> renderer;
    
    std::atomic<bool> running{false};
    std::thread worker_thread;
    std::condition_variable cv;
    
    std::queue<FramePtr> frame_queue;
    static constexpr usize MAX_QUEUE_SIZE = 30;
};

Pipeline::Pipeline() : impl_(std::make_unique<Impl>()) {}

Pipeline::~Pipeline() {
    Stop();
}

Result<void> Pipeline::Configure(const PipelineConfig& config) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    
    if (impl_->state != PipelineState::Created) {
        return Error::Make(ErrorCode::InvalidState, "Pipeline already configured");
    }
    
    impl_->config = config;
    impl_->state = PipelineState::Configured;
    return {};
}

Result<void> Pipeline::Start() {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    
    if (impl_->state != PipelineState::Configured) {
        return Error::Make(ErrorCode::InvalidState, "Pipeline not configured");
    }
    
    impl_->running = true;
    impl_->state = PipelineState::Running;
    
    impl_->worker_thread = std::thread([this]() {
        while (impl_->running) {
            ProcessFrame();
        }
    });
    
    return {};
}

void Pipeline::Pause() {
    impl_->running = false;
    impl_->state = PipelineState::Paused;
}

void Pipeline::Resume() {
    impl_->running = true;
    impl_->state = PipelineState::Running;
}

void Pipeline::Stop() {
    impl_->running = false;
    impl_->state = PipelineState::Stopped;
    
    if (impl_->worker_thread.joinable()) {
        impl_->worker_thread.join();
    }
    
    std::lock_guard<std::mutex> lock(impl_->mutex);
    while (!impl_->frame_queue.empty()) {
        impl_->frame_queue.pop();
    }
}

PipelineState Pipeline::GetState() const {
    return impl_->state;
}

Result<void> Pipeline::AddDemuxer(std::unique_ptr<Demuxer> demuxer) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->demuxer = std::move(demuxer);
    return {};
}

Result<void> Pipeline::AddDecoder(std::unique_ptr<Decoder> decoder) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    
    // Determine decoder type based on context
    if (!impl_->video_decoder) {
        impl_->video_decoder = std::move(decoder);
    } else if (!impl_->audio_decoder) {
        impl_->audio_decoder = std::move(decoder);
    }
    return {};
}

Result<void> Pipeline::AddFilter(std::unique_ptr<Filter> filter) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    
    if (!filter) {
        return Error::Make(ErrorCode::InvalidArgument, "Null filter");
    }
    
    if (filter->GetType() == FilterType::Video) {
        impl_->video_filters.push_back(std::move(filter));
    } else if (filter->GetType() == FilterType::Audio) {
        impl_->audio_filters.push_back(std::move(filter));
    }
    
    return {};
}

Result<void> Pipeline::AddRenderer(std::unique_ptr<Renderer> renderer) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->renderer = std::move(renderer);
    return {};
}

Result<void> Pipeline::ProcessFrame() {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    
    if (!impl_->demuxer) {
        return Error::Make(ErrorCode::InvalidState, "No demuxer");
    }
    
    // Read packet
    auto packet_result = impl_->demuxer->ReadPacket();
    if (!packet_result) {
        return packet_result.error();
    }
    
    // Decode based on packet type
    FramePtr frame;
    if ((*packet_result)->type == MediaType::Video && impl_->video_decoder) {
        auto decode_result = impl_->video_decoder->Decode(**packet_result);
        if (!decode_result) {
            return decode_result.error();
        }
        frame = std::move(decode_result.value());
    } else if ((*packet_result)->type == MediaType::Audio && impl_->audio_decoder) {
        auto decode_result = impl_->audio_decoder->Decode(**packet_result);
        if (!decode_result) {
            return decode_result.error();
        }
        frame = std::move(decode_result.value());
    }
    
    if (!frame) {
        return {};
    }
    
    // Apply filters
    if (frame->GetType() == Frame::Type::Video) {
        for (auto& filter : impl_->video_filters) {
            auto filter_result = filter->Process(std::move(frame));
            if (!filter_result) {
                return filter_result.error();
            }
            frame = std::move(filter_result.value());
        }
    }
    
    // Queue frame
    if (impl_->frame_queue.size() >= Impl::MAX_QUEUE_SIZE) {
        impl_->frame_queue.pop();
    }
    impl_->frame_queue.push(std::move(frame));
    
    return {};
}

Result<void> Pipeline::Flush() {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    
    if (impl_->video_decoder) {
        impl_->video_decoder->Flush();
    }
    if (impl_->audio_decoder) {
        impl_->audio_decoder->Flush();
    }
    
    while (!impl_->frame_queue.empty()) {
        impl_->frame_queue.pop();
    }
    
    return {};
}

Result<void> Pipeline::Reset() {
    Stop();
    impl_->state = PipelineState::Created;
    return {};
}

std::unique_ptr<Pipeline> CreatePipeline(const PipelineConfig& config) {
    auto pipeline = std::make_unique<Pipeline>();
    pipeline->Configure(config);
    return pipeline;
}

} // namespace aether
