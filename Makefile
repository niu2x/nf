test:
	cmake -S. -Bbuild -DCMAKE_BUILD_TYPE=Debug; cmake --build build

.PHONY: test