// ═══════════════════════════════════════════════════════════════════════════════
// FILE: include/aether/pipeline/decoder.hpp
// DESCRIPTION: Decoder interface
// ═══════════════════════════════════════════════════════════════════════════════

#ifndef AETHER_PIPELINE_DECODER_HPP
#define AETHER_PIPELINE_DECODER_HPP

#include "aether/export.hpp"
#include "aether/core/types.hpp"
#include "aether/media/packet.hpp"
#include "aether/media/frame.hpp"
#include "aether/media/codec.hpp"

#include <memory>
#include <string>

namespace aether {

/**
 * @struct DecoderConfig
 * @brief Decoder configuration
 */
struct AETHER_API DecoderConfig {
    CodecID codec_id = CodecID::Unknown;
    
    // Hardware acceleration
    bool hardware_accel = true;
    std::string hardware_device;  // e.g., "/dev/dri/renderD128" for VAAPI
    
    // Thread count
    u32 threads = 0;  // 0 = auto
    
    // Output format
    PixelFormat output_format = PixelFormat::Unknown;
    SampleFormat audio_format = SampleFormat::Unknown;
    
    // Low latency
    bool low_latency = false;
    
    // Discard settings
    bool discard_damaged = false;
};

/**
 * @struct DecoderInfo
 * @brief Decoder capabilities
 */
struct AETHER_API DecoderInfo {
    std::string name;
    std::string long_name;
    CodecType type;
    bool is_hardware = false;
    std::vector<CodecID> supported_codecs;
};

/**
 * @class Decoder
 * @brief Abstract decoder interface
 */
class AETHER_API Decoder {
public:
    virtual ~Decoder() = default;

    /**
     * @brief Open decoder
     */
    virtual Result<void> Open(const DecoderConfig& config) = 0;

    /**
     * @brief Close decoder
     */
    virtual void Close() = 0;

    /**
     * @brief Decode packet to frame
     */
    virtual Result<FramePtr> Decode(const Packet& packet) = 0;

    /**
     * @brief Send packet for decoding
     */
    virtual Result<void> SendPacket(const Packet& packet) = 0;

    /**
     * @brief Receive decoded frame
     */
    virtual Result<FramePtr> ReceiveFrame() = 0;

    /**
     * @brief Flush decoder
     */
    virtual Result<void> Flush() = 0;

    /**
     * @brief Get codec info
     */
    [[nodiscard]] virtual CodecInfo GetCodecInfo() const = 0;

    /**
     * @brief Get decoder info
     */
    [[nodiscard]] virtual DecoderInfo GetInfo() const = 0;

    /**
     * @brief Check if hardware decoder
     */
    [[nodiscard]] virtual bool IsHardware() const = 0;
};

/**
 * @brief Create video decoder
 */
AETHER_API std::unique_ptr<Decoder> CreateVideoDecoder(CodecID codec_id, const DecoderConfig& config = DecoderConfig());

/**
 * @brief Create audio decoder
 */
AETHER_API std::unique_ptr<Decoder> CreateAudioDecoder(CodecID codec_id, const DecoderConfig& config = DecoderConfig());

/**
 * @brief Create FFmpeg-based decoder
 */
AETHER_API std::unique_ptr<Decoder> CreateFFmpegDecoder(CodecID codec_id);

} // namespace aether

#endif // AETHER_PIPELINE_DECODER_HPP
