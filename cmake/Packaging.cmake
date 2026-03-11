# Packaging.cmake
# CPack configuration for AETHER Media Engine

set(CPACK_PACKAGE_NAME "AetherMediaEngine")
set(CPACK_PACKAGE_VENDOR "DarkHat")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Enterprise-Grade Cross-Platform AI-Powered Media Player Suite")
set(CPACK_PACKAGE_VERSION_MAJOR ${AETHER_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${AETHER_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${AETHER_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION "${AETHER_VERSION_STRING}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "AetherMediaEngine")

# Package file name
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")

# Resource files
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")

# Generator-specific settings
if(WIN32)
    # NSIS installer
    set(CPACK_GENERATOR "NSIS;ZIP")
    set(CPACK_NSIS_DISPLAY_NAME "Aether Media Engine")
    set(CPACK_NSIS_HELP_LINK "https://github.com/devTechs001/aether-media-engine")
    set(CPACK_NSIS_URL_INFO_ABOUT "https://github.com/devTechs001/aether-media-engine")
    set(CPACK_NSIS_CONTACT "devTechs001")
    set(CPACK_NSIS_MODIFY_PATH ON)
elseif(APPLE)
    # macOS bundle
    set(CPACK_GENERATOR "DragNDrop;TGZ")
    set(CPACK_BUNDLE_NAME "Aether Media Engine")
else()
    # Linux packages
    set(CPACK_GENERATOR "DEB;RPM;TGZ")

    # DEB specific
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "DarkHat")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "qt6-base, qt6-multimedia, libvulkan1")

    # RPM specific
    set(CPACK_RPM_PACKAGE_LICENSE "LGPL-3.0")
    set(CPACK_RPM_PACKAGE_REQUIRES "qt6-base, qt6-multimedia, vulkan")
endif()

# Source package
set(CPACK_SOURCE_GENERATOR "TGZ;ZIP")
set(CPACK_SOURCE_IGNORE_FILES
    /build/
    /\\.git/
    /\\.github/
    /\\.vscode/
    /\\.qtcreator/
    \\.swp$
    ~$
)

include(CPack)
