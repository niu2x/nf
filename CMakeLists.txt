cmake_minimum_required(VERSION 3.13.0 FATAL_ERROR)
project(NIU2X_NF VERSION 0.0.1 LANGUAGES CXX C)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
include(external_deps)
include(warning_as_error)

set(CMAKE_CXX_STANDARD 17)
add_subdirectory(3rd)
add_subdirectory(nf)


if(${NF_BUILD_TEST})
	enable_testing()
	add_subdirectory(tests)
endif()

