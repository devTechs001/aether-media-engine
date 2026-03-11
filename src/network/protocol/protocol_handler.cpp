// ═══════════════════════════════════════════════════════════════════════════════
// FILE: src/network/protocol/protocol_handler.cpp
// ═══════════════════════════════════════════════════════════════════════════════

#include "aether/network/protocol.hpp"
#include "aether/utils/logging.hpp"

#include <mutex>
#include <unordered_map>
#include <functional>

namespace aether {

// ═══════════════════════════════════════════════════════════════════════════════
// Protocol Handler Registry
// ═══════════════════════════════════════════════════════════════════════════════

class ProtocolRegistry {
public:
    static ProtocolRegistry& Instance() {
        static ProtocolRegistry instance;
        return instance;
    }

    void RegisterHandler(ProtocolType type, std::function<std::unique_ptr<ProtocolHandler>()> factory) {
        std::lock_guard<std::mutex> lock(mutex_);
        factories_[type] = std::move(factory);
    }

    std::unique_ptr<ProtocolHandler> CreateHandler(ProtocolType type) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = factories_.find(type);
        if (it != factories_.end()) {
            return it->second();
        }
        return nullptr;
    }

private:
    std::mutex mutex_;
    std::unordered_map<ProtocolType, std::function<std::unique_ptr<ProtocolHandler>()>> factories_;
};

// ═══════════════════════════════════════════════════════════════════════════════
// File Protocol Handler
// ═══════════════════════════════════════════════════════════════════════════════

class FileProtocolHandler : public ProtocolHandler {
public:
    FileProtocolHandler() = default;
    ~FileProtocolHandler() override { Close(); }

    Result<void> Open(const std::string& url) override {
        // Remove file:// prefix
        std::string path = url;
        if (path.find("file://") == 0) {
            path = path.substr(7);
        }

        file_.open(path, std::ios::binary | std::ios::in);
        if (!file_.is_open()) {
            return Error::Make(ErrorCode::FileNotFound, "Cannot open file: {}", path);
        }

        // Get file size
        file_.seekg(0, std::ios::end);
        size_ = file_.tellg();
        file_.seekg(0, std::ios::beg);

        position_ = 0;
        return {};
    }

    void Close() override {
        if (file_.is_open()) {
            file_.close();
        }
        size_ = 0;
        position_ = 0;
    }

    Result<usize> Read(void* buffer, usize size) override {
        if (!file_.is_open()) {
            return Error::Make(ErrorCode::InvalidState, "File not open");
        }

        file_.read(static_cast<char*>(buffer), static_cast<std::streamsize>(size));
        usize bytes_read = static_cast<usize>(file_.gcount());
        position_ += bytes_read;
        return bytes_read;
    }

    Result<usize> Write(const void* buffer, usize size) override {
        return Error::Make(ErrorCode::NotSupported, "File protocol is read-only");
    }

    Result<void> Seek(i64 position) override {
        if (!file_.is_open()) {
            return Error::Make(ErrorCode::InvalidState, "File not open");
        }

        file_.seekg(position, std::ios::beg);
        position_ = position;
        return {};
    }

    [[nodiscard]] i64 Tell() const override {
        return position_;
    }

    [[nodiscard]] i64 Size() const override {
        return size_;
    }

    [[nodiscard]] ProtocolType GetType() const override {
        return ProtocolType::File;
    }

private:
    std::ifstream file_;
    i64 size_ = 0;
    i64 position_ = 0;
};

// ═══════════════════════════════════════════════════════════════════════════════
// HTTP Protocol Handler (simplified)
// ═══════════════════════════════════════════════════════════════════════════════

class HTTPProtocolHandler : public ProtocolHandler {
public:
    HTTPProtocolHandler() = default;
    ~HTTPProtocolHandler() override { Close(); }

    Result<void> Open(const std::string& url) override {
        url_ = url;
        
        // In production, would use libcurl or similar
        // For now, just mark as open
        is_open_ = true;
        position_ = 0;
        size_ = -1;  // Unknown for HTTP

        GetLogger().Debug("Opened HTTP URL: {}", url);
        return {};
    }

    void Close() override {
        is_open_ = false;
        position_ = 0;
        size_ = -1;
    }

    Result<usize> Read(void* buffer, usize size) override {
        if (!is_open_) {
            return Error::Make(ErrorCode::InvalidState, "Not open");
        }

        // In production, would read from network
        // For now, return 0 (EOF)
        return 0;
    }

    Result<usize> Write(const void* buffer, usize size) override {
        return Error::Make(ErrorCode::NotSupported, "HTTP is read-only");
    }

    Result<void> Seek(i64 position) override {
        if (!is_open_) {
            return Error::Make(ErrorCode::InvalidState, "Not open");
        }

        // HTTP supports byte-range requests
        position_ = position;
        return {};
    }

    [[nodiscard]] i64 Tell() const override {
        return position_;
    }

    [[nodiscard]] i64 Size() const override {
        return size_;
    }

    [[nodiscard]] ProtocolType GetType() const override {
        return ProtocolType::HTTP;
    }

    // HTTP-specific methods
    void SetHeader(const std::string& key, const std::string& value) {
        headers_[key] = value;
    }

    void SetTimeout(u32 timeout_ms) {
        timeout_ms_ = timeout_ms;
    }

private:
    std::string url_;
    std::unordered_map<std::string, std::string> headers_;
    u32 timeout_ms_ = 30000;
    i64 position_ = 0;
    i64 size_ = -1;
    bool is_open_ = false;
};

// ═══════════════════════════════════════════════════════════════════════════════
// Factory Functions
// ═══════════════════════════════════════════════════════════════════════════════

std::unique_ptr<ProtocolHandler> CreateProtocolHandler(ProtocolType type) {
    switch (type) {
        case ProtocolType::File:
            return std::make_unique<FileProtocolHandler>();
        case ProtocolType::HTTP:
        case ProtocolType::HTTPS:
            return std::make_unique<HTTPProtocolHandler>();
        default:
            return nullptr;
    }
}

std::unique_ptr<ProtocolHandler> CreateProtocolHandlerForURL(const std::string& url) {
    ProtocolType type = GetProtocolTypeFromURL(url);
    return CreateProtocolHandler(type);
}

ProtocolType GetProtocolTypeFromURL(const std::string& url) {
    if (url.find("file://") == 0) {
        return ProtocolType::File;
    }
    if (url.find("https://") == 0) {
        return ProtocolType::HTTPS;
    }
    if (url.find("http://") == 0) {
        return ProtocolType::HTTP;
    }
    if (url.find("rtsp://") == 0) {
        return ProtocolType::RTSP;
    }
    if (url.find("rtmp://") == 0) {
        return ProtocolType::RTMP;
    }
    if (url.find("srt://") == 0) {
        return ProtocolType::SRT;
    }
    // Default to file
    return ProtocolType::File;
}

void RegisterProtocolHandler(const std::string& scheme,
                            std::function<std::unique_ptr<ProtocolHandler>()> factory) {
    (void)scheme;
    (void)factory;
    // Would register custom protocol handler
}

} // namespace aether
