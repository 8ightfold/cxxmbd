# cxxmbd

**cxxmbd** is a set of utilities for embedding and decoding files as binary. 
It consists of two parts: a command line application, and a header only decoding utility. 
These can be used in combination to include raw data in your c++ applications, and then decode them on a user's device. 
Gone is the era of using xxd to create arrays, it's time for c++ to shine!

## Modes of operation

Building the embedder is very simple. All you have to do is adjust some settings in your cmake file.
**cxxmbd** has 3 different build modes, depending on your intent: *attach*, *include* and *standalone*.
The first mode, *attach*, is used to build the utility in the hot reloading embed mode.
The other two, *include* and *standalone*, are different methods of creating a command line interface.

### *Attaching cxxmbd to your project*

Adding the utility to your project is relatively easy, only requiring two things: a ``.toml`` file,
and a dependency. First let's look at adding the dependency:
```cmake
set(CXXMBD_MODE attach)

add_subdirectory(cxxmbd)
add_executable(foo main.cpp lib.h)
target_link_libraries(foo PUBLIC cxxmbd)
add_dependencies(foo mbdrun)
```
With this, an executable will be generated, and when building, the program will look for a file called
``mbdconfig.toml`` in your root build directory. An example ``.toml`` file can be found in ``examples``.

### *Building the CLI(s)*

Now let's look at building the program as a CLI. We will first look at *include*, 
which allows you to add the **cxxmbd** source files to an unrelated project. 
The following is an example of how to build in include mode:
```cmake
set(CXXMBD_MODE include)

add_subdirectory(cxxmbd)
add_executable(foo main.cpp lib.h)
target_link_libraries(foo PUBLIC cxxmbd)
```

Next, we have *standalone*. This mode will build **cxxmbd** as a standalone project, with the method of inputting
arguments left up to the user.

To select the regular ``main`` function, use the following:
```cmake
set(CXXMBD_MODE standalone)
set(CXXMBD_WINMAIN false)

add_subdirectory(cxxmbd)
add_executable(foo $ENV{MAIN_PATH})
target_link_libraries(foo PUBLIC cxxmbd)
```
``WinMain`` can be enabled by changing the associated variable to ``true``.

### *Using the CLI*

To embed data in a source file, you must add an ``EMBED_POINT`` to the source file. You can then run this:
```bash
cxxmbd --embed <path-to-output> <path-to-source(s)>
```
You can also dump the data to its own text file with the name of your choice by running:
```bash
cxxmbd --dump <name> <path-to-source(s)>
```
For more info, run:
```bash
cxxmbd --help
```

## Decoding your data

Your data will be embedded using the name(s) of the files you input. For example, if you embed the file ``example.txt``, 
``struct<N> example {...};`` will be created. With this, you can now decode your files.

To decode a struct, all you need to do is include ``mbddecoder.hpp`` and then write:
```cpp
cxxmbd::decode_embed(example);
```
This will create a new file in the program's current directory. You can also supply a custom path to create the file in.

## Notes

This program is in its very early stages, so if you encounter any bugs, be sure to let me know!
