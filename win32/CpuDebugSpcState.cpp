// Isolated bAPU state reader for the CPU debugger dialog. The bAPU headers
// declare `int min(int,int)` (and similar), which clash with windows.h's
// min/max macros once that header is included. Keeping this in its own .cpp
// avoids that — only the lightweight C-style helper is exported.

#include "../snes9x.h"
#include "../apu/bapu/snes/snes.hpp"

struct CpuDebugSpcState {
    uint16_t pc;
    uint8_t  a, x, y, sp;
    uint8_t  apuram_F4, apuram_F5, apuram_F6, apuram_F7;
    uint8_t  cpu_regs_0, cpu_regs_1, cpu_regs_2, cpu_regs_3;
};

void S9xGetCpuDebugSpcState(CpuDebugSpcState *out)
{
    if (!out) return;
    out->pc        = SNES::smp.regs.pc;
    out->a         = SNES::smp.regs.B.a;
    out->x         = SNES::smp.regs.x;
    out->y         = SNES::smp.regs.B.y;
    out->sp        = SNES::smp.regs.sp;
    out->apuram_F4 = SNES::smp.apuram ? SNES::smp.apuram[0xF4] : 0;
    out->apuram_F5 = SNES::smp.apuram ? SNES::smp.apuram[0xF5] : 0;
    out->apuram_F6 = SNES::smp.apuram ? SNES::smp.apuram[0xF6] : 0;
    out->apuram_F7 = SNES::smp.apuram ? SNES::smp.apuram[0xF7] : 0;
    out->cpu_regs_0 = SNES::cpu.registers[0];
    out->cpu_regs_1 = SNES::cpu.registers[1];
    out->cpu_regs_2 = SNES::cpu.registers[2];
    out->cpu_regs_3 = SNES::cpu.registers[3];
}
