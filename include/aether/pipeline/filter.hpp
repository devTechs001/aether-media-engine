// ═══════════════════════════════════════════════════════════════════════════════
// FILE: include/aether/pipeline/filter.hpp
// DESCRIPTION: Filter interface
// ═══════════════════════════════════════════════════════════════════════════════

#ifndef AETHER_PIPELINE_FILTER_HPP
#define AETHER_PIPELINE_FILTER_HPP

#include "aether/export.hpp"
#include "aether/core/types.hpp"
#include "aether/media/frame.hpp"

#include <memory>
#include <string>
#include <unordered_map>
#include <any>

namespace aether {

/**
 * @enum FilterType
 * @brief Type of filter
 */
enum class FilterType : u8 {
    Video,
    Audio,
    Subtitle
};

/**
 * @struct FilterConfig
 * @brief Filter configuration
 */
struct AETHER_API FilterConfig {
    std::string name;
    std::unordered_map<std::string, std::any> options;
};

/**
 * @class Filter
 * @brief Abstract filter interface
 */
class AETHER_API Filter {
public:
    virtual ~Filter() = default;

    /**
     * @brief Configure filter
     */
    virtual Result<void> Configure(const FilterConfig& config) = 0;

    /**
     * @brief Process frame
     */
    virtual Result<FramePtr> Process(FramePtr frame) = 0;

    /**
     * @brief Get filter type
     */
    [[nodiscard]] virtual FilterType GetType() const = 0;

    /**
     * @brief Get filter name
     */
    [[nodiscard]] virtual std::string GetName() const = 0;
};

/**
 * @class VideoFilter
 * @brief Base class for video filters
 */
class AETHER_API VideoFilter : public Filter {
public:
    [[nodiscard]] FilterType GetType() const override { return FilterType::Video; }

protected:
    virtual Result<FramePtr> ProcessVideo(FramePtr frame) = 0;
    Result<FramePtr> Process(FramePtr frame) override final;
};

/**
 * @class AudioFilter
 * @brief Base class for audio filters
 */
class AETHER_API AudioFilter : public Filter {
public:
    [[nodiscard]] FilterType GetType() const override { return FilterType::Audio; }

protected:
    virtual Result<FramePtr> ProcessAudio(FramePtr frame) = 0;
    Result<FramePtr> Process(FramePtr frame) override final;
};

// Common video filters
AETHER_API std::unique_ptr<VideoFilter> CreateScaleFilter(int width, int height);
AETHER_API std::unique_ptr<VideoFilter> CreateCropFilter(int x, int y, int width, int height);
AETHER_API std::unique_ptr<VideoFilter> CreateRotateFilter(float angle);
AETHER_API std::unique_ptr<VideoFilter> CreateDeinterlaceFilter();
AETHER_API std::unique_ptr<VideoFilter> CreateHDRFilter();
AETHER_API std::unique_ptr<VideoFilter> CreateColorFilter(float brightness, float contrast, float saturation);

// Common audio filters
AETHER_API std::unique_ptr<AudioFilter> CreateEqualizerFilter();
AETHER_API std::unique_ptr<AudioFilter> CreateCompressorFilter();
AETHER_API std::unique_ptr<AudioFilter> CreateReverbFilter();
AETHER_API std::unique_ptr<AudioFilter> CreateSpatialFilter();

} // namespace aether

#endif // AETHER_PIPELINE_FILTER_HPP
