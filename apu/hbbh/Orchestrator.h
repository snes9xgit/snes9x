// Node Graph

// single sfx branch trace
// sfx -> pitch -> more_dsp -> output

// One branch per sound
#include "miniaudio.h"

class Orchestrator {
  ma_node_graph *graph;
  ma_node *output;
  ma_engine engine;
  ma_uint32 channels;
  ma_uint32 sampleRate;
  ma_sound testSound;

  public:
  Orchestrator();
  ~Orchestrator();
  void ForwardEvent(int snd_queue);
};
