build:
	@$(MAKE) --no-print-directory --directory build

test:
	ctest --test-dir build/test

SOURCES = $(shell find src/ -name '*.cpp')
HEADERS = $(shell find include/ -name '*.h')

fmt:
	@for src in $(SOURCES) ; do \
		echo "Formatting $$src..." ; \
		clang-format -i "$$src" ; \
		clang-tidy -checks='-*,readability-identifier-naming' \
		    -config="{CheckOptions: [ \
		    { key: readability-identifier-naming.NamespaceCase, value: CamelCase },\
		    { key: readability-identifier-naming.ClassCase, value: CamelCase  },\
		    { key: readability-identifier-naming.ClassMemberCase, value: lower_case},\
		    { key: readability-identifier-naming.ClassMethodCase, value: CamelCase},\
		    { key: readability-identifier-naming.StructCase, value: CamelCase  },\
		    { key: readability-identifier-naming.FunctionCase, value: CamelCase },\
		    { key: readability-identifier-naming.VariableCase, value: lower_case },\
		    { key: readability-identifier-naming.GlobalConstantCase, value: UPPER_CASE }, \
		    { key: readability-identifier-naming.StaticConstantCase, value: UPPER_CASE }, \
		    { key: readability-identifier-naming.StaticVariableCase, value: UPPER_CASE }, \
		    ]}" "$$src" ; \
	done
	@for src in $(HEADERS) ; do \
		echo "Formatting $$src..." ; \
		clang-format -i "$$src" ; \
		clang-tidy -checks='-*,readability-identifier-naming' \
		    -config="{CheckOptions: [ \
		    { key: readability-identifier-naming.NamespaceCase, value: CamelCase },\
		    { key: readability-identifier-naming.ClassCase, value: CamelCase  },\
		    { key: readability-identifier-naming.ClassMemberCase, value: lower_case},\
		    { key: readability-identifier-naming.ClassMethodCase, value: CamelCase},\
		    { key: readability-identifier-naming.StructCase, value: CamelCase  },\
		    { key: readability-identifier-naming.FunctionCase, value: CamelCase },\
		    { key: readability-identifier-naming.VariableCase, value: lower_case },\
		    { key: readability-identifier-naming.GlobalConstantCase, value: UPPER_CASE }, \
		    { key: readability-identifier-naming.GlobalConstantCase, value: UPPER_CASE }, \
		    { key: readability-identifier-naming.StaticConstantCase, value: UPPER_CASE }, \
		    { key: readability-identifier-naming.StaticVariableCase, value: UPPER_CASE }, \
		    ]}" "$$src" ; \
	done
	@echo "Done"

tidy:
	@for src in $(HEADERS) ; \
	do \
		clang-tidy -checks="-*,modernize-use-auto,modernize-use-nullptr,\
			readability-simplify-boolean-expr,\
			readability-redundant-member-init,modernize-use-default-member-init,\
			modernize-use-equals-default,modernize-use-equals-delete,\
			modernize-use-using,modernize-loop-convert,\
			cppcoreguidelines-no-malloc,misc-redundant-expression" \
			"$$src" ; \
	done
	@for src in $(SOURCES) ; \
	do \
		clang-tidy -checks="-*,modernize-use-auto,modernize-use-nullptr,\
			readability-simplify-boolean-expr,\
			readability-redundant-member-init,modernize-use-default-member-init,\
			modernize-use-equals-default,modernize-use-equals-delete,\
			modernize-use-using,modernize-loop-convert,\
			cppcoreguidelines-no-malloc,misc-redundant-expression" \
			"$$src" ; \
	done
	@echo "Done"

.PHONY: build fmt tidy test
