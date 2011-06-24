#include "snes/snes.hpp"

namespace SNES {

#include "dsp/blargg_endian.h"

void SMP::save_state(uint8 **block) {
  uint8 *ptr = *block;
  memcpy(ptr, apuram, 64 * 1024);
  ptr += 64 * 1024;

#undef INT32
#define INT32(i) set_le32(ptr, (i)); ptr += sizeof(int32)
  INT32(clock);

  INT32(opcode_number);
  INT32(opcode_cycle);

  INT32(regs.pc);
  INT32(regs.sp);
  INT32(regs.a);
  INT32(regs.x);
  INT32(regs.y);

  INT32(regs.p.n);
  INT32(regs.p.v);
  INT32(regs.p.p);
  INT32(regs.p.b);
  INT32(regs.p.h);
  INT32(regs.p.i);
  INT32(regs.p.z);
  INT32(regs.p.c);

  INT32(status.iplrom_enable);

  INT32(status.dsp_addr);

  INT32(status.ram00f8);
  INT32(status.ram00f9);

  INT32(timer0.enable);
  INT32(timer0.target);
  INT32(timer0.stage1_ticks);
  INT32(timer0.stage2_ticks);
  INT32(timer0.stage3_ticks);

  INT32(timer1.enable);
  INT32(timer1.target);
  INT32(timer1.stage1_ticks);
  INT32(timer1.stage2_ticks);
  INT32(timer1.stage3_ticks);

  INT32(timer2.enable);
  INT32(timer2.target);

  INT32(timer2.stage1_ticks);
  INT32(timer2.stage2_ticks);
  INT32(timer2.stage3_ticks);

  *block = ptr;
}

void SMP::load_state(uint8 **block) {
  uint8 *ptr = *block;
  memcpy(apuram, ptr, 64 * 1024);
  ptr += 64 * 1024;

#undef INT32
#define INT32(i) i = get_le32(ptr); ptr += sizeof(int32)
  INT32(clock);

  INT32(opcode_number);
  INT32(opcode_cycle);

  INT32(regs.pc);
  INT32(regs.sp);
  INT32(regs.a);
  INT32(regs.x);
  INT32(regs.y);

  INT32(regs.p.n);
  INT32(regs.p.v);
  INT32(regs.p.p);
  INT32(regs.p.b);
  INT32(regs.p.h);
  INT32(regs.p.i);
  INT32(regs.p.z);
  INT32(regs.p.c);

  INT32(status.iplrom_enable);

  INT32(status.dsp_addr);

  INT32(status.ram00f8);
  INT32(status.ram00f9);

  INT32(timer0.enable);
  INT32(timer0.target);
  INT32(timer0.stage1_ticks);
  INT32(timer0.stage2_ticks);
  INT32(timer0.stage3_ticks);

  INT32(timer1.enable);
  INT32(timer1.target);
  INT32(timer1.stage1_ticks);
  INT32(timer1.stage2_ticks);
  INT32(timer1.stage3_ticks);

  INT32(timer2.enable);
  INT32(timer2.target);

  INT32(timer2.stage1_ticks);
  INT32(timer2.stage2_ticks);
  INT32(timer2.stage3_ticks);

  *block = ptr;
}

}
