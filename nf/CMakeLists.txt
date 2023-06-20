set(EXE_NAME nf)

add_executable(${EXE_NAME} "")



if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
    find_package(FLEX 2.6 REQUIRED)
    find_package(BISON 2 REQUIRED)

    flex_target(LEXER "src/nf.l" "${CMAKE_CURRENT_BINARY_DIR}/nf_lexer.c" 
        DEFINES_FILE "${CMAKE_CURRENT_BINARY_DIR}/nf_lexer.h"
        COMPILE_FLAGS "--prefix=nf_"
    )

    bison_target(PARSER "src/nf.y" "${CMAKE_CURRENT_BINARY_DIR}/nf_parser.c" 
        DEFINES_FILE "${CMAKE_CURRENT_BINARY_DIR}/nf_parser.h"
        COMPILE_FLAGS "--name-prefix=nf_"
    )

    add_flex_bison_dependency(LEXER PARSER)

    target_sources(${EXE_NAME} PRIVATE 
        ${CMAKE_CURRENT_BINARY_DIR}/nf_lexer.c
        ${CMAKE_CURRENT_BINARY_DIR}/nf_parser.c
    )
endif()


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
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/>  
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>  
    $<INSTALL_INTERFACE:include> 
)

target_link_libraries(${EXE_NAME} PRIVATE nf_3rd)