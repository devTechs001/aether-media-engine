// ═══════════════════════════════════════════════════════════════════════════════
// FILE: include/aether/utils/threading.hpp
// DESCRIPTION: Threading utilities
// ═══════════════════════════════════════════════════════════════════════════════

#ifndef AETHER_UTILS_THREADING_HPP
#define AETHER_UTILS_THREADING_HPP

#include "aether/export.hpp"
#include "aether/core/types.hpp"

#include <thread>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <atomic>
#include <functional>
#include <future>

namespace aether {

/**
 * @brief Get number of logical CPU cores
 */
AETHER_API u32 GetHardwareConcurrency();

/**
 * @brief Set thread name
 */
AETHER_API void SetThreadName(std::thread& thread, const std::string& name);

/**
 * @brief Set current thread name
 */
AETHER_API void SetCurrentThreadName(const std::string& name);

/**
 * @brief Get current thread name
 */
AETHER_API std::string GetCurrentThreadName();

/**
 * @brief Thread pool for task execution
 */
class AETHER_API ThreadPool {
public:
    explicit ThreadPool(u32 num_threads = 0);  // 0 = hardware concurrency
    ~ThreadPool();

    /**
     * @brief Submit task to pool
     */
    template<typename F, typename... Args>
    auto Submit(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>>;

    /**
     * @brief Wait for all tasks to complete
     */
    void WaitAll();

    /**
     * @brief Get number of pending tasks
     */
    [[nodiscard]] usize PendingTasks() const;

    /**
     * @brief Get number of active threads
     */
    [[nodiscard]] u32 ActiveThreads() const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

/**
 * @brief Reader-writer lock wrapper
 */
template<typename T>
class AETHER_API ReadWriteLock {
public:
    ReadWriteLock() = default;

    [[nodiscard]] T Read() const {
        std::shared_lock lock(m_mutex);
        return m_value;
    }

    void Write(const T& value) {
        std::unique_lock lock(m_mutex);
        m_value = value;
    }

    template<typename F>
    void Update(F&& f) {
        std::unique_lock lock(m_mutex);
        f(m_value);
    }

private:
    mutable std::shared_mutex m_mutex;
    T m_value;
};

/**
 * @brief Atomic counter
 */
class AETHER_API AtomicCounter {
public:
    explicit AtomicCounter(i64 initial = 0) : m_counter(initial) {}

    i64 Increment() { return ++m_counter; }
    i64 Decrement() { return --m_counter; }
    i64 Add(i64 value) { return m_counter.fetch_add(value) + value; }
    i64 Subtract(i64 value) { return m_counter.fetch_sub(value) - value; }
    
    [[nodiscard]] i64 Value() const { return m_counter.load(); }
    void Reset(i64 value = 0) { m_counter.store(value); }

private:
    std::atomic<i64> m_counter;
};

// Template implementation
template<typename F, typename... Args>
auto ThreadPool::Submit(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>> {
    using return_type = std::invoke_result_t<F, Args...>;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<return_type> result = task->get_future();
    
    // Implementation would queue task
    return result;
}

} // namespace aether

#endif // AETHER_UTILS_THREADING_HPP
