// Copyright 2026 NotAlexNoyle. Apache-2.0.
//
// Jihad Browser — can a process in OUR runtime environment produce sound at all?
//
// WHY THIS EXISTS. The device's Flash reaches audio by runtime dlopen: libflashplayer.so has
// no audio library in its NEEDED list and imports no audio symbol, yet /usr/lib/libasound.so.2
// appears in the plugin-container's maps the moment a SWF with a DefineSound tag plays. Its
// whole Luna service list (registerServerStatus, db/find, display/control, getLockStatus,
// getPreferences, telephony/subscribe) contains no audio service, so ALSA is the path.
//
// That makes Flash audio a property of OUR PROCESS ENVIRONMENT, not of Flash: the daemon and
// plugin-container run on a bundled glibc 2.23 under a bundled loader, while libasound, its
// pulse PCM plugin and libpulse are the device's own glibc-2.8 libraries. That is exactly the
// shape that produced the PmLogLib deadlock, so it must be measured, not assumed.
//
// This probe does what Flash does — dlopen libasound, resolve the four calls, open "default",
// write a square wave — and prints before every step, flushing each time, so the last line
// printed is the call that failed or hung. It deliberately declares the handful of snd_*
// signatures it needs instead of including <alsa/asoundlib.h>: the cross-sysroot has no ALSA
// headers, and resolving by dlsym is also precisely what the plugin does.
//
// Run it BOTH ways to bisect the environment, exactly as plugin_mime_probe.c did:
//   ./ld-2.23.so --library-path $HL ./alsa-probe      (our runtime: what Flash actually gets)
//   ./alsa-probe                                      (the device's own loader: the control)
//
// Usage: alsa-probe [device] [seconds] [hz]
#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// From <alsa/pcm.h>. Values are ABI, not headers, so quoting them here is safe.
#define SND_PCM_STREAM_PLAYBACK      0
#define SND_PCM_FORMAT_S16_LE        2
#define SND_PCM_ACCESS_RW_INTERLEAVED 3

typedef struct _snd_pcm snd_pcm_t;

static void step(const char* what)
{
  printf("[alsa-probe] -> %s\n", what);
  fflush(stdout);
}

int main(int argc, char** argv)
{
  const char* dev = (argc > 1) ? argv[1] : "default";
  double seconds  = (argc > 2) ? atof(argv[2]) : 2.0;
  double hz       = (argc > 3) ? atof(argv[3]) : 440.0;
  const unsigned rate = 11025, channels = 1;

  printf("[alsa-probe] pid=%d dev=%s %.1fs %.0fHz rate=%u ch=%u\n",
         (int)getpid(), dev, seconds, hz, rate, channels);
  fflush(stdout);

  step("dlopen libasound.so.2");
  void* h = dlopen("libasound.so.2", RTLD_NOW | RTLD_LOCAL);
  if (!h) {
    printf("[alsa-probe] dlopen FAILED: %s\n", dlerror());
    return 1;
  }
  printf("[alsa-probe] libasound handle=%p\n", h);
  fflush(stdout);

  int (*pcm_open)(snd_pcm_t**, const char*, int, int) =
      (int (*)(snd_pcm_t**, const char*, int, int))dlsym(h, "snd_pcm_open");
  int (*set_params)(snd_pcm_t*, int, int, unsigned, unsigned, int, unsigned) =
      (int (*)(snd_pcm_t*, int, int, unsigned, unsigned, int, unsigned))dlsym(h, "snd_pcm_set_params");
  long (*writei)(snd_pcm_t*, const void*, unsigned long) =
      (long (*)(snd_pcm_t*, const void*, unsigned long))dlsym(h, "snd_pcm_writei");
  int (*pcm_drain)(snd_pcm_t*) = (int (*)(snd_pcm_t*))dlsym(h, "snd_pcm_drain");
  int (*pcm_close)(snd_pcm_t*) = (int (*)(snd_pcm_t*))dlsym(h, "snd_pcm_close");
  const char* (*strerr)(int) = (const char* (*)(int))dlsym(h, "snd_strerror");

  printf("[alsa-probe] syms open=%p set_params=%p writei=%p close=%p\n",
         (void*)pcm_open, (void*)set_params, (void*)writei, (void*)pcm_close);
  fflush(stdout);
  if (!pcm_open || !set_params || !writei || !pcm_close) {
    printf("[alsa-probe] missing symbols — not the ALSA this plugin expects\n");
    return 1;
  }

  // snd_pcm_open on "default" is where the pulse PCM plugin gets loaded and connects to the
  // PulseAudio server. If PulseAudio is not running, or our glibc cannot talk to its client
  // library, this is the call that reports it.
  // Print the ACTUAL device: this line used to say "default" unconditionally, which
  // made a hw:0,0 run look like a default run in the transcript.
  { char m[128]; snprintf(m, sizeof m, "snd_pcm_open(%s, PLAYBACK)", dev); step(m); }
  snd_pcm_t* pcm = NULL;
  int rc = pcm_open(&pcm, dev, SND_PCM_STREAM_PLAYBACK, 0);
  if (rc < 0) {
    printf("[alsa-probe] snd_pcm_open FAILED rc=%d (%s)\n", rc, strerr ? strerr(rc) : "?");
    return 2;
  }
  printf("[alsa-probe] pcm=%p\n", (void*)pcm);
  fflush(stdout);

  step("snd_pcm_set_params(S16_LE, mono, 11025, 500ms latency)");
  rc = set_params(pcm, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED,
                  channels, rate, 1 /*soft_resample*/, 500000 /*latency us*/);
  if (rc < 0) {
    printf("[alsa-probe] set_params FAILED rc=%d (%s)\n", rc, strerr ? strerr(rc) : "?");
    return 3;
  }

  const unsigned long frames = (unsigned long)(rate * seconds);
  short* buf = (short*)malloc(frames * sizeof(short));
  if (!buf) return 4;
  const unsigned long period = (unsigned long)(rate / hz);
  for (unsigned long i = 0; i < frames; i++)
    buf[i] = ((i % period) < (period / 2)) ? 12000 : -12000;

  step("snd_pcm_writei (this is where sound should come out)");
  long n = writei(pcm, buf, frames);
  printf("[alsa-probe] writei -> %ld (wanted %lu)\n", n, frames);
  fflush(stdout);
  if (n < 0) printf("[alsa-probe] write error: %s\n", strerr ? strerr((int)n) : "?");

  if (pcm_drain) { step("snd_pcm_drain"); pcm_drain(pcm); }
  step("snd_pcm_close");
  pcm_close(pcm);
  free(buf);
  printf("[alsa-probe] DONE — the full playback sequence completed\n");
  fflush(stdout);
  return 0;
}
