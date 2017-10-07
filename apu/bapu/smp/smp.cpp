#define CYCLE_ACCURATE
#define PSEUDO_CYCLE

#include "snes/snes.hpp"
#include "smp.hpp"

#define SMP_CPP
namespace SNES {

#if defined(DEBUGGER)
  #include "debugger/debugger.cpp"
  #include "debugger/disassembler.cpp"
  SMPDebugger smp;
#else
  SMP smp;
#endif

//#include "core.cpp"
  void SMP::tick() {
      timer0.tick();
      timer1.tick();
      timer2.tick();

#ifndef SNES9X
      clock += cycle_step_cpu;
      dsp.clock -= 24;
      synchronize_dsp();
#else
      clock++;
      dsp.clock++;
#endif
  }

  void SMP::tick(unsigned clocks) {
      timer0.tick(clocks);
      timer1.tick(clocks);
      timer2.tick(clocks);

      clock += clocks;
      dsp.clock += clocks;
  }

  void SMP::op_io() {
#if defined(CYCLE_ACCURATE)
      tick();
#endif
  }

  void SMP::op_io(unsigned clocks) {
      tick(clocks);
  }

  uint8 SMP::op_read(uint16 addr) {
#if defined(CYCLE_ACCURATE)
      tick();
#endif
      if ((addr & 0xfff0) == 0x00f0) return mmio_read(addr);
      if (addr >= 0xffc0 && status.iplrom_enable) return iplrom[addr & 0x3f];
      return apuram[addr];
  }

  void SMP::op_write(uint16 addr, uint8 data) {
#if defined(CYCLE_ACCURATE)
      tick();
#endif
      if ((addr & 0xfff0) == 0x00f0) mmio_write(addr, data);
      apuram[addr] = data;  //all writes go to RAM, even MMIO writes
  }

  void SMP::op_step() {
#define op_readpc() op_read(regs.pc++)
#define op_readdp(addr) op_read((regs.p.p << 8) + ((addr) & 0xff))
#define op_writedp(addr, data) op_write((regs.p.p << 8) + ((addr) & 0xff), data)
#define op_readaddr(addr) op_read(addr)
#define op_writeaddr(addr, data) op_write(addr, data)
#define op_readstack() op_read(0x0100 | ++regs.sp)
#define op_writestack(data) op_write(0x0100 | regs.sp--, data)

#if defined(CYCLE_ACCURATE)

#define OPCODE_FOR_SWITCH opcode_number

#if defined(PSEUDO_CYCLE)

      if (opcode_cycle == 0)
          opcode_number = op_readpc();

#else

      if (opcode_cycle == 0) {
          opcode_number = op_readpc();
          opcode_cycle++;
      }
      else

#endif // defined(PSEUDO_CYCLE)
#else

#define OPCODE_FOR_SWITCH opcode

      unsigned opcode = op_readpc();

#endif


      switch (OPCODE_FOR_SWITCH) {
#include "core/oppseudo_misc.cpp"
#include "core/oppseudo_mov.cpp"
#include "core/oppseudo_pc.cpp"
#include "core/oppseudo_read.cpp"
#include "core/oppseudo_rmw.cpp"
      }

#undef OPCODE_FOR_SWITCH

#if !defined (CYCLE_ACCURATE)

      //TODO: untaken branches should consume less cycles

      timer0.tick(cycle_count_table[opcode]);
      timer1.tick(cycle_count_table[opcode]);
      timer2.tick(cycle_count_table[opcode]);

#ifndef SNES9X
      clock += cycle_table_cpu[opcode];
      dsp.clock -= cycle_table_dsp[opcode];
      synchronize_dsp();
#else
      clock += cycle_count_table[opcode];
      dsp.clock += cycle_count_table[opcode];
#endif

#endif // defined(CYCLE_ACCURATE)
  }

