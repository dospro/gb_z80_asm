gb-z80asm por instru (dospro@gmail.com)
Web: www.geocities.com/instructivus


-What is gb-z80asm?
gb-z80asm is a Gameboy Original/Pocket/Color assembler to create software fo the Nintendo handheld console.

-How to use:
gb-z80asm recieves an asm file which analizes it and outputs a gameboy rom file.

-Specifications:
Here is a list of every feature and lack of gb-z80asm

The file containing the assembly code may have any extension, in fact, it may not have any. The assembler will analize the file even though it doesn't have any code. The program will recognize all asm code, and reject any other thing.

gb-z80asm treats lowcase and highcase separately. "ROUTINE" is not the same as "routine" for gb-z80asm. 
ALL ASM OPCODES MUST BE WRITTEN IN LOWCASE.

	For this version there are NOT special instructions for:
	-Data storage
	-Program begginning
	-Interrupts
	
The program will start with the first asm line.
For number notation you can use decimal and hexadecimal. For hexadecimal you can use "$" or "0x".

Examples:

and 11 ; Will be and $B
or 0xFF; 
ld bc, $4000

You may use any name you want for jumps, routines, etc.

Examples:(All of the are valid... believe or not)

2342dfgdfnnoi_sub:

call 2342dfgdfnnoi_sub

-.-�+!"�:

jp -.-�+!"�:

sub_routine:

jr sub_routine



You must use ";" for comments

;Esto es un comentario

Comments must not exceed one line.

Use jr only for small jumps (-127 +128). If you need a bigger jump use jp.

gb-z80asm has a good error detection system, but there may be cases that gb-z80asm throws an error message where there isn't one or write a bad opcode. This may be because a bug. Try to identify the context and report it to my mail.


Here is a list of all supported opcodes.

*- Means a byte
()- Reference instead of value
**-Means a word
dir- Means a routine name
n-Means there must be a number between 0 and 7


