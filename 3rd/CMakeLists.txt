add_library(nf_3rd STATIC "")
target_sources(nf_3rd PRIVATE
	strbuf.c
)
target_include_directories(nf_3rd PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})