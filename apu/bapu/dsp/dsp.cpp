#include <snes/snes.hpp>

#define DSP_CPP
namespace SNES {

DSP dsp;

#include "SPC_DSP.cpp"

uint8 DSP::read(uint8 addr) {
  return spc_dsp.read(addr);
}

void DSP::write(uint8 addr, uint8 data) {
  spc_dsp.write(addr, data);
}

void DSP::power() {
  spc_dsp.init(smp.apuram);
  spc_dsp.reset();
}

void DSP::reset() {
  spc_dsp.soft_reset();
}

void DSP::channel_enable(unsigned channel, bool enable) {
  channel_enabled[channel & 7] = enable;
  unsigned mask = 0;
  for(unsigned i = 0; i < 8; i++) {
    if(channel_enabled[i] == false) mask |= 1 << i;
  }
  spc_dsp.mute_voices(mask);
}

DSP::DSP() {
  for(unsigned i = 0; i < 8; i++) channel_enabled[i] = true;
}

}
