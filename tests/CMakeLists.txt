if(EXISTS ${GTest_DIR}) 
	find_package(GTest REQUIRED)
	file(GLOB TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/inner-test-*.cpp")

	foreach(item IN LISTS TEST_FILES)
		get_filename_component(name ${item} NAME_WE)
		add_executable(${name} ${item})
		target_link_libraries(${name} PRIVATE nf_lib GTest::gtest_main)
		target_include_directories(${name} PRIVATE 
			../nf/src 
    		${boost_include}
		)
		gtest_discover_tests(${name})
	endforeach()

	if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
		file(COPY "${PROJECT_BINARY_DIR}/external_deps/bin/gtest_main.dll" DESTINATION Debug)
		file(COPY "${PROJECT_BINARY_DIR}/external_deps/bin/gtest.dll" DESTINATION Debug)
		file(COPY "${PROJECT_BINARY_DIR}/external_deps/bin/gtest_main.dll" DESTINATION Release)
		file(COPY "${PROJECT_BINARY_DIR}/external_deps/bin/gtest.dll" DESTINATION Release)
	endif()

endif()

