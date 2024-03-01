# biner

Combine and separate text files

## Dependencies

- C++17 compiler
- meson build system

## Installation

- `meson setup build/ --prefix=/usr`
- `meson install -C build/`

## Usage

Run `biner -c file1 file2 file3 ... -o combined` to
combine a few text files and output it to `combined`.
If the `-o` parameter is omitted, it will be output to
standard input.

When editing the combined file, make sure to keep the biner
markers intact. Otherwise biner will fail to combine them.

To split the files, run `biner -s combined`. This will output
the same files in the current directory.

See `-h` for more information.

## License

Copyright(c) speedie 2024

This project is free software licensed under the
GNU General Public License version 3.

See LICENSE file for more information.
