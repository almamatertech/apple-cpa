# make               builds the test program in three variants, and the arm64e-only library
# make disassembly   compiles the examples for both slices and writes their disassembly
# make clean         removes build

CC = xcrun clang
VARIANTS = full mte-only none

all: $(VARIANTS:%=build/tests-%) build/library.dylib

# The same arm64e.x1 code, signed with tests/full.plist, tests/mte-only.plist or tests/none.plist.
build/tests-%: tests/main.c tests/checks.S tests/%.plist
	@mkdir -p build
	$(CC) -arch arm64e.x1 -O2 tests/main.c tests/checks.S -o $@
	codesign -s - -f --entitlements tests/$*.plist $@

# A library with only an arm64e slice.
build/library.dylib: tests/library.S
	@mkdir -p build
	$(CC) -arch arm64e -dynamiclib $< -o $@
	codesign -s - -f $@

# Each example for arm64e and arm64e.x1 at -O0, -Os and -O2, then add_index with the software check.
disassembly:
	@mkdir -p build/disassembly
	@for example in add_index arithmetic signed; do \
	    for slice in arm64e arm64e.x1; do \
	        for level in O0 Os O2; do \
	            out=build/disassembly/$$example.$$slice.$$level; \
	            $(CC) -arch $$slice -$$level -mmacosx-version-min=27.0 -c examples/$$example.c -o $$out.o || exit 1; \
	            otool -tV $$out.o > $$out.txt; rm $$out.o; \
	        done; \
	    done; \
	done
	@out=build/disassembly/add_index.arm64e.O2.checked; \
	$(CC) -arch arm64e -O2 -mmacosx-version-min=27.0 -fchecked-pointer-arithmetic=checked -c examples/add_index.c -o $$out.o && \
	otool -tV $$out.o > $$out.txt && rm $$out.o
	@ls build/disassembly

clean:
	rm -rf build

.PHONY: all disassembly clean
