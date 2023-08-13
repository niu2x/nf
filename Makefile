test: test-release test-debug

build-nf-release: configure-release
	cmake -S. -Bbuild-release -DCMAKE_BUILD_TYPE=Release -DNF_BUILD_TEST=ON; 
	cmake --build  build-release/ 

configure-release:
	cmake -S. -Bbuild-release -DCMAKE_BUILD_TYPE=Release -DNF_BUILD_TEST=ON; 
	cmake --build  build-release/ --target external_boost
	cmake --build  build-release/ --target external_cxxopts
	cmake --build  build-release/ --target external_googletest

test-release: build-nf-release
	cd build-release; ctest; cd ..;
	./tests/test.sh build-release/nf/nf


build-nf-debug: configure-debug
	cmake -S. -Bbuild-debug -DCMAKE_BUILD_TYPE=Debug -DNF_BUILD_TEST=ON; 
	cmake --build  build-debug/ 
configure-debug:
	cmake -S. -Bbuild-debug -DCMAKE_BUILD_TYPE=Debug -DNF_BUILD_TEST=ON; 
	cmake --build  build-debug/ --target external_boost
	cmake --build  build-debug/ --target external_cxxopts
	cmake --build  build-debug/ --target external_googletest

test-debug: build-nf-debug
	cd build-debug; ctest; cd ..;
	./tests/test.sh build-debug/nf/nf

.PHONY: test-release build-nf-release configure-release \
		test-debug build-nf-debug configure-debug \
		test  