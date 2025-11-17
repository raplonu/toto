# Toto, a C++ and python module

`toto` is an example project that shows how to build and package a C++ library and python module using `uv`, `scikit` and `conan`. It can be built/packaged as a standalone C++ library or can be embedded inside a python module.

Both component can be build and installed in parallel or can be used in editable mode.

As an example, `toto` will depends on [`emu`](https://github.com/raplonu/emu/), a C++ library that is **not** packaged on `conancenter`, the default conan index. This part is not mandatory, you can remove `emu` from `conanfile.py`'s `requirements` method, from the `CMakeLists.txt` file and comment the code in `src/python/toto.cpp`.

## File hierarchy

```bash
.
├── README.md
├── pyproject.toml
├── CMakeLists.txt
├── conanfile.py
├── cmake
│   └── conan_provider.cmake
├── include
├── src
│   ├── lib
│   ├── python
│   └── toto
├── build
│   ├── cp312-cp312-linux_x86_64
│   ├── Debug
│   └── Release
└── test_package
```

At root level, `pyproject.toml`, `CMakeLists.txt`, `conanfile.py` will gather all information regarding the C++ lib, the python module and their dependencies.

All of python source code is located in `src/<package-name>` or `src/toto`. It is one of the wheels' default location ([see documentation](https://scikit-build-core.readthedocs.io/en/stable/configuration/index.html?utm_source=chatgpt.com#customizing-the-built-wheel)).

For C++, as usual, it's a bit more complex. Public header goes in `include` folder and library source code is in `src/lib`. It is recommended to keep header in `include` since it is the default location `conan` is going to look for using editable mode.

In order to use the C++ lib with python, we need to compile a wrapper module. Its source code is located in `src/python`.

`conan_provider.cmake` is a utility developed by the conan team that will "override" the `CMake` `find_package` function to automatically call `conan install` and use the generated xxxConfig.cmake files.

`test_package` is a special folder name used by `conan`. When you package you project. Conan will automatically try to expose `toto` to the `test_package` project. Handy if you want to test header, link and `cmake` target.

Finally, when building the python package, `scikit` will use the `build/<wheel_tag>` folder and `CMake`/`conan` will use the `build/<build_type>` folders (if used with single config generator such as `make` or `ninja`).

## Tools installation

The first tool to install is `uv`. To install it, refer to [the documentation](https://docs.astral.sh/uv/#installation) or type:

```bash
curl -LsSf https://astral.sh/uv/install.sh | sh
```

Then, it is recommended to create a virtual environment:

```bash
# Create a venv at current location in .venv folder
uv venv
# Activate it
source .venv/bin/activate
```

Even though installing the project will fetch all build dependencies, it is recommended to install them at the venv level.

```bash
uv pip install cmake conan ninja
```

## Conan

`conan` is now installed. However, we still need to configure it.

First, we want to generate a conan profile using:

```bash
conan profile detect
```

You may consider modifying the generated profile to fit you need. The profile path can be obtained using:

```bash
# Using detect, the profile name will be `default`
conan profile path default
```

Then, we can for instance change `compiler.cppstd` value to `gnu20`.

Another file to modify is `$(conan config home)/global.conf`. Some possible addition:

- `tools.cmake.cmaketoolchain:generator="Ninja"`: Will use `ninja` instead of `make`
- `tools.build:skip_test=True`: Will no fetch, build and run unit tests by default. Can be overridden using the `-c` flag with conan commands (install, build, etc.)
- `tools.cmake.cmakedeps:new=will_break_next`: This is needed in order to use `emu`, more on that bellow.

To get the complete list, see [conan config documenation](https://docs.conan.io/2/reference/commands/config.html#conan-config-list) or type:

```bash
conan config list [pattern]
```

Finally, we want to install an additional repository (one that have `emu`). `conan` recommend to setup a `conan` server and adds it as a remote. However, in our case, we only have a simple github repository [`raplonu/cosmic-center-index`](https://github.com/raplonu/cosmic-center-index/). Fortunately, `conan` allow to consider a directory as a remote.

To add `cosmic-center-index`, type:

```bash
curl -sS https://raw.githubusercontent.com/raplonu/cosmic-center-index/refs/heads/main/install.sh | bash
```

It will download the github repo in the `conan` folder and adds it as a remote.

You can check it has been sucefully installed using:

```bash
conan remote list
```

And also check for emu using:

```bash
conan search -r cosmic emu
```

Note: `-r cosmic` is not mandatory.

## Build

### C++ library

To compile the library locally, just type:

```bash
conan build -b missing
```

The command will do the following:

```bash
conan install -b missing # Fetch and compile deps
cmake --preset conan-release # cmake configure with default build type (release)
cmake --build --preset conan-release # build the library
```

`conan build/install` is only needed once. It will generate a `cmake` preset `conan-<build_type>` that can be used to configure and build the `cmake` project.

Note: `-b missing` will build dependencies without pre-build binaries. Such binaries will be stored in `conan` local cache. Subsequent call to `conan` will reuse the binaries thus `-b missing` will no longer be needed.

In addition, we can upload the package to the local cache using:

```bash
conan create
```

You can check the package have been installed using:

```bash
conan list toto
```

### Python module

To install the python module, type:

```bash
uv pip install .
```

This command will build the library and the python wrapper module inside `build/<wheel_tag>` and install it at the venv level.

Note: Be careful, despite build the package in a persistent directory, you cannot reuse the content to perform incremental build. To do so, see the editable section.

## Editable

Both the C++ library and the python module support editable mode. The next sections will explain how to do it.

### C++ library

See [conan editable package](https://docs.conan.io/2/tutorial/developing_packages/editable_packages.html#editable-packages)

`conan` has a editable mode. It will create an editable package that will by default reuse the header in the `include` folder and the libraries listed in `conanfile.py`'s `package_info` method.

To create an editable package:

```bash
conan editable add
```

You can check is it in editable mode using:

```bash
conan editable list
```

Notes:

- This command will not build you project. You need to do it yourself (before or after).
- Be careful to build the package using the same configuration that the user package, that includes: build type, options, conan profile and more.
- An editable mode will always be selected first if version match, be aware of that.

To stop use a package in editable mode:

```bash
conan editable remove
```

### Python module

See [scikit editable install](https://scikit-build-core.readthedocs.io/en/stable/configuration/index.html#editable-installs)

To install the package in editable mode, type:

```bash
uv pip install --no-build-isolation -e .
```

Doing this will install the python package in editable mode. Thank to scikit, it will also cover the C++ code. When importing your package, `scikit` will call the build and install step, producing this output:

```python
>>> from toto import add
Running cmake --build & --install in […]/build/cp312-cp312-linux_x86_64
ninja: no work to do.
-- Install configuration: "Release"
-- Up-to-date: […]/.venv/lib/python3.12/site-packages/toto/_toto.cpython-312-x86_64-linux-gnu.so
-- Installing: […]/.venv/lib/python3.12/site-packages/toto/libtoto.a
>>> add(1,2)
Adding 1 and 2
3
```

Note: Sometimes, the code won't compile and the output might be difficult to read from the python terminal. Since their is no build isolation, you can directly call ninja

```bash
ninja -C build/<wheel_tag>
# or
cmake --build build/<wheel_tag>
```
