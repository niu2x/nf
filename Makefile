test:
	cmake -S. -Bbuild -DCMAKE_BUILD_TYPE=Debug -DNF_BUILD_TEST=ON; cmake --build build

.PHONY: test