test: build
	python ./testsuite.py

slow: build
	python ./testsuite.py slow

.PHONY: build
build:
	make -C ./build

.PHONY: clean
clean:
	make -C ./build clean

