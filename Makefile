build:
	mkdir -p build/debug
	cd build/debug && cmake -GNinja -DCMAKE_BUILD_TYPE=Debug ../..
