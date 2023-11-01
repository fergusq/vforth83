# vforth83

vforth83 is an implementation of the Forth 83 programming language. It conforms to the Forth 83 Standard, as well as to follow as closely as possible the design choices of the Forth 83 Model Implementation by Henry Laxen and Mike Perry. It implements some emulation for MS-DOS system calls via the builtin BDOS command as well as BIOS system calls. Its promise is to provide an easy way to run Forth 83 programs on modern system with good debugging tools not available when running the programs on DOS and CP/M emulators.

This is work in progress, and not everything is implemented yet.

## Implementation Status

Implemented:

- Standard library
  - Most of the Required Word Set (TODO: document missing)
  - Double Number Extension Word Set
  - System Extension Word Set
  - Many controlled and uncontrolled reference words
  - Words from the Model Implementation
- DOS: Emulation of a subset of DOS system calls
- Debugger: Basic implentations for SEE, DEBUG and VIEW + simple tracing

Not implemented:

- BIOS 10H system calls
- Advanced debugging utilities
  - SEE is missing features
  - Tracing not fully implemented
- Advanced filesystem utilities for bypassing DOS system calls if no emulation is needed
- SAVE-SYSTEM

## Credits

The `system.f` file contains definitions copied from the Forth 83 Model Implementation (C) Henry Laxen and Mike Perry 1984.

Other files and source code (C) Iikka Hauhio 2023.