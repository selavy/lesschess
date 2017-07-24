test: build_target
	python ./testsuite.py

slow: build_target
	python ./testsuite.py slow

build_target:
	make -C ./build

