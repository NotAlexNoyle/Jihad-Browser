/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// The plugin's sound streams raised the moment they are opened (T-657, cavekit-flash.md R9).
//
// webOS's audio policy opens a stream from a process it does not know at 0 % (T-614), and the
// plugin plays through ALSA's pulse plugin as "ALSA plug-in [ld-2.23.so]". The daemon raised those
// streams from a script once a second, so a sound shorter than the wait was never heard: a spoken
// symbol of the phonemic chart, a game's click (2026-11-16: the sink read 0 over a tapped symbol,
// 7500 over the same symbol when a stream happened to be up). Here a thread subscribes to
// PulseAudio's sink-input events and sets a new plugin stream to 100 % as it appears; the script
// is the second line, run only while this watch does not stand (no libpulse, the server away).
//
// libpulse is the device's own (0.9.22), opened with dlopen as the engine's cubeb opens it; the
// build sysroot's headers are that version's too (pulse/version.h), so the types here are the
// library's own. A stream is raised at most four times a second (the policy may zero it again for an
// alert: an endless answer to that would spin), said when it opens and at most once in ten seconds after.
#ifndef JIHAD_FLASH_AUDIO_H
#define JIHAD_FLASH_AUDIO_H

#if defined(__has_include)
#  if __has_include(<pulse/pulseaudio.h>)
#    define JIHAD_HAVE_PULSE_HEADERS 1
#  endif
#endif

#ifdef JIHAD_HAVE_PULSE_HEADERS
#include <pulse/pulseaudio.h>
#include <dlfcn.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