  const unsigned SMP::cycle_count_table[256] = {
#define c 12
      //0 1 2 3   4 5 6 7   8 9 A B   C D E F
      2,8,4,7,  3,4,3,6,  2,6,5,4,  5,4,6,8,  //0
      4,8,4,7,  4,5,5,6,  5,5,6,5,  2,2,4,6,  //1
      2,8,4,7,  3,4,3,6,  2,6,5,4,  5,4,7,4,  //2
      4,8,4,7,  4,5,5,6,  5,5,6,5,  2,2,3,8,  //3

      2,8,4,7,  3,4,3,6,  2,6,4,4,  5,4,6,6,  //4
      4,8,4,7,  4,5,5,6,  5,5,4,5,  2,2,4,3,  //5
      2,8,4,7,  3,4,3,6,  2,6,4,4,  5,4,7,5,  //6
      4,8,4,7,  4,5,5,6,  5,5,5,5,  2,2,3,6,  //7

      2,8,4,7,  3,4,3,6,  2,6,5,4,  5,2,4,5,  //8
      4,8,4,7,  4,5,5,6,  5,5,5,5,  2,2,c,5,  //9
      3,8,4,7,  3,4,3,6,  2,6,4,4,  5,2,4,4,  //A
      4,8,4,7,  4,5,5,6,  5,5,5,5,  2,2,3,4,  //B

      3,8,4,7,  4,5,4,7,  2,5,6,4,  5,2,4,9,  //C
      4,8,4,7,  5,6,6,7,  4,5,5,5,  2,2,8,3,  //D
      2,8,4,7,  3,4,3,6,  2,4,5,3,  4,3,4,1,  //E
      4,8,4,7,  4,5,5,6,  3,4,5,4,  2,2,6,1,  //F

#undef c
  };


//#include "iplrom.cpp"
#ifdef SMP_CPP

//this is the IPLROM for the S-SMP coprocessor.
//the S-SMP does not allow writing to the IPLROM.
//all writes are instead mapped to the extended
//RAM region, accessible when $f1.d7 is clear.

  const uint8 SMP::iplrom[64] = {
      /*ffc0*/  0xcd, 0xef,        //mov   x,#$ef
      /*ffc2*/  0xbd,              //mov   sp,x
      /*ffc3*/  0xe8, 0x00,        //mov   a,#$00
      /*ffc5*/  0xc6,              //mov   (x),a
      /*ffc6*/  0x1d,              //dec   x
      /*ffc7*/  0xd0, 0xfc,        //bne   $ffc5
      /*ffc9*/  0x8f, 0xaa, 0xf4,  //mov   $f4,#$aa
      /*ffcc*/  0x8f, 0xbb, 0xf5,  //mov   $f5,#$bb
      /*ffcf*/  0x78, 0xcc, 0xf4,  //cmp   $f4,#$cc
      /*ffd2*/  0xd0, 0xfb,        //bne   $ffcf
      /*ffd4*/  0x2f, 0x19,        //bra   $ffef
      /*ffd6*/  0xeb, 0xf4,        //mov   y,$f4
      /*ffd8*/  0xd0, 0xfc,        //bne   $ffd6
      /*ffda*/  0x7e, 0xf4,        //cmp   y,$f4
      /*ffdc*/  0xd0, 0x0b,        //bne   $ffe9
      /*ffde*/  0xe4, 0xf5,        //mov   a,$f5
      /*ffe0*/  0xcb, 0xf4,        //mov   $f4,y
      /*ffe2*/  0xd7, 0x00,        //mov   ($00)+y,a
      /*ffe4*/  0xfc,              //inc   y
      /*ffe5*/  0xd0, 0xf3,        //bne   $ffda
      /*ffe7*/  0xab, 0x01,        //inc   $01
      /*ffe9*/  0x10, 0xef,        //bpl   $ffda
      /*ffeb*/  0x7e, 0xf4,        //cmp   y,$f4
      /*ffed*/  0x10, 0xeb,        //bpl   $ffda
      /*ffef*/  0xba, 0xf6,        //movw  ya,$f6
      /*fff1*/  0xda, 0x00,        //movw  $00,ya
      /*fff3*/  0xba, 0xf4,        //movw  ya,$f4
      /*fff5*/  0xc4, 0xf4,        //mov   $f4,a
      /*fff7*/  0xdd,              //mov   a,y
      /*fff8*/  0x5d,              //mov   x,a
      /*fff9*/  0xd0, 0xdb,        //bne   $ffd6
      /*fffb*/  0x1f, 0x00, 0x00,  //jmp   ($0000+x)
      /*fffe*/  0xc0, 0xff         //reset vector location ($ffc0)
};

#endif

//#include "memory.cpp"
  unsigned SMP::port_read(unsigned addr) {
      return apuram[0xf4 + (addr & 3)];
  }

