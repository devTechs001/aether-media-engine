// ═══════════════════════════════════════════════════════════════════════════════
// FILE: src/core/event_system.cpp
// ═══════════════════════════════════════════════════════════════════════════════

#include "aether/core/event_system.hpp"
#include "aether/utils/threading.hpp"

#include <mutex>
#include <vector>
#include <queue>
#include <unordered_map>

namespace aether {

class EventDispatcher::Impl {
public:
    u64 Subscribe(Callback callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        u64 id = next_id_++;
        subscribers_[id] = std::move(callback);
        return id;
    }

    void Unsubscribe(u64 id) {
        std::lock_guard<std::mutex> lock(mutex_);
        subscribers_.erase(id);
    }

    void Clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        subscribers_.clear();
    }

    void DispatchEvent(const PlayerEvent& event) {
        std::vector<Callback> callbacks;

        {
            std::lock_guard<std::mutex> lock(mutex_);
            callbacks.reserve(subscribers_.size());
            for (const auto& [id, callback] : subscribers_) {
                callbacks.push_back(callback);
            }
        }

        for (const auto& callback : callbacks) {
            try {
                callback(event);
            } catch (...) {
                // Swallow exceptions from callbacks
            }
        }
    }

    void DispatchAsync(PlayerEvent event) {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        pending_events_.push(std::move(event));
    }

    void ProcessPending() {
        std::queue<PlayerEvent> events;

        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            events.swap(pending_events_);
        }

        while (!events.empty()) {
            DispatchEvent(events.front());
            events.pop();
        }
    }

    usize SubscriberCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return subscribers_.size();
    }

private:
    mutable std::mutex mutex_;
    std::mutex queue_mutex_;

    std::unordered_map<u64, Callback> subscribers_;
    u64 next_id_ = 1;

    std::queue<PlayerEvent> pending_events_;
};

EventDispatcher::EventDispatcher() : impl_(std::make_unique<Impl>()) {}

EventDispatcher::~EventDispatcher() = default;

u64 EventDispatcher::Subscribe(Callback callback) {
    return impl_->Subscribe(std::move(callback));
}

void EventDispatcher::Unsubscribe(u64 id) {
    impl_->Unsubscribe(id);
}

void EventDispatcher::Clear() {
    impl_->Clear();
}

void EventDispatcher::DispatchEvent(const PlayerEvent& event) {
    impl_->DispatchEvent(event);
}

void EventDispatcher::DispatchAsync(PlayerEvent event) {
    impl_->DispatchAsync(std::move(event));
}

void EventDispatcher::ProcessPending() {
    impl_->ProcessPending();
}

usize EventDispatcher::SubscriberCount() const {
    return impl_->SubscriberCount();
}

std::string_view GetEventName(const PlayerEvent& event) {
    return std::visit([](const auto& e) -> std::string_view {
        using T = std::decay_t<decltype(e)>;

        if constexpr (std::is_same_v<T, StateChangedEvent>) return "StateChanged";
        else if constexpr (std::is_same_v<T, MediaOpenedEvent>) return "MediaOpened";
        else if constexpr (std::is_same_v<T, MediaClosedEvent>) return "MediaClosed";
        else if constexpr (std::is_same_v<T, PositionChangedEvent>) return "PositionChanged";
        else if constexpr (std::is_same_v<T, SeekingEvent>) return "Seeking";
        else if constexpr (std::is_same_v<T, SeekedEvent>) return "Seeked";
        else if constexpr (std::is_same_v<T, BufferingEvent>) return "Buffering";
        else if constexpr (std::is_same_v<T, VolumeChangedEvent>) return "VolumeChanged";
        else if constexpr (std::is_same_v<T, TrackChangedEvent>) return "TrackChanged";
        else if constexpr (std::is_same_v<T, ChapterChangedEvent>) return "ChapterChanged";
        else if constexpr (std::is_same_v<T, SubtitleEvent>) return "Subtitle";
        else if constexpr (std::is_same_v<T, ErrorEvent>) return "Error";
        else if constexpr (std::is_same_v<T, EndOfMediaEvent>) return "EndOfMedia";
        else if constexpr (std::is_same_v<T, VideoResizedEvent>) return "VideoResized";
        else return "Unknown";
    }, event);
}

} // namespace aether
