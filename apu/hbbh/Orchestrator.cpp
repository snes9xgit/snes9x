#include <iostream>
#include "Orchestrator.h"
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

// Constructor
Orchestrator::Orchestrator() {
    ma_result result = ma_engine_init(NULL, &engine);
    if (result != MA_SUCCESS) {
        printf("Failed to initialize audio engine.");
    }

    channels   = ma_engine_get_channels(&engine);
    sampleRate = ma_engine_get_sample_rate(&engine);

    graph = ma_engine_get_node_graph(&engine);
    output = ma_engine_get_endpoint(&engine);

    result = ma_sound_init_from_file(&engine, "/Users/bh/Documents/game-audio/snes9x/apu/hbbh/sfxFiles/blip.wav", MA_SOUND_FLAG_NO_DEFAULT_ATTACHMENT, NULL, NULL, &testSound);
    if (result != MA_SUCCESS) {
        printf("Failed to initialize testSound.");
    }

    result = ma_node_attach_output_bus(&testSound, 0, output, 0);
    if (result != MA_SUCCESS) {
        // Failed to attach node.
    }
}

// Destructor
Orchestrator::~Orchestrator() {
    ma_sound_uninit(&testSound);
    ma_engine_uninit(&engine);
}

// Method to forward an event
void Orchestrator::ForwardEvent(int snd_queue) {
    if (snd_queue == 3) {
        ma_sound_start(&testSound);
    }
    
    std::cout << "Snd Queue: " << snd_queue << std::endl;
}