  void SMP::port_write(unsigned addr, unsigned data) {
      apuram[0xf4 + (addr & 3)] = data;
  }

  unsigned SMP::mmio_read(unsigned addr) {
      switch (addr) {

      case 0xf2:
          return status.dsp_addr;

      case 0xf3:
          return dsp.read(status.dsp_addr & 0x7f);

      case 0xf4:
      case 0xf5:
      case 0xf6:
      case 0xf7:
          synchronize_cpu();
          return cpu.port_read(addr);

      case 0xf8:
          return status.ram00f8;

      case 0xf9:
          return status.ram00f9;

      case 0xfd: {
          unsigned result = timer0.stage3_ticks & 15;
          timer0.stage3_ticks = 0;
          return result;
      }

      case 0xfe: {
          unsigned result = timer1.stage3_ticks & 15;
          timer1.stage3_ticks = 0;
          return result;
      }

      case 0xff: {
          unsigned result = timer2.stage3_ticks & 15;
          timer2.stage3_ticks = 0;
          return result;
      }

      }

      return 0x00;
}

  void SMP::mmio_write(unsigned addr, unsigned data) {
      switch (addr) {

      case 0xf1:
          status.iplrom_enable = data & 0x80;

          if (data & 0x30) {
              synchronize_cpu();
              if (data & 0x20) {
                  cpu.port_write(3, 0x00);
                  cpu.port_write(2, 0x00);
              }
              if (data & 0x10) {
                  cpu.port_write(1, 0x00);
                  cpu.port_write(0, 0x00);
              }
          }

          if (timer2.enable == false && (data & 0x04)) {
              timer2.stage2_ticks = 0;
              timer2.stage3_ticks = 0;
          }
          timer2.enable = data & 0x04;

          if (timer1.enable == false && (data & 0x02)) {
              timer1.stage2_ticks = 0;
              timer1.stage3_ticks = 0;
          }
          timer1.enable = data & 0x02;

          if (timer0.enable == false && (data & 0x01)) {
              timer0.stage2_ticks = 0;
              timer0.stage3_ticks = 0;
          }
          timer0.enable = data & 0x01;

          break;

      case 0xf2:
          status.dsp_addr = data;
          break;

      case 0xf3:
          if (status.dsp_addr & 0x80) break;
          dsp.write(status.dsp_addr, data);
          break;

      case 0xf4:
      case 0xf5:
      case 0xf6:
      case 0xf7:
          synchronize_cpu();
          port_write(addr, data);
          break;

      case 0xf8:
          status.ram00f8 = data;
          break;

      case 0xf9:
          status.ram00f9 = data;
          break;

      case 0xfa:
          timer0.target = data;
          break;

      case 0xfb:
          timer1.target = data;
          break;

      case 0xfc:
          timer2.target = data;
          break;

      }
  }

//#include "timing.cpp"
  template<unsigned cycle_frequency>
  void SMP::Timer<cycle_frequency>::tick() {
      if (++stage1_ticks < cycle_frequency) return;

      stage1_ticks = 0;
      if (enable == false) return;

      if (++stage2_ticks != target) return;

      stage2_ticks = 0;
      stage3_ticks = (stage3_ticks + 1) & 15;
  }

