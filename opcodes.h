/*
 *     Proyect: gbz80Asm
 *    Filename: opcodes.h
 *     Version: v2.0
 * Description: Gameboy Z80 Asembler
 *     License: GPLv2
 *
 *      Author: Copyright (C) Rubén Daniel Gutiérrez Cruz <dospro@gmail.com>
 *        Date: 14-8-2007
 *
 *
 *	This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version. 
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details. 
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

typedef struct _opcode{
	char op_name[6];
	char op_args[12];
	int op_val, op_val2;
}opcode;

typedef struct _INCLUDES{
	char file[16];
	int id;
}INCLUDES;

typedef struct _ROUTINES{
	char r_name[64];//Routine name(ej. sub_process:
	int r_address;//where the routine will be written
	int r_c_address;//where the routine is called(to use jr instead
	int r_end_address;//This will contain the addres where the ret is found
	struct _ROUTINES *next;
}ROUTINE;

typedef struct CALL_JP{
	char j_name[64];//The name of this call
	int j_address;//Where is the call
	int j_value;//This will have a 0 as generic before getting the right value
	int bits;//8 if its a byte, 16 if its a word(2 bytes addres)
	struct CALL_JP *next;//Next node
}JUMPS;


opcode opcode_table[]={
	{"adc", "a,a", 0x8F, 0},/*Add with carry*/
	{"adc", "a,b", 0x88, 0},
	{"adc", "a,c", 0x89, 0},
	{"adc", "a,d", 0x8a, 0},
	{"adc", "a,e", 0x8b, 0},
	{"adc", "a,h", 0x8c, 0},
	{"adc", "a,l", 0x8d, 0},
	{"adc", "a,(hl)", 0x8e, 0},
	{"adc", "a,[hl]", 0x8e, 0},
	{"adc", "a,*", 0xce},

	{"add", "a,a", 0x87},/*Addition*/
	{"add", "a,b", 0x80},
	{"add", "a,c", 0x81},
	{"add", "a,d", 0x82},
	{"add", "a,e", 0x83},
	{"add", "a,h", 0x84},
	{"add", "a,l", 0x85},
	{"add", "a,(hl)", 0x86},
	{"add", "a,[hl]", 0x86},
	{"add", "a,*", 0xC6},

	{"add", "hl,bc", 0x09},
	{"add", "hl,de", 0x19},
	{"add", "hl,hl", 0x29},
	{"add", "hl,sp", 0x39},
	{"add", "sp,*",  0xE8},

	{"and", "a", 0xa7},
	{"and", "b", 0xa0},
	{"and", "c", 0xa1},
	{"and", "d", 0xa2},
	{"and", "e", 0xa3},
	{"and", "h", 0xa4},
	{"and", "l", 0xa5},
	{"and", "(hl)", 0xa6},
	{"and", "[hl]", 0xa6},
	{"and", "*", 0xe6},

	{"bit", "n,a", 0xCB, 0x47},
	{"bit", "n,b", 0xCB, 0x40},
	{"bit", "n,c", 0xCB, 0x41},
	{"bit", "n,d", 0xCB, 0x42},
	{"bit", "n,e", 0xCB, 0x43},
	{"bit", "n,h", 0xCB, 0x44},
	{"bit", "n,l", 0xCB, 0x45},
	{"bit", "n,(hl)", 0xCB, 0x46},
	{"bit", "n,[hl]", 0xCB, 0x46},

	{"call", "nz,dir", 0xC4},
	{"call", "z,dir",  0xCC},
	{"call", "nc,dir", 0xD4},
	{"call", "c,dir",  0xDC},
	{"call", "dir",    0xC4},

	{"ccf", "-", 0x3f},
	{"cpl", "-", 0x2F},
	
	{"cp", "a", 0xBF},
	{"cp", "b", 0xB8},
	{"cp", "c", 0xB9},
	{"cp", "d", 0xBa},
	{"cp", "e", 0xBb},
	{"cp", "h", 0xBc},
	{"cp", "l", 0xBd},
	{"cp", "(hl)", 0xBe},
	{"cp", "[hl]", 0xBe},//The same as before
	{"cp", "*", 0xFe},

	{"daa", "-", 0x27},

	{"dec", "a", 0x3D},
	{"dec", "b", 0x05},
	{"dec", "c", 0x0D},
	{"dec", "d", 0x15},
	{"dec", "e", 0x1D},
	{"dec", "h", 0x25},
	{"dec", "l", 0x2D},
	{"dec", "(hl)", 0x35},
	{"dec", "[hl]", 0x35},//the same as before
	{"dec", "bc", 0x0b},
	{"dec", "de", 0x1b},
	{"dec", "hl", 0x2b},
	{"dec", "sp", 0x3b},

	{"di", "-", 0xF3},
	{"ei", "-", 0xfb},
	{"halt", "-", 0x76},

	{"inc", "a", 0x3C},
	{"inc", "b", 0x04},
	{"inc", "c", 0x0c},
	{"inc", "d", 0x14},
	{"inc", "e", 0x1C},
	{"inc", "h", 0x24},
	{"inc", "l", 0x2C},
	{"inc", "(hl)", 0x34},
	{"inc", "[hl]", 0x34},
	{"inc", "bc", 0x03},
	{"inc", "de", 0x13},
	{"inc", "hl", 0x23},
	{"inc", "sp", 0x33},

	{"jp", "(hl)",   0xe9},
	{"jp", "[hl]",   0xe9},
	{"jp", "nz,dir", 0xc2},
	{"jp", "z,dir",  0xca},
	{"jp", "nc,dir", 0xd2},
	{"jp", "c,dir",  0xda},
	{"jp", "dir",    0xC3},

	{"jr", "nz,dir", 0x20},
	{"jr", "z,dir",  0x28},
	{"jr", "nc,dir", 0x30},
	{"jr", "c,dir",  0x38},
	{"jr", "dir",    0x18},

	/*All this are the different ways of writing the opcode parameters*/
	/*The next opcodes mean the same*/
	{"ld", "(ff00+c),a", 0xE2},
	{"ld", "[ff00+c],a", 0xE2},
	{"ld", "(c),a", 0xE2},
	{"ld", "[c],a", 0xE2},
	/*Finish the opcodes that means the same*/

	/*All this is the same opcode*/
	{"ld", "(hli),a", 0x22},
	{"ld", "[hli],a", 0x22},
	{"ld", "(hl+),a", 0x22},
	{"ld", "[hl+],a", 0x22},
	{"ldi", "(hl),a", 0x22},
	{"ldi", "[hl],a", 0x22},
	/*Ends*/

	/*All this are the same*/
	{"ld", "(hld),a", 0x32},
	{"ld", "[hld],a", 0x32},
	{"ld", "(hl-),a", 0x32},
	{"ld", "[hl-],a", 0x32},
	{"ldd", "(hl),a", 0x32},
	{"ldd", "[hl],a", 0x32},
	/*ends*/

	{"ld", "(**),SP", 0x08},
	{"ld", "[**],SP", 0x08},//the same as before

	/*The same*/
	{"ld", "a,(ff00+c)", 0xF2},
	{"ld", "a,[ff00+c]", 0xF2},
	{"ld", "a,(c)", 0xF2},
	{"ld", "a,[c]", 0xF2},
	/*ends*/

	/*All this is the same opcode*/
	{"ld", "a,(hli)", 0x2A},
	{"ld", "a,[hli]", 0x2a},
	{"ld", "a,(hl+)", 0x2a},
	{"ld", "a,[hl+]", 0x2a},
	{"ldi", "a,(hl)", 0x2a},
	{"ldi", "a,[hl]", 0x2a},
	/*Ends*/

	/*All this are the same*/
	{"ld", "a,(hld)", 0x3a},
	{"ld", "a,[hld]", 0x3a},
	{"ld", "a,(hl-)", 0x3a},
	{"ld", "a,[hl-]", 0x3a},
	{"ldd", "a,(hl)", 0x3a},
	{"ldd", "a,[hl]", 0x3a},
	/*ends*/

	{"ld", "a,a", 0x7f},
	{"ld", "a,b", 0x78},
	{"ld", "a,c", 0x79},
	{"ld", "a,d", 0x7a},
	{"ld", "a,e", 0x7b},
	{"ld", "a,h", 0x7c},
	{"ld", "a,l", 0x7d},
	{"ld", "a,(bc)", 0x0a},
	{"ld", "a,[bc]", 0x0a},////
	{"ld", "a,(de)", 0x1a},
	{"ld", "a,[de]", 0x1a},///
	{"ld", "a,(hl)", 0x7e},
	{"ld", "a,[hl]", 0x7e},///
	{"ld", "a,(**)", 0xfa},
	{"ld", "a,[**]", 0xfa},///
	{"ld", "a,*", 0x3e},

	/*the same*/
	{"ld", "(ff00+*),a", 0xe0},
	{"ld", "[ff00+*],a", 0xe0},
	{"ldh", "(*),a", 0xe0},
	{"ldh", "[*],a", 0xe0},
	/*ends*/

	/*the same*/
	{"ld", "a,(ff00+*)", 0xf0},
	{"ld", "a,[ff00+*]", 0xf0},
	{"ldh", "a,(*)", 0xf0},
	{"ldh", "a,[*]", 0xf0},
	/*ends*/

	{"ld", "b,a", 0x47},
	{"ld", "c,a", 0x4f},
	{"ld", "d,a", 0x57},
	{"ld", "e,a", 0x5f},
	{"ld", "h,a", 0x67},
	{"ld", "l,a", 0x6f},
	{"ld", "(bc),a", 0x02},
	{"ld", "[bc],a", 0x02},
	{"ld", "(de),a", 0x12},
	{"ld", "[de],a", 0x12},
	{"ld", "(hl),a", 0x77},
	{"ld", "[hl],a", 0x77},
	{"ld", "(**),a", 0xea},
	{"ld", "[**],a", 0xea},

	{"ld", "bc,**", 0x01},
	{"ld", "de,**", 0x11},
	{"ld", "hl,**", 0x21},
	{"ld", "sp,**", 0x31},

	{"ld", "b,*", 0x06},
	{"ld", "c,*", 0x0e},
	{"ld", "d,*", 0x16},
	{"ld", "e,*", 0x1e},
	{"ld", "h,*", 0x26},
	{"ld", "l,*", 0x2e},

	{"ld", "b,b", 0x40},
	{"ld", "b,c", 0x41},
	{"ld", "b,d", 0x42},
	{"ld", "b,e", 0x43},
	{"ld", "b,h", 0x44},
	{"ld", "b,l", 0x45},
	{"ld", "b,(hl)", 0x46},
	{"ld", "b,[hl]", 0x46},
	{"ld", "c,b", 0x48},
	{"ld", "c,c", 0x49},
	{"ld", "c,d", 0x4a},
	{"ld", "c,e", 0x4b},
	{"ld", "c,h", 0x4c},
	{"ld", "c,l", 0x4d},
	{"ld", "c,(hl)", 0x4e},
	{"ld", "c,[hl]", 0x4e},
	{"ld", "d,b", 0x50},
	{"ld", "d,c", 0x51},
	{"ld", "d,d", 0x52},
	{"ld", "d,e", 0x53},
	{"ld", "d,h", 0x54},
	{"ld", "d,l", 0x55},
	{"ld", "d,(hl)", 0x56},
	{"ld", "d,[hl]", 0x56},
	{"ld", "e,b", 0x58},
	{"ld", "e,c", 0x59},
	{"ld", "e,d", 0x5a},
	{"ld", "e,e", 0x5b},
	{"ld", "e,h", 0x5c},
	{"ld", "e,l", 0x5d},
	{"ld", "e,(hl)", 0x5e},
	{"ld", "e,[hl]", 0x5e},
	{"ld", "h,b", 0x60},
	{"ld", "h,c", 0x61},
	{"ld", "h,d", 0x62},
	{"ld", "h,e", 0x63},
	{"ld", "h,h", 0x64},
	{"ld", "h,l", 0x65},
	{"ld", "h,(hl)", 0x66},
	{"ld", "h,[hl]", 0x66},
	{"ld", "l,b", 0x68},
	{"ld", "l,c", 0x69},
	{"ld", "l,d", 0x6a},
	{"ld", "l,e", 0x6b},
	{"ld", "l,h", 0x6c},
	{"ld", "l,l", 0x6d},
	{"ld", "l,(hl)", 0x6e},
	{"ld", "l,[hl]", 0x6e},
	{"ld", "(hl),b", 0x70},
	{"ld", "(hl),c", 0x71},
	{"ld", "(hl),d", 0x72},
	{"ld", "(hl),e", 0x73},
	{"ld", "(hl),h", 0x74},
	{"ld", "(hl),l", 0x75},
	{"ld", "(hl),*", 0x36},
	{"ld", "[hl],b", 0x70},
	{"ld", "[hl],c", 0x71},
	{"ld", "[hl],d", 0x72},
	{"ld", "[hl],e", 0x73},
	{"ld", "[hl],h", 0x74},
	{"ld", "[hl],l", 0x75},
	{"ld", "[hl],*", 0x36},

	{"ld", "sp,hl", 0xf9},

	{"nop", "-", 0x0},

	{"or", "a", 0xB7},
	{"or", "b", 0xB0},
	{"or", "c", 0xB1},
	{"or", "d", 0xB2},
	{"or", "e", 0xB3},
	{"or", "h", 0xB4},
	{"or", "l", 0xB5},
	{"or", "(hl)", 0xB6},
	{"or", "[hl]", 0xB6},
	{"or", "*", 0xF6},

	{"pop", "af", 0xF1},
	{"pop", "bc", 0xC1},
	{"pop", "de", 0xD1},
	{"pop", "hl", 0xE1},

	{"push", "af", 0xF5},
	{"push", "bc", 0xC5},
	{"push", "de", 0xD5},
	{"push", "hl", 0xE5},

	{"res", "n,a", 0xCB, 0x87},
	{"res", "n,b", 0xCB, 0x80},
	{"res", "n,c", 0xCB, 0x81},
	{"res", "n,d", 0xCB, 0x82},
	{"res", "n,e", 0xCB, 0x83},
	{"res", "n,h", 0xCB, 0x84},
	{"res", "n,l", 0xCB, 0x85},
	{"res", "n,(hl)", 0xCB, 0x86},
	{"res", "n,[hl]", 0xCB, 0x86},

	{"ret", "-", 0xC9},
	{"ret", "nz", 0xC0},
	{"ret", "z", 0xC8},
	{"ret", "nc", 0xD0},
	{"ret", "c", 0xD8},
	{"reti", "-", 0xD9},

	{"rla", "-", 0x17},
	{"rlca", "-", 0x07},
	{"rlc", "a", 0xCB, 0x07},
	{"rlc", "b", 0xCB, 0x00},
	{"rlc", "c", 0xCB, 0x01},
	{"rlc", "d", 0xCB, 0x02},
	{"rlc", "e", 0xCB, 0x03},
	{"rlc", "h", 0xCB, 0x04},
	{"rlc", "l", 0xCB, 0x05},
	{"rlc", "(hl)", 0xCB, 0x06},
	{"rlc", "[hl]", 0xCB, 0x06},

	{"rl", "a", 0xCB, 0x17},
	{"rl", "b", 0xCB, 0x10},
	{"rl", "c", 0xCB, 0x11},
	{"rl", "d", 0xCB, 0x12},
	{"rl", "e", 0xCB, 0x13},
	{"rl", "h", 0xCB, 0x14},
	{"rl", "l", 0xCB, 0x15},
	{"rl", "(hl)", 0xCB, 0x16},
	{"rl", "[hl]", 0xCB, 0x16},

	{"rra", "-", 0x1F},
	{"rrca", "-", 0x0F},

	{"rrc", "a", 0xCB, 0x0F},
	{"rrc", "b", 0xCB, 0x08},
	{"rrc", "c", 0xCB, 0x09},
	{"rrc", "d", 0xCB, 0x0A},
	{"rrc", "e", 0xCB, 0x0B},
	{"rrc", "h", 0xCB, 0x0C},
	{"rrc", "l", 0xCB, 0x0D},
	{"rrc", "(hl)", 0xCB, 0x0E},
	{"rrc", "[hl]", 0xCB, 0x0E},

	{"rr", "a", 0xCB, 0x1F},
	{"rr", "b", 0xCB, 0x18},
	{"rr", "c", 0xCB, 0x19},
	{"rr", "d", 0xCB, 0x1A},
	{"rr", "e", 0xCB, 0x1B},
	{"rr", "h", 0xCB, 0x1C},
	{"rr", "l", 0xCB, 0x1D},
	{"rr", "(hl)", 0xCB, 0x1E},
	{"rr", "[hl]", 0xCB, 0x1E},

	{"rst", "00", 0xC7},
	{"rst", "08", 0xCF},
	{"rst", "10", 0xD7},
	{"rst", "18", 0xDF},
	{"rst", "20", 0xE7},
	{"rst", "28", 0xEF},
	{"rst", "30", 0xF7},
	{"rst", "38", 0xFF},

	{"sbc", "a,a", 0x9F},
	{"sbc", "a,b", 0x98},
	{"sbc", "a,c", 0x99},
	{"sbc", "a,d", 0x9A},
	{"sbc", "a,e", 0x9B},
	{"sbc", "a,h", 0x9C},
	{"sbc", "a,l", 0x9D},
	{"sbc", "a,(hl)", 0x9E},
	{"sbc", "a,[hl]", 0x9E},
	{"sbc", "a,*", 0xDE},

	{"scf", "-", 0x37},

	{"set", "n,a", 0xCB, 0xC7},
	{"set", "n,b", 0xCB, 0xC0},
	{"set", "n,c", 0xCB, 0xC1},
	{"set", "n,d", 0xCB, 0xC2},
	{"set", "n,e", 0xCB, 0xC3},
	{"set", "n,h", 0xCB, 0xC4},
	{"set", "n,l", 0xCB, 0xC5},
	{"set", "n,(hl)", 0xCB, 0xC6},
	{"set", "n,[hl]", 0xCB, 0xC6},

	{"sla", "a", 0xCB, 0x27},
	{"sla", "b", 0xCB, 0x20},
	{"sla", "c", 0xCB, 0x21},
	{"sla", "d", 0xCB, 0x22},
	{"sla", "e", 0xCB, 0x23},
	{"sla", "h", 0xCB, 0x24},
	{"sla", "l", 0xCB, 0x25},
	{"sla", "(hl)", 0xCB, 0x26},
	{"sla", "[hl]", 0xCB, 0x26},

	{"sra", "a", 0xCB, 0x2F},
	{"sra", "b", 0xCB, 0x28},
	{"sra", "c", 0xCB, 0x29},
	{"sra", "d", 0xCB, 0x2A},
	{"sra", "e", 0xCB, 0x2B},
	{"sra", "h", 0xCB, 0x2C},
	{"sra", "l", 0xCB, 0x2D},
	{"sra", "(hl)", 0xCB, 0x2E},
	{"sra", "[hl]", 0xCB, 0x2E},

	{"srl", "a", 0xCB, 0x3F},
	{"srl", "b", 0xCB, 0x38},
	{"srl", "c", 0xCB, 0x39},
	{"srl", "d", 0xCB, 0x3A},
	{"srl", "e", 0xCB, 0x3B},
	{"srl", "h", 0xCB, 0x3C},
	{"srl", "l", 0xCB, 0x3D},
	{"srl", "(hl)", 0xCB, 0x3E},
	{"srl", "[hl]", 0xCB, 0x3E},

	{"stop", "-", 0x10, 00},

	{"sub", "a", 0x97},
	{"sub", "b", 0x90},
	{"sub", "c", 0x91},
	{"sub", "d", 0x92},
	{"sub", "e", 0x93},
	{"sub", "h", 0x94},
	{"sub", "l", 0x95},
	{"sub", "(hl)", 0x96},
	{"sub", "[hl]", 0x96},
	{"sub", "*", 0xD6},

	{"swap", "a", 0xCB, 0x37},
	{"swap", "b", 0xCB, 0x30},
	{"swap", "c", 0xCB, 0x31},
	{"swap", "d", 0xCB, 0x32},
	{"swap", "e", 0xCB, 0x33},
	{"swap", "h", 0xCB, 0x34},
	{"swap", "l", 0xCB, 0x35},
	{"swap", "(hl)", 0xCB, 0x36},
	{"swap", "[hl]", 0xCB, 0x36},

	{"xor", "a", 0xAF},
	{"xor", "b", 0xA8},
	{"xor", "c", 0xA9},
	{"xor", "d", 0xAA},
	{"xor", "e", 0xAB},
	{"xor", "h", 0xAC},
	{"xor", "l", 0xAD},
	{"xor", "(hl)", 0xAE},
	{"xor", "[hl]", 0xAE},
	{"xor", "*", 0xEE},

	{"ld", "hl,sp+*", 0xF8},
	{"ldhl", "sp,*", 0xF8},
};


ROUTINE *r_list=NULL;
JUMPS *j_list=NULL;
