# gb_z80_asm

## What is gb_z80_asm?

gb_z80_asm is a Game Boy Original/Pocket/Color assembler to create software for the Nintendo handheld console entirely written in C11.


## Features
* Supports all Game Boy's custom opcodes.
* You can use different variations of the same opcode. For example `ld hli,a` and `ldi hl,a` translate to the same opcode.
* You can use parenthesis or square brackets for addresses.
* Supports normal assembly comments `add a ;comment`
* Supports named routines and jumps
* Also supports named relative jumps through the instruction `jr`. Just make sure you won't overpass the jump limit.
* Customize entry point using `.main` meta instruction.



## Future improvements
Although you can use gb_z80_asm to code large Game Boy projects, the current version lacks some capabilities. 
This is like a wishlist, some things might get implemented some might not.
* Customize rom header. This includes cartridge type, memory controllers, etc.
* Specify named raw data (this is commonly done with the `db` pseudo instruction).
* Assemble multiple sources into a single rom file.
* Specify arbitrary addresses to put code.
* Specify rst routines

## How to use
gb_z80_asm is very simple. You just need to specify your asm file:
```shell
$> gb_z80_asm source_file.asm
```

## Compiling
You need the following tools to compile gb_z80_asm
* A C compiler that supports C11. gcc, clang, lc(visual studio) work fine.
* XCode for MacOS only
* make
* cmake

For Linux, MacOS or cygwin like environments you can simply do:
```shell
$ cmake .
$ make
```

If you want to use Microsoft's Visual Studio, you just need to create a console application project, add all source files
and build. CMake also has project generators for VS but I haven't tried them yet.

### Coding specifications
* If you don't specify a `.main` entry point, it will default to the first instruction on the source file
* You can use either `$` or `0x` as prefixes for hex numbers
    ```asm
    and 11 ; Will be and $B
    or 0xFF; 
    ld bc, $4000
    ```
* Labels or routine names must be on separate line
    ```asm
  my_routine:
  ld a,b
  sub a
  ...
  
  bad_label: ld b,b ; This won't work
    ```
* Label names can have pretty much any ascii character(except `:`), although I encourage you to use pretty names. 
* Comments can go any line and they start with `;`. Once you open a comment, the rest of the line becomes a comment
* Use `jr` only for small jumps (-127 +128). If you need a bigger jump use jp.

## Notes about the included files
Besides the source of the assembler, I also included a List.txt file which is pretty much the loop up table used by the
assembler to generate each opcode.

There is also an Examples folder that includes a test.asm file so you can try it and see for yourself how it works.
