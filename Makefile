test: build
	python ./testsuite.py

slow: build
	python ./testsuite.py slow

zobr: build
	./build/lesschess zobrist

.PHONY: build
build:
	make -C ./build

.PHONY: clean
clean:
	make -C ./build clean

