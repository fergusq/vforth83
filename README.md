# vforth83

vforth83 is an implementation of the Forth 83 programming language. It conforms to the Forth 83 Standard, as well as to follow as closely as possible the design choices of the Forth 83 Model Implementation by Henry Laxen and Mike Perry. It implements some emulation for MS-DOS system calls via the builtin BDOS command as well as BIOS system calls. Its promise is to provide an easy way to run Forth 83 programs on modern system with good debugging tools not available when running the programs on DOS and CP/M emulators.

This is work in progress, and not everything is implemented yet.

## Implementation Status

Implemented:

- Words defined by the Forth 83 Standard (see [list of implemented words](./IMPLEMENTED.md))
  - Required Word Set
  - Double Number Extension Word Set
  - System Extension Word Set
  - Many controlled and uncontrolled reference words
  - Words from the Model Implementation
- DOS: Emulation of a subset of DOS system calls
- BIOS 10H system calls
  - Basic text mode I/O with curses
- Debugger: Basic implentations for SEE, DEBUG and VIEW + simple tracing

Not implemented:

- Multitasking and USER words
- Hercules Graphics emulation
- Advanced debugging utilities
  - SEE is missing features
  - Tracing not fully implemented
- Advanced filesystem utilities for bypassing DOS system calls if no emulation is needed
- Writing to files

## Dependencies

- ncursesw

## Compilation

```sh
mkdir build
cd build
cmake ..
make
```

The ANS Forth test suite by John Hayes modified to accommodate Forth 83 is included in the `tests` directory.
The tests can be run with:

```sh
make test
```

## Usage

To run a file, pass `-s` to vforth83. Typically, you want to load `system.fs` before any other files.

```sh
vforth83 -s system.fs -s file.fs
```

You can save a snapshot of the dictionary to a file to speed up loading files.
This can also be used if you wish to distribute files in compiled form instead of their source code.

```sh
vforth83 -s system.fs -M system.bin
vforth83 -m system.bin -s file.fs
```

The `-i` flag can be used to start the interpreter mode.

```sh
vforth83 -s system.fs -i
# or
vforth83 -m system.bin -i
```

## Sampo 2

The source code files for the [Sampo 2 Programming Language](https://fi.wikipedia.org/wiki/Sampo_(ohjelmointikieli)) written in Forth 83 have been included in the `sampo` folder. The original files can be found in [Sampo's GitHub repository](https://github.com/sampo-lang/sampo/tree/master/Sampo_Fenix). You can start Sampo with the following command:

```sh
cd sampo
vforth83 -s ../system.fs -s sampo.fs --curses --finnish
```

## Credits

The `system.f` file contains definitions copied from the Forth 83 Model Implementation (C) Henry Laxen and Mike Perry 1984.

The `core.fr` and `tester.fr` files by John Hayes (C) Johns Hopkins University / Applied Physics Laboratory 1995.

Files in the `sampo` folder (C) Systiimi Oy 1987.

Other files and source code (C) Iikka Hauhio 2023.