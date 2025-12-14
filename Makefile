.PHONY: debug
debug: cmake-debug build test

.PHONY: cmake-debug
cmake-debug:
	cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug

.PHONY: release
release: cmake-release build test

.PHONY: cmake-release
cmake-release:
	cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release

WIN_DIR = /Documents/code/roc-win64
FULL_WIN_DIR := $(join ${WIN_HOME_DIR}, $(WIN_DIR))
TOP_LEVEL = $(shell find . -maxdepth 1 -type f)
SRC_DIRS = include src test

.PHONY: sync
sync:
	@for ff in $(TOP_LEVEL) ; do \
		rsync -ruvhP $$ff $(FULL_WIN_DIR) ; \
	done
	@for dd in $(SRC_DIRS) ; do \
		rsync -ruvhP $$dd $(FULL_WIN_DIR) ; \
	done

.PHONY: build
build:
	cmake --build build
	cp build/compile_commands.json .

.PHONY: test
test:
ifndef testregex
	ctest --test-dir build/test --output-on-failure
else
	ctest --test-dir build/test --output-on-failure -R $(testregex)
endif

.PHONY: fmt
fmt:
	nix fmt .

.PHONY: tidy
tidy:
	@for src in $(HEADERS) ; \
	do \
		clang-tidy "$$src" ; \
	done
	@for src in $(SOURCES) ; \
	do \
		clang-tidy "$$src" ; \
	done
	@echo "Done"

.PHONY: cloc
cloc:
	cloc src/ include/ test/

.PHONY: clean
clean:
	rm -rf clean/
	rm -rf build/
