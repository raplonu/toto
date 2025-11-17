cmake_minimum_required(VERSION 3.28)

project(toto LANGUAGES CXX)

set(
    CONAN_ARGS
    "" CACHE STRING
    "Additional conan args"
)

# Only when building the python module
if(SKBUILD)
    # CONAN_INSTALL_ARGS is used by cmake-conan.
    # We want to tell conan to call the install command with the python_module
    # option set to True.
    set(CONAN_INSTALL_ARGS "--build=missing;--options=toto/*:python_module=True;${CONAN_ARGS}")
endif()

add_library(toto)

# Modern CMake target sources description
target_sources(toto
    PRIVATE
        src/lib/fun.cpp
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            ${CMAKE_CURRENT_SOURCE_DIR}/include
        FILES
            ${CMAKE_CURRENT_SOURCE_DIR}/include/toto/fun.hpp
            ${CMAKE_CURRENT_SOURCE_DIR}/include/toto.hpp
)

# Set the minimum C++ standard required
target_compile_features(toto PUBLIC cxx_std_20)

find_package(fmt CONFIG REQUIRED)
find_package(emu CONFIG REQUIRED)

target_link_libraries(toto PUBLIC
    fmt::fmt
    emu::emu
)

if (NOT SKBUILD)

    # Only install the headers and the library when building the C++ library.
    install(TARGETS toto
        FILE_SET HEADERS
    )

else() # When building with scikit-build

    # Let pybind11 find the python interpreter by itself.
    set(PYBIND11_FINDPYTHON ON)
    find_package(pybind11 CONFIG REQUIRED)

    pybind11_add_module(_toto src/python/toto.cpp)

    target_link_libraries(_toto PRIVATE toto)

    # We only install the module and library. No need to install the headers for toto and _toto.
    install(TARGETS _toto DESTINATION ${SKBUILD_PROJECT_NAME})

    # We install all the libs next to each other in the toto package.
    # Let them simply link with each other when installed.
    set(CMAKE_INSTALL_RPATH "$ORIGIN")

    # We also copy/bundle every shared libraries to the destination
    install(TARGETS toto DESTINATION ${SKBUILD_PROJECT_NAME})
    file(GLOB SHARED_LIBS "${CMAKE_BINARY_DIR}/[!_]*.so*")
    install(FILES ${SHARED_LIBS} DESTINATION ${SKBUILD_PROJECT_NAME})
endif()