  template<unsigned cycle_frequency>
  void SMP::Timer<cycle_frequency>::tick(unsigned clocks) {
      stage1_ticks += clocks;
      if (stage1_ticks < cycle_frequency) return;

      stage1_ticks -= cycle_frequency;
      if (enable == false) return;

      if (++stage2_ticks != target) return;

      stage2_ticks = 0;
      stage3_ticks = (stage3_ticks + 1) & 15;
  }


void SMP::synchronize_cpu() {
#ifndef SNES9X
  if(CPU::Threaded == true) {
  //if(clock >= 0 && scheduler.sync != Scheduler::SynchronizeMode::All) co_switch(cpu.thread);
  } else {
    while(clock >= 0) cpu.enter();
  }
#endif
}

void SMP::synchronize_dsp() {
#ifndef SNES9X
  if(DSP::Threaded == true) {
  //if(dsp.clock < 0 && scheduler.sync != Scheduler::SynchronizeMode::All) co_switch(dsp.thread);
  } else {
    while(dsp.clock < 0) dsp.enter();
  }
#endif
}

void SMP::enter() {
  while(clock < 0) op_step();
}

void SMP::power() {
#ifndef SNES9X
  Processor::frequency = system.apu_frequency();
#endif

  Processor::clock = 0;

  timer0.target = 0;
  timer1.target = 0;
  timer2.target = 0;

  for(unsigned n = 0; n < 256; n++) {
    cycle_table_dsp[n] = (cycle_count_table[n] * 24);
    cycle_table_cpu[n] = (cycle_count_table[n] * 24) * cpu.frequency;
  }

  cycle_step_cpu = 24 * cpu.frequency;

  reset();
}

void SMP::reset() {
  for(unsigned n = 0x0000; n <= 0xffff; n++) apuram[n] = 0x00;

  opcode_number = 0;
  opcode_cycle = 0;

  regs.pc = 0xffc0;
  regs.sp = 0xef;
  regs.B.a = 0x00;
  regs.x = 0x00;
  regs.B.y = 0x00;
  regs.p = 0x02;

  //$00f1
  status.iplrom_enable = true;

  //$00f2
  status.dsp_addr = 0x00;

  //$00f8,$00f9
  status.ram00f8 = 0x00;
  status.ram00f9 = 0x00;

  //timers
  timer0.enable = timer1.enable = timer2.enable = false;
  timer0.stage1_ticks = timer1.stage1_ticks = timer2.stage1_ticks = 0;
  timer0.stage2_ticks = timer1.stage2_ticks = timer2.stage2_ticks = 0;
  timer0.stage3_ticks = timer1.stage3_ticks = timer2.stage3_ticks = 0;
}

#ifndef SNES9X
void SMP::serialize(serializer &s) {
  Processor::serialize(s);

  s.array(apuram, 64 * 1024);

  s.integer(opcode_number);
  s.integer(opcode_cycle);

  s.integer(regs.pc);
  s.integer(regs.sp);
  s.integer(regs.a);
  s.integer(regs.x);
  s.integer(regs.y);

  s.integer(regs.p.n);
  s.integer(regs.p.v);
  s.integer(regs.p.p);
  s.integer(regs.p.b);
  s.integer(regs.p.h);
  s.integer(regs.p.i);
  s.integer(regs.p.z);
  s.integer(regs.p.c);

  s.integer(status.iplrom_enable);

  s.integer(status.dsp_addr);

  s.integer(status.ram00f8);
  s.integer(status.ram00f9);

  s.integer(timer0.enable);
  s.integer(timer0.target);
  s.integer(timer0.stage1_ticks);
  s.integer(timer0.stage2_ticks);
  s.integer(timer0.stage3_ticks);

  s.integer(timer1.enable);
  s.integer(timer1.target);
  s.integer(timer1.stage1_ticks);
  s.integer(timer1.stage2_ticks);
  s.integer(timer1.stage3_ticks);

  s.integer(timer2.enable);
  s.integer(timer2.target);

  s.integer(timer2.stage1_ticks);
  s.integer(timer2.stage2_ticks);
  s.integer(timer2.stage3_ticks);
}
#endif

SMP::SMP() {
  apuram = new uint8[64 * 1024];
}

SMP::~SMP() {
}

}
