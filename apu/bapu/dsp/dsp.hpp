#include "SPC_DSP.h"

class DSP : public Processor {
public:
  enum { Threaded = false };
  alwaysinline void synchronize_smp();

  uint8 read(uint8 addr);
  void write(uint8 addr, uint8 data);

  void power();
  void reset();

  void channel_enable(unsigned channel, bool enable);

  DSP();

  SPC_DSP spc_dsp;

private:
  bool channel_enabled[8];
};

extern DSP dsp;
