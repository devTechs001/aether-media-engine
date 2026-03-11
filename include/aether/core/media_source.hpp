// ═══════════════════════════════════════════════════════════════════════════════
// FILE: include/aether/core/media_source.hpp
// DESCRIPTION: Media source interface
// ═══════════════════════════════════════════════════════════════════════════════

#ifndef AETHER_CORE_MEDIA_SOURCE_HPP
#define AETHER_CORE_MEDIA_SOURCE_HPP

#include "aether/export.hpp"
#include "aether/core/types.hpp"

#include <memory>
#include <string>
#include <span>

namespace aether {

/**
 * @enum SourceType
 * @brief Type of media source
 */
enum class SourceType : u8 {
    File,
    Network,
    Memory,
    Stream,
    Capture
};

/**
 * @struct SourceInfo
 * @brief Information about a media source
 */
struct SourceInfo {
    SourceType type = SourceType::File;
    std::string location;  // File path or URL
    std::string mime_type;
    i64 size_bytes = 0;
    bool seekable = true;
    bool live = false;
};

/**
 * @class MediaSource
 * @brief Abstract base class for media sources
 */
class AETHER_API MediaSource {
public:
    virtual ~MediaSource() = default;

    /**
     * @brief Open the source
     * @return Success or error
     */
    virtual Result<void> Open() = 0;

    /**
     * @brief Close the source
     */
    virtual void Close() = 0;

    /**
     * @brief Check if source is open
     */
    [[nodiscard]] virtual bool IsOpen() const = 0;

    /**
     * @brief Read data from source
     * @param buffer Buffer to read into
     * @param size Number of bytes to read
     * @return Number of bytes read or error
     */
    virtual Result<usize> Read(void* buffer, usize size) = 0;

    /**
     * @brief Seek to position
     * @param position Byte position
     * @param origin Seek origin (start, current, end)
     * @return Success or error
     */
    virtual Result<void> Seek(i64 position, int origin = SEEK_SET) = 0;

    /**
     * @brief Get current position
     * @return Current byte position
     */
    [[nodiscard]] virtual i64 Tell() const = 0;

    /**
     * @brief Check if end of file
     * @return True if at end
     */
    [[nodiscard]] virtual bool Eof() const = 0;

    /**
     * @brief Get source information
     * @return Source info
     */
    [[nodiscard]] virtual SourceInfo GetInfo() const = 0;

    /**
     * @brief Get available size (if known)
     * @return Size in bytes, -1 if unknown
     */
    [[nodiscard]] virtual i64 GetSize() const = 0;
};

/**
 * @class FileSource
 * @brief File-based media source
 */
class AETHER_API FileSource : public MediaSource {
public:
    explicit FileSource(const std::string& path);
    ~FileSource() override;

    Result<void> Open() override;
    void Close() override;
    [[nodiscard]] bool IsOpen() const override;
    Result<usize> Read(void* buffer, usize size) override;
    Result<void> Seek(i64 position, int origin) override;
    [[nodiscard]] i64 Tell() const override;
    [[nodiscard]] bool Eof() const override;
    [[nodiscard]] SourceInfo GetInfo() const override;
    [[nodiscard]] i64 GetSize() const override;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

/**
 * @class NetworkSource
 * @brief Network-based media source (HTTP, RTSP, etc.)
 */
class AETHER_API NetworkSource : public MediaSource {
public:
    explicit NetworkSource(const std::string& url);
    ~NetworkSource() override;

    Result<void> Open() override;
    void Close() override;
    [[nodiscard]] bool IsOpen() const override;
    Result<usize> Read(void* buffer, usize size) override;
    Result<void> Seek(i64 position, int origin) override;
    [[nodiscard]] i64 Tell() const override;
    [[nodiscard]] bool Eof() const override;
    [[nodiscard]] SourceInfo GetInfo() const override;
    [[nodiscard]] i64 GetSize() const override;

    /**
     * @brief Set connection timeout
     * @param timeout_ms Timeout in milliseconds
     */
    void SetTimeout(u32 timeout_ms);

    /**
     * @brief Set HTTP headers
     * @param headers HTTP headers map
     */
    void SetHeaders(const std::unordered_map<std::string, std::string>& headers);

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

/**
 * @class MemorySource
 * @brief Memory-based media source
 */
class AETHER_API MemorySource : public MediaSource {
public:
    explicit MemorySource(std::span<const u8> data);
    ~MemorySource() override;

    Result<void> Open() override;
    void Close() override;
    [[nodiscard]] bool IsOpen() const override;
    Result<usize> Read(void* buffer, usize size) override;
    Result<void> Seek(i64 position, int origin) override;
    [[nodiscard]] i64 Tell() const override;
    [[nodiscard]] bool Eof() const override;
    [[nodiscard]] SourceInfo GetInfo() const override;
    [[nodiscard]] i64 GetSize() const override;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace aether

#endif // AETHER_CORE_MEDIA_SOURCE_HPP
