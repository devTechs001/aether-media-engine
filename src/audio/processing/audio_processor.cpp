// ═══════════════════════════════════════════════════════════════════════════════
// FILE: src/audio/processing/audio_processor.cpp
// ═══════════════════════════════════════════════════════════════════════════════

#include "aether/audio/audio_processor.hpp"
#include "aether/utils/logging.hpp"

#include <cmath>
#include <algorithm>

namespace aether {

// ═══════════════════════════════════════════════════════════════════════════════
// Volume Processor
// ═══════════════════════════════════════════════════════════════════════════════

class VolumeProcessor : public AudioProcessor {
public:
    explicit VolumeProcessor(f32 gain) : gain_(gain) {}

    Result<void> Configure(u32 sample_rate, u32 channels) override {
        sample_rate_ = sample_rate;
        channels_ = channels;
        return {};
    }

    Result<void> Process(std::span<f32> buffer) override {
        for (auto& sample : buffer) {
            sample *= gain_;
            // Clip prevention
            sample = std::clamp(sample, -1.0f, 1.0f);
        }
        return {};
    }

    void Reset() override {}

    [[nodiscard]] std::string GetName() const override { return "volume"; }

    void SetGain(f32 gain) { gain_ = gain; }
    [[nodiscard]] f32 GetGain() const { return gain_; }

private:
    f32 gain_;
    u32 sample_rate_ = 0;
    u32 channels_ = 0;
};

// ═══════════════════════════════════════════════════════════════════════════════
// Equalizer Processor (simplified)
// ═══════════════════════════════════════════════════════════════════════════════

class EqualizerProcessor : public AudioProcessor {
public:
    EqualizerProcessor() {
        // Initialize 10-band EQ with flat response
        bands_.resize(10, 0.0f);
        center_frequencies_ = {31, 62, 125, 250, 500, 1000, 2000, 4000, 8000, 16000};
    }

    Result<void> Configure(u32 sample_rate, u32 channels) override {
        sample_rate_ = sample_rate;
        channels_ = channels;
        return {};
    }

    Result<void> Process(std::span<f32> buffer) override {
        // Simplified - in production would apply actual EQ filters
        // Using biquad filters for each band
        (void)buffer;
        return {};
    }

    void Reset() override {}

    [[nodiscard]] std::string GetName() const override { return "equalizer"; }

    void SetBand(u32 band, f32 gain_db) {
        if (band < bands_.size()) {
            bands_[band] = gain_db;
        }
    }

    [[nodiscard]] f32 GetBand(u32 band) const {
        if (band < bands_.size()) {
            return bands_[band];
        }
        return 0.0f;
    }

    void SetPreset(const std::string& preset) {
        // Apply EQ preset
        if (preset == "flat") {
            std::fill(bands_.begin(), bands_.end(), 0.0f);
        } else if (preset == "bass") {
            bands_ = {6, 5, 4, 2, 0, 0, 0, 0, 0, 0};
        } else if (preset == "treble") {
            bands_ = {0, 0, 0, 0, 0, 1, 2, 4, 5, 6};
        } else if (preset == "rock") {
            bands_ = {5, 4, 3, 1, -1, -1, 0, 2, 3, 4};
        } else if (preset == "pop") {
            bands_ = {-1, 2, 4, 5, 3, 0, -1, -1, -1, -1};
        } else if (preset == "jazz") {
            bands_ = {3, 2, 1, 2, -2, -2, 0, 1, 2, 3};
        } else if (preset == "classical") {
            bands_ = {4, 3, 2, 1, -1, -1, 0, 2, 3, 4};
        } else if (preset == "vocal") {
            bands_ = {-2, -1, 0, 3, 5, 5, 3, 0, -1, -2};
        }
    }

private:
    std::vector<f32> bands_;
    std::vector<u32> center_frequencies_;
    u32 sample_rate_ = 0;
    u32 channels_ = 0;
};

// ═══════════════════════════════════════════════════════════════════════════════
// Compressor Processor (simplified)
// ═══════════════════════════════════════════════════════════════════════════════

class CompressorProcessor : public AudioProcessor {
public:
    CompressorProcessor() = default;

    Result<void> Configure(u32 sample_rate, u32 channels) override {
        sample_rate_ = sample_rate;
        channels_ = channels;
        return {};
    }

    Result<void> Process(std::span<f32> buffer) override {
        // Simplified - in production would apply dynamic range compression
        for (auto& sample : buffer) {
            // Soft clipping
            if (sample > threshold_) {
                sample = threshold_ + (sample - threshold_) / ratio_;
            } else if (sample < -threshold_) {
                sample = -threshold_ + (sample + threshold_) / ratio_;
            }
            sample = std::clamp(sample, -1.0f, 1.0f);
        }
        return {};
    }

    void Reset() override {
        gain_reduction_ = 0.0f;
    }

    [[nodiscard]] std::string GetName() const override { return "compressor"; }

    void SetThreshold(f32 db) { threshold_ = std::pow(10.0f, db / 20.0f); }
    void SetRatio(f32 ratio) { ratio_ = ratio; }
    void SetAttack(f32 ms) { attack_ms_ = ms; }
    void SetRelease(f32 ms) { release_ms_ = ms; }
    void SetMakeup(f32 db) { makeup_gain_ = std::pow(10.0f, db / 20.0f); }

