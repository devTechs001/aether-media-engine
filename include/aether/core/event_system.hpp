// ═══════════════════════════════════════════════════════════════════════════════
// FILE: include/aether/core/event_system.hpp
// DESCRIPTION: Event system interface
// ═══════════════════════════════════════════════════════════════════════════════

#ifndef AETHER_CORE_EVENT_SYSTEM_HPP
#define AETHER_CORE_EVENT_SYSTEM_HPP

#include "aether/export.hpp"
#include "aether/core/types.hpp"

#include <string>
#include <functional>
#include <memory>
#include <any>
#include <unordered_map>
#include <vector>

namespace aether {

/**
 * @enum EventType
 * @brief Built-in event types
 */
enum class EventType : u32 {
    // Player events
    PlayerStateChanged = 0,
    PlayerPositionChanged,
    PlayerDurationChanged,
    PlayerError,
    PlayerMediaOpened,
    PlayerMediaClosed,

    // Media events
    MediaLoaded,
    MediaUnloaded,
    TrackChanged,
    SubtitleChanged,
    ChapterChanged,

    // Playback events
    PlaybackStarted,
    PlaybackPaused,
    PlaybackResumed,
    PlaybackStopped,
    PlaybackEnded,
    PlaybackSeeking,
    PlaybackSeeked,

    // Network events
    NetworkConnected,
    NetworkDisconnected,
    NetworkError,
    BufferingStarted,
    BufferingProgress,
    BufferingCompleted,

    // System events
    EngineInitialized,
    EngineShutdown,
    PluginLoaded,
    PluginUnloaded,
    ConfigChanged,

    // Custom events
    UserEvent = 1000
};

/**
 * @struct Event
 * @brief Base event structure
 */
struct AETHER_API Event {
    EventType type = EventType::UserEvent;
    i64 timestamp = 0;
    std::string source;
    std::any data;

    Event() = default;
    Event(EventType t, std::any d = {}) 
        : type(t), timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count()), data(std::move(d)) {}
    virtual ~Event() = default;
};

/**
 * @class EventListener
 * @brief Event listener interface
 */
class AETHER_API EventListener {
public:
    virtual ~EventListener() = default;
    virtual void OnEvent(const Event& event) = 0;
};

/**
 * @class EventBus
 * @brief Central event bus for pub/sub messaging
 */
class AETHER_API EventBus {
public:
    using EventHandler = std::function<void(const Event&)>;
    using ConnectionId = u64;

    /**
     * @brief Subscribe to an event type
     * @param type Event type to subscribe to
     * @param handler Event handler callback
     * @return Connection ID for unsubscribing
     */
    ConnectionId Subscribe(EventType type, EventHandler handler);

    /**
     * @brief Subscribe to an event type with listener
     * @param type Event type to subscribe to
     * @param listener Event listener object
     * @return Connection ID for unsubscribing
     */
    ConnectionId Subscribe(EventType type, EventListener* listener);

    /**
     * @brief Unsubscribe from an event
     * @param connection_id Connection ID from Subscribe
     */
    void Unsubscribe(ConnectionId connection_id);

    /**
     * @brief Unsubscribe all from an event type
     * @param type Event type to unsubscribe from
     */
    void UnsubscribeAll(EventType type);

    /**
     * @brief Publish an event
     * @param event Event to publish
     */
    void Publish(const Event& event);

    /**
     * @brief Publish an event asynchronously
     * @param event Event to publish
     */
    void PublishAsync(const Event& event);

    /**
     * @brief Get singleton instance
     */
    static EventBus& Instance();

private:
    EventBus() = default;
    ~EventBus() = default;

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace aether

#endif // AETHER_CORE_EVENT_SYSTEM_HPP
