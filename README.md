# MyDisassembler

## Install

```bash
./script/build.sh
```

## Usage

```bash
> ./build/script/mydisas example/jmp.o

section: .text ----

40 <_start>:
 40: mov  eax 0x00000000                      ( b8 0 0 0 0 )
 45: cmp  eax 0x00                            ( 83 f8 0 )
 48: jz 4e <zero_label> ; relative offset = 4 ( 74 4 )
 4a: jmp 52 <end_label> ; relative offset = 6 ( eb 6 )
 4c: jmp 40 <_start> ; relative offset = -14  ( eb f2 )

4e <zero_label>:
 4e: push  esp                                ( 54 )
 4f: xor  eax eax                             ( 31 c0 )
 51: ret                                      ( c3 )

52 <end_label>:
 52: push  edi                                ( 57 )
 53: xor  ecx ecx                             ( 31 c9 )
 55: ret                                      ( c3 )
-------------------
Done!
```


## Features


