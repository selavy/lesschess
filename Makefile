build: build/debug build/release

build/debug:
	mkdir -p build/debug
	cd build/debug && cmake -GNinja -DCMAKE_BUILD_TYPE=Debug ../..

build/release:
	mkdir -p build/release
	cd build/release && cmake -GNinja -DCMAKE_BUILD_TYPE=Release ../..
