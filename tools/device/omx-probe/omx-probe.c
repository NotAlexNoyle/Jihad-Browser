// Copyright 2026 NotAlexNoyle. Apache-2.0.
//
// omx-probe.c -- T-594 (cavekit-device-headroom.md R4): can a process like the daemon decode H.264 on the TouchPad's
// hardware decoder through OpenMAX IL, and at what cost? Loads the device's OMX core, lists its components, opens the
// AVC decoder, feeds it an Annex-B stream one access unit a buffer (SPS/PPS as codec config) and counts the decoded
// pictures it hands back; prints the wall time, the pictures a second, and this process's CPU time over the decode
// (the component's own threads run in this process, so their time counts; a separate media server's would not).
//
//   omx-probe [--list] CORE.so COMPONENT WIDTH HEIGHT FILE.h264
//     e.g. omx-probe /usr/lib/libOmxCore.so OMX.qcom.video.decoder.avc 640 360 /media/internal/clip30h264-360.h264
//   Run as the daemon runs: ./ld-2.23.so --library-path <hl>:/usr/lib:/lib ./omx-probe ...   (build.sh beside it)
//
// One line per event of note, then "RESULT frames=N wall_ms=W fps=F cpu_ms=C cpu_pct=P" (cpu_pct of one core).
#include <dlfcn.h>
#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>
#include "OMX_Core.h"
#include "OMX_Component.h"
#include "OMX_Video.h"

#define INIT(x) do { memset(&(x), 0, sizeof(x)); (x).nSize = sizeof(x); (x).nVersion.s.nVersionMajor = 1; \
                     (x).nVersion.s.nVersionMinor = 1; (x).nVersion.s.nRevision = 2; (x).nVersion.s.nStep = 0; } while (0)
#define MAXBUF 64

typedef OMX_ERRORTYPE (*InitFn)(void);
typedef OMX_ERRORTYPE (*DeinitFn)(void);
typedef OMX_ERRORTYPE (*EnumFn)(OMX_STRING, OMX_U32, OMX_U32);
typedef OMX_ERRORTYPE (*GetHandleFn)(OMX_HANDLETYPE*, OMX_STRING, OMX_PTR, OMX_CALLBACKTYPE*);
typedef OMX_ERRORTYPE (*FreeHandleFn)(OMX_HANDLETYPE);

static pthread_mutex_t mu = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
static OMX_BUFFERHEADERTYPE* inFree[MAXBUF]; static int nInFree;      // input buffers the component gave back
static OMX_BUFFERHEADERTYPE* outDone[MAXBUF]; static int nOutDone;    // output buffers filled
static int cmdDone[8];             // OMX_CommandStateSet completions by state, port disable/enable counts
static int portChanged, sawEos, errors, pictures;
static OMX_U32 outPort = 1, inPort = 0;

static double now_ms(void) { struct timeval tv; gettimeofday(&tv, NULL); return tv.tv_sec * 1e3 + tv.tv_usec / 1e3; }
static double cpu_ms(void) {
  struct rusage ru; getrusage(RUSAGE_SELF, &ru);
  return (ru.ru_utime.tv_sec + ru.ru_stime.tv_sec) * 1e3 + (ru.ru_utime.tv_usec + ru.ru_stime.tv_usec) / 1e3;
}

static OMX_ERRORTYPE onEvent(OMX_HANDLETYPE h, OMX_PTR app, OMX_EVENTTYPE e, OMX_U32 d1, OMX_U32 d2, OMX_PTR data) {
  (void)h; (void)app; (void)data;
  pthread_mutex_lock(&mu);
  if (e == OMX_EventCmdComplete && d1 == OMX_CommandStateSet && d2 < 8) cmdDone[d2]++;
  else if (e == OMX_EventCmdComplete && d1 == OMX_CommandPortDisable) cmdDone[6]++;
  else if (e == OMX_EventCmdComplete && d1 == OMX_CommandPortEnable) cmdDone[7]++;
  else if (e == OMX_EventPortSettingsChanged) { portChanged = 1; printf("event: port settings changed (port %lu, %lx)\n", (unsigned long)d1, (unsigned long)d2); }
  else if (e == OMX_EventBufferFlag && (d2 & OMX_BUFFERFLAG_EOS)) sawEos = 1;
  else if (e == OMX_EventError) { errors++; printf("event: error 0x%lx (%lu)\n", (unsigned long)d1, (unsigned long)d2); }
  pthread_cond_broadcast(&cv);
  pthread_mutex_unlock(&mu);
  return OMX_ErrorNone;
}
static OMX_ERRORTYPE onEmptyDone(OMX_HANDLETYPE h, OMX_PTR app, OMX_BUFFERHEADERTYPE* b) {
  (void)h; (void)app;
  pthread_mutex_lock(&mu); if (nInFree < MAXBUF) inFree[nInFree++] = b; pthread_cond_broadcast(&cv); pthread_mutex_unlock(&mu);
  return OMX_ErrorNone;
}
static OMX_ERRORTYPE onFillDone(OMX_HANDLETYPE h, OMX_PTR app, OMX_BUFFERHEADERTYPE* b) {
  (void)h; (void)app;
  pthread_mutex_lock(&mu);
  if (b->nFilledLen > 0) pictures++;
  if (b->nFlags & OMX_BUFFERFLAG_EOS) sawEos = 1;
  if (nOutDone < MAXBUF) outDone[nOutDone++] = b;
  pthread_cond_broadcast(&cv); pthread_mutex_unlock(&mu);
  return OMX_ErrorNone;
}

