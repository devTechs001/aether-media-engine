// ═══════════════════════════════════════════════════════════════════════════════
// FILE: include/aether/video/video_frame.hpp
// DESCRIPTION: Video frame utilities
// ═══════════════════════════════════════════════════════════════════════════════

#ifndef AETHER_VIDEO_VIDEO_FRAME_HPP
#define AETHER_VIDEO_VIDEO_FRAME_HPP

#include "aether/export.hpp"
#include "aether/core/types.hpp"
#include "aether/media/frame.hpp"

#include <memory>
#include <span>

namespace aether {

/**
 * @brief Convert video frame to different pixel format
 */
AETHER_API Result<VideoFrame> ConvertPixelFormat(const VideoFrame& input, PixelFormat output_format);

/**
 * @brief Scale video frame
 */
AETHER_API Result<VideoFrame> ScaleVideoFrame(const VideoFrame& input, const SizeU& output_size);

/**
 * @brief Copy video frame
 */
AETHER_API VideoFrame CopyVideoFrame(const VideoFrame& input);

/**
 * @brief Get bytes per pixel for format
 */
AETHER_API int GetBytesPerPixel(PixelFormat format);

/**
 * @brief Get plane count for format
 */
AETHER_API int GetPlaneCount(PixelFormat format);

/**
 * @brief Check if format is planar
 */
AETHER_API bool IsPlanarFormat(PixelFormat format);

/**
 * @brief Check if format is hardware format
 */
AETHER_API bool IsHardwareFormat(PixelFormat format);

/**
 * @brief Check if format is HDR
 */
AETHER_API bool IsHDRFormat(PixelFormat format);

/**
 * @brief Get chroma subsampling for format
 */
AETHER_API void GetChromaSubsampling(PixelFormat format, int& h, int& v);

} // namespace aether

#endif // AETHER_VIDEO_VIDEO_FRAME_HPP
