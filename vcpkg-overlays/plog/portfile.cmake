# EDITED:
# https://github.com/microsoft/vcpkg/tree/master/ports/plog
# Changed pinned commit to include the fix for u8string
# https://github.com/SergiusTheBest/plog/issues/273
# ====================

# Header-only library
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO SergiusTheBest/plog
    REF 199734337bec45e72956c4e82f848a716cbf7ac3
    SHA512 283aff7bea115ffbbb0dd7439682442cec283310022e33763fa6af5aab30cc93cbb2a2e73991ff4bb8a6d7253a4a484f9dd9fceec78eaadfb8bad46603106768
    HEAD_REF master
)

vcpkg_cmake_configure(SOURCE_PATH ${SOURCE_PATH} OPTIONS -DPLOG_BUILD_SAMPLES=OFF)
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/${PORT})

file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug ${CURRENT_PACKAGES_DIR}/lib)

# Copy usage file
file(COPY "${CURRENT_PORT_DIR}/usage" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")

# Put the licence file where vcpkg expects it
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