static int waitFor(int* counter, int want, int ms) {   // under no lock; returns 1 when *counter reached want
  struct timespec ts; struct timeval tv; gettimeofday(&tv, NULL);
  ts.tv_sec = tv.tv_sec + ms / 1000; ts.tv_nsec = tv.tv_usec * 1000 + (ms % 1000) * 1000000L;
  if (ts.tv_nsec >= 1000000000L) { ts.tv_sec++; ts.tv_nsec -= 1000000000L; }
  pthread_mutex_lock(&mu);
  while (*counter < want) if (pthread_cond_timedwait(&cv, &mu, &ts) == ETIMEDOUT) break;
  int ok = *counter >= want; pthread_mutex_unlock(&mu); return ok;
}

// the next NAL unit at or after *pos: its payload start (after the start code) and the next start code's position
static int nextNal(const uint8_t* d, size_t n, size_t* pos, size_t* start, size_t* end) {
  size_t i = *pos;
  while (i + 3 <= n && !(d[i] == 0 && d[i + 1] == 0 && (d[i + 2] == 1 || (i + 3 < n && d[i + 2] == 0 && d[i + 3] == 1)))) i++;
  if (i + 3 > n) return 0;
  size_t s = i + (d[i + 2] == 1 ? 3 : 4);
  size_t j = s;
  while (j + 3 <= n && !(d[j] == 0 && d[j + 1] == 0 && (d[j + 2] == 1 || (j + 3 < n && d[j + 2] == 0 && d[j + 3] == 1)))) j++;
  if (j + 3 > n) j = n;
  *start = i; *end = j; *pos = j; (void)s;
  return 1;
}

