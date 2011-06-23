#include "SPC_DSP.h"

class DSP : public Processor {
public:
  inline uint8 read(uint8 addr) {
    return spc_dsp.read(addr);
  }

  inline void write(uint8 addr, uint8 data) {
    spc_dsp.write(addr, data);
  }

  void power();
  void reset();

  DSP();

  SPC_DSP spc_dsp;
};

extern DSP dsp;
