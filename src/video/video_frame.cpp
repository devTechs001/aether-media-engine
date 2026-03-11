// ═══════════════════════════════════════════════════════════════════════════════
// FILE: src/video/video_frame.cpp
// ═══════════════════════════════════════════════════════════════════════════════

#include "aether/video/video_frame.hpp"
#include "aether/utils/logging.hpp"

#include <cstring>
#include <algorithm>
#include <mutex>

namespace aether {

// ═══════════════════════════════════════════════════════════════════════════════
// Pixel Format Helpers
// ═══════════════════════════════════════════════════════════════════════════════

int GetPlaneCount(PixelFormat format) {
    switch (format) {
        case PixelFormat::YUV420P:
        case PixelFormat::YUV422P:
        case PixelFormat::YUV444P:
        case PixelFormat::YUV420P10:
            return 3;
        case PixelFormat::NV12:
        case PixelFormat::NV21:
        case PixelFormat::P010:
            return 2;
        case PixelFormat::RGB24:
        case PixelFormat::BGR24:
        case PixelFormat::RGBA32:
        case PixelFormat::BGRA32:
        case PixelFormat::ARGB32:
        case PixelFormat::Gray8:
        case PixelFormat::Gray16:
            return 1;
        default:
            return 0;
    }
}

int GetBytesPerPixel(PixelFormat format) {
    switch (format) {
        case PixelFormat::YUV420P:
            return 1;  // 1.5 bytes per pixel (averaged)
        case PixelFormat::YUV422P:
            return 2;
        case PixelFormat::YUV444P:
            return 3;
        case PixelFormat::YUV420P10:
        case PixelFormat::P010:
            return 2;  // 10-bit, 2 bytes per component
        case PixelFormat::NV12:
            return 1;  // 1.5 bytes per pixel (averaged)
        case PixelFormat::RGB24:
        case PixelFormat::BGR24:
            return 3;
        case PixelFormat::RGBA32:
        case PixelFormat::BGRA32:
        case PixelFormat::ARGB32:
            return 4;
        case PixelFormat::Gray8:
            return 1;
        case PixelFormat::Gray16:
            return 2;
        default:
            return 0;
    }
}

bool IsPlanarFormat(PixelFormat format) {
    switch (format) {
        case PixelFormat::YUV420P:
        case PixelFormat::YUV422P:
        case PixelFormat::YUV444P:
        case PixelFormat::YUV420P10:
        case PixelFormat::YUV420P12:
        case PixelFormat::YUV420P16:
        case PixelFormat::YUV422P10:
        case PixelFormat::YUV444P10:
        case PixelFormat::YUV444P16:
            return true;
        default:
            return false;
    }
}

bool IsHardwareFormat(PixelFormat format) {
    switch (format) {
        case PixelFormat::D3D11:
        case PixelFormat::DXVA2:
        case PixelFormat::VAAPI:
        case PixelFormat::VDPAU:
        case PixelFormat::VideoToolbox:
        case PixelFormat::MediaCodec:
        case PixelFormat::CUDA:
        case PixelFormat::QSV:
        case PixelFormat::Vulkan:
            return true;
        default:
            return false;
    }
}

bool IsHDRFormat(PixelFormat format) {
    switch (format) {
        case PixelFormat::YUV420P10_HDR10:
        case PixelFormat::YUV420P10_HLG:
        case PixelFormat::YUV420P10_DolbyVision:
            return true;
        default:
            return false;
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
// VideoFrame Implementation
// ═══════════════════════════════════════════════════════════════════════════════

VideoFrame::VideoFrame() = default;

VideoFrame::VideoFrame(u32 width, u32 height, PixelFormat format) {
    Allocate(width, height, format);
}

VideoFrame::~VideoFrame() {
    Release();
}

VideoFrame::VideoFrame(const VideoFrame& other) {
    *this = other;
}

VideoFrame& VideoFrame::operator=(const VideoFrame& other) {
    if (this != &other) {
        Release();

        width_ = other.width_;
        height_ = other.height_;
        format_ = other.format_;
        timestamp_ = other.timestamp_;
        duration_ = other.duration_;
        color_space_ = other.color_space_;
        color_range_ = other.color_range_;
        hdr_type_ = other.hdr_type_;
        key_frame_ = other.key_frame_;
        interlaced_ = other.interlaced_;
        rotation_ = other.rotation_;

        if (other.owns_data_ && other.IsValid()) {
            Allocate(width_, height_, format_);

            for (u32 p = 0; p < PlaneCount(); ++p) {
                if (data_[p] && other.data_[p]) {
                    usize plane_size = stride_[p] * PlaneHeight(p);
                    std::memcpy(data_[p], other.data_[p], plane_size);
                }
            }
        } else if (!other.owns_data_) {
            data_ = other.data_;
            stride_ = other.stride_;
            owns_data_ = false;
        }
    }
    return *this;
}

VideoFrame::VideoFrame(VideoFrame&& other) noexcept {
    *this = std::move(other);
}

VideoFrame& VideoFrame::operator=(VideoFrame&& other) noexcept {
    if (this != &other) {
        Release();

        width_ = other.width_;
        height_ = other.height_;
        format_ = other.format_;
        data_ = other.data_;
        stride_ = other.stride_;
        owns_data_ = other.owns_data_;
        timestamp_ = other.timestamp_;
        duration_ = other.duration_;
        color_space_ = other.color_space_;
        color_range_ = other.color_range_;
        hdr_type_ = other.hdr_type_;
        key_frame_ = other.key_frame_;
        interlaced_ = other.interlaced_;
        rotation_ = other.rotation_;

        other.width_ = 0;
        other.height_ = 0;
        other.format_ = PixelFormat::Unknown;
        other.data_ = {};
        other.stride_ = {};
        other.owns_data_ = false;
    }
    return *this;
}

void VideoFrame::Allocate(u32 width, u32 height, PixelFormat format) {
    Release();

    width_ = width;
    height_ = height;
    format_ = format;
    owns_data_ = true;

    int plane_count = GetPlaneCount(format);
    if (plane_count == 0) {
        return;
    }

    // Calculate strides and allocate data
    for (int p = 0; p < plane_count; ++p) {
        u32 plane_width = PlaneWidth(p);
        u32 plane_height = PlaneHeight(p);
        u32 bytes_per_pixel = (format == PixelFormat::YUV420P10 || 
                               format == PixelFormat::P010) ? 2 : 1;

        stride_[p] = plane_width * bytes_per_pixel;
        usize plane_size = stride_[p] * plane_height;

        data_[p] = new u8[plane_size];
        std::memset(data_[p], 0, plane_size);
    }
}

void VideoFrame::Release() {
    if (owns_data_) {
        for (int p = 0; p < 4; ++p) {
            if (data_[p]) {
                delete[] data_[p];
                data_[p] = nullptr;
            }
        }
    }
    data_ = {};
    stride_ = {};
    owns_data_ = false;
    width_ = 0;
    height_ = 0;
    format_ = PixelFormat::Unknown;
}

bool VideoFrame::IsValid() const {
    return width_ > 0 && height_ > 0 && format_ != PixelFormat::Unknown &&
           data_[0] != nullptr;
}

u32 VideoFrame::PlaneWidth(u32 plane) const {
    u32 subsample_w = (format_ == PixelFormat::YUV420P || 
                       format_ == PixelFormat::YUV420P10 ||
                       format_ == PixelFormat::NV12) ? 2 : 1;
    return (plane == 0) ? width_ : (width_ + subsample_w - 1) / subsample_w;
}

u32 VideoFrame::PlaneHeight(u32 plane) const {
    u32 subsample_h = (format_ == PixelFormat::YUV420P || 
                       format_ == PixelFormat::YUV420P10 ||
                       format_ == PixelFormat::NV12) ? 2 : 1;
    return (plane == 0) ? height_ : (height_ + subsample_h - 1) / subsample_h;
}

u32 VideoFrame::PlaneCount() const {
    return static_cast<u32>(GetPlaneCount(format_));
}

usize VideoFrame::DataSize() const {
    usize total = 0;
    for (u32 p = 0; p < PlaneCount(); ++p) {
        total += stride_[p] * PlaneHeight(p);
    }
    return total;
}

VideoFrame VideoFrame::Create(u32 width, u32 height, PixelFormat format) {
    return VideoFrame(width, height, format);
}

VideoFrame VideoFrame::Wrap(u8* data, u32 width, u32 height, i32 stride,
                            PixelFormat format) {
    VideoFrame frame;
    frame.width_ = width;
    frame.height_ = height;
    frame.format_ = format;
    frame.data_[0] = data;
    frame.stride_[0] = stride;
    frame.owns_data_ = false;
    return frame;
}

VideoFrame VideoFrame::Copy() const {
    return VideoFrame(*this);
}

void VideoFrame::Zero() {
    for (u32 p = 0; p < PlaneCount(); ++p) {
        if (data_[p]) {
            std::memset(data_[p], 0, stride_[p] * PlaneHeight(p));
        }
    }
}

void VideoFrame::Fill(u8 value) {
    for (u32 p = 0; p < PlaneCount(); ++p) {
        if (data_[p]) {
            std::memset(data_[p], value, stride_[p] * PlaneHeight(p));
        }
    }
}

} // namespace aether
