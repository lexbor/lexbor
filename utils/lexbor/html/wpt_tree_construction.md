# WPT HTML Tree Construction Tests

Run the WPT tree construction tests with:

    cmake -S . -B build -DLEXBOR_BUILD_TESTS=ON
    cmake --build build
    python3 utils/lexbor/html/wpt_tree_construction.py

If no downloaded `.dat` files are present, the helper downloads them before
running the C runner.  By default, the runner and resources are located under
`build`.  Use `--build-dir` to select another build directory.

To refresh the downloaded WPT resources and remove files that disappeared from
the upstream set:

    python3 utils/lexbor/html/wpt_tree_construction.py --force-download

To prepare or refresh files without running the tests:

    python3 utils/lexbor/html/wpt_tree_construction.py --no-run
    python3 utils/lexbor/html/wpt_tree_construction.py --force-download --no-run

To remove the downloaded WPT resources:

    python3 utils/lexbor/html/wpt_tree_construction.py --clean

To enable the WPT runner as part of CTest:

    cmake -S . -B build -DLEXBOR_BUILD_TESTS=ON -DLEXBOR_ENABLE_TEST_HTML_WPT=ON
    cmake --build build
    ctest --test-dir build -R lexbor_html_wpt_tree_construction

The first CTest run may download WPT resources from GitHub.  In the CMake
scenario, downloaded resources live under
`<build-dir>/test/files/lexbor/html/wpt_tree_construction`.

The C runner consumes the original WPT `.dat` files directly.  Downloaded WPT
resources for direct script runs also live under
`<build-dir>/test/files/lexbor/html/wpt_tree_construction` unless `--dest` is
specified, and may be removed by `--clean`.
