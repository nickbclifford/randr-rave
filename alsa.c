#include <stdio.h>
#include <alsa/asoundlib.h>

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

    printf("state %d\n", snd_pcm_state(handle));

    snd_pcm_hw_params_t* params;
    if ((err = snd_pcm_hw_params_current(handle, params)) < 0) {
        fprintf(stderr, "error getting device parameters: %s\n", snd_strerror(err));
        return 1;
    }

    snd_pcm_uframes_t frames;
    int direction;
    if ((err = snd_pcm_hw_params_get_period_size(params, &frames, &direction)) < 0) {
        fprintf(stderr, "error getting period size: %s\n", snd_strerror(err));
        return 1;
    }

    printf("%d frames, sub unit direction %d\n", frames, direction);

    snd_pcm_close(handle);

    return 0;
}