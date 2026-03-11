// ═══════════════════════════════════════════════════════════════════════════════
// FILE: src/core/error.cpp
// ═══════════════════════════════════════════════════════════════════════════════

#include "aether/core/error_handling.hpp"
#include <sstream>
#include <cerrno>

namespace aether {

std::string Error::ToString() const {
    std::ostringstream ss;

    ss << "[" << GetErrorDescription(code) << "]";

    if (!message.empty()) {
        ss << " " << message;
    }

    if (!details.empty()) {
        ss << " (" << details << ")";
    }

    if (!source.empty()) {
        ss << " at " << source << ":" << line;
    }

    return ss.str();
}

std::string GetErrorDescription(ErrorCode code) {
    switch (code) {
        case ErrorCode::Success: return "Success";
        case ErrorCode::Unknown: return "Unknown";
        case ErrorCode::InvalidArgument: return "Invalid Argument";
        case ErrorCode::InvalidState: return "Invalid State";
        case ErrorCode::NotInitialized: return "Not Initialized";
        case ErrorCode::NotSupported: return "Not Supported";
        case ErrorCode::NotFound: return "Not Found";
        case ErrorCode::AlreadyExists: return "Already Exists";
        case ErrorCode::OutOfMemory: return "Out of Memory";
        case ErrorCode::PermissionDenied: return "Permission Denied";
        case ErrorCode::Timeout: return "Timeout";
        case ErrorCode::Cancelled: return "Cancelled";

        case ErrorCode::FileNotFound: return "File Not Found";
        case ErrorCode::FileAccessDenied: return "File Access Denied";
        case ErrorCode::FileCorrupt: return "File Corrupt";
        case ErrorCode::FileReadError: return "File Read Error";
        case ErrorCode::FileWriteError: return "File Write Error";

        case ErrorCode::UnsupportedFormat: return "Unsupported Format";
        case ErrorCode::UnsupportedCodec: return "Unsupported Codec";
        case ErrorCode::DecoderError: return "Decoder Error";
        case ErrorCode::EncoderError: return "Encoder Error";
        case ErrorCode::DemuxerError: return "Demuxer Error";
        case ErrorCode::RenderError: return "Render Error";

        case ErrorCode::NetworkError: return "Network Error";
        case ErrorCode::ConnectionFailed: return "Connection Failed";
        case ErrorCode::ConnectionTimeout: return "Connection Timeout";
        case ErrorCode::ProtocolError: return "Protocol Error";
        case ErrorCode::AuthenticationFailed: return "Authentication Failed";

        case ErrorCode::DRMError: return "DRM Error";
        case ErrorCode::LicenseExpired: return "License Expired";
        case ErrorCode::LicenseNotFound: return "License Not Found";

        case ErrorCode::PluginError: return "Plugin Error";
        case ErrorCode::PluginNotFound: return "Plugin Not Found";
        case ErrorCode::PluginLoadFailed: return "Plugin Load Failed";

        case ErrorCode::InferenceError: return "Inference Error";
        case ErrorCode::ModelNotFound: return "Model Not Found";
        case ErrorCode::ModelLoadFailed: return "Model Load Failed";

        default: return "Unknown Error";
    }
}

std::string GetErrorCategory(ErrorCode code) {
    if (code >= ErrorCode::FileNotFound && code <= ErrorCode::FileWriteError) {
        return "File/IO";
    }
    if (code >= ErrorCode::UnsupportedFormat && code <= ErrorCode::RenderError) {
        return "Media";
    }
    if (code >= ErrorCode::NetworkError && code <= ErrorCode::AuthenticationFailed) {
        return "Network";
    }
    if (code >= ErrorCode::DRMError && code <= ErrorCode::LicenseNotFound) {
        return "DRM";
    }
    if (code >= ErrorCode::PluginError && code <= ErrorCode::PluginLoadFailed) {
        return "Plugin";
    }
    if (code >= ErrorCode::InferenceError && code <= ErrorCode::ModelLoadFailed) {
        return "AI/ML";
    }
    return "General";
}

bool IsRecoverableError(ErrorCode code) {
    switch (code) {
        case ErrorCode::Timeout:
        case ErrorCode::Cancelled:
        case ErrorCode::ConnectionTimeout:
        case ErrorCode::Buffering:
            return true;
        default:
            return false;
    }
}

} // namespace aether
