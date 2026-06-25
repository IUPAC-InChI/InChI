# Builds the OSS-Fuzz fuzz targets for InChI. Driven by the OSS-Fuzz toolchain
# environment ($CC/$CXX/$CFLAGS/$CXXFLAGS/$LIB_FUZZING_ENGINE/$OUT/$WORK). Every
# ossfuzz/*_fuzzer.c is compiled and linked against the InChI library.

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(dirname "$HERE")"

cd "$ROOT/INCHI-1-SRC"

# Compile the library sources (exclude ichimain.c, the standalone CLI main).
SRC_FILES=$(ls INCHI_BASE/src/*.c INCHI_API/libinchi/src/*.c INCHI_API/libinchi/src/ixa/*.c | grep -v ichimain.c)
$CC $CFLAGS -Wno-everything -DTARGET_API_LIB -c $SRC_FILES
ar rcs "$WORK/libinchi.a" *.o

# Compile + link each fuzz target.
for fuzzer in "$HERE"/*_fuzzer.c; do
  fuzzer_basename=$(basename -s .c "$fuzzer")

  $CC $CFLAGS \
      -I INCHI_BASE/src/ \
      -I INCHI_API/libinchi/src/ \
      -I INCHI_API/libinchi/src/ixa/ \
      "$fuzzer" -c -o "${fuzzer_basename}.o"

  $CXX $CXXFLAGS \
      "${fuzzer_basename}.o" -o "$OUT/$fuzzer_basename" \
      $LIB_FUZZING_ENGINE "$WORK/libinchi.a"
done
