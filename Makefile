build:
	cmake -S. -Bbuild -DCMAKE_BUILD_TYPE=Debug -DNF_BUILD_TEST=ON; cmake --build build

test: build
	cd build; ctest;

.PHONY: test build