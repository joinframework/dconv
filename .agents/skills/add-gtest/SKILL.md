---
name: add-gtest
description: Add and register a gtest file for a dconv header or class. Use when creating tests for new or untested code.
---

# Add a gtest

1. Read the header and a neighbouring test (e.g. `dconv/tests/schubfach_test.cpp`) for the license header, includes and `main`.
2. Create `dconv/tests/<header>_test.cpp` (or `<class>_test.cpp` when a header holds several classes) with one `TEST (<suite>, <function>)` per function (suite = namespace, header or class; constructors in `create`, assignments in `assign`), each documented with a three-line `/**` / ` * @brief <function> test.` / ` */` block and covering success, errors, boundaries, overloads and edge doubles (`±0`, subnormals, `DBL_MIN`/`DBL_MAX`, halfway cases, > 19 digits, `inf`/`nan`, invalid input).
3. Register it in `dconv/tests/CMakeLists.txt` next to its neighbours:

   ```cmake
   add_executable(<name>.gtest <name>_test.cpp)
   target_link_libraries(<name>.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
   gtest_discover_tests(<name>.gtest)
   ```
4. Build and run that target only with `gcc-debug` (skills `compile` and `test`).

If a test reveals a bug, report the failing assertion and propose a diff; do not change the code under test unprompted.
