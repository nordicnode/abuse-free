/*
 *  Abuse - dark 2D side-scrolling platform game
 *  Copyright (c) 2001 Anthony Kruize <trandor@labyrinth.net.au>
 *  Copyright (c) 2005-2011 Sam Hocevar <sam@hocevar.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software Foundation,
 *  Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 */

#ifdef HAVE_SDL_MIXER
#include <SDL_mixer.h>
#endif

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "hmi.h"
#include "setup.h"
#include "sound.h"
#include "specs.h"

extern flags_struct flags;
static int sound_enabled = 0;
#ifdef HAVE_SDL_MIXER
static SDL_AudioSpec audioObtained;
#endif

//
// sound_init()
// Initialise audio
//
int sound_init(int argc, char **argv) {
#ifndef HAVE_SDL_MIXER
  printf("Sound: Disabled (SDL2_mixer not found during build)\n");
  return 0;
#else
  char *sfxdir, *datadir;
  FILE *fd = NULL;

  // Disable sound if requested.
  if (flags.nosound) {
    // User requested that sound be disabled
    printf("Sound: Disabled (-nosound)\n");
    return 0;
  }

  // Check for the sfx directory, disable sound if we can't find it.
  datadir = get_filename_prefix();
  sfxdir = (char *)malloc(strlen(datadir) + 5 + 1);
  sprintf(sfxdir, "%s/sfx/", datadir);
  if ((fd = fopen(sfxdir, "r")) == NULL) {
    // Didn't find the directory, so disable sound.
    printf("Sound: Disabled (couldn't find the sfx directory)\n");
    return 0;
  }
  free(sfxdir);

  if (Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 128) < 0) {
    printf("Sound: Unable to open audio - %s\nSound: Disabled (error)\n",
           SDL_GetError());
    return 0;
  }

  Mix_AllocateChannels(50);

  int tempChannels = 0;
  Mix_QuerySpec(&audioObtained.freq, &audioObtained.format, &tempChannels);
  audioObtained.channels = tempChannels & 0xFF;

  sound_enabled = SFX_INITIALIZED | MUSIC_INITIALIZED;

  printf("Sound: Enabled\n");

  // It's all good
  return sound_enabled;
#endif
}

//
// sound_uninit
//
// Shutdown audio and release any memory left over.
//
void sound_uninit() {
  if (!sound_enabled)
    return;

#ifdef HAVE_SDL_MIXER
  Mix_CloseAudio();
#endif
}

//
// sound_effect constructor
//
// Read in the requested .wav file.
//
sound_effect::sound_effect(char const *filename) {
#ifdef HAVE_SDL_MIXER
  if (!sound_enabled)
    return;

  jFILE fp(filename, "rb");
  if (fp.open_failure())
    return;

  void *temp_data = malloc(fp.file_size());
  fp.read(temp_data, fp.file_size());
  SDL_RWops *rw = SDL_RWFromMem(temp_data, fp.file_size());
  m_chunk = Mix_LoadWAV_RW(rw, 1);
  free(temp_data);
#endif
}

//
// sound_effect destructor
//
// Release the audio data.
//
sound_effect::~sound_effect() {
#ifdef HAVE_SDL_MIXER
  if (!sound_enabled)
    return;

  // Sound effect deletion only happens on level load, so there
  // is no problem in stopping everything. But the original playing
  // code handles the sound effects and the "playlist" differently.
  // Therefore with SDL_mixer, a sound that has not finished playing
  // on a level load will cut off in the middle. This is most noticable
  // for the button sound of the load savegame dialog.
  Mix_FadeOutGroup(-1, 100);
  while (Mix_Playing(-1))
    SDL_Delay(10);
  Mix_FreeChunk(m_chunk);
#endif
}

//
// sound_effect::play
//
// Add a new sample for playing.
// panpot defines the pan position for the sound effect.
//   0   - Completely to the right.
//   128 - Centered.
//   255 - Completely to the left.
//
void sound_effect::play(int volume, int pitch, int panpot) {
#ifdef HAVE_SDL_MIXER
  if (!sound_enabled)
    return;

  int channel = Mix_PlayChannel(-1, m_chunk, 0);
  if (channel > -1) {
    Mix_Volume(channel, volume);
    Mix_SetPanning(channel, panpot, 255 - panpot);
  }
#endif
}

// Play music using SDL_Mixer

song::song(char const *filename) {
  data = NULL;
  Name = strdup(filename);
  song_id = 0;

#ifdef HAVE_SDL_MIXER
  rw = NULL;
  music = NULL;

  char realname[255];
  strcpy(realname, get_filename_prefix());
  strcat(realname, filename);

  uint32_t data_size;
  data = load_hmi(realname, data_size);

  if (!data) {
    printf("Sound: ERROR - could not load %s\n", realname);
    return;
  }

  rw = SDL_RWFromMem(data, data_size);
  music = Mix_LoadMUS_RW(rw);

  if (!music) {
    printf("Sound: ERROR - %s while loading %s\n", Mix_GetError(), realname);
    return;
  }
#endif
}

song::~song() {
#ifdef HAVE_SDL_MIXER
  if (playing())
    stop();
  free(data);
  free(Name);

  Mix_FreeMusic(music);
  SDL_FreeRW(rw);
#endif
}

void song::play(unsigned char volume) {
  song_id = 1;

#ifdef HAVE_SDL_MIXER
  Mix_PlayMusic(this->music, 0);
  Mix_VolumeMusic(volume);
#endif
}

void song::stop(long fadeout_time) {
  song_id = 0;

#ifdef HAVE_SDL_MIXER
  Mix_FadeOutMusic(100);
#endif
}

int song::playing() {
#ifdef HAVE_SDL_MIXER
  return Mix_PlayingMusic();
#else
  return 0;
#endif
}

void song::set_volume(int volume) {
#ifdef HAVE_SDL_MIXER
  Mix_VolumeMusic(volume);
#endif
}
