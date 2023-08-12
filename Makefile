build:
	cmake -S. -Bbuild -DCMAKE_BUILD_TYPE=Release -DNF_BUILD_TEST=ON; 
	cmake --build  build/ --target external_boost
	cmake --build  build/ --target external_cxxopts
	cmake --build  build/ --target external_googletest
	cmake --build  build/ --target nf_lib
	cmake --build  build/ --target nf

test: build
	cd build; ctest;

.PHONY: test build