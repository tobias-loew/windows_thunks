# requirement

* install vcpkg from <https://github.com/microsoft/vcpkg>
* install boost unit test
```shell
vcpkg install boost-test
```

# build & test
```shell
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake
cmake --build build --target RUN_TESTS
```
