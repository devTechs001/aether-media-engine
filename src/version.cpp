// ═══════════════════════════════════════════════════════════════════════════════
// FILE: src/version.cpp
// ═══════════════════════════════════════════════════════════════════════════════

#include "aether/version.hpp"

namespace aether {

// Version strings
constexpr std::string_view VERSION_STRING = "1.0.0";
constexpr std::string_view CODENAME = "Prometheus";

// Build information (populated at build time)
const char* GIT_COMMIT_HASH = "unknown";
const char* GIT_BRANCH = "unknown";
const char* BUILD_DATE = __DATE__;
const char* BUILD_TIME = __TIME__;

// Build info
BuildInfo GetBuildInfo() {
    BuildInfo info;
    info.version_major = VERSION_MAJOR;
    info.version_minor = VERSION_MINOR;
    info.version_patch = VERSION_PATCH;
    info.version_string = VERSION_STRING;
    info.codename = CODENAME;
    info.build_type = BUILD_TYPE;
    info.git_commit = GIT_COMMIT_HASH;
    info.git_branch = GIT_BRANCH;
    info.build_date = BUILD_DATE;
    info.build_time = BUILD_TIME;
    info.compiler = AETHER_COMPILER_ID;
    info.compiler_version = AETHER_COMPILER_VERSION;
    info.platform = AETHER_PLATFORM;
    info.architecture = AETHER_ARCHITECTURE;
    return info;
}

const char* GetVersionString() {
    return VERSION_STRING.data();
}

const char* GetCodename() {
    return CODENAME.data();
}

bool IsVersionAtLeast(u32 major, u32 minor, u32 patch) {
    return VERSION >= (major * 10000 + minor * 100 + patch);
}

} // namespace aether
