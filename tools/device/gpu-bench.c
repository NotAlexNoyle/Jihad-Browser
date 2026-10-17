/* Copyright 2026 NotAlexNoyle. Apache-2.0.
 *
 * gpu-bench — what the TouchPad's Adreno 220 costs per 1024x768 frame through the vendor EGL/GLES2
 * (dlopen'd from /usr/lib, no headers, no link libs: the same libs LunaSysMgr and Atlas use), and
 * what the CPU costs for the same work in software. The numbers feed the rendering-options report
 * (docs/RENDERING-OPTIONS.md): every GPU route for Jihad is "draw/composite on the GPU, then get
 * the pixels back for the card" (until LunaSysMgr takes GPU buffers), so the readback is the
 * number that decides.
 *
 *   build: $TC/arm-webos-linux-gnueabi-gcc -O2 -march=armv7-a -mfpu=neon -mfloat-abi=softfp -o gpu-bench gpu-bench.c -ldl -lrt
 *   run (device): LD_LIBRARY_PATH=/usr/lib ./gpu-bench [frames]
 */
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#ifdef __ARM_NEON__
#include <arm_neon.h>
#endif

typedef void* EGLDisplay; typedef void* EGLConfig; typedef void* EGLSurface; typedef void* EGLContext;
typedef int EGLint; typedef unsigned EGLBoolean; typedef unsigned GLenum; typedef unsigned GLuint; typedef int GLint; typedef int GLsizei; typedef float GLfloat; typedef unsigned char GLboolean; typedef unsigned GLbitfield; typedef char GLchar;
#define EGL_DEFAULT_DISPLAY ((void*)0)
#define EGL_NONE 0x3038
#define EGL_SURFACE_TYPE 0x3033
#define EGL_PBUFFER_BIT 0x0001
#define EGL_RENDERABLE_TYPE 0x3040
#define EGL_OPENGL_ES2_BIT 0x0004
#define EGL_RED_SIZE 0x3024
#define EGL_GREEN_SIZE 0x3023
#define EGL_BLUE_SIZE 0x3022
#define EGL_ALPHA_SIZE 0x3021
#define EGL_WIDTH 0x3057
#define EGL_HEIGHT 0x3056
#define EGL_CONTEXT_CLIENT_VERSION 0x3098
#define EGL_NO_CONTEXT ((void*)0)
#define EGL_NO_SURFACE ((void*)0)
#define EGL_VENDOR 0x3053
#define EGL_VERSION 0x3054
#define EGL_EXTENSIONS 0x3055
#define GL_VERSION 0x1F02
#define GL_RENDERER 0x1F01
#define GL_EXTENSIONS 0x1F03
#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_TEXTURE_2D 0x0DE1
#define GL_RGBA 0x1908
#define GL_BGRA_EXT 0x80E1
#define GL_UNSIGNED_BYTE 0x1401
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_LINEAR 0x2601
#define GL_NEAREST 0x2600
#define GL_VERTEX_SHADER 0x8B31
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_FLOAT 0x1406
#define GL_FALSE 0
#define GL_TRIANGLE_STRIP 0x0005
#define GL_BLEND 0x0BE2
#define GL_SRC_ALPHA 0x0302
#define GL_ONE_MINUS_SRC_ALPHA 0x0303
#define GL_NO_ERROR 0
#define GL_TEXTURE0 0x84C0