----------------------
| Opcode | Arguments |
|     adc|        a,a|
|     adc|        a,b|
|     adc|        a,c|
|     adc|        a,d|
|     adc|        a,e|
|     adc|        a,h|
|     adc|        a,l|
|     adc|     a,(hl)|
|     adc|     a,[hl]|
|     adc|        a,*|
|     add|        a,a|
|     add|        a,b|
|     add|        a,c|
|     add|        a,d|
|     add|        a,e|
|     add|        a,h|
|     add|        a,l|
|     add|     a,(hl)|
|     add|     a,[hl]|
|     add|        a,*|
|     add|      hl,bc|
|     add|      hl,de|
|     add|      hl,hl|
|     add|      hl,sp|
|     add|       sp,*|
|     and|          a|
|     and|          b|
|     and|          c|
|     and|          d|
|     and|          e|
|     and|          h|
|     and|          l|
|     and|       (hl)|
|     and|       [hl]|
|     and|          *|
|     bit|        n,a|
|     bit|        n,b|
|     bit|        n,c|
|     bit|        n,d|
|     bit|        n,e|
|     bit|        n,h|
|     bit|        n,l|
|     bit|     n,(hl)|
|     bit|     n,[hl]|
|    call|     nz,dir|
|    call|      z,dir|
|    call|     nc,dir|
|    call|      c,dir|
|    call|        dir|
|     ccf|          -|
|     cpl|          -|
|      cp|          a|
|      cp|          b|
|      cp|          c|
|      cp|          d|
|      cp|          e|
|      cp|          h|
|      cp|          l|
|      cp|       (hl)|
|      cp|       [hl]|
|      cp|          *|
|     daa|          -|
|     dec|          a|
|     dec|          b|
|     dec|          c|
|     dec|          d|
|     dec|          e|
|     dec|          h|
|     dec|          l|
|     dec|       (hl)|
|     dec|       [hl]|
|     dec|         bc|
|     dec|         de|
|     dec|         hl|
|     dec|         sp|
|      di|          -|
|      ei|          -|
|    halt|          -|
|     inc|          a|
|     inc|          b|
|     inc|          c|
|     inc|          d|
|     inc|          e|
|     inc|          h|
|     inc|          l|
|     inc|       (hl)|
|     inc|       [hl]|
|     inc|         bc|
|     inc|         de|
|     inc|         hl|
|     inc|         sp|
|      jp|       (hl)|
|      jp|       [hl]|
|      jp|     nz,dir|
|      jp|      z,dir|
|      jp|     nc,dir|
|      jp|      c,dir|
|      jp|        dir|
|      jr|     nz,dir|
|      jr|      z,dir|
|      jr|     nc,dir|
|      jr|      c,dir|
|      jr|        dir|
|      ld| (ff00+c),a|
|      ld| [ff00+c],a|
|      ld|      (c),a|
|      ld|      [c],a|
|      ld|    (hli),a|
|      ld|    [hli],a|
|      ld|    (hl+),a|
|      ld|    [hl+],a|
|     ldi|     (hl),a|
|     ldi|     [hl],a|
|      ld|    (hld),a|
|      ld|    [hld],a|
|      ld|    (hl-),a|
|      ld|    [hl-],a|
|     ldd|     (hl),a|
|     ldd|     [hl],a|
|      ld|    (**),SP|
|      ld|    [**],SP|
|      ld| a,(ff00+c)|
|      ld| a,[ff00+c]|
|      ld|      a,(c)|
|      ld|      a,[c]|
|      ld|    a,(hli)|
|      ld|    a,[hli]|
|      ld|    a,(hl+)|
|      ld|    a,[hl+]|
|     ldi|     a,(hl)|
|     ldi|     a,[hl]|
|      ld|    a,(hld)|
|      ld|    a,[hld]|
|      ld|    a,(hl-)|
|      ld|    a,[hl-]|
|     ldd|     a,(hl)|
|     ldd|     a,[hl]|
|      ld|        a,a|
|      ld|        a,b|
|      ld|        a,c|
|      ld|        a,d|
|      ld|        a,e|
|      ld|        a,h|
|      ld|        a,l|
|      ld|     a,(bc)|
|      ld|     a,[bc]|
|      ld|     a,(de)|
|      ld|     a,[de]|
|      ld|     a,(hl)|
|      ld|     a,[hl]|
|      ld|     a,(**)|
|      ld|     a,[**]|
|      ld|        a,*|
|      ld| (ff00+*),a|
|      ld| [ff00+*],a|
|     ldh|      (*),a|
|     ldh|      [*],a|
|      ld| a,(ff00+*)|
|      ld| a,[ff00+*]|
|     ldh|      a,(*)|
|     ldh|      a,[*]|
|      ld|        b,a|
|      ld|        c,a|
|      ld|        d,a|
|      ld|        e,a|
|      ld|        h,a|
|      ld|        l,a|
|      ld|     (bc),a|
|      ld|     [bc],a|
|      ld|     (de),a|
|      ld|     [de],a|
|      ld|     (hl),a|
|      ld|     [hl],a|
|      ld|     (**),a|
|      ld|     [**],a|
|      ld|      bc,**|
|      ld|      de,**|
|      ld|      hl,**|
|      ld|      sp,**|
|      ld|        b,*|
|      ld|        c,*|
|      ld|        d,*|
|      ld|        e,*|
|      ld|        h,*|
|      ld|        l,*|
|      ld|        b,b|
|      ld|        b,c|
|      ld|        b,d|
|      ld|        b,e|
|      ld|        b,h|
|      ld|        b,l|
|      ld|     b,(hl)|
|      ld|     b,[hl]|
|      ld|        c,b|
|      ld|        c,c|
|      ld|        c,d|
|      ld|        c,e|
|      ld|        c,h|
|      ld|        c,l|
|      ld|     c,(hl)|
|      ld|     c,[hl]|
|      ld|        d,b|
|      ld|        d,c|
|      ld|        d,d|
|      ld|        d,e|
|      ld|        d,h|
|      ld|        d,l|
|      ld|     d,(hl)|
|      ld|     d,[hl]|
|      ld|        e,b|
|      ld|        e,c|
|      ld|        e,d|
|      ld|        e,e|
|      ld|        e,h|
|      ld|        e,l|
|      ld|     e,(hl)|
|      ld|     e,[hl]|
|      ld|        h,b|
|      ld|        h,c|
|      ld|        h,d|
|      ld|        h,e|
|      ld|        h,h|
|      ld|        h,l|
|      ld|     h,(hl)|
|      ld|     h,[hl]|
|      ld|        l,b|
|      ld|        l,c|
|      ld|        l,d|
|      ld|        l,e|
|      ld|        l,h|
|      ld|        l,l|
|      ld|     l,(hl)|
|      ld|     l,[hl]|
|      ld|     (hl),b|
|      ld|     (hl),c|
|      ld|     (hl),d|
|      ld|     (hl),e|
|      ld|     (hl),h|
|      ld|     (hl),l|
|      ld|     (hl),*|
|      ld|     [hl],b|
|      ld|     [hl],c|
|      ld|     [hl],d|
|      ld|     [hl],e|
|      ld|     [hl],h|
|      ld|     [hl],l|
|      ld|     [hl],*|
|      ld|      sp,hl|
|     nop|          -|
|      or|          a|
|      or|          b|
|      or|          c|
|      or|          d|
|      or|          e|
|      or|          h|
|      or|          l|
|      or|       (hl)|
|      or|       [hl]|
|      or|          *|
|     pop|         af|
|     pop|         bc|
|     pop|         de|
|     pop|         hl|
|    push|         af|
|    push|         bc|
|    push|         de|
|    push|         hl|
|     res|        n,a|
|     res|        n,b|
|     res|        n,c|
|     res|        n,d|
|     res|        n,e|
|     res|        n,h|
|     res|        n,l|
|     res|     n,(hl)|
|     res|     n,[hl]|
|     ret|          -|
|     ret|         nz|
|     ret|          z|
|     ret|         nc|
|     ret|          c|
|    reti|          -|
|     rla|          -|
|    rlca|          -|
|     rlc|          a|
|     rlc|          b|
|     rlc|          c|
|     rlc|          d|
|     rlc|          e|
|     rlc|          h|
|     rlc|          l|
|     rlc|       (hl)|
|     rlc|       [hl]|
|      rl|          a|
|      rl|          b|
|      rl|          c|
|      rl|          d|
|      rl|          e|
|      rl|          h|
|      rl|          l|
|      rl|       (hl)|
|      rl|       [hl]|
|     rra|          -|
|    rrca|          -|
|     rrc|          a|
|     rrc|          b|
|     rrc|          c|
|     rrc|          d|
|     rrc|          e|
|     rrc|          h|
|     rrc|          l|
|     rrc|       (hl)|
|     rrc|       [hl]|
|      rr|          a|
|      rr|          b|
|      rr|          c|
|      rr|          d|
|      rr|          e|
|      rr|          h|
|      rr|          l|
|      rr|       (hl)|
|      rr|       [hl]|
|     rst|         00|
|     rst|         08|
|     rst|         10|
|     rst|         18|
|     rst|         20|
|     rst|         28|
|     rst|         30|
|     rst|         38|
|     sbc|        a,a|
|     sbc|        a,b|
|     sbc|        a,c|
|     sbc|        a,d|
|     sbc|        a,e|
|     sbc|        a,h|
|     sbc|        a,l|
|     sbc|     a,(hl)|
|     sbc|     a,[hl]|
|     sbc|        a,*|
|     scf|          -|
|     set|        n,a|
|     set|        n,b|
|     set|        n,c|
|     set|        n,d|
|     set|        n,e|
|     set|        n,h|
|     set|        n,l|
|     set|     n,(hl)|
|     set|     n,[hl]|
|     sla|          a|
|     sla|          b|
|     sla|          c|
|     sla|          d|
|     sla|          e|
|     sla|          h|
|     sla|          l|
|     sla|       (hl)|
|     sla|       [hl]|
|     sra|          a|
|     sra|          b|
|     sra|          c|
|     sra|          d|
|     sra|          e|
|     sra|          h|
|     sra|          l|
|     sra|       (hl)|
|     sra|       [hl]|
|     srl|          a|
|     srl|          b|
|     srl|          c|
|     srl|          d|
|     srl|          e|
|     srl|          h|
|     srl|          l|
|     srl|       (hl)|
|     srl|       [hl]|
|    stop|          -|
|     sub|          a|
|     sub|          b|
|     sub|          c|
|     sub|          d|
|     sub|          e|
|     sub|          h|
|     sub|          l|
|     sub|       (hl)|
|     sub|       [hl]|
|     sub|          *|
|    swap|          a|
|    swap|          b|
|    swap|          c|
|    swap|          d|
|    swap|          e|
|    swap|          h|
|    swap|          l|
|    swap|       (hl)|
|    swap|       [hl]|
|     xor|          a|
|     xor|          b|
|     xor|          c|
|     xor|          d|
|     xor|          e|
|     xor|          h|
|     xor|          l|
|     xor|       (hl)|
|     xor|       [hl]|
|     xor|          *|
|      ld|    hl,sp+*|
|    ldhl|       sp,*|
----------------------

