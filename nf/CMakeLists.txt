set(EXE_NAME nf)

add_executable(${EXE_NAME} "")

list(APPEND source_directories "src/*.cpp")
file(GLOB sources ${source_directories})
target_sources(${EXE_NAME} PRIVATE ${sources})