static void* E; static void* G;
#define F(ret, name, args) static ret (*name) args
F(EGLDisplay, eglGetDisplay, (void*)); F(EGLBoolean, eglInitialize, (EGLDisplay, EGLint*, EGLint*)); F(EGLBoolean, eglChooseConfig, (EGLDisplay, const EGLint*, EGLConfig*, EGLint, EGLint*));
F(EGLSurface, eglCreatePbufferSurface, (EGLDisplay, EGLConfig, const EGLint*)); F(EGLContext, eglCreateContext, (EGLDisplay, EGLConfig, EGLContext, const EGLint*)); F(EGLBoolean, eglMakeCurrent, (EGLDisplay, EGLSurface, EGLSurface, EGLContext));
F(const char*, eglQueryString, (EGLDisplay, EGLint)); F(EGLint, eglGetError, (void)); F(EGLBoolean, eglBindAPI, (unsigned));
F(void, glClearColor, (float, float, float, float)); F(void, glClear, (GLbitfield)); F(void, glFinish, (void)); F(void, glReadPixels, (GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, void*));
F(void, glGenTextures, (GLsizei, GLuint*)); F(void, glBindTexture, (GLenum, GLuint)); F(void, glTexImage2D, (GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const void*)); F(void, glTexSubImage2D, (GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const void*));
F(void, glTexParameteri, (GLenum, GLenum, GLint)); F(GLuint, glCreateShader, (GLenum)); F(void, glShaderSource, (GLuint, GLsizei, const GLchar**, const GLint*)); F(void, glCompileShader, (GLuint)); F(void, glGetShaderiv, (GLuint, GLenum, GLint*));
F(GLuint, glCreateProgram, (void)); F(void, glAttachShader, (GLuint, GLuint)); F(void, glLinkProgram, (GLuint)); F(void, glGetProgramiv, (GLuint, GLenum, GLint*)); F(void, glUseProgram, (GLuint)); F(GLint, glGetAttribLocation, (GLuint, const GLchar*)); F(GLint, glGetUniformLocation, (GLuint, const GLchar*));
F(void, glVertexAttribPointer, (GLuint, GLint, GLenum, GLboolean, GLsizei, const void*)); F(void, glEnableVertexAttribArray, (GLuint)); F(void, glDrawArrays, (GLenum, GLint, GLsizei)); F(void, glUniform1i, (GLint, GLint)); F(void, glUniform4f, (GLint, float, float, float, float));
F(void, glEnable, (GLenum)); F(void, glDisable, (GLenum)); F(void, glBlendFunc, (GLenum, GLenum)); F(void, glViewport, (GLint, GLint, GLsizei, GLsizei)); F(GLenum, glGetError, (void)); F(const unsigned char*, glGetString, (GLenum)); F(void, glActiveTexture, (GLenum));

#define LOAD(lib, name) do { name = dlsym(lib, #name); if (!name) { fprintf(stderr, "no %s\n", #name); return 2; } } while (0)
static double now_ms(void) { struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts); return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6; }
static GLuint shader(GLenum type, const char* src) { GLuint s = glCreateShader(type); glShaderSource(s, 1, &src, NULL); glCompileShader(s); GLint ok = 0; glGetShaderiv(s, GL_COMPILE_STATUS, &ok); if (!ok) { fprintf(stderr, "shader compile failed\n"); exit(3); } return s; }

