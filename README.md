# cxxmbd

cxxmbd is a set of utilities for embedding and decoding files as binary. It consists of two parts: a command line application, and a header only decoding utility. These can be used in combination to include raw data in your c++ applications, and then decode them on a user's device. Gone is the era of using xxd to create arrays, it's time for c++ to shine!

## Building the CLI

Building the embedder is very simple. All you have to do is include ``linux_main.cpp`` for linux/osx, or ``windows_main.cpp`` for Windows.

If you wish to build it manually, create a main file on your IDE of choice, include ``cxxmbd.hpp``, and then write the following:
```cpp
int main(int argc, char* argv[]) {
  try {
    cxxmbd::handle_cl_args(argc, argv);
  }
  catch(std::exception& e) {
    std::cout << e.what() << "\n\n";
  }
}
```
You can then get started on embedding your files!

## Using the CLI

To embed a file, you must add an ``EMBED_POINT`` to the source file. You can then run this:
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

## Decoding the data

Your data will be embedded using the name(s) of the files you input. For example, if you embed the file ``example.txt``, 
``struct<N> example {...};`` will be created. With this, you can now decode your files.

To decode a struct, all you need to do is include ``mbddecoder.hpp`` and then write:
```cpp
cxxmbd::decode_embed(example);
```
This will create a new file in the program's current directory. You can also supply a custom path to create the file in.

## Notes

This program is in its very early stages, so if you encounter any bugs, be sure to let me know!
