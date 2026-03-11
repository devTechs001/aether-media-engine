// ═══════════════════════════════════════════════════════════════════════════════
// FILE: include/aether/media/packet.hpp
// DESCRIPTION: Media packet types
// ═══════════════════════════════════════════════════════════════════════════════

#ifndef AETHER_MEDIA_PACKET_HPP
#define AETHER_MEDIA_PACKET_HPP

#include "aether/export.hpp"
#include "aether/core/types.hpp"

#include <memory>
#include <vector>
#include <span>

namespace aether {

/**
 * @struct Packet
 * @brief Media packet (compressed data)
 */
struct AETHER_API Packet {
    MediaType type = MediaType::Unknown;
    CodecID codec_id = CodecID::Unknown;

    // Timing
    i64 pts = 0;
    i64 dts = 0;
    i64 duration = 0;

    // Data
    std::vector<u8> data;
    bool is_keyframe = false;

    // Stream info
    i32 stream_index = -1;

    Packet() = default;

    /**
     * @brief Create packet from data
     */
    static Packet FromData(std::span<const u8> data, MediaType type);

    /**
     * @brief Create video packet
     */
    static Packet CreateVideo(std::span<const u8> data, i64 pts, i64 dts, bool keyframe);

    /**
     * @brief Create audio packet
     */
    static Packet CreateAudio(std::span<const u8> data, i64 pts, i64 dts);

    /**
     * @brief Get packet size
     */
    [[nodiscard]] usize size() const { return data.size(); }

    /**
     * @brief Clear packet data
     */
    void clear();

    /**
     * @brief Check if packet is valid
     */
    [[nodiscard]] bool is_valid() const { return !data.empty(); }
};

using PacketPtr = std::unique_ptr<Packet>;

/**
 * @class PacketQueue
 * @brief Thread-safe packet queue
 */
class AETHER_API PacketQueue {
public:
    explicit PacketQueue(usize max_size = 1000);
    ~PacketQueue();

    /**
     * @brief Push packet to queue
     */
    bool Push(PacketPtr packet);

    /**
     * @brief Pop packet from queue
     */
    PacketPtr Pop();

    /**
     * @brief Try to pop with timeout
     */
    PacketPtr TryPop(Milliseconds timeout);

    /**
     * @brief Get queue size
     */
    [[nodiscard]] usize Size() const;

    /**
     * @brief Check if queue is empty
     */
    [[nodiscard]] bool Empty() const;

    /**
     * @brief Clear queue
     */
    void Clear();

    /**
     * @brief Signal end of stream
     */
    void SignalEOS();

    /**
     * @brief Check if EOS signaled
     */
    [[nodiscard]] bool IsEOS() const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace aether

#endif // AETHER_MEDIA_PACKET_HPP