int main(int argc, char** argv) {
  int frames = argc > 1 ? atoi(argv[1]) : 30; const int W = 1024, H = 768;
  E = dlopen("/usr/lib/libEGL.so", RTLD_NOW | RTLD_GLOBAL); if (!E) E = dlopen("libEGL.so.1", RTLD_NOW | RTLD_GLOBAL);
  G = dlopen("/usr/lib/libGLESv2.so", RTLD_NOW | RTLD_GLOBAL); if (!G) G = dlopen("libGLESv2.so.2", RTLD_NOW | RTLD_GLOBAL);
  if (!E || !G) { fprintf(stderr, "dlopen: %s\n", dlerror()); return 1; }
  LOAD(E, eglGetDisplay); LOAD(E, eglInitialize); LOAD(E, eglChooseConfig); LOAD(E, eglCreatePbufferSurface); LOAD(E, eglCreateContext); LOAD(E, eglMakeCurrent); LOAD(E, eglQueryString); LOAD(E, eglGetError); LOAD(E, eglBindAPI);
  LOAD(G, glClearColor); LOAD(G, glClear); LOAD(G, glFinish); LOAD(G, glReadPixels); LOAD(G, glGenTextures); LOAD(G, glBindTexture); LOAD(G, glTexImage2D); LOAD(G, glTexSubImage2D); LOAD(G, glTexParameteri);
  LOAD(G, glCreateShader); LOAD(G, glShaderSource); LOAD(G, glCompileShader); LOAD(G, glGetShaderiv); LOAD(G, glCreateProgram); LOAD(G, glAttachShader); LOAD(G, glLinkProgram); LOAD(G, glGetProgramiv); LOAD(G, glUseProgram);
  LOAD(G, glGetAttribLocation); LOAD(G, glGetUniformLocation); LOAD(G, glVertexAttribPointer); LOAD(G, glEnableVertexAttribArray); LOAD(G, glDrawArrays); LOAD(G, glUniform1i); LOAD(G, glUniform4f); LOAD(G, glEnable); LOAD(G, glDisable); LOAD(G, glBlendFunc); LOAD(G, glViewport); LOAD(G, glGetError); LOAD(G, glGetString); LOAD(G, glActiveTexture);

  double t0 = now_ms();
  EGLDisplay dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY); EGLint maj = 0, mn = 0;
  if (!eglInitialize(dpy, &maj, &mn)) { fprintf(stderr, "eglInitialize failed 0x%x\n", eglGetError()); return 4; }
  printf("EGL %d.%d vendor=%s version=%s\n", maj, mn, eglQueryString(dpy, EGL_VENDOR), eglQueryString(dpy, EGL_VERSION));
  printf("EGL extensions: %s\n", eglQueryString(dpy, EGL_EXTENSIONS));
  eglBindAPI(0x30A0 /* EGL_OPENGL_ES_API */);
  const EGLint cfgAttr[] = { EGL_SURFACE_TYPE, EGL_PBUFFER_BIT, EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 8, EGL_NONE };
  EGLConfig cfg; EGLint n = 0;
  if (!eglChooseConfig(dpy, cfgAttr, &cfg, 1, &n) || n < 1) { fprintf(stderr, "no pbuffer config 0x%x\n", eglGetError()); return 5; }
  const EGLint pbAttr[] = { EGL_WIDTH, W, EGL_HEIGHT, H, EGL_NONE };
  EGLSurface surf = eglCreatePbufferSurface(dpy, cfg, pbAttr); if (!surf) { fprintf(stderr, "pbuffer failed 0x%x\n", eglGetError()); return 6; }
  const EGLint ctxAttr[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
  EGLContext ctx = eglCreateContext(dpy, cfg, EGL_NO_CONTEXT, ctxAttr); if (!ctx) { fprintf(stderr, "context failed 0x%x\n", eglGetError()); return 7; }
  if (!eglMakeCurrent(dpy, surf, surf, ctx)) { fprintf(stderr, "makeCurrent failed 0x%x\n", eglGetError()); return 8; }
  printf("GL renderer=%s version=%s\n", glGetString(GL_RENDERER), glGetString(GL_VERSION));
  const char* ext = (const char*)glGetString(GL_EXTENSIONS); int bgra = ext && strstr(ext, "GL_EXT_texture_format_BGRA8888") != NULL; int readbgra = ext && strstr(ext, "GL_EXT_read_format_bgra") != NULL;
  printf("GL_EXT_texture_format_BGRA8888=%d GL_EXT_read_format_bgra=%d\n", bgra, readbgra);
  printf("setup: %.1f ms\n", now_ms() - t0);
  glViewport(0, 0, W, H);

  const char* vs = "attribute vec2 p; attribute vec2 t; varying vec2 v; void main(){ v = t; gl_Position = vec4(p, 0.0, 1.0); }";
  const char* fs = "precision mediump float; varying vec2 v; uniform sampler2D s; uniform vec4 m; void main(){ gl_FragColor = texture2D(s, v) * m; }";
  GLuint prog = glCreateProgram(); glAttachShader(prog, shader(GL_VERTEX_SHADER, vs)); glAttachShader(prog, shader(GL_FRAGMENT_SHADER, fs)); glLinkProgram(prog);
  GLint linked = 0; glGetProgramiv(prog, GL_LINK_STATUS, &linked); if (!linked) { fprintf(stderr, "link failed\n"); return 9; }
  glUseProgram(prog);
  const GLfloat quad[] = { -1, -1, 0, 0,  1, -1, 1, 0,  -1, 1, 0, 1,  1, 1, 1, 1 };
  GLint ap = glGetAttribLocation(prog, "p"), at = glGetAttribLocation(prog, "t"), um = glGetUniformLocation(prog, "m"), us = glGetUniformLocation(prog, "s");
  glVertexAttribPointer(ap, 2, GL_FLOAT, GL_FALSE, 16, quad); glEnableVertexAttribArray(ap);
  glVertexAttribPointer(at, 2, GL_FLOAT, GL_FALSE, 16, quad + 2); glEnableVertexAttribArray(at);
  glUniform1i(us, 0); glActiveTexture(GL_TEXTURE0);

  uint8_t* pix = malloc((size_t)W * H * 4); for (size_t i = 0; i < (size_t)W * H * 4; i++) pix[i] = (uint8_t)(i * 7);
  uint8_t* tile = malloc(256 * 256 * 4); memset(tile, 0x80, 256 * 256 * 4);
  uint8_t* back = malloc((size_t)W * H * 4);
  GLuint tex[4]; glGenTextures(4, tex);
  // 1. one full-frame texture upload (RGBA 1024x768 = 3 MB)
  double t = now_ms();
  for (int i = 0; i < 4; i++) { glBindTexture(GL_TEXTURE_2D, tex[i]); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, W, H, 0, GL_RGBA, GL_UNSIGNED_BYTE, pix); }
  glFinish(); printf("upload 4 x 1024x768 RGBA textures: %.1f ms (%.1f ms each)\n", now_ms() - t, (now_ms() - t) / 4);
  // 2. composite: one opaque layer per frame
  glDisable(GL_BLEND); glUniform4f(um, 1, 1, 1, 1);
  t = now_ms(); for (int f = 0; f < frames; f++) { glClear(GL_COLOR_BUFFER_BIT); glBindTexture(GL_TEXTURE_2D, tex[0]); glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); } glFinish();
  printf("composite 1 opaque full-screen layer: %.2f ms/frame\n", (now_ms() - t) / frames);
  // 3. composite: 4 full-screen layers, 3 blended (a page: content + fixed header + overlay + scrollbar-ish)
  t = now_ms(); for (int f = 0; f < frames; f++) { glClear(GL_COLOR_BUFFER_BIT); glDisable(GL_BLEND); glBindTexture(GL_TEXTURE_2D, tex[0]); glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); glUniform4f(um, 1, 1, 1, 0.5f); for (int l = 1; l < 4; l++) { glBindTexture(GL_TEXTURE_2D, tex[l]); glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); } glUniform4f(um, 1, 1, 1, 1); } glFinish();
  printf("composite 4 full-screen layers (3 blended): %.2f ms/frame\n", (now_ms() - t) / frames);
  // 4. tile upload per frame: 12 x 256x256 (what a scroll repaints under OMTC), then one draw
  glDisable(GL_BLEND);
  t = now_ms(); for (int f = 0; f < frames; f++) { glBindTexture(GL_TEXTURE_2D, tex[0]); for (int k = 0; k < 12; k++) glTexSubImage2D(GL_TEXTURE_2D, 0, (k % 4) * 256, (k / 4) * 256, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, tile); glClear(GL_COLOR_BUFFER_BIT); glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); } glFinish();
  printf("12 tile uploads (256x256, 3 MB total) + composite: %.2f ms/frame\n", (now_ms() - t) / frames);
  // 5. readback: the cost every GPU route pays until LunaSysMgr takes GPU buffers
  t = now_ms(); for (int f = 0; f < frames; f++) { glReadPixels(0, 0, W, H, GL_RGBA, GL_UNSIGNED_BYTE, back); }
  printf("glReadPixels 1024x768 RGBA: %.2f ms/frame\n", (now_ms() - t) / frames);
  if (readbgra) { t = now_ms(); for (int f = 0; f < frames; f++) { glReadPixels(0, 0, W, H, GL_BGRA_EXT, GL_UNSIGNED_BYTE, back); } printf("glReadPixels 1024x768 BGRA_EXT: %.2f ms/frame\n", (now_ms() - t) / frames); }
  // 6. a whole GPU frame as the readback routes would run it: tiles up, composite, read back
  t = now_ms(); for (int f = 0; f < frames; f++) { glBindTexture(GL_TEXTURE_2D, tex[0]); for (int k = 0; k < 12; k++) glTexSubImage2D(GL_TEXTURE_2D, 0, (k % 4) * 256, (k / 4) * 256, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, tile); glClear(GL_COLOR_BUFFER_BIT); glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); glEnable(GL_BLEND); glUniform4f(um, 1, 1, 1, 0.5f); glBindTexture(GL_TEXTURE_2D, tex[1]); glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); glDisable(GL_BLEND); glUniform4f(um, 1, 1, 1, 1); glReadPixels(0, 0, W, H, GL_RGBA, GL_UNSIGNED_BYTE, back); }
  printf("GPU frame with readback (12 tiles up + 2 layers + read): %.2f ms/frame\n", (now_ms() - t) / frames);
  printf("GL error state: 0x%x\n", glGetError());

  // CPU side, for the software route
  t = now_ms(); for (int f = 0; f < frames; f++) memcpy(back, pix, (size_t)W * H * 4); printf("CPU memcpy 3 MB: %.2f ms\n", (now_ms() - t) / frames);
  t = now_ms(); for (int f = 0; f < frames; f++) { uint32_t* d = (uint32_t*)back; const uint32_t* a = (const uint32_t*)pix; for (size_t i = 0; i < (size_t)W * H; i++) { uint32_t s = a[i], b = d[i]; uint32_t al = (s >> 24) & 0xff, ia = 255 - al; d[i] = ((((s & 0xff00ff) * al + (b & 0xff00ff) * ia) >> 8) & 0xff00ff) | ((((s & 0xff00) * al + (b & 0xff00) * ia) >> 8) & 0xff00) | 0xff000000; } }
  printf("CPU blend one 1024x768 ARGB layer over another (plain C): %.2f ms\n", (now_ms() - t) / frames);
  t = now_ms(); for (int f = 0; f < frames; f++) { const int SW = 1000; uint32_t* d = (uint32_t*)back; const uint32_t* a = (const uint32_t*)pix; for (int y = 0; y < H; y++) { const uint32_t* row = a + (size_t)(y * SW / 768 % H) * W; for (int x = 0; x < 768; x++) d[(size_t)y * W + x] = row[x * SW / 768]; } }
  printf("CPU nearest-neighbour scale 1000->768 wide, 768 rows: %.2f ms\n", (now_ms() - t) / frames);
  t = now_ms(); for (int f = 0; f < frames; f++) memset(back, f, (size_t)W * H * 4); printf("CPU memset 3 MB: %.2f ms\n", (now_ms() - t) / frames);
#ifdef __ARM_NEON__
  // the same blend with NEON intrinsics, eight pixels at a time: what option 1 (software tuning) buys
  t = now_ms();
  for (int f = 0; f < frames; f++) {
    uint8_t* d = back; const uint8_t* a = pix; size_t n = (size_t)W * H;
    for (size_t i = 0; i + 8 <= n; i += 8, d += 32, a += 32) {
      uint8x8x4_t s4 = vld4_u8(a), d4 = vld4_u8(d);
      uint8x8_t al = s4.val[3], ia = vsub_u8(vdup_n_u8(255), al);
      for (int c = 0; c < 3; c++) {
        uint16x8_t m = vmlal_u8(vmull_u8(s4.val[c], al), d4.val[c], ia);
        d4.val[c] = vshrn_n_u16(m, 8);
      }
      d4.val[3] = vdup_n_u8(255);
      vst4_u8(d, d4);
    }
  }
  printf("CPU blend one 1024x768 ARGB layer over another (NEON): %.2f ms\n", (now_ms() - t) / frames);
  // and a NEON row copy with the fit scale folded in as a plain per-row memcpy of the scaled row (upper bound for a NEON scaler)
#endif
  return 0;
}
