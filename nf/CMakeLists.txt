set(EXE_NAME nf)
set(LIB_NAME nf_lib)
set(PRIVATE_LIB_NAME nf_private_lib)

add_executable(${EXE_NAME} "")
add_library(${LIB_NAME} "")
add_library(${PRIVATE_LIB_NAME} "")

list(APPEND source_directories "src/*.cpp")
file(GLOB sources ${source_directories})

configure_file(include/nf/config.h.in include/nf/config.h @ONLY)

target_include_directories(${LIB_NAME} PRIVATE
    src/
)
target_include_directories(${LIB_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>  
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/>  
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>  
    $<INSTALL_INTERFACE:include> 
)

target_include_directories(${PRIVATE_LIB_NAME} PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}/include 
    ${CMAKE_CURRENT_BINARY_DIR}/ 
    ${CMAKE_CURRENT_SOURCE_DIR}/include 
    ${CMAKE_CURRENT_SOURCE_DIR}/src 
)

target_sources(${PRIVATE_LIB_NAME} PRIVATE ${sources})
target_sources(${LIB_NAME} PRIVATE ${sources})

target_sources(${EXE_NAME} PRIVATE "main.cpp")

if(cxxopts_ROOT)
    find_package(cxxopts REQUIRED)
    target_link_libraries(${EXE_NAME} PRIVATE cxxopts::cxxopts)
endif()

target_link_libraries(${EXE_NAME} PRIVATE ${LIB_NAME})

warning_as_error_enable(nf)
warning_as_error_enable(nf_lib)
