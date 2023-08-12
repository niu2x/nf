include(ExternalProject)


# cxxopts
set(TMP_CMAKE_ARGS "")
list(APPEND TMP_CMAKE_ARGS "-DCMAKE_BUILD_TYPE=Release")
list(APPEND TMP_CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${PROJECT_BINARY_DIR}/external_deps")
list(APPEND TMP_CMAKE_ARGS "-DBUILD_SHARED_LIBS=OFF")
list(APPEND TMP_CMAKE_ARGS "-DCMAKE_POSITION_INDEPENDENT_CODE=ON")
list(APPEND TMP_CMAKE_ARGS "-DCXXOPTS_BUILD_TESTS=OFF")
list(APPEND TMP_CMAKE_ARGS "-DCXXOPTS_BUILD_EXAMPLES=OFF")

ExternalProject_Add(external_cxxopts
    GIT_REPOSITORY  https://gitee.com/ant1423/cxxopts
    GIT_TAG         v2.2.1
    CMAKE_ARGS      ${TMP_CMAKE_ARGS}
)
set(cxxopts_ROOT "${PROJECT_BINARY_DIR}/external_deps")



# googletest
set(TMP_CMAKE_ARGS "")
list(APPEND TMP_CMAKE_ARGS "-DCMAKE_BUILD_TYPE=Release")
list(APPEND TMP_CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${PROJECT_BINARY_DIR}/external_deps")
list(APPEND TMP_CMAKE_ARGS "-DBUILD_SHARED_LIBS=ON")
ExternalProject_Add(external_googletest
    GIT_REPOSITORY  https://gitee.com/zqx5449/googletest
    GIT_TAG         v1.13.0   
    CMAKE_ARGS      ${TMP_CMAKE_ARGS}
)
set(GTest_DIR "${PROJECT_BINARY_DIR}/external_deps/lib/cmake/GTest")


# boost header-only
set(TMP_CMAKE_ARGS "")
ExternalProject_Add(external_boost
    GIT_REPOSITORY  https://gitee.com/add358/boost
    GIT_TAG         1.71.0
    CMAKE_ARGS      ${TMP_CMAKE_ARGS}
    CONFIGURE_COMMAND ""
    # BUILD_COMMAND  ""
    BUILD_COMMAND   ""
    INSTALL_COMMAND ""
)
set(boost_include ${PROJECT_BINARY_DIR}/external_boost-prefix/src/external_boost)
