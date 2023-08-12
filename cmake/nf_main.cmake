set(EXE_NAME nf)

add_executable(${EXE_NAME} "")


target_sources(${EXE_NAME} PRIVATE "main.cpp")

if(EXISTS ${cxxopts_ROOT})
    message(STATUS "cxxopts_ROOT ${cxxopts_ROOT}")
    find_package(cxxopts REQUIRED)
    target_link_libraries(${EXE_NAME} PRIVATE cxxopts::cxxopts)
endif()

target_link_libraries(${EXE_NAME} PRIVATE ${LIB_NAME})

warning_as_error_enable(${EXE_NAME})