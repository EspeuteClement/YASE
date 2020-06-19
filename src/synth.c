#include "synth.h"
#include <math.h>

#define PITCH_FIX 4

#define LUT_OSCILLATOR_SHIFT 16

#define LUT_OSCILLATOR_INCREMENT_SIZE 97
static uint32_t lut_oscillator_increments[LUT_OSCILLATOR_INCREMENT_SIZE];

const int16_t highest_octave = 128;

const int16_t wav_sine[] = {
  -32512, -32502, -32473, -32423,
  -32356, -32265, -32160, -32031,
  -31885, -31719, -31533, -31331,
  -31106, -30864, -30605, -30324,
  -30028, -29712, -29379, -29026,
  -28658, -28272, -27868, -27449,
  -27011, -26558, -26089, -25604,
  -25103, -24588, -24056, -23512,
  -22953, -22378, -21793, -21191,
  -20579, -19954, -19316, -18667,
  -18006, -17334, -16654, -15960,
  -15259, -14548, -13828, -13100,
  -12363, -11620, -10868, -10112,
   -9347,  -8578,  -7805,  -7023,
   -6241,  -5453,  -4662,  -3868,
   -3073,  -2274,  -1474,   -674,
	 126,    929,   1729,   2527,
	3326,   4123,   4916,   5707,
	6495,   7278,   8057,   8833,
	9601,  10366,  11122,  11874,
   12618,  13353,  14082,  14802,
   15512,  16216,  16906,  17589,
   18260,  18922,  19569,  20207,
   20834,  21446,  22045,  22634,
   23206,  23765,  24311,  24842,
   25357,  25858,  26343,  26812,
   27266,  27701,  28123,  28526,
   28912,  29281,  29632,  29966,
   30281,  30579,  30859,  31118,
   31361,  31583,  31788,  31973,
   32139,  32286,  32412,  32521,
   32608,  32679,  32725,  32757,
   32766,  32757,  32725,  32679,
   32608,  32521,  32412,  32286,
   32139,  31973,  31788,  31583,
   31361,  31118,  30859,  30579,
   30281,  29966,  29632,  29281,
   28912,  28526,  28123,  27701,
   27266,  26812,  26343,  25858,
   25357,  24842,  24311,  23765,
   23206,  22634,  22045,  21446,
   20834,  20207,  19569,  18922,
   18260,  17589,  16906,  16216,
   15512,  14802,  14082,  13353,
   12618,  11874,  11122,  10366,
	9601,   8833,   8057,   7278,
	6495,   5707,   4916,   4123,
	3326,   2527,   1729,    929,
	 126,   -674,  -1474,  -2274,
   -3073,  -3868,  -4662,  -5453,
   -6241,  -7023,  -7805,  -8578,
   -9347, -10112, -10868, -11620,
  -12363, -13100, -13828, -14548,
  -15259, -15960, -16654, -17334,
  -18006, -18667, -19316, -19954,
  -20579, -21191, -21793, -22378,
  -22953, -23512, -24056, -24588,
  -25103, -25604, -26089, -26558,
  -27011, -27449, -27868, -28272,
  -28658, -29026, -29379, -29712,
  -30028, -30324, -30605, -30864,
  -31106, -31331, -31533, -31719,
  -31885, -32031, -32160, -32265,
  -32356, -32423, -32473, -32502,
  -32512,
};

void yase_sound_init(yase_sound_context* ctxt, int32_t target_sample_rate)
{

	const int32_t a4_midi = 69;
	const double a4_pitch = 440.0;
	double sample_length = 1.0 / (double)target_sample_rate;

	for (int32_t current_note_index = 0; current_note_index < LUT_OSCILLATOR_INCREMENT_SIZE; ++current_note_index)
	{
		double note = ((double)highest_octave * 128.0 + (current_note_index * 16)) - a4_midi * 128;

		double n_samples = pow(2.0, note / (12.0 * 128.0)) * a4_pitch;

		lut_oscillator_increments[current_note_index] = (int32_t)(n_samples * (double) UINT32_MAX / (double) target_sample_rate);
	}

    ctxt->phase = 0;
    ctxt->pitch = 69 * 128;
}

const int16_t pitchTableStart = 128 * 128;

uint32_t _get_phase_increment(int16_t midi_pitch)
{
    if (midi_pitch >= pitchTableStart)
    {
        midi_pitch = pitchTableStart - 1;
    }

    int32_t ref_pitch = midi_pitch - pitchTableStart;

    size_t num_shifts = 0;

    while (ref_pitch < 0)
    {
        ref_pitch += 12 * 128;
        ++num_shifts;
    }

    uint32_t a = lut_oscillator_increments[ref_pitch >> 4];
	uint32_t b = lut_oscillator_increments[(ref_pitch >> 4) + 1];

    uint32_t phase_incr = a + ((b - a) * (ref_pitch & 0x0f) >> 4);
    phase_incr >>= num_shifts;

    return phase_incr;
}

__forceinline int16_t _interpolate_8_24(const int16_t* const wave, uint32_t phase)
{
    uint32_t a = wave[phase >> 24];
    uint32_t b = wave[(phase >> 24) + 1];

    return a + ((b - a) * (uint32_t)((phase >> 8) & 0xffff) >> 16);
}

void yase_sound_render(yase_sound_context* ctxt, int16_t* buffer, size_t size)
{
    uint32_t phaseIncrement = _get_phase_increment(ctxt->pitch);
    uint32_t phase = ctxt->phase;

    while (size--)
    {
        phase += phaseIncrement;

        *buffer++ = _interpolate_8_24(wav_sine, phase);
    }

    ctxt->phase = phase;
}

