# MyDisassembler

Welcome to MyDisassembler, a project designed to help learners understand X86-64 machine codes, assembly language, ELF format, and disassembly strategies. With this tool, we can explore the intricacies of low-level programming and gain insights into how binaries are structured and executed.

## Install

To get started with MyDisassembler, follow these simple steps:

- Clone the Repository:

```bash
git clone https://github.com:Koukyosyumei/MyDisassembler.git
```

- Build the Project:

```bash
cd MyDisassembler
./script/build.sh
```

## Usage

Once installed, you can use MyDisassembler to disassemble binary files and delve into their assembly code. Here's how you can use it:

```bash
./build/script/mydisas example/jmp.o
```

```yaml
section: .text ----

40 <_start>:
 40: mov  eax 0x00000000                      ( b8 0 0 0 0 )
 45: cmp  eax 0x00                            ( 83 f8 0 )
 48: jz 4e <zero_label> ; relative offset = 4 ( 74 4 )
 4a: jmp 52 <end_label> ; relative offset = 6 ( eb 6 )
 4c: jmp 40 <_start> ; relative offset = -14  ( eb f2 )

4e <zero_label>:
 4e: push  rsp                                ( 54 )
 4f: xor  eax eax                             ( 31 c0 )
 51: ret                                      ( c3 )

52 <end_label>:
 52: push  rdi                                ( 57 )
 53: xor  ecx ecx                             ( 31 c9 )
 55: ret                                      ( c3 )
-------------------
Done!
```

## Features

- Implemented entirely from scratch in C++
- Supports both linear sweeping and recursive descent disassembly strategies
- Handles most basic operations with precision
- Capable of parsing ELF headers for deeper analysis

## Future Improvements

- Expand test coverage for enhanced reliability
- Add support for VEX Prefix
- Incorporate additional instructions, including floating-point operations
- Introduce support for AT&T syntax

Feel free to contribute to MyDisassembler and make it even better!
