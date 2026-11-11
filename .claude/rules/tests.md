---
paths:
  - "**/tests/**"
---

# Tests

- GoogleTest. **One file per header**, or per class when a header holds several (`probe.hpp` -> `string_probe_test.cpp`, `stream_probe_test.cpp`). Suite named after the namespace, header or class (`TEST (schubfach, floorLog2Pow10)`, `TEST (StreamProbe, peek)`, `TEST (ErrorCategory, name)`). **One `TEST` per function**, all its paths (success, errors, boundaries, overloads) inside; constructors in `TEST (<Class>, create)`, assignments in `TEST (<Class>, assign)`. Cross-cutting tests (`roundtrip_test.cpp`) are the exception.
- Each test has a three-line `/**` / ` * @brief <function> test.` / ` */` block and the file ends with the `main` of its neighbours. `ASSERT_*` when later lines depend on it, `EXPECT_*` otherwise.
- Register in `dconv/tests/CMakeLists.txt`:

```cmake
add_executable(<name>.gtest <name>_test.cpp)
target_link_libraries(<name>.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(<name>.gtest)
```

- Cover edge doubles: `±0`, subnormals (`5e-324`), `DBL_MIN`/`DBL_MAX`, halfway cases, > 19-digit mantissas, `inf`/`nan`, invalid input. Output changes need a case in `roundtrip_test.cpp`.
- Never weaken or delete an assertion to make a test pass.
