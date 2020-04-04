#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <time.h>
#include <math.h>

#include <pulse/simple.h>
#include <pulse/error.h>

#include "kissfft/kiss_fft.h"

#define SAMPLE_RATE 44100
#define CHANNELS 2

#define FRAMES_PER_PERIOD 1024

#define BUFFER_SIZE (2 * FRAMES_PER_PERIOD * sizeof(int16_t))

#define BASS_FREQ_LIMIT 200

long current_time_ms();

int main(int argc, char** argv) {
    long startingTime = current_time_ms();

    if (argc < 2) {
        fprintf(stderr, "please specify a device\n");
        return 1;
    }

    pa_sample_spec sample = {
        .format = PA_SAMPLE_S16LE,
        .rate = SAMPLE_RATE,
        .channels = CHANNELS
    };

    pa_simple* s;
    int err;
    if (!(s = pa_simple_new(NULL, "randr-rave", PA_STREAM_RECORD, argv[1], "rave", &sample, NULL, NULL, &err))) {
        fprintf(stderr, "error connecting to pulseaudio server: %s\n", pa_strerror(err));
        return 1;
    }


    int16_t* framebuffer = malloc(BUFFER_SIZE);

    // prepare FFT
    kiss_fft_cfg cfg = kiss_fft_alloc(FRAMES_PER_PERIOD, 0, NULL, NULL);

    kiss_fft_cpx* fftIn = malloc(FRAMES_PER_PERIOD * sizeof(kiss_fft_cpx));
    kiss_fft_cpx* fftOut = malloc(FRAMES_PER_PERIOD * sizeof(kiss_fft_cpx));

    for (int w = 0; w < 250; w++) {
        if (pa_simple_read(s, framebuffer, BUFFER_SIZE, &err) < 0) {
            fprintf(stderr, "error reading audio from device: %s\n", pa_strerror(err));
            return 1;
        }

        for (int i = 0; i < 2 * FRAMES_PER_PERIOD; i += 2) {
            fftIn[i].r = framebuffer[i] + framebuffer[i + 1];
            fftIn[i].i = 0;
        }

        kiss_fft(cfg, fftIn, fftOut);

        if (pa_simple_flush(s, &err) < 0) {
            fprintf(stderr, "error flushing stream: %s\n", pa_strerror(err));
            return 1;
        }

        double totalBass = 0;

        for (int i = 0; i < BASS_FREQ_LIMIT; i++) {
            kiss_fft_cpx val = fftOut[i];
            kiss_fft_scalar magnitude = sqrt((val.r * val.r) + (val.i * val.i));
            totalBass += magnitude;
        }
        
        printf("[window %d, %d ms] total bass: %f\n", w, current_time_ms() - startingTime, totalBass);
    }

    free(fftIn);
    free(fftOut);
    kiss_fft_free(cfg);
    free(framebuffer);
    pa_simple_free(s);

    return 0;
}

long current_time_ms() {
    struct timespec spec;
    clock_gettime(CLOCK_MONOTONIC, &spec);

    return (spec.tv_sec * 1000) + (spec.tv_nsec / 1000000) /* ns to ms */;
}