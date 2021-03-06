CC=gcc-8
CXX=g++-8

build: debug release

.PHONY: run
run: debug
	./build/debug/bin/lesschess

.PHONY: debug
debug: build/debug
	cd build/debug && ninja

.PHONY: release
release: build/release
	cd build/release && ninja

build/debug:
	mkdir -p build/debug
	cd build/debug && CC=$(CC) CXX=$(CXX) cmake -GNinja -DCMAKE_BUILD_TYPE=Debug ../..

build/release:
	mkdir -p build/release
	cd build/release && CC=$(CC) CXX=$(CXX) cmake -GNinja -DCMAKE_BUILD_TYPE=Release ../..

test: debug
	./build/debug/bin/unittest "$(name)"

reltest: release
	./build/release/bin/unittest "$(name)"

perft: release
	ninja -C build/release perft && ./build/release/bin/perft

.PHONY: clean
clean:
	cd build/debug && ninja clean
	cd build/release && ninja clean

.PHONY: fullclean
fullclean:
	rm -rf build