namespace jihad {

struct FlashAudioPulse {
  pa_mainloop* (*mainloop_new)(void);
  pa_mainloop_api* (*mainloop_get_api)(pa_mainloop*);
  int (*mainloop_run)(pa_mainloop*, int*);
  void (*mainloop_quit)(pa_mainloop*, int);
  void (*mainloop_free)(pa_mainloop*);
  pa_context* (*context_new)(pa_mainloop_api*, const char*);
  int (*context_connect)(pa_context*, const char*, pa_context_flags_t, const pa_spawn_api*);
  void (*context_set_state_callback)(pa_context*, pa_context_notify_cb_t, void*);
  pa_context_state_t (*context_get_state)(pa_context*);
  void (*context_set_subscribe_callback)(pa_context*, pa_context_subscribe_cb_t, void*);
  pa_operation* (*context_subscribe)(pa_context*, pa_subscription_mask_t, pa_context_success_cb_t, void*);
  pa_operation* (*context_get_sink_input_info)(pa_context*, uint32_t, pa_sink_input_info_cb_t, void*);
  pa_operation* (*context_get_sink_input_info_list)(pa_context*, pa_sink_input_info_cb_t, void*);
  pa_operation* (*context_set_sink_input_volume)(pa_context*, uint32_t, const pa_cvolume*, pa_context_success_cb_t, void*);
  void (*context_disconnect)(pa_context*);
  void (*context_unref)(pa_context*);
  void (*operation_unref)(pa_operation*);
  const char* (*proplist_gets)(pa_proplist*, const char*);
  pa_mainloop* loop;
};

inline FlashAudioPulse& FlashAudioFns() { static FlashAudioPulse f; return f; }

inline void FlashAudioInfo(pa_context* c, const pa_sink_input_info* i, int eol, void*) {
  FlashAudioPulse& f = FlashAudioFns();
  if (eol || !i || !i->proplist) return;
  const char* app = f.proplist_gets(i->proplist, "application.name");
  if (!app || strcmp(app, "ALSA plug-in [ld-2.23.so]")) return;
  const unsigned ch = i->sample_spec.channels;
  if (ch < 1 || ch > PA_CHANNELS_MAX) return;
  bool low = false;
  for (unsigned k = 0; k < ch && k < i->volume.channels; ++k) if (i->volume.values[k] < PA_VOLUME_NORM) low = true;
  if (!low && i->volume.channels == ch) return;
  // The policy module may zero the stream again later (seen as a symbol cut short or lost, 2026-11-16), so the bound
  // is a rate, not a lifetime count: at most 4 raises a second for one stream, which a fight with another hand cannot
  // turn into a busy loop, and which never leaves a stream silent for good.
  static uint32_t sIdx[8]; static unsigned char sUsed[8]; static unsigned char sInWin[8]; static int64_t sWinMs[8];
  static int64_t sLogMs[8]; static unsigned sAgain[8]; static unsigned sNext = 0;   // this thread's alone
  struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
  const int64_t now = (int64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
  unsigned slot = 8;
  for (unsigned k = 0; k < 8; ++k) if (sUsed[k] && sIdx[k] == i->index) slot = k;
  const bool first = slot == 8;
  if (first) { slot = sNext++ % 8; sIdx[slot] = i->index; sUsed[slot] = 1; sInWin[slot] = 0; sWinMs[slot] = now; sLogMs[slot] = now; sAgain[slot] = 0; }
  if (now - sWinMs[slot] >= 1000) { sWinMs[slot] = now; sInWin[slot] = 0; }
  if (sInWin[slot] >= 4) return;
  ++sInWin[slot];
  if (!first) ++sAgain[slot];
  pa_cvolume v; memset(&v, 0, sizeof v); v.channels = (uint8_t)ch;
  for (unsigned k = 0; k < ch; ++k) v.values[k] = PA_VOLUME_NORM;
  if (pa_operation* o = f.context_set_sink_input_volume(c, i->index, &v, nullptr, nullptr)) f.operation_unref(o);
  if (first) fprintf(stderr, "[jihad-bs] flash audio: the plugin's stream %u raised to 100 %% as it opened\n", (unsigned)i->index);
  else if (sAgain[slot] == 1 || now - sLogMs[slot] >= 10000) {   // one line at the first return, then one in ten seconds
    sLogMs[slot] = now;
    fprintf(stderr, "[jihad-bs] flash audio: the plugin's stream %u found low again, raised (%u times now)\n", (unsigned)i->index, sAgain[slot]);
  }
}

inline void FlashAudioEvent(pa_context* c, pa_subscription_event_type_t t, uint32_t idx, void*) {
  FlashAudioPulse& f = FlashAudioFns();
  if ((t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK) != PA_SUBSCRIPTION_EVENT_SINK_INPUT) return;
  if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE) return;   // new, or changed (the policy's own hand)
  if (pa_operation* o = f.context_get_sink_input_info(c, idx, FlashAudioInfo, nullptr)) f.operation_unref(o);
}

// Whether the watch stands (connected and subscribed): while it does, the daemon's once-a-second script is not run
// (2026-11-17: `pactl list` every second and the level script every 400 ms cost a game most of a core).
inline volatile int& FlashAudioWatching() { static volatile int on = 0; return on; }

inline void FlashAudioState(pa_context* c, void*) {
  FlashAudioPulse& f = FlashAudioFns();
  const pa_context_state_t s = f.context_get_state(c);
  if (s == PA_CONTEXT_READY) {
    f.context_set_subscribe_callback(c, FlashAudioEvent, nullptr);
    if (pa_operation* o = f.context_subscribe(c, PA_SUBSCRIPTION_MASK_SINK_INPUT, nullptr, nullptr)) f.operation_unref(o);
    if (pa_operation* o = f.context_get_sink_input_info_list(c, FlashAudioInfo, nullptr)) f.operation_unref(o);   // those already there
    fprintf(stderr, "[jihad-bs] flash audio: watching the sound server for the plugin's streams\n");
    FlashAudioWatching() = 1;
  } else if (s == PA_CONTEXT_FAILED || s == PA_CONTEXT_TERMINATED) {
    FlashAudioWatching() = 0;
    f.mainloop_quit(f.loop, 1);
  }
}

inline void* FlashAudioThread(void*) {
  FlashAudioPulse& f = FlashAudioFns();
  void* h = dlopen("libpulse.so.0", RTLD_NOW);
  if (!h) { fprintf(stderr, "[jihad-bs] flash audio: no libpulse (%s); the once-a-second script alone\n", dlerror()); return nullptr; }
#define JIHAD_PA(field, sym) *(void**)&f.field = dlsym(h, sym); if (!f.field) { fprintf(stderr, "[jihad-bs] flash audio: libpulse lacks %s\n", sym); return nullptr; }
  JIHAD_PA(mainloop_new, "pa_mainloop_new") JIHAD_PA(mainloop_get_api, "pa_mainloop_get_api") JIHAD_PA(mainloop_run, "pa_mainloop_run")
  JIHAD_PA(mainloop_quit, "pa_mainloop_quit") JIHAD_PA(mainloop_free, "pa_mainloop_free") JIHAD_PA(context_new, "pa_context_new")
  JIHAD_PA(context_connect, "pa_context_connect") JIHAD_PA(context_set_state_callback, "pa_context_set_state_callback")
  JIHAD_PA(context_get_state, "pa_context_get_state") JIHAD_PA(context_set_subscribe_callback, "pa_context_set_subscribe_callback")
  JIHAD_PA(context_subscribe, "pa_context_subscribe") JIHAD_PA(context_get_sink_input_info, "pa_context_get_sink_input_info")
  JIHAD_PA(context_get_sink_input_info_list, "pa_context_get_sink_input_info_list")
  JIHAD_PA(context_set_sink_input_volume, "pa_context_set_sink_input_volume") JIHAD_PA(context_disconnect, "pa_context_disconnect")
  JIHAD_PA(context_unref, "pa_context_unref") JIHAD_PA(operation_unref, "pa_operation_unref") JIHAD_PA(proplist_gets, "pa_proplist_gets")
#undef JIHAD_PA
  for (;;) {                       // the server restarted or not up yet: again after a rest
    f.loop = f.mainloop_new();
    if (f.loop) {
      pa_context* c = f.context_new(f.mainloop_get_api(f.loop), "Jihad Browser");
      if (c) {
        f.context_set_state_callback(c, FlashAudioState, nullptr);
        int ret = 0;
        if (f.context_connect(c, nullptr, PA_CONTEXT_NOAUTOSPAWN, nullptr) >= 0) f.mainloop_run(f.loop, &ret);
        FlashAudioWatching() = 0;
        f.context_disconnect(c); f.context_unref(c);
      }
      f.mainloop_free(f.loop); f.loop = nullptr;
    }
    sleep(5);
  }
  return nullptr;
}

inline void StartFlashAudioWatch() {
  static bool started = false;
  if (started) return;
  started = true;
  pthread_t t;
  if (pthread_create(&t, nullptr, FlashAudioThread, nullptr) == 0) pthread_detach(t);
}

}  // namespace jihad
#else
namespace jihad { inline void StartFlashAudioWatch() {} inline volatile int& FlashAudioWatching() { static volatile int on = 0; return on; } }
#endif

#endif  // JIHAD_FLASH_AUDIO_H
