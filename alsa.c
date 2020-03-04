#include <stdio.h>
#include <alsa/asoundlib.h>

#define SAMPLE_RATE 44100
#define CHANNELS 2

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "specify an ALSA device to capture from\n");
        return 1;
    }

    int err;
    snd_pcm_t* handle;

    if ((err = snd_pcm_open(&handle, argv[1], SND_PCM_STREAM_CAPTURE, 0 /* blocking */)) < 0) {
        fprintf(stderr, "error opening device: %s\n", snd_strerror(err));
        return 1;
    }

    snd_pcm_hw_params_t* params;
    if ((err = snd_pcm_hw_params_malloc(&params)) < 0) {
        fprintf(stderr, "error allocating device parameters: %s\n", snd_strerror(err));
        return 1;
    }

    if ((err = snd_pcm_hw_params_any(handle, params)) < 0) {
        fprintf(stderr, "error initializing device parameters: %s\n", snd_strerror(err));
        return 1;
    }
    
    if ((err = snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        fprintf(stderr, "error setting access: %s\n", snd_strerror(err));
        return 1;
    }

    if ((err = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE)) < 0) {
        fprintf(stderr, "error setting format: %s\n", snd_strerror(err));
        return 1;
    }

    int direction = 0;
    if ((err = snd_pcm_hw_params_set_rate(handle, params, SAMPLE_RATE, direction)) < 0) {
        fprintf(stderr, "error setting sample rate: %s\n", snd_strerror(err));
        return 1;
    }

    if ((err = snd_pcm_hw_params_set_channels(handle, params, CHANNELS)) < 0) {
        fprintf(stderr, "error setting channels: %s\n", snd_strerror(err));
        return 1;
    }

    if ((err = snd_pcm_hw_params(handle, params)) < 0) {
        fprintf(stderr, "error setting device parameters: %s\n", snd_strerror(err));
        return 1;
    }

    snd_pcm_uframes_t frames;
    if ((err = snd_pcm_hw_params_get_period_size(params, &frames, &direction)) < 0) {
        fprintf(stderr, "error getting period size: %s\n", snd_strerror(err));
        return 1;
    }

    printf("%d frames, sub unit direction %d\n", frames, direction);

    snd_pcm_hw_params_free(params);
    snd_pcm_close(handle);

    return 0;
}