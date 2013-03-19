/*
  Copyright (C) 1997-2011 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/

/* Program to load a wave file and loop playing it using SDL sound */

/* loopwaves.c is much more robust in handling WAVE files -- 
	This is only for simple WAVEs
*/
#include "SDL_config.h"

#include <android/log.h>
#include <stdio.h>
#include <stdlib.h>

#if HAVE_SIGNAL_H
#include <signal.h>
#endif

#include "SDL.h"
#include "SDL_audio.h"

#define TAG "loopwave"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)
struct
{
    SDL_AudioSpec spec;
    Uint8 *sound;               /* Pointer to wave data */
    Uint32 soundlen;            /* Length of wave data */
    int soundpos;               /* Current play position */
} wave;


/* Call this instead of exit(), so we can clean up SDL: atexit() is evil. */
static void
quit(int rc)
{
    SDL_Quit();
    exit(rc);
}


void SDLCALL
fillerup(void *unused, Uint8 * stream, int len)
{
    Uint8 *waveptr;
    int waveleft;

    /* Set up the pointers */
    waveptr = wave.sound + wave.soundpos;
    waveleft = wave.soundlen - wave.soundpos;

    /* Go! */
    while (waveleft <= len) {
        SDL_memcpy(stream, waveptr, waveleft);
        stream += waveleft;
        len -= waveleft;
        waveptr = wave.sound;
        waveleft = wave.soundlen;
        wave.soundpos = 0;
    }
    SDL_memcpy(stream, waveptr, len);
    wave.soundpos += len;
}

static int done = 0;
void
poked(int sig)
{
    done = 1;
}

int
main(int argc, char *argv[])
{
    SDL_Window* window;
    SDL_Renderer* renderer;

    /* Load the SDL library */
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) < 0) {
        LOGV("Couldn't initialize SDL: %s\n", SDL_GetError());
        return (1);
    }

      // Create the window where we will draw.
        window = SDL_CreateWindow("SDL_RenderClear",
                        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                        512, 512,
                        SDL_WINDOW_SHOWN);

        // We must call SDL_CreateRenderer in order for draw calls to affect this window.
        renderer = SDL_CreateRenderer(window, -1, 0);

        // Select the color for drawing. It is set to red here.
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

        // Clear the entire screen to our selected color.
        SDL_RenderClear(renderer);

        // Up until now everything was drawn behind the scenes.
        // This will show the new, red contents of the window.
        SDL_RenderPresent(renderer);

        // Give us time to see the window.
        SDL_Delay(5000);



    if (argv[1] == NULL) {
        argv[1] = "/mnt/sdcard/test.mp4";
    }
    /* Load the wave file into memory */
    if (SDL_LoadWAV(argv[1], &wave.spec, &wave.sound, &wave.soundlen) == NULL) {
        LOGV("Couldn't load %s: %s\n", argv[1], SDL_GetError());
        //quit(1);
    }

    wave.spec.callback = fillerup;
#if HAVE_SIGNAL_H
    /* Set the signals */
#ifdef SIGHUP
    signal(SIGHUP, poked);
#endif
    signal(SIGINT, poked);
#ifdef SIGQUIT
    signal(SIGQUIT, poked);
#endif
    signal(SIGTERM, poked);
#endif /* HAVE_SIGNAL_H */

    /* Initialize fillerup() variables */
    if (SDL_OpenAudio(&wave.spec, NULL) < 0) {
        LOGV("Couldn't open audio: %s\n", SDL_GetError());
        SDL_FreeWAV(wave.sound);
        quit(2);
    }

    LOGV("Using audio driver: %s\n", SDL_GetCurrentAudioDriver());

    /* Let the audio run */
    SDL_PauseAudio(0);
    while (!done && (SDL_GetAudioStatus() == SDL_AUDIO_PLAYING))
        SDL_Delay(1000);


    /* Clean up on signal */
    SDL_CloseAudio();
    SDL_FreeWAV(wave.sound);
    SDL_Quit();
    return (0);
}
