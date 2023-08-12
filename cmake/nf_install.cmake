target_include_directories(${LIB_NAME} PUBLIC
    $<INSTALL_INTERFACE:include> 
)

target_link_directories(${LIB_NAME} PUBLIC
    $<INSTALL_INTERFACE:lib> 
)

install(TARGETS ${LIB_NAME}
    EXPORT nf-targets
    DESTINATION lib)

install(TARGETS ${EXE_NAME}
    DESTINATION bin)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/nf DESTINATION include)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/nf DESTINATION include)

install(EXPORT nf-targets
    FILE nf-targets.cmake
    DESTINATION lib/cmake/niu2x/
    NAMESPACE niu2x::
)

include(CMakePackageConfigHelpers)

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake_install_config.in
    "${CMAKE_CURRENT_BINARY_DIR}/nf-config.cmake"
    INSTALL_DESTINATION "lib/cmake/niu2x"
    NO_SET_AND_CHECK_MACRO
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/nf-config-version.cmake"
    VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
    COMPATIBILITY AnyNewerVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/nf-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/nf-config-version.cmake
    DESTINATION lib/cmake/niu2x
)
