.PHONY: cmake
cmake:
	cmake -S . -B build

WIN_DIR = /Documents/code/roc-win64
FULL_WIN_DIR := $(join ${WIN_HOME_DIR}, $(WIN_DIR))
TOP_LEVEL = $(shell find . -maxdepth 1 -type f)
SRC_DIRS = cmake include src test

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
	@$(MAKE) --no-print-directory --directory build

.PHONY: test
test:
ifndef testregex
	ctest --test-dir build/test --output-on-failure
else
	ctest --test-dir build/test --output-on-failure -R $(testregex)
endif

SOURCES = $(shell find src/ -name '*.cpp')
HEADERS = $(shell find include/ -name '*.h')

.PHONY: fmt
fmt:
	@for src in $(SOURCES) ; do \
		echo "Formatting $$src..." ; \
		clang-format -i "$$src" ; \
	done
	@for src in $(HEADERS) ; do \
		echo "Formatting $$src..." ; \
		clang-format -i "$$src" ; \
	done
	@echo "Done"

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

.PHONY: release
release: sync build test
