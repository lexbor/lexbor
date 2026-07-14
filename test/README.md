# Lexbor Tests

This directory contains unit, integration, fuzzing, and optional WPT-based
tests for Lexbor.  Tests are disabled by default and are enabled with CMake
options.

## Build And Run All C Tests

Configure with tests enabled:

```sh
cmake -S . -B build -DLEXBOR_BUILD_TESTS=ON
```

Build the project and test executables:

```sh
cmake --build build
```

Run all registered tests:

```sh
ctest --test-dir build --output-on-failure
```

## C++ Tests

C++ tests are built only when both test options are enabled:

```sh
cmake -S . -B build \
  -DLEXBOR_BUILD_TESTS=ON \
  -DLEXBOR_BUILD_TESTS_CPP=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

## Amalgamation Test

The amalgamation test is a CTest test and requires normal tests to be enabled:

```sh
cmake -S . -B build \
  -DLEXBOR_BUILD_TESTS=ON \
  -DLEXBOR_TEST_AMALGAMATION=ON
cmake --build build
ctest --test-dir build -R '^amalgamation$' --output-on-failure
```

## HTML WPT Tree Construction Tests

The WPT HTML tree construction runner is disabled by default because its first
run can download WPT resources from GitHub.  Enable it explicitly:

```sh
cmake -S . -B build \
  -DLEXBOR_BUILD_TESTS=ON \
  -DLEXBOR_ENABLE_TEST_HTML_WPT=ON
cmake --build build
ctest --test-dir build -R '^lexbor_html_wpt_tree_construction$' --output-on-failure
```

When run through CTest, downloaded WPT resources are stored in:

```text
<build-dir>/test/files/lexbor/html/wpt_tree_construction
```

The helper script can also be run directly from the source tree:

```sh
python3 utils/lexbor/html/wpt_tree_construction.py
```

See `utils/lexbor/html/wpt_tree_construction.md` for helper-specific options
such as `--force-download`, `--clean`, and `--no-run`.

## Sanitizer Builds

AddressSanitizer:

```sh
cmake -S . -B build-asan \
  -DLEXBOR_BUILD_TESTS=ON \
  -DLEXBOR_BUILD_WITH_ASAN=ON
cmake --build build-asan
ctest --test-dir build-asan --output-on-failure
```

MemorySanitizer:

```sh
cmake -S . -B build-msan \
  -DLEXBOR_BUILD_TESTS=ON \
  -DLEXBOR_BUILD_WITH_MSAN=ON
cmake --build build-msan
ctest --test-dir build-msan --output-on-failure
```

Sanitizer support depends on the compiler and platform.  CMake prints whether
the requested sanitizer feature was enabled.

## Fuzzer Builds

Fuzzer builds are a separate mode and require compiler support for
`-fsanitize=fuzzer`:

```sh
cmake -S . -B build-fuzzer \
  -DLEXBOR_BUILD_WITH_FUZZER=ON \
  -DLEXBOR_BUILD_TESTS=ON
cmake --build build-fuzzer
```

Fuzzer targets are built from `test/fuzzers/lexbor`.

## Listing Tests

List all registered tests without running them:

```sh
ctest --test-dir build -N
```

List tests matching a name pattern:

```sh
ctest --test-dir build -N -R html
```

## Running One Test

Run one registered CTest test by exact name:

```sh
ctest --test-dir build -R '^lexbor_html_tokenizer_tokens$' --output-on-failure
```

Run one group by regex:

```sh
ctest --test-dir build -R '^lexbor_html_' --output-on-failure
```

You can also run the built executable directly.  For tests that need fixture
files, pass the same path CTest uses.  For example:

```sh
build/test/lexbor/html/tokenizer_tokens test/files/lexbor/html/tokenizer
```

For tests that do not need extra arguments:

```sh
build/test/lexbor/html/parse
```

CTest is usually preferred for repeatable local and CI runs because it supplies
the registered arguments automatically.
