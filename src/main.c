// Example program:
// Using SDL2 to create an application window

#include <SDL.h>
#include <stdio.h>
#include <stdint.h>

#include "synth.h"

#define DR_WAV_IMPLEMENTATION
#include "lib/dr_wav.h"

yase_sound_context sound_context;

void MyAudioCallback(void* userdata, Uint8* stream, int len)
{
	yase_sound_render(&sound_context, stream, len/sizeof(int16_t));
}

SDL_AudioDeviceID sdl_audio_device;

int init_audio()
{
	SDL_AudioSpec want, have;

	SDL_zero(want);
	want.freq = 96000;
	want.format = AUDIO_S16;
	want.channels = 1;
	want.samples = 4096;
	want.callback = MyAudioCallback;

	sdl_audio_device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);

#define PRINT(var) printf("%s : want %d, have %d\n", #var, want. ## var, have. ## var);
	PRINT(freq)
	PRINT(format)
	PRINT(channels)
	PRINT(samples)
#undef PRINT


	if (sdl_audio_device == 0)
	{
		fprintf(stderr, "Couldn't open audio device : %s\n", SDL_GetError());
		return 0;
	}
	else
	{
		printf("Audio device opened\n");
		SDL_PauseAudioDevice(sdl_audio_device, 0); // Start audio device
	}
}

int main(int argc, char* argv[]) {

	SDL_Window* window;                    // Declare a pointer

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);              // Initialize SDL2

	// Create an application window with the following settings:
	window = SDL_CreateWindow(
		"An SDL2 window",                  // window title
		SDL_WINDOWPOS_UNDEFINED,           // initial x position
		SDL_WINDOWPOS_UNDEFINED,           // initial y position
		640,                               // width, in pixels
		480,                               // height, in pixels
		0
	);

	// Check that the window was successfully created
	if (window == NULL) {
		// In the case that the window could not be made...
		printf("Could not create window: %s\n", SDL_GetError());
		return 1;
	}

	// The window is open: could enter program loop here (see SDL_PollEvent())

	printf("SDL successfully init\n");


	yase_sound_init(&sound_context, 96000);

#ifdef DEBUG_OUT
	drwav wav;
	drwav_data_format format;
	format.container = drwav_container_riff;
	format.format = DR_WAVE_FORMAT_PCM;
	format.channels = 1;
	format.bitsPerSample = 16;
	format.sampleRate = 96000;
	drwav_init_file_write(&wav, "yase_out.wav", &format, NULL);

	int16_t buffer[4096];
	for (int i = 0; i < 256; ++i)
	{
		yase_sound_render(&sound_context, buffer, 4096);
		drwav_uint64 framesWritten = drwav_write_pcm_frames(&wav, 4096, buffer);
	}

	drwav_uninit(&wav);
#else
	init_audio();
#endif

	uint8_t bShouldQuit = 0;
	while (!bShouldQuit)
	{
		SDL_Event sdlEvent;
		while (SDL_PollEvent(&sdlEvent))
		{
			if (sdlEvent.type == SDL_QUIT)
			{
				bShouldQuit = 1;
			}
		}
	}

	// Close and destroy the window
	SDL_DestroyWindow(window);

	// Clean up
	SDL_Quit();
	return 0;
}