    [[nodiscard]] f32 GetGainReduction() const { return gain_reduction_; }

private:
    f32 threshold_ = 0.707f;  // -3dB
    f32 ratio_ = 4.0f;
    f32 attack_ms_ = 10.0f;
    f32 release_ms_ = 100.0f;
    f32 makeup_gain_ = 1.0f;
    f32 gain_reduction_ = 0.0f;
    u32 sample_rate_ = 0;
    u32 channels_ = 0;
};

// ═══════════════════════════════════════════════════════════════════════════════
// Normalizer Processor
// ═══════════════════════════════════════════════════════════════════════════════

class NormalizerProcessor : public AudioProcessor {
public:
    explicit NormalizerProcessor(f32 target_lufs = -14.0f) 
        : target_lufs_(target_lufs) {}

    Result<void> Configure(u32 sample_rate, u32 channels) override {
        sample_rate_ = sample_rate;
        channels_ = channels;
        return {};
    }

    Result<void> Process(std::span<f32> buffer) override {
        // Find peak
        f32 peak = 0.0f;
        for (f32 sample : buffer) {
            peak = std::max(peak, std::abs(sample));
        }

        // Calculate gain needed
        if (peak > 0.0f) {
            f32 target_peak = std::pow(10.0f, target_lufs_ / 20.0f);
            f32 gain = target_peak / peak;
            
            // Apply gain
            for (auto& sample : buffer) {
                sample *= gain;
            }
        }

        return {};
    }

    void Reset() override {}

    [[nodiscard]] std::string GetName() const override { return "normalizer"; }

    void SetTargetLUFS(f32 lufs) { target_lufs_ = lufs; }
    [[nodiscard]] f32 GetTargetLUFS() const { return target_lufs_; }

private:
    f32 target_lufs_;
    u32 sample_rate_ = 0;
    u32 channels_ = 0;
};

// ═══════════════════════════════════════════════════════════════════════════════
// Resampler (simplified)
// ═══════════════════════════════════════════════════════════════════════════════

class Resampler : public AudioProcessor {
public:
    Resampler(u32 input_rate, u32 output_rate)
        : input_rate_(input_rate), output_rate_(output_rate) {
        ratio_ = static_cast<f64>(output_rate) / static_cast<f64>(input_rate);
    }

    Result<void> Configure(u32 sample_rate, u32 channels) override {
        channels_ = channels;
        return {};
    }

    Result<void> Process(std::span<f32> buffer) override {
        // Simplified - in production would use libsamplerate or similar
        // for high-quality sample rate conversion
        (void)buffer;
        return {};
    }

    void Reset() override {}

    [[nodiscard]] std::string GetName() const override { return "resampler"; }

    [[nodiscard]] u32 GetInputRate() const { return input_rate_; }
    [[nodiscard]] u32 GetOutputRate() const { return output_rate_; }
    [[nodiscard]] f64 GetRatio() const { return ratio_; }

private:
    u32 input_rate_;
    u32 output_rate_;
    f64 ratio_;
    u32 channels_ = 0;
};

// ═══════════════════════════════════════════════════════════════════════════════
// Channel Mapper
// ═══════════════════════════════════════════════════════════════════════════════

class ChannelMapper : public AudioProcessor {
public:
    ChannelMapper(ChannelLayout input, ChannelLayout output)
        : input_layout_(input), output_layout_(output) {}

    Result<void> Configure(u32 sample_rate, u32 channels) override {
        (void)sample_rate;
        input_channels_ = GetChannelCount(input_layout_);
        output_channels_ = GetChannelCount(output_layout_);
        return {};
    }

    Result<void> Process(std::span<f32> buffer) override {
        // Simplified - in production would properly map channels
        // based on input and output layouts
        (void)buffer;
        return {};
    }

    void Reset() override {}

    [[nodiscard]] std::string GetName() const override { return "channel_mapper"; }

    [[nodiscard]] ChannelLayout GetInputLayout() const { return input_layout_; }
    [[nodiscard]] ChannelLayout GetOutputLayout() const { return output_layout_; }

private:
    ChannelLayout input_layout_;
    ChannelLayout output_layout_;
    u32 input_channels_ = 0;
    u32 output_channels_ = 0;
};

// ═══════════════════════════════════════════════════════════════════════════════
// Factory Functions
// ═══════════════════════════════════════════════════════════════════════════════

std::unique_ptr<AudioProcessor> CreateVolumeProcessor(f32 gain) {
    return std::make_unique<VolumeProcessor>(gain);
}

std::unique_ptr<AudioProcessor> CreateEqualizerProcessor() {
    return std::make_unique<EqualizerProcessor>();
}

std::unique_ptr<AudioProcessor> CreateCompressorProcessor() {
    return std::make_unique<CompressorProcessor>();
}

std::unique_ptr<AudioProcessor> CreateNormalizerProcessor() {
    return std::make_unique<NormalizerProcessor>();
}

std::unique_ptr<AudioProcessor> CreateSpatialProcessor() {
    return nullptr;  // Would create spatial processor
}

std::unique_ptr<AudioProcessor> CreateResampler(u32 input_rate, u32 output_rate) {
    return std::make_unique<Resampler>(input_rate, output_rate);
}

std::unique_ptr<AudioProcessor> CreateChannelMapper(ChannelLayout input, ChannelLayout output) {
    return std::make_unique<ChannelMapper>(input, output);
}

} // namespace aether
