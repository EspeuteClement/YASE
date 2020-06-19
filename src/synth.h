#ifndef YASE_SYNTH_H
#define YASE_SYNTH_H

#include <stdint.h>

typedef struct
{
	uint32_t phase;
	int16_t pitch;

} yase_sound_context;

// Init the yase engine. Initialise the look up tables and stuff
void yase_sound_init(yase_sound_context* ctxt, int32_t target_sample_rate);

// Fill size samples in buffer.
void yase_sound_render(yase_sound_context* ctxt, int16_t* buffer, size_t size);

#endif