include(ExternalProject)


# cxxopts
set(TMP_CMAKE_ARGS "")
list(APPEND TMP_CMAKE_ARGS "-DCMAKE_BUILD_TYPE=Release")
list(APPEND TMP_CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${PROJECT_BINARY_DIR}/external_deps")
list(APPEND TMP_CMAKE_ARGS "-DBUILD_SHARED_LIBS=OFF")
list(APPEND TMP_CMAKE_ARGS "-DCMAKE_POSITION_INDEPENDENT_CODE=ON")

ExternalProject_Add(external_cxxopts
    GIT_REPOSITORY  https://gitee.com/ant1423/cxxopts
    GIT_TAG         v2.2.1
    CMAKE_ARGS      ${TMP_CMAKE_ARGS}
)
set(cxxopts_ROOT "${PROJECT_BINARY_DIR}/external_deps")
# add_dependencies(external_cxxopts external_zlib)
