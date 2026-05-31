/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

// SM83 non-CB opcode dispatcher. Each case adds exact T-cycles per
// Pan Docs. Cycle counts for conditional branches vary based on whether
// the condition is taken.
//
// Rows are grouped 16 opcodes at a time to match the standard DMG
// reference table. Undefined opcodes ($D3 $DB $DD $E3 $E4 $EB $EC $ED
// $F4 $FC $FD) lock up the real CPU; we stop the CPU and tick an
// illegal-op counter so upper layers can flag it.

#include "gb_ops.h"

namespace SGB {

void Dispatch(CpuState &s, Memory &mem, uint8_t op)
{
	switch (op)
	{
	// ===== 0x00-0x0F =====
	case 0x00: s.t_cycles += 4; break;                                                         // NOP
	case 0x01: s.r.bc = Fetch16(s, mem); s.t_cycles += 12; break;                              // LD BC,nn
	case 0x02: MemWrite(mem, s.r.bc, s.r.a); s.t_cycles += 8; break;                           // LD (BC),A
	case 0x03: s.r.bc++; s.t_cycles += 8; break;                                               // INC BC
	case 0x04: s.r.b = AluInc(s, s.r.b); s.t_cycles += 4; break;                               // INC B
	case 0x05: s.r.b = AluDec(s, s.r.b); s.t_cycles += 4; break;                               // DEC B
	case 0x06: s.r.b = Fetch8(s, mem); s.t_cycles += 8; break;                                 // LD B,n
	case 0x07: AluRlca(s); s.t_cycles += 4; break;                                             // RLCA
	case 0x08: { uint16_t a = Fetch16(s, mem); MemWrite16(mem, a, s.r.sp); s.t_cycles += 20; } break;  // LD (nn),SP
	case 0x09: s.r.hl = AluAdd16(s, s.r.hl, s.r.bc); s.t_cycles += 8; break;                   // ADD HL,BC
	case 0x0A: s.r.a = MemRead(mem, s.r.bc); s.t_cycles += 8; break;                           // LD A,(BC)
	case 0x0B: s.r.bc--; s.t_cycles += 8; break;                                               // DEC BC
	case 0x0C: s.r.c = AluInc(s, s.r.c); s.t_cycles += 4; break;                               // INC C
	case 0x0D: s.r.c = AluDec(s, s.r.c); s.t_cycles += 4; break;                               // DEC C
	case 0x0E: s.r.c = Fetch8(s, mem); s.t_cycles += 8; break;                                 // LD C,n
	case 0x0F: AluRrca(s); s.t_cycles += 4; break;                                             // RRCA

	// ===== 0x10-0x1F =====
	case 0x10: Fetch8(s, mem); s.stopped = true; s.t_cycles += 4; break;                       // STOP (consumes filler byte)
	case 0x11: s.r.de = Fetch16(s, mem); s.t_cycles += 12; break;                              // LD DE,nn
	case 0x12: MemWrite(mem, s.r.de, s.r.a); s.t_cycles += 8; break;                           // LD (DE),A
	case 0x13: s.r.de++; s.t_cycles += 8; break;                                               // INC DE
	case 0x14: s.r.d = AluInc(s, s.r.d); s.t_cycles += 4; break;                               // INC D
	case 0x15: s.r.d = AluDec(s, s.r.d); s.t_cycles += 4; break;                               // DEC D
	case 0x16: s.r.d = Fetch8(s, mem); s.t_cycles += 8; break;                                 // LD D,n
	case 0x17: AluRla(s); s.t_cycles += 4; break;                                              // RLA
	case 0x18: { int8_t e = static_cast<int8_t>(Fetch8(s, mem)); s.r.pc = static_cast<uint16_t>(s.r.pc + e); s.t_cycles += 12; } break;  // JR e
	case 0x19: s.r.hl = AluAdd16(s, s.r.hl, s.r.de); s.t_cycles += 8; break;                   // ADD HL,DE
	case 0x1A: s.r.a = MemRead(mem, s.r.de); s.t_cycles += 8; break;                           // LD A,(DE)
	case 0x1B: s.r.de--; s.t_cycles += 8; break;                                               // DEC DE
	case 0x1C: s.r.e = AluInc(s, s.r.e); s.t_cycles += 4; break;                               // INC E
	case 0x1D: s.r.e = AluDec(s, s.r.e); s.t_cycles += 4; break;                               // DEC E
	case 0x1E: s.r.e = Fetch8(s, mem); s.t_cycles += 8; break;                                 // LD E,n
	case 0x1F: AluRra(s); s.t_cycles += 4; break;                                              // RRA

	// ===== 0x20-0x2F =====
	case 0x20: { int8_t e = static_cast<int8_t>(Fetch8(s, mem));
	             if (!FlagZ(s)) { s.r.pc = static_cast<uint16_t>(s.r.pc + e); s.t_cycles += 12; }
	             else            { s.t_cycles += 8; } } break;                                 // JR NZ,e
	case 0x21: s.r.hl = Fetch16(s, mem); s.t_cycles += 12; break;                              // LD HL,nn
	case 0x22: MemWrite(mem, s.r.hl, s.r.a); s.r.hl++; s.t_cycles += 8; break;                 // LD (HL+),A
	case 0x23: s.r.hl++; s.t_cycles += 8; break;                                               // INC HL
	case 0x24: s.r.h = AluInc(s, s.r.h); s.t_cycles += 4; break;                               // INC H
	case 0x25: s.r.h = AluDec(s, s.r.h); s.t_cycles += 4; break;                               // DEC H
	case 0x26: s.r.h = Fetch8(s, mem); s.t_cycles += 8; break;                                 // LD H,n
	case 0x27: AluDaa(s); s.t_cycles += 4; break;                                              // DAA
	case 0x28: { int8_t e = static_cast<int8_t>(Fetch8(s, mem));
	             if (FlagZ(s))  { s.r.pc = static_cast<uint16_t>(s.r.pc + e); s.t_cycles += 12; }
	             else            { s.t_cycles += 8; } } break;                                 // JR Z,e
	case 0x29: s.r.hl = AluAdd16(s, s.r.hl, s.r.hl); s.t_cycles += 8; break;                   // ADD HL,HL
	case 0x2A: s.r.a = MemRead(mem, s.r.hl); s.r.hl++; s.t_cycles += 8; break;                 // LD A,(HL+)
	case 0x2B: s.r.hl--; s.t_cycles += 8; break;                                               // DEC HL
	case 0x2C: s.r.l = AluInc(s, s.r.l); s.t_cycles += 4; break;                               // INC L
	case 0x2D: s.r.l = AluDec(s, s.r.l); s.t_cycles += 4; break;                               // DEC L
	case 0x2E: s.r.l = Fetch8(s, mem); s.t_cycles += 8; break;                                 // LD L,n
	case 0x2F: AluCpl(s); s.t_cycles += 4; break;                                              // CPL

	// ===== 0x30-0x3F =====
	case 0x30: { int8_t e = static_cast<int8_t>(Fetch8(s, mem));
	             if (!FlagC(s)) { s.r.pc = static_cast<uint16_t>(s.r.pc + e); s.t_cycles += 12; }
	             else            { s.t_cycles += 8; } } break;                                 // JR NC,e
	case 0x31: s.r.sp = Fetch16(s, mem); s.t_cycles += 12; break;                              // LD SP,nn
	case 0x32: MemWrite(mem, s.r.hl, s.r.a); s.r.hl--; s.t_cycles += 8; break;                 // LD (HL-),A
	case 0x33: s.r.sp++; s.t_cycles += 8; break;                                               // INC SP
	case 0x34: { uint8_t v = MemRead(mem, s.r.hl); MemWrite(mem, s.r.hl, AluInc(s, v)); s.t_cycles += 12; } break;  // INC (HL)
	case 0x35: { uint8_t v = MemRead(mem, s.r.hl); MemWrite(mem, s.r.hl, AluDec(s, v)); s.t_cycles += 12; } break;  // DEC (HL)
	case 0x36: MemWrite(mem, s.r.hl, Fetch8(s, mem)); s.t_cycles += 12; break;                 // LD (HL),n
	case 0x37: AluScf(s); s.t_cycles += 4; break;                                              // SCF
	case 0x38: { int8_t e = static_cast<int8_t>(Fetch8(s, mem));
	             if (FlagC(s))  { s.r.pc = static_cast<uint16_t>(s.r.pc + e); s.t_cycles += 12; }
	             else            { s.t_cycles += 8; } } break;                                 // JR C,e
	case 0x39: s.r.hl = AluAdd16(s, s.r.hl, s.r.sp); s.t_cycles += 8; break;                   // ADD HL,SP
	case 0x3A: s.r.a = MemRead(mem, s.r.hl); s.r.hl--; s.t_cycles += 8; break;                 // LD A,(HL-)
	case 0x3B: s.r.sp--; s.t_cycles += 8; break;                                               // DEC SP
	case 0x3C: s.r.a = AluInc(s, s.r.a); s.t_cycles += 4; break;                               // INC A
	case 0x3D: s.r.a = AluDec(s, s.r.a); s.t_cycles += 4; break;                               // DEC A
	case 0x3E: s.r.a = Fetch8(s, mem); s.t_cycles += 8; break;                                 // LD A,n
	case 0x3F: AluCcf(s); s.t_cycles += 4; break;                                              // CCF

	// ===== 0x40-0x4F : LD B,r / LD C,r =====
	case 0x40: s.t_cycles += 4; break;                                                         // LD B,B (no-op)
	case 0x41: s.r.b = s.r.c; s.t_cycles += 4; break;                                          // LD B,C
	case 0x42: s.r.b = s.r.d; s.t_cycles += 4; break;                                          // LD B,D
	case 0x43: s.r.b = s.r.e; s.t_cycles += 4; break;                                          // LD B,E
	case 0x44: s.r.b = s.r.h; s.t_cycles += 4; break;                                          // LD B,H
	case 0x45: s.r.b = s.r.l; s.t_cycles += 4; break;                                          // LD B,L
	case 0x46: s.r.b = MemRead(mem, s.r.hl); s.t_cycles += 8; break;                           // LD B,(HL)
	case 0x47: s.r.b = s.r.a; s.t_cycles += 4; break;                                          // LD B,A
	case 0x48: s.r.c = s.r.b; s.t_cycles += 4; break;                                          // LD C,B
	case 0x49: s.t_cycles += 4; break;                                                         // LD C,C
	case 0x4A: s.r.c = s.r.d; s.t_cycles += 4; break;                                          // LD C,D
	case 0x4B: s.r.c = s.r.e; s.t_cycles += 4; break;                                          // LD C,E
	case 0x4C: s.r.c = s.r.h; s.t_cycles += 4; break;                                          // LD C,H
	case 0x4D: s.r.c = s.r.l; s.t_cycles += 4; break;                                          // LD C,L
	case 0x4E: s.r.c = MemRead(mem, s.r.hl); s.t_cycles += 8; break;                           // LD C,(HL)
	case 0x4F: s.r.c = s.r.a; s.t_cycles += 4; break;                                          // LD C,A

	// ===== 0x50-0x5F : LD D,r / LD E,r =====
	case 0x50: s.r.d = s.r.b; s.t_cycles += 4; break;                                          // LD D,B
	case 0x51: s.r.d = s.r.c; s.t_cycles += 4; break;                                          // LD D,C
	case 0x52: s.t_cycles += 4; break;                                                         // LD D,D
	case 0x53: s.r.d = s.r.e; s.t_cycles += 4; break;                                          // LD D,E
	case 0x54: s.r.d = s.r.h; s.t_cycles += 4; break;                                          // LD D,H
	case 0x55: s.r.d = s.r.l; s.t_cycles += 4; break;                                          // LD D,L
	case 0x56: s.r.d = MemRead(mem, s.r.hl); s.t_cycles += 8; break;                           // LD D,(HL)
	case 0x57: s.r.d = s.r.a; s.t_cycles += 4; break;                                          // LD D,A
	case 0x58: s.r.e = s.r.b; s.t_cycles += 4; break;                                          // LD E,B
	case 0x59: s.r.e = s.r.c; s.t_cycles += 4; break;                                          // LD E,C
	case 0x5A: s.r.e = s.r.d; s.t_cycles += 4; break;                                          // LD E,D
	case 0x5B: s.t_cycles += 4; break;                                                         // LD E,E
	case 0x5C: s.r.e = s.r.h; s.t_cycles += 4; break;                                          // LD E,H
	case 0x5D: s.r.e = s.r.l; s.t_cycles += 4; break;                                          // LD E,L
	case 0x5E: s.r.e = MemRead(mem, s.r.hl); s.t_cycles += 8; break;                           // LD E,(HL)
	case 0x5F: s.r.e = s.r.a; s.t_cycles += 4; break;                                          // LD E,A

	// ===== 0x60-0x6F : LD H,r / LD L,r =====
	case 0x60: s.r.h = s.r.b; s.t_cycles += 4; break;                                          // LD H,B
	case 0x61: s.r.h = s.r.c; s.t_cycles += 4; break;                                          // LD H,C
	case 0x62: s.r.h = s.r.d; s.t_cycles += 4; break;                                          // LD H,D
	case 0x63: s.r.h = s.r.e; s.t_cycles += 4; break;                                          // LD H,E
	case 0x64: s.t_cycles += 4; break;                                                         // LD H,H
	case 0x65: s.r.h = s.r.l; s.t_cycles += 4; break;                                          // LD H,L
	case 0x66: s.r.h = MemRead(mem, s.r.hl); s.t_cycles += 8; break;                           // LD H,(HL)
	case 0x67: s.r.h = s.r.a; s.t_cycles += 4; break;                                          // LD H,A
	case 0x68: s.r.l = s.r.b; s.t_cycles += 4; break;                                          // LD L,B
	case 0x69: s.r.l = s.r.c; s.t_cycles += 4; break;                                          // LD L,C
	case 0x6A: s.r.l = s.r.d; s.t_cycles += 4; break;                                          // LD L,D
	case 0x6B: s.r.l = s.r.e; s.t_cycles += 4; break;                                          // LD L,E
	case 0x6C: s.r.l = s.r.h; s.t_cycles += 4; break;                                          // LD L,H
	case 0x6D: s.t_cycles += 4; break;                                                         // LD L,L
	case 0x6E: s.r.l = MemRead(mem, s.r.hl); s.t_cycles += 8; break;                           // LD L,(HL)
	case 0x6F: s.r.l = s.r.a; s.t_cycles += 4; break;                                          // LD L,A

	// ===== 0x70-0x7F : LD (HL),r / LD A,r =====
	case 0x70: MemWrite(mem, s.r.hl, s.r.b); s.t_cycles += 8; break;                           // LD (HL),B
	case 0x71: MemWrite(mem, s.r.hl, s.r.c); s.t_cycles += 8; break;                           // LD (HL),C
	case 0x72: MemWrite(mem, s.r.hl, s.r.d); s.t_cycles += 8; break;                           // LD (HL),D
	case 0x73: MemWrite(mem, s.r.hl, s.r.e); s.t_cycles += 8; break;                           // LD (HL),E
	case 0x74: MemWrite(mem, s.r.hl, s.r.h); s.t_cycles += 8; break;                           // LD (HL),H
	case 0x75: MemWrite(mem, s.r.hl, s.r.l); s.t_cycles += 8; break;                           // LD (HL),L
	case 0x76:  // HALT
		s.t_cycles += 4;
		if (!s.ime && !s.ime_pending && (mem.ie & mem.if_ & IRQ_ALL)) s.halt_bug = true;
		else                                                          s.halted   = true;
		break;
	case 0x77: MemWrite(mem, s.r.hl, s.r.a); s.t_cycles += 8; break;                           // LD (HL),A
	case 0x78: s.r.a = s.r.b; s.t_cycles += 4; break;                                          // LD A,B
	case 0x79: s.r.a = s.r.c; s.t_cycles += 4; break;                                          // LD A,C
	case 0x7A: s.r.a = s.r.d; s.t_cycles += 4; break;                                          // LD A,D
	case 0x7B: s.r.a = s.r.e; s.t_cycles += 4; break;                                          // LD A,E
	case 0x7C: s.r.a = s.r.h; s.t_cycles += 4; break;                                          // LD A,H
	case 0x7D: s.r.a = s.r.l; s.t_cycles += 4; break;                                          // LD A,L
	case 0x7E: s.r.a = MemRead(mem, s.r.hl); s.t_cycles += 8; break;                           // LD A,(HL)
	case 0x7F: s.t_cycles += 4; break;                                                         // LD A,A

	// ===== 0x80-0x8F : ADD A,r / ADC A,r =====
	case 0x80: s.r.a = AluAdd(s, s.r.a, s.r.b); s.t_cycles += 4; break;                        // ADD A,B
	case 0x81: s.r.a = AluAdd(s, s.r.a, s.r.c); s.t_cycles += 4; break;                        // ADD A,C
	case 0x82: s.r.a = AluAdd(s, s.r.a, s.r.d); s.t_cycles += 4; break;                        // ADD A,D
	case 0x83: s.r.a = AluAdd(s, s.r.a, s.r.e); s.t_cycles += 4; break;                        // ADD A,E
	case 0x84: s.r.a = AluAdd(s, s.r.a, s.r.h); s.t_cycles += 4; break;                        // ADD A,H
	case 0x85: s.r.a = AluAdd(s, s.r.a, s.r.l); s.t_cycles += 4; break;                        // ADD A,L
	case 0x86: s.r.a = AluAdd(s, s.r.a, MemRead(mem, s.r.hl)); s.t_cycles += 8; break;         // ADD A,(HL)
	case 0x87: s.r.a = AluAdd(s, s.r.a, s.r.a); s.t_cycles += 4; break;                        // ADD A,A
	case 0x88: s.r.a = AluAdc(s, s.r.a, s.r.b); s.t_cycles += 4; break;                        // ADC A,B
	case 0x89: s.r.a = AluAdc(s, s.r.a, s.r.c); s.t_cycles += 4; break;                        // ADC A,C
	case 0x8A: s.r.a = AluAdc(s, s.r.a, s.r.d); s.t_cycles += 4; break;                        // ADC A,D
	case 0x8B: s.r.a = AluAdc(s, s.r.a, s.r.e); s.t_cycles += 4; break;                        // ADC A,E
	case 0x8C: s.r.a = AluAdc(s, s.r.a, s.r.h); s.t_cycles += 4; break;                        // ADC A,H
	case 0x8D: s.r.a = AluAdc(s, s.r.a, s.r.l); s.t_cycles += 4; break;                        // ADC A,L
	case 0x8E: s.r.a = AluAdc(s, s.r.a, MemRead(mem, s.r.hl)); s.t_cycles += 8; break;         // ADC A,(HL)
	case 0x8F: s.r.a = AluAdc(s, s.r.a, s.r.a); s.t_cycles += 4; break;                        // ADC A,A

	// ===== 0x90-0x9F : SUB A,r / SBC A,r =====
	case 0x90: s.r.a = AluSub(s, s.r.a, s.r.b); s.t_cycles += 4; break;                        // SUB A,B
	case 0x91: s.r.a = AluSub(s, s.r.a, s.r.c); s.t_cycles += 4; break;                        // SUB A,C
	case 0x92: s.r.a = AluSub(s, s.r.a, s.r.d); s.t_cycles += 4; break;                        // SUB A,D
	case 0x93: s.r.a = AluSub(s, s.r.a, s.r.e); s.t_cycles += 4; break;                        // SUB A,E
	case 0x94: s.r.a = AluSub(s, s.r.a, s.r.h); s.t_cycles += 4; break;                        // SUB A,H
	case 0x95: s.r.a = AluSub(s, s.r.a, s.r.l); s.t_cycles += 4; break;                        // SUB A,L
	case 0x96: s.r.a = AluSub(s, s.r.a, MemRead(mem, s.r.hl)); s.t_cycles += 8; break;         // SUB A,(HL)
	case 0x97: s.r.a = AluSub(s, s.r.a, s.r.a); s.t_cycles += 4; break;                        // SUB A,A
	case 0x98: s.r.a = AluSbc(s, s.r.a, s.r.b); s.t_cycles += 4; break;                        // SBC A,B
	case 0x99: s.r.a = AluSbc(s, s.r.a, s.r.c); s.t_cycles += 4; break;                        // SBC A,C
	case 0x9A: s.r.a = AluSbc(s, s.r.a, s.r.d); s.t_cycles += 4; break;                        // SBC A,D
	case 0x9B: s.r.a = AluSbc(s, s.r.a, s.r.e); s.t_cycles += 4; break;                        // SBC A,E
	case 0x9C: s.r.a = AluSbc(s, s.r.a, s.r.h); s.t_cycles += 4; break;                        // SBC A,H
	case 0x9D: s.r.a = AluSbc(s, s.r.a, s.r.l); s.t_cycles += 4; break;                        // SBC A,L
	case 0x9E: s.r.a = AluSbc(s, s.r.a, MemRead(mem, s.r.hl)); s.t_cycles += 8; break;         // SBC A,(HL)
	case 0x9F: s.r.a = AluSbc(s, s.r.a, s.r.a); s.t_cycles += 4; break;                        // SBC A,A

	// ===== 0xA0-0xAF : AND A,r / XOR A,r =====
	case 0xA0: s.r.a = AluAnd(s, s.r.a, s.r.b); s.t_cycles += 4; break;                        // AND A,B
	case 0xA1: s.r.a = AluAnd(s, s.r.a, s.r.c); s.t_cycles += 4; break;                        // AND A,C
	case 0xA2: s.r.a = AluAnd(s, s.r.a, s.r.d); s.t_cycles += 4; break;                        // AND A,D
	case 0xA3: s.r.a = AluAnd(s, s.r.a, s.r.e); s.t_cycles += 4; break;                        // AND A,E
	case 0xA4: s.r.a = AluAnd(s, s.r.a, s.r.h); s.t_cycles += 4; break;                        // AND A,H
	case 0xA5: s.r.a = AluAnd(s, s.r.a, s.r.l); s.t_cycles += 4; break;                        // AND A,L
	case 0xA6: s.r.a = AluAnd(s, s.r.a, MemRead(mem, s.r.hl)); s.t_cycles += 8; break;         // AND A,(HL)
	case 0xA7: s.r.a = AluAnd(s, s.r.a, s.r.a); s.t_cycles += 4; break;                        // AND A,A
	case 0xA8: s.r.a = AluXor(s, s.r.a, s.r.b); s.t_cycles += 4; break;                        // XOR A,B
	case 0xA9: s.r.a = AluXor(s, s.r.a, s.r.c); s.t_cycles += 4; break;                        // XOR A,C
	case 0xAA: s.r.a = AluXor(s, s.r.a, s.r.d); s.t_cycles += 4; break;                        // XOR A,D
	case 0xAB: s.r.a = AluXor(s, s.r.a, s.r.e); s.t_cycles += 4; break;                        // XOR A,E
	case 0xAC: s.r.a = AluXor(s, s.r.a, s.r.h); s.t_cycles += 4; break;                        // XOR A,H
	case 0xAD: s.r.a = AluXor(s, s.r.a, s.r.l); s.t_cycles += 4; break;                        // XOR A,L
	case 0xAE: s.r.a = AluXor(s, s.r.a, MemRead(mem, s.r.hl)); s.t_cycles += 8; break;         // XOR A,(HL)
	case 0xAF: s.r.a = AluXor(s, s.r.a, s.r.a); s.t_cycles += 4; break;                        // XOR A,A

	// ===== 0xB0-0xBF : OR A,r / CP A,r =====
	case 0xB0: s.r.a = AluOr(s, s.r.a, s.r.b); s.t_cycles += 4; break;                         // OR A,B
	case 0xB1: s.r.a = AluOr(s, s.r.a, s.r.c); s.t_cycles += 4; break;                         // OR A,C
	case 0xB2: s.r.a = AluOr(s, s.r.a, s.r.d); s.t_cycles += 4; break;                         // OR A,D
	case 0xB3: s.r.a = AluOr(s, s.r.a, s.r.e); s.t_cycles += 4; break;                         // OR A,E
	case 0xB4: s.r.a = AluOr(s, s.r.a, s.r.h); s.t_cycles += 4; break;                         // OR A,H
	case 0xB5: s.r.a = AluOr(s, s.r.a, s.r.l); s.t_cycles += 4; break;                         // OR A,L
	case 0xB6: s.r.a = AluOr(s, s.r.a, MemRead(mem, s.r.hl)); s.t_cycles += 8; break;          // OR A,(HL)
	case 0xB7: s.r.a = AluOr(s, s.r.a, s.r.a); s.t_cycles += 4; break;                         // OR A,A
	case 0xB8: AluCp(s, s.r.a, s.r.b); s.t_cycles += 4; break;                                 // CP A,B
	case 0xB9: AluCp(s, s.r.a, s.r.c); s.t_cycles += 4; break;                                 // CP A,C
	case 0xBA: AluCp(s, s.r.a, s.r.d); s.t_cycles += 4; break;                                 // CP A,D
	case 0xBB: AluCp(s, s.r.a, s.r.e); s.t_cycles += 4; break;                                 // CP A,E
	case 0xBC: AluCp(s, s.r.a, s.r.h); s.t_cycles += 4; break;                                 // CP A,H
	case 0xBD: AluCp(s, s.r.a, s.r.l); s.t_cycles += 4; break;                                 // CP A,L
	case 0xBE: AluCp(s, s.r.a, MemRead(mem, s.r.hl)); s.t_cycles += 8; break;                  // CP A,(HL)
	case 0xBF: AluCp(s, s.r.a, s.r.a); s.t_cycles += 4; break;                                 // CP A,A

	// ===== 0xC0-0xCF =====
	case 0xC0: if (!FlagZ(s)) { s.r.pc = Pop16(s, mem); s.t_cycles += 20; } else { s.t_cycles += 8; } break;    // RET NZ
	case 0xC1: s.r.bc = Pop16(s, mem); s.t_cycles += 12; break;                                // POP BC
	case 0xC2: { uint16_t a = Fetch16(s, mem); if (!FlagZ(s)) { s.r.pc = a; s.t_cycles += 16; } else { s.t_cycles += 12; } } break;  // JP NZ,nn
	case 0xC3: s.r.pc = Fetch16(s, mem); s.t_cycles += 16; break;                              // JP nn
	case 0xC4: { uint16_t a = Fetch16(s, mem); if (!FlagZ(s)) { Push16(s, mem, s.r.pc); s.r.pc = a; s.t_cycles += 24; } else { s.t_cycles += 12; } } break;  // CALL NZ,nn
	case 0xC5: Push16(s, mem, s.r.bc); s.t_cycles += 16; break;                                // PUSH BC
	case 0xC6: s.r.a = AluAdd(s, s.r.a, Fetch8(s, mem)); s.t_cycles += 8; break;               // ADD A,n
	case 0xC7: Push16(s, mem, s.r.pc); s.r.pc = 0x0000; s.t_cycles += 16; break;               // RST 00h
	case 0xC8: if (FlagZ(s))  { s.r.pc = Pop16(s, mem); s.t_cycles += 20; } else { s.t_cycles += 8; } break;    // RET Z
	case 0xC9: s.r.pc = Pop16(s, mem); s.t_cycles += 16; break;                                // RET
	case 0xCA: { uint16_t a = Fetch16(s, mem); if (FlagZ(s))  { s.r.pc = a; s.t_cycles += 16; } else { s.t_cycles += 12; } } break;  // JP Z,nn
	case 0xCB: { uint8_t cb = Fetch8(s, mem); DispatchCB(s, mem, cb); } break;                 // CB prefix
	case 0xCC: { uint16_t a = Fetch16(s, mem); if (FlagZ(s))  { Push16(s, mem, s.r.pc); s.r.pc = a; s.t_cycles += 24; } else { s.t_cycles += 12; } } break;  // CALL Z,nn
	case 0xCD: { uint16_t a = Fetch16(s, mem); Push16(s, mem, s.r.pc); s.r.pc = a; s.t_cycles += 24; } break;   // CALL nn
	case 0xCE: s.r.a = AluAdc(s, s.r.a, Fetch8(s, mem)); s.t_cycles += 8; break;               // ADC A,n
	case 0xCF: Push16(s, mem, s.r.pc); s.r.pc = 0x0008; s.t_cycles += 16; break;               // RST 08h

	// ===== 0xD0-0xDF =====
	case 0xD0: if (!FlagC(s)) { s.r.pc = Pop16(s, mem); s.t_cycles += 20; } else { s.t_cycles += 8; } break;    // RET NC
	case 0xD1: s.r.de = Pop16(s, mem); s.t_cycles += 12; break;                                // POP DE
	case 0xD2: { uint16_t a = Fetch16(s, mem); if (!FlagC(s)) { s.r.pc = a; s.t_cycles += 16; } else { s.t_cycles += 12; } } break;  // JP NC,nn
	case 0xD3: ++s.illegal_ops; s.stopped = true; s.t_cycles += 4; break;                      // *** undefined ***
	case 0xD4: { uint16_t a = Fetch16(s, mem); if (!FlagC(s)) { Push16(s, mem, s.r.pc); s.r.pc = a; s.t_cycles += 24; } else { s.t_cycles += 12; } } break;  // CALL NC,nn
	case 0xD5: Push16(s, mem, s.r.de); s.t_cycles += 16; break;                                // PUSH DE
	case 0xD6: s.r.a = AluSub(s, s.r.a, Fetch8(s, mem)); s.t_cycles += 8; break;               // SUB A,n
	case 0xD7: Push16(s, mem, s.r.pc); s.r.pc = 0x0010; s.t_cycles += 16; break;               // RST 10h
	case 0xD8: if (FlagC(s))  { s.r.pc = Pop16(s, mem); s.t_cycles += 20; } else { s.t_cycles += 8; } break;    // RET C
	case 0xD9: s.r.pc = Pop16(s, mem); s.ime = true; s.ime_pending = false; s.t_cycles += 16; break;   // RETI
	case 0xDA: { uint16_t a = Fetch16(s, mem); if (FlagC(s))  { s.r.pc = a; s.t_cycles += 16; } else { s.t_cycles += 12; } } break;  // JP C,nn
	case 0xDB: ++s.illegal_ops; s.stopped = true; s.t_cycles += 4; break;                      // *** undefined ***
	case 0xDC: { uint16_t a = Fetch16(s, mem); if (FlagC(s))  { Push16(s, mem, s.r.pc); s.r.pc = a; s.t_cycles += 24; } else { s.t_cycles += 12; } } break;  // CALL C,nn
	case 0xDD: ++s.illegal_ops; s.stopped = true; s.t_cycles += 4; break;                      // *** undefined ***
	case 0xDE: s.r.a = AluSbc(s, s.r.a, Fetch8(s, mem)); s.t_cycles += 8; break;               // SBC A,n
	case 0xDF: Push16(s, mem, s.r.pc); s.r.pc = 0x0018; s.t_cycles += 16; break;               // RST 18h

	// ===== 0xE0-0xEF =====
	case 0xE0: { uint16_t a = static_cast<uint16_t>(0xFF00 + Fetch8(s, mem)); MemWrite(mem, a, s.r.a); s.t_cycles += 12; } break;  // LDH (n),A
	case 0xE1: s.r.hl = Pop16(s, mem); s.t_cycles += 12; break;                                // POP HL
	case 0xE2: MemWrite(mem, static_cast<uint16_t>(0xFF00 + s.r.c), s.r.a); s.t_cycles += 8; break;  // LD (C),A
	case 0xE3: ++s.illegal_ops; s.stopped = true; s.t_cycles += 4; break;                      // *** undefined ***
	case 0xE4: ++s.illegal_ops; s.stopped = true; s.t_cycles += 4; break;                      // *** undefined ***
	case 0xE5: Push16(s, mem, s.r.hl); s.t_cycles += 16; break;                                // PUSH HL
	case 0xE6: s.r.a = AluAnd(s, s.r.a, Fetch8(s, mem)); s.t_cycles += 8; break;               // AND A,n
	case 0xE7: Push16(s, mem, s.r.pc); s.r.pc = 0x0020; s.t_cycles += 16; break;               // RST 20h
	case 0xE8: { int8_t e = static_cast<int8_t>(Fetch8(s, mem)); s.r.sp = AluAddSpE(s, s.r.sp, e); s.t_cycles += 16; } break;  // ADD SP,e
	case 0xE9: s.r.pc = s.r.hl; s.t_cycles += 4; break;                                        // JP HL  (a.k.a JP (HL))
	case 0xEA: { uint16_t a = Fetch16(s, mem); MemWrite(mem, a, s.r.a); s.t_cycles += 16; } break;  // LD (nn),A
	case 0xEB: ++s.illegal_ops; s.stopped = true; s.t_cycles += 4; break;                      // *** undefined ***
	case 0xEC: ++s.illegal_ops; s.stopped = true; s.t_cycles += 4; break;                      // *** undefined ***
	case 0xED: ++s.illegal_ops; s.stopped = true; s.t_cycles += 4; break;                      // *** undefined ***
	case 0xEE: s.r.a = AluXor(s, s.r.a, Fetch8(s, mem)); s.t_cycles += 8; break;               // XOR A,n
	case 0xEF: Push16(s, mem, s.r.pc); s.r.pc = 0x0028; s.t_cycles += 16; break;               // RST 28h

	// ===== 0xF0-0xFF =====
	case 0xF0: s.r.a = MemRead(mem, static_cast<uint16_t>(0xFF00 + Fetch8(s, mem))); s.t_cycles += 12; break;   // LDH A,(n)
	case 0xF1: s.r.af = static_cast<uint16_t>(Pop16(s, mem) & 0xFFF0); s.t_cycles += 12; break; // POP AF — F low nibble always 0
	case 0xF2: s.r.a = MemRead(mem, static_cast<uint16_t>(0xFF00 + s.r.c)); s.t_cycles += 8; break;  // LD A,(C)
	case 0xF3: s.ime = false; s.ime_pending = false; s.t_cycles += 4; break;                   // DI
	case 0xF4: ++s.illegal_ops; s.stopped = true; s.t_cycles += 4; break;                      // *** undefined ***
	case 0xF5: Push16(s, mem, s.r.af); s.t_cycles += 16; break;                                // PUSH AF
	case 0xF6: s.r.a = AluOr(s, s.r.a, Fetch8(s, mem)); s.t_cycles += 8; break;                // OR A,n
	case 0xF7: Push16(s, mem, s.r.pc); s.r.pc = 0x0030; s.t_cycles += 16; break;               // RST 30h
	case 0xF8: { int8_t e = static_cast<int8_t>(Fetch8(s, mem)); s.r.hl = AluAddSpE(s, s.r.sp, e); s.t_cycles += 12; } break;  // LD HL,SP+e
	case 0xF9: s.r.sp = s.r.hl; s.t_cycles += 8; break;                                        // LD SP,HL
	case 0xFA: { uint16_t a = Fetch16(s, mem); s.r.a = MemRead(mem, a); s.t_cycles += 16; } break;  // LD A,(nn)
	case 0xFB: s.ime_pending = true; s.t_cycles += 4; break;                                   // EI
	case 0xFC: ++s.illegal_ops; s.stopped = true; s.t_cycles += 4; break;                      // *** undefined ***
	case 0xFD: ++s.illegal_ops; s.stopped = true; s.t_cycles += 4; break;                      // *** undefined ***
	case 0xFE: AluCp(s, s.r.a, Fetch8(s, mem)); s.t_cycles += 8; break;                        // CP A,n
	case 0xFF: Push16(s, mem, s.r.pc); s.r.pc = 0x0038; s.t_cycles += 16; break;               // RST 38h
	}
}

} // namespace SGB