int main(int argc, char** argv) {
  int list = 0, a = 1;
  if (argc > 1 && !strcmp(argv[1], "--list")) { list = 1; a = 2; }
  if (argc - a < (list ? 1 : 5)) { fprintf(stderr, "usage: omx-probe [--list] CORE.so COMPONENT WIDTH HEIGHT FILE.h264\n"); return 2; }
  const char* corePath = argv[a];
  void* core = dlopen(corePath, RTLD_NOW | RTLD_GLOBAL);
  if (!core) { printf("FAIL dlopen %s: %s\n", corePath, dlerror()); return 1; }
  InitFn init = (InitFn)dlsym(core, "OMX_Init"); DeinitFn deinit = (DeinitFn)dlsym(core, "OMX_Deinit");
  EnumFn enumc = (EnumFn)dlsym(core, "OMX_ComponentNameEnum");
  GetHandleFn geth = (GetHandleFn)dlsym(core, "OMX_GetHandle"); FreeHandleFn freeh = (FreeHandleFn)dlsym(core, "OMX_FreeHandle");
  if (!init || !geth || !freeh) { printf("FAIL the core lacks OMX_Init/OMX_GetHandle/OMX_FreeHandle\n"); return 1; }
  OMX_ERRORTYPE r = init();
  printf("OMX_Init: 0x%x\n", (unsigned)r);
  if (enumc) for (OMX_U32 i = 0; i < 64; i++) {
    char name[OMX_MAX_STRINGNAME_SIZE];
    if (enumc(name, sizeof name, i) != OMX_ErrorNone) break;
    printf("component %lu: %s\n", (unsigned long)i, name);
  }
  if (list) { if (deinit) deinit(); return 0; }
  const char* comp = argv[a + 1]; const int W = atoi(argv[a + 2]), H = atoi(argv[a + 3]); const char* path = argv[a + 4];
  FILE* f = fopen(path, "rb"); if (!f) { printf("FAIL open %s\n", path); return 1; }
  fseek(f, 0, SEEK_END); size_t n = (size_t)ftell(f); fseek(f, 0, SEEK_SET);
  uint8_t* d = (uint8_t*)malloc(n); if (!d || fread(d, 1, n, f) != n) { printf("FAIL read\n"); return 1; } fclose(f);

  OMX_CALLBACKTYPE cb = { onEvent, onEmptyDone, onFillDone };
  OMX_HANDLETYPE h = NULL;
  r = geth(&h, (OMX_STRING)comp, NULL, &cb);
  if (r != OMX_ErrorNone || !h) { printf("FAIL OMX_GetHandle %s: 0x%x\n", comp, (unsigned)r); return 1; }
  printf("handle: %s\n", comp);
  OMX_PORT_PARAM_TYPE pp; INIT(pp);
  if (OMX_GetParameter(h, OMX_IndexParamVideoInit, &pp) == OMX_ErrorNone) { inPort = pp.nStartPortNumber; outPort = inPort + 1; }
  OMX_PARAM_PORTDEFINITIONTYPE in, out; INIT(in); INIT(out); in.nPortIndex = inPort; out.nPortIndex = outPort;
  OMX_GetParameter(h, OMX_IndexParamPortDefinition, &in);
  in.format.video.nFrameWidth = W; in.format.video.nFrameHeight = H; in.format.video.eCompressionFormat = OMX_VIDEO_CodingAVC;
  r = OMX_SetParameter(h, OMX_IndexParamPortDefinition, &in);
  printf("input port %lu: set %dx%d AVC: 0x%x\n", (unsigned long)inPort, W, H, (unsigned)r);
  OMX_GetParameter(h, OMX_IndexParamPortDefinition, &in);
  OMX_GetParameter(h, OMX_IndexParamPortDefinition, &out);
  out.format.video.nFrameWidth = W; out.format.video.nFrameHeight = H;
  OMX_SetParameter(h, OMX_IndexParamPortDefinition, &out);
  OMX_GetParameter(h, OMX_IndexParamPortDefinition, &out);
  printf("input: %lu buffers of %lu B; output: %lu buffers of %lu B, colour format 0x%x, stride %ld\n",
         (unsigned long)in.nBufferCountActual, (unsigned long)in.nBufferSize, (unsigned long)out.nBufferCountActual,
         (unsigned long)out.nBufferSize, (unsigned)out.format.video.eColorFormat, (long)out.format.video.nStride);
  OMX_BUFFERHEADERTYPE* ib[MAXBUF]; OMX_BUFFERHEADERTYPE* ob[MAXBUF];
  int nib = (int)in.nBufferCountActual, nob = (int)out.nBufferCountActual;
  if (nib > MAXBUF || nob > MAXBUF) { printf("FAIL too many buffers\n"); return 1; }
  OMX_SendCommand(h, OMX_CommandStateSet, OMX_StateIdle, NULL);
  for (int i = 0; i < nib; i++) if ((r = OMX_AllocateBuffer(h, &ib[i], inPort, NULL, in.nBufferSize)) != OMX_ErrorNone) { printf("FAIL AllocateBuffer in %d: 0x%x\n", i, (unsigned)r); return 1; }
  for (int i = 0; i < nob; i++) if ((r = OMX_AllocateBuffer(h, &ob[i], outPort, NULL, out.nBufferSize)) != OMX_ErrorNone) { printf("FAIL AllocateBuffer out %d: 0x%x\n", i, (unsigned)r); return 1; }
  if (!waitFor(&cmdDone[OMX_StateIdle], 1, 5000)) { printf("FAIL no Idle\n"); return 1; }
  OMX_SendCommand(h, OMX_CommandStateSet, OMX_StateExecuting, NULL);
  if (!waitFor(&cmdDone[OMX_StateExecuting], 1, 5000)) { printf("FAIL no Executing\n"); return 1; }
  printf("executing\n");
  pthread_mutex_lock(&mu); for (int i = 0; i < nib; i++) inFree[nInFree++] = ib[i]; pthread_mutex_unlock(&mu);
  for (int i = 0; i < nob; i++) OMX_FillThisBuffer(h, ob[i]);

  const double t0 = now_ms(), c0 = cpu_ms();
  size_t pos = 0, ns, ne; int fed = 0, configSent = 0; int eosSent = 0;
  uint8_t cfg[1024]; size_t cfgLen = 0;
  while (!eosSent) {
    // the next access unit: non-VCL NALs up to and including one slice (one slice a picture in the probe's clips)
    size_t auStart = (size_t)-1, auEnd = 0; int vcl = 0;
    while (nextNal(d, n, &pos, &ns, &ne)) {
      const size_t hdr = ns + ((d[ns + 2] == 1) ? 3 : 4); const int type = d[hdr] & 0x1f;
      if (type == 7 || type == 8) { if (!configSent && cfgLen + (ne - ns) <= sizeof cfg) { memcpy(cfg + cfgLen, d + ns, ne - ns); cfgLen += ne - ns; } continue; }
      if (auStart == (size_t)-1) auStart = ns;
      auEnd = ne;
      if (type == 1 || type == 5) { vcl = 1; break; }
    }
    // a free input buffer
    pthread_mutex_lock(&mu);
    while (nInFree == 0 && !errors) pthread_cond_wait(&cv, &mu);
    OMX_BUFFERHEADERTYPE* b = nInFree ? inFree[--nInFree] : NULL;
    pthread_mutex_unlock(&mu);
    if (!b) break;
    b->nOffset = 0; b->nFlags = 0; b->nTimeStamp = (OMX_TICKS)fed * 33333;
    if (!configSent && cfgLen) {
      memcpy(b->pBuffer, cfg, cfgLen); b->nFilledLen = cfgLen; b->nFlags = OMX_BUFFERFLAG_CODECCONFIG | OMX_BUFFERFLAG_ENDOFFRAME;
      configSent = 1;
      if (auStart != (size_t)-1) pos = auStart;   // the access unit read ahead goes in the next buffer
    } else if (vcl && auEnd - auStart <= b->nAllocLen) {
      memcpy(b->pBuffer, d + auStart, auEnd - auStart); b->nFilledLen = auEnd - auStart; b->nFlags = OMX_BUFFERFLAG_ENDOFFRAME; fed++;
    } else {
      b->nFilledLen = 0; b->nFlags = OMX_BUFFERFLAG_EOS; eosSent = 1;
    }
    if ((r = OMX_EmptyThisBuffer(h, b)) != OMX_ErrorNone) { printf("FAIL EmptyThisBuffer: 0x%x\n", (unsigned)r); break; }
    // give the filled pictures back to the decoder; a port change is reported, not followed (the probe's clip is
    // the size given, as YouTube's formats are)
    pthread_mutex_lock(&mu);
    while (nOutDone) { OMX_BUFFERHEADERTYPE* o = outDone[--nOutDone]; pthread_mutex_unlock(&mu); o->nFilledLen = 0; o->nFlags = 0; OMX_FillThisBuffer(h, o); pthread_mutex_lock(&mu); }
    pthread_mutex_unlock(&mu);
    if (portChanged && fed > 5 && pictures == 0) { printf("FAIL the output port changed and no picture came: the probe would need its reconfiguration\n"); break; }
  }
  // drain: until EOS comes out or 5 s pass
  const double tEnd = now_ms() + 5000;
  while (!sawEos && now_ms() < tEnd) {
    pthread_mutex_lock(&mu);
    while (nOutDone) { OMX_BUFFERHEADERTYPE* o = outDone[--nOutDone]; pthread_mutex_unlock(&mu); o->nFilledLen = 0; o->nFlags = 0; OMX_FillThisBuffer(h, o); pthread_mutex_lock(&mu); }
    struct timespec ts = { 0, 0 }; struct timeval tv; gettimeofday(&tv, NULL); ts.tv_sec = tv.tv_sec + 1; ts.tv_nsec = tv.tv_usec * 1000;
    pthread_cond_timedwait(&cv, &mu, &ts);
    pthread_mutex_unlock(&mu);
  }
  const double wall = now_ms() - t0, cpu = cpu_ms() - c0;
  printf("RESULT frames=%d fed=%d wall_ms=%.0f fps=%.1f cpu_ms=%.0f cpu_pct=%.0f eos=%d errors=%d\n", pictures, fed, wall,
         wall > 0 ? pictures * 1000.0 / wall : 0.0, cpu, wall > 0 ? 100.0 * cpu / wall : 0.0, sawEos, errors);
  OMX_SendCommand(h, OMX_CommandStateSet, OMX_StateIdle, NULL);
  waitFor(&cmdDone[OMX_StateIdle], 2, 5000);
  OMX_SendCommand(h, OMX_CommandStateSet, OMX_StateLoaded, NULL);
  for (int i = 0; i < nib; i++) OMX_FreeBuffer(h, inPort, ib[i]);
  for (int i = 0; i < nob; i++) OMX_FreeBuffer(h, outPort, ob[i]);
  waitFor(&cmdDone[OMX_StateLoaded], 1, 5000);
  freeh(h);
  if (deinit) deinit();
  return pictures > 0 ? 0 : 1;
}
