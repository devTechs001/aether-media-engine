// ═══════════════════════════════════════════════════════════════════════════════
// FILE: include/aether/audio/audio_processor.hpp
// DESCRIPTION: Audio processing interface
// ═══════════════════════════════════════════════════════════════════════════════

#ifndef AETHER_AUDIO_AUDIO_PROCESSOR_HPP
#define AETHER_AUDIO_AUDIO_PROCESSOR_HPP

#include "aether/export.hpp"
#include "aether/core/types.hpp"

#include <memory>
#include <span>

namespace aether {

/**
 * @class AudioProcessor
 * @brief Audio processing base class
 */
class AETHER_API AudioProcessor {
public:
    virtual ~AudioProcessor() = default;

    /**
     * @brief Configure processor
     */
    virtual Result<void> Configure(u32 sample_rate, u32 channels) = 0;

    /**
     * @brief Process audio buffer
     */
    virtual Result<void> Process(std::span<f32> buffer) = 0;

    /**
     * @brief Reset processor state
     */
    virtual void Reset() = 0;

    /**
     * @brief Get processor name
     */
    [[nodiscard]] virtual std::string GetName() const = 0;
};

/**
 * @brief Create volume control processor
 */
AETHER_API std::unique_ptr<AudioProcessor> CreateVolumeProcessor(f32 gain);

/**
 * @brief Create equalizer processor
 */
AETHER_API std::unique_ptr<AudioProcessor> CreateEqualizerProcessor();

/**
 * @brief Create compressor processor
 */
AETHER_API std::unique_ptr<AudioProcessor> CreateCompressorProcessor();

/**
 * @brief Create normalizer processor
 */
AETHER_API std::unique_ptr<AudioProcessor> CreateNormalizerProcessor();

/**
 * @brief Create spatial audio processor
 */
AETHER_API std::unique_ptr<AudioProcessor> CreateSpatialProcessor();

/**
 * @brief Create resampler
 */
AETHER_API std::unique_ptr<AudioProcessor> CreateResampler(u32 input_rate, u32 output_rate);

/**
 * @brief Create channel mapper
 */
AETHER_API std::unique_ptr<AudioProcessor> CreateChannelMapper(ChannelLayout input, ChannelLayout output);

} // namespace aether

#endif // AETHER_AUDIO_AUDIO_PROCESSOR_HPP
