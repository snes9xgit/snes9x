#include <snes/snes.hpp>

#define DSP_CPP
namespace SNES {

DSP dsp;

#include "SPC_DSP.cpp"

void DSP::power() {
  spc_dsp.init(smp.apuram);
  spc_dsp.reset();
}

void DSP::reset() {
  spc_dsp.soft_reset();
}

DSP::DSP() {
}

}
