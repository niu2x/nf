set(LIB_NAME nf_lib)

add_library(${LIB_NAME} "")

set_target_properties(${LIB_NAME} PROPERTIES 
    OUTPUT_NAME nf)

list(APPEND source_directories "src/*.cpp")
file(GLOB sources ${source_directories})

configure_file(include/nf/config.h.in include/nf/config.h @ONLY)

target_include_directories(${LIB_NAME} PRIVATE
    src
    ${boost_include}
)

target_include_directories(${LIB_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>  
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/>  
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>  
)

target_sources(${LIB_NAME} PRIVATE ${sources})

warning_as_error_enable(${LIB_NAME})