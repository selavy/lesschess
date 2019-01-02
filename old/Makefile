test: build
	python ./testsuite.py

slow: build
	python ./testsuite.py slow

perft: build
	python ./testsuite.py perft

tactics: build
	python ./testsuite.py tactics

.PHONY: build
build:
	make -C ./build

.PHONY: clean
clean:
	make -C ./build clean

