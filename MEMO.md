## Opcode and Operand

In assembly language, the opcode indicates the type of instruction, while the operand specifies the target of the instruction.

```
- example

Opcode   Operand
   add    eax, 0x01
```

Different types of operands exist, each with its own encoding, known as Op/En.

| Op/En | Operand 1        | Operand 2         | Operand 3 | Operand 4 |
|-------|------------------|-------------------|-----------|-----------|
| RM    | r(8,16,32,64)    | r/m(8,16,32,64)   | N/A       | N/A       |
| MR    | r/m(8,16,32,64)  | r(8,16,32,64)     | N/A       | N/A       |
| MI    | r/m(8,16,32,64)  | imm(8,16,32,64)   | N/A       | N/A       |
| I     | AL/AX/EAX/RAX    | imm(8,16,32,64)   | N/A       | N/A       |

Here, imm represents an immediate value, r denotes a register, and r/m indicates an addressing mode specified by the ModR/M and SIB bytes, which will be explained later. There are more and more operand types, and you can check it in [this amazing website](https://www.felixcloutier.com/x86/).

## Instruction Format

In x86-64, an instruction typically follows this format:

```
|prefix | REX prefix  |     opecode      |   ModR/M    |   SIB       |     address offset     |      immediate         |
|------ |-------------|------------------|-------------|-------------|------------------------|------------------------|
|       | 0 or 1 byte | 1, 2, or 3 bytes | 0 or 1 byte | 0 or 1 byte | 0, 1, 2, 3, or 4 bytes | 0, 1, 2, 3, or 4 bytes |
```

### Prefix

In x86-64, variable-length prefixes can be used to extend the functionality of instructions or change operand lengths. Some common prefixes are prefix instructions and the operand-size prefix.

- Prefix Instruction

Prefix instructions, placed before other instructions, alter the subsequent instruction's behavior.

```
- example

REP: Repeats string operation instructions.
LOCK: Ensures that memory access instructions like ADD or XOR are atomic.
```

- Operand-size Prefix

The operand-size prefix (0x66) changes the operand size to 16 bits.

### Opecode

In x86-64, the mnemonic and operand types are determined by the prefix instruction, opcode, and the reg field of the ModR/M byte.

```
- example

   Prefix    |  Opcode |  reg |   Instruction
-------------|---------|------|-----------------
     -       |  0x81   |  0   | ADD r/m32, imm32
     -       |  0x81   |  5   | SUB r/m32, imm32
   REX.W     |  0x81   |  0   | ADD r/m64, imm32
```

### REX Prefix

The REX prefix is used to extend the ModR/M and SIB bytes, specify operand size, and access registers r8 to r15.

```
bit   |7|6|5|4|3|2|1|0|
------|-|-|-|-|-|-|-|-|
field |0|1|0|0|W|R|X|B|
```

```
Field Name | Bit Position | 
-----------|--------------|-----------------------------------------
           |    7 - 4     | Indicates a REX prefix when set to 0100
      W    |        3     | 0 = default operand size, 1 = 64-bit operand size
      R    |        2     | Extends the ModR/M reg field
      X    |        1     | Extends the SIB index field
      B    |        0     | Extends the ModR/M r/m field, SIB base, or opcode reg field
```


### ModR/M (Mode Register Memory)

The ModR/M byte specifies operand registers and addressing modes.

```
 bit   |7|6|5|4|3|2|1|0|
-------|---|-----|-----|
 field |mod| reg | r/m |
-------|---|-----|-----|
 rex   |   |  r  |  b  |
```

- reg

The reg field specifies the operand register. When the REX prefix's R field is set, it extends the range to r8-r15.

| reg | Register (rex.r = 0) | Register (rex.r = 1) |
| --- | --- | --- |
| 000 | RAX | R8  |
| 001 | RCX | R9  |
| 010 | RDX | R10 |
| 011 | RBX | R11 | 
| 100 | RSP | R12 |
| 101 | RBP | R13 |
| 110 | RSI | R14 |
| 111 | RDI | R15 |

- mod and r/m

The addressing mode is determined by the mod and r/m fields, along with the REX prefix's B field. When mod is 11, the operand is directly specified by the register. Otherwise, it specifies a memory address.

Additionally, when the r/m field is 101 and the mod field is 00, RIP-relative addressing is used, enabling Position Independent Code (PIC).

|                |     |                |     mod       |                |     |
|----------------|-----|----------------|---------------|----------------|-----|
|                |     | 00             | 01            | 10             | 11  |
|                | 000 | [RAX]          | [RAX + disp8] | [RAX + disp32] | RAX |
|                | 001 | [RCX]          | [RCX + disp8] | [RCX + disp32] | RCX |
|                | 010 | [RDX]          | [RDX + disp8] | [RDX + disp32] | RDX |
|r/m (rex.b = 0) | 011 | [RBX]          | [RBX + disp8] | [RBX + disp32] | RBX |
|                | 100 | [SIB]          | [SIB + disp8] | [SIB + disp32] | RSP |
|                | 101 | [RIP + disp32] | [RBP + disp8] | [RBP+ disp32]  | RBP |
|                | 110 | [RSI]          | [RSI + disp8] | [RSI + disp32] | RSI |
|                | 111 | [RDI]          | [RDI + disp8] | [RDI + disp32] | RDI |

|                |     |                |     mod       |                |     |
|----------------|-----|----------------|---------------|----------------|-----|
|                |     | 00             | 01            | 10             | 11  |
|                | 000 | [R8]           | [R8 + disp8]  | [R8 + disp32]  | R8  |
|                | 001 | [R9]           | [R9 + disp8]  | [R9+ disp32]   | R9  |
|                | 010 | [R10]          | [R10 + disp8] | [R10 + disp32] | R10 |
|r/m (rex.b = 1) | 011 | [R11]          | [R11 + disp8] | [R11 + disp32] | R11 |
|                | 100 | [SIB]          | [SIB + disp8] | [SIB + disp32] | R12 |
|                | 101 | [RIP + disp32] | [R13 + disp8] | [R13+ disp32]  | R13 |
|                | 110 | [R14]          | [R14 + disp8] | [R14 + disp32] | R14 |
|                | 111 | [R15]          | [R15 + disp8] | [R15 + disp32] | R15 |

### SIB (Scale Index Base) 

```
|  bit  | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
|-------|---|---|---|---|---|---|---|---|
| field | scale |   index   |   base    |
|-------|-------|-----------|-----------|
```

When the ModR/M byte specifies SIB, it defines memory addresses in the form base register + (index register * scale) + offset. The base and index registers can be any general-purpose registers, and the scale can be 1, 2, 4, or 8.

The example usages of this form include using the base register for the start address of an array, the index register for the array index, and the scale for the size of each array element.

- base

The base field specifies the base register. To specify extended general-purpose registers (r8-r15), set the REX prefix's B field.

| base (reb.b = 0) | mod | Register |
|------------------|-----|---------|
| 000 | | RAX |
| 001 | | RCX |
| 010 | | RDX |
| 011 | | RBX |
| 100 | | RSP |
| 101 |00 | disp32 |
| 101 |01 | RBP + disp8 |
| 101 |10 | RBP + disp32 |
| 110 | | RSI |
| 111 | | RDI |

| base (reb.b = 1) | mod | Register |
|------------------|-----|---------|
| 000 | | R8 |
| 001 | | R9 |
| 010 | | R10 |
| 011 | | R11 |
| 100 | | R12 |
| 101 |00 | disp32 |
| 101 |01 | R13 + disp8 |
| 101 |10 | R13 + disp32 |
| 110 | | R14 |
| 111 | | R15 |

- index

The index field specifies the index register. To specify extended general-purpose registers (r8-r15), set the REX prefix's X field.

| index (rex.x = 0) | Register |
|-------------------|---------|
| 000 | RAX |
| 001 | RCX |
| 010 | RDX |
| 011 | RBX |
| 100 | (RSP) |
| 101 | RBP |
| 110 | RSI |
| 111 | RDI |

| index (rex.x = 1) | Register |
|-------------------|---------|
| 000 | R8 |
| 001 | R9 |
| 010 | R10 |
| 011 | R11 |
| 100 | R12 |
| 101 | R13 |
| 110 | R14 |
| 111 | R15 |

- scale

The scale field specifies the multiplier for the index register.

|scale|multiplier|
|-----|----|
|00 | 1 |
|01 | 2 |
|10 | 4 |
|11 | 8 |

Note that since rsp is generally used to point to the top of the stack, the index register is not used when the index field is 100 (rsp).

## Disassembler

The disassembler we are implementing will decode each instruction one byte at a time, following these steps:

```
1. If the current byte matches a byte of prefix instructions -> a prefix instruction exists & move forward one byte
2. If the current byte is 0x66 -> an operand-size prefix exists & move forward one byte
3. If the first 4 bits are 0b0100 -> the current byte is a REX prefix & move forward one byte
4.a: If the current byte is part of a multi-byte instruction -> if the first two bytes match a known opcode, move forward two bytes; otherwise, move forward one byte
4.b: If the current byte is not part of a multi-byte instruction -> the current byte is an opcode & move forward one byte
5. If the opcode requires the ModR/M byte's reg field to determine the mnemonic (instruction type), interpret the current byte as the ModR/M byte to determine the mnemonic.
6. If a prefix instruction was found, determine its type considering the mnemonic (e.g., if the prefix byte is 0xF2 and the mnemonic is a control flow instruction, it is `bnd`; if it is a string instruction, it is `repne`)
7. Determine the operand types from the combination of prefix, mnemonic, and opcode
8. If the operands require a ModR/M byte, the current byte is the ModR/M byte & move forward one byte
9. If the ModR/M byte specifies an SIB byte, the current byte is the SIB byte & move forward one byte
10.a: If the ModR/M byte specifies an 8-bit displacement (disp8), the current byte is the displacement & move forward one byte
10.b: If the ModR/M byte specifies a 32-bit displacement (disp32), the first four bytes are the displacement & move forward four bytes
10.c: Decode the endianness and complement representation of the displacement obtained in steps 10.a or 10.b
11. Using the decoded operand types, ModR/M byte, SIB byte, and displacement, determine the specific values of the operands. If the operands include an immediate value, move forward according to its size.
```

### Examples

- `0x8b, 0x88, 0x00, 0x01, 0x00, 0x00`

```
- breakdown

1. 0x8b - Opcode for mov r32, r/m32.
   This operand type is RM, requiring the ModR/M byte
2. 0x88 - ModR/M byte:
      mod = 10 (memory with 32-bit displacement)
      reg = 001 (ecx)
      r/m = 000 (rax)
3. 0x00, 0x01, 0x00, 0x00 - 32-bit displacement 0x00000100.
```

Thus, the full instruction `0x8b, 0x88, 0x00, 0x01, 0x00, 0x00` translates to `mov ecx, [rax + 0x00000100]`.

- `0x41, 0x01, 0x04, 0x91`

```
1. 0x41 - REX prefix
   W = 0
   R = 0
   X = 0
   B = 1
2. 0x01 - Opcode for add r/m32, r32.
3. 0x04 - ModR/M byte:
      mod = 00 (register indirect addressing, no displacement).
      reg = 000 (eax).
      r/m = 100 (indicates SIB follows).
4. 0x91 - SIB byte:
      scale = 10 (multiplier of 4).
      index = 010 (rdx).
      base = 001 (r9).
```

Thus, the full instruction `0x41, 0x01, 0x04, 0x91` translates to `add [r9 + rdx * 4], eax`.
