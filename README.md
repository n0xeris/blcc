# Binary Loader for C and C++

Simple utility to bundle binary files into byte arrays inside a header file.

## Building
```sh
$ git clone https://github.com/n0xeris/blcc.git
$ cd blcc
$ make
$ cp dist/blcc <install_location>
```

## Usage
```sh
# Specifying output directory (must be first parameter).
$ blcc -o <output_dir> file1.png file2.wav ...

# If not informed, the header files will be generated on the same directory.
$ blcc file1.png file2.wav ...
```

The output will be:
```c
#ifndef BINARY_DATA_file1_png_H
#define BINARY_DATA_file1_png_H
#define BINARY_DATA_file1_png_SIZE 41767

unsigned char binary_data_file1_png[] = {
	0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x00,
	// ...
	0x45, 0x4E, 0x44, 0xAE, 0x42, 0x60, 0x82
};

#endif // BINARY_DATA_file1_png_H
```
