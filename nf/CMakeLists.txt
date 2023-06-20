set(EXE_NAME nf)

add_executable(${EXE_NAME} "")

list(APPEND source_directories "src/*.cpp")
file(GLOB sources ${source_directories})
target_sources(${EXE_NAME} PRIVATE ${sources})

configure_file(include/nf/config.h.in include/nf/config.h @ONLY)

if(cxxopts_ROOT)
    find_package(cxxopts REQUIRED)
    target_link_libraries(${EXE_NAME} PRIVATE cxxopts::cxxopts)
endif()

target_include_directories(${EXE_NAME} PRIVATE
    src/
)
target_include_directories(${EXE_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>  
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>  
    $<INSTALL_INTERFACE:include> 
)