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
#define GL_FRAMEBUFFER 0x8D40
#define GL_RENDERBUFFER 0x8D41
#define GL_RGBA4 0x8056
#define GL_COLOR_ATTACHMENT0 0x8CE0
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
F(void, glClearColor, (float, float, float, float)); F(void, glClear, (GLbitfield)); F(void, glFinish, (void)); F(void, glFlush, (void)); F(void, glReadPixels, (GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, void*));
F(void*, eglGetProcAddress, (const char*));
F(EGLBoolean, eglQuerySurface, (EGLDisplay, EGLSurface, EGLint, EGLint*));
F(EGLBoolean, eglDestroyContext, (EGLDisplay, EGLContext));
F(EGLBoolean, eglDestroySurface, (EGLDisplay, EGLSurface));
F(void, glGenRenderbuffers, (GLsizei, GLuint*)); F(void, glBindRenderbuffer, (GLenum, GLuint)); F(void, glRenderbufferStorage, (GLenum, GLenum, GLsizei, GLsizei)); F(void, glFramebufferRenderbuffer, (GLenum, GLenum, GLenum, GLuint)); F(void, glDeleteRenderbuffers, (GLsizei, const GLuint*));
F(void, glGenFramebuffers, (GLsizei, GLuint*)); F(void, glBindFramebuffer, (GLenum, GLuint)); F(void, glFramebufferTexture2D, (GLenum, GLenum, GLenum, GLuint, GLint)); F(GLenum, glCheckFramebufferStatus, (GLenum)); F(void, glDeleteFramebuffers, (GLsizei, const GLuint*)); F(void, glDeleteTextures, (GLsizei, const GLuint*));
F(void, glGenTextures, (GLsizei, GLuint*)); F(void, glBindTexture, (GLenum, GLuint)); F(void, glTexImage2D, (GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const void*)); F(void, glTexSubImage2D, (GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const void*));
F(void, glTexParameteri, (GLenum, GLenum, GLint)); F(GLuint, glCreateShader, (GLenum)); F(void, glShaderSource, (GLuint, GLsizei, const GLchar**, const GLint*)); F(void, glCompileShader, (GLuint)); F(void, glGetShaderiv, (GLuint, GLenum, GLint*));
F(GLuint, glCreateProgram, (void)); F(void, glAttachShader, (GLuint, GLuint)); F(void, glLinkProgram, (GLuint)); F(void, glGetProgramiv, (GLuint, GLenum, GLint*)); F(void, glUseProgram, (GLuint)); F(GLint, glGetAttribLocation, (GLuint, const GLchar*)); F(GLint, glGetUniformLocation, (GLuint, const GLchar*));
F(void, glVertexAttribPointer, (GLuint, GLint, GLenum, GLboolean, GLsizei, const void*)); F(void, glEnableVertexAttribArray, (GLuint)); F(void, glDrawArrays, (GLenum, GLint, GLsizei)); F(void, glUniform1i, (GLint, GLint)); F(void, glUniform4f, (GLint, float, float, float, float));
F(void, glEnable, (GLenum)); F(void, glDisable, (GLenum)); F(void, glBlendFunc, (GLenum, GLenum)); F(void, glViewport, (GLint, GLint, GLsizei, GLsizei)); F(void, glScissor, (GLint, GLint, GLsizei, GLsizei)); F(GLenum, glGetError, (void)); F(const unsigned char*, glGetString, (GLenum)); F(void, glActiveTexture, (GLenum));

#define LOAD(lib, name) do { name = dlsym(lib, #name); if (!name) { fprintf(stderr, "no %s\n", #name); return 2; } } while (0)
static double now_ms(void) { struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts); return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6; }
static GLuint shader(GLenum type, const char* src) { GLuint s = glCreateShader(type); glShaderSource(s, 1, &src, NULL); glCompileShader(s); GLint ok = 0; glGetShaderiv(s, GL_COMPILE_STATUS, &ok); if (!ok) { fprintf(stderr, "shader compile failed\n"); exit(3); } return s; }

int main(int argc, char** argv) {
  int frames = argc > 1 ? atoi(argv[1]) : 30; const int W = 1024, H = 768;
  E = dlopen("/usr/lib/libEGL.so", RTLD_NOW | RTLD_GLOBAL); if (!E) E = dlopen("libEGL.so.1", RTLD_NOW | RTLD_GLOBAL);
  G = dlopen("/usr/lib/libGLESv2.so", RTLD_NOW | RTLD_GLOBAL); if (!G) G = dlopen("libGLESv2.so.2", RTLD_NOW | RTLD_GLOBAL);
  if (!E || !G) { fprintf(stderr, "dlopen: %s\n", dlerror()); return 1; }
  LOAD(E, eglGetDisplay); LOAD(E, eglInitialize); LOAD(E, eglChooseConfig); LOAD(E, eglCreatePbufferSurface); LOAD(E, eglCreateContext); LOAD(E, eglMakeCurrent); LOAD(E, eglQueryString); LOAD(E, eglGetError); LOAD(E, eglBindAPI);
  LOAD(G, glClearColor); LOAD(G, glClear); LOAD(G, glFinish); LOAD(G, glFlush); LOAD(G, glReadPixels); LOAD(E, eglGetProcAddress); LOAD(E, eglQuerySurface); LOAD(E, eglDestroyContext); LOAD(E, eglDestroySurface); LOAD(G, glGenRenderbuffers); LOAD(G, glBindRenderbuffer); LOAD(G, glRenderbufferStorage); LOAD(G, glFramebufferRenderbuffer); LOAD(G, glDeleteRenderbuffers); LOAD(G, glGenFramebuffers); LOAD(G, glBindFramebuffer); LOAD(G, glFramebufferTexture2D); LOAD(G, glCheckFramebufferStatus); LOAD(G, glDeleteFramebuffers); LOAD(G, glDeleteTextures); LOAD(G, glGenTextures); LOAD(G, glBindTexture); LOAD(G, glTexImage2D); LOAD(G, glTexSubImage2D); LOAD(G, glTexParameteri);
  LOAD(G, glCreateShader); LOAD(G, glShaderSource); LOAD(G, glCompileShader); LOAD(G, glGetShaderiv); LOAD(G, glCreateProgram); LOAD(G, glAttachShader); LOAD(G, glLinkProgram); LOAD(G, glGetProgramiv); LOAD(G, glUseProgram);
  LOAD(G, glGetAttribLocation); LOAD(G, glGetUniformLocation); LOAD(G, glVertexAttribPointer); LOAD(G, glEnableVertexAttribArray); LOAD(G, glDrawArrays); LOAD(G, glUniform1i); LOAD(G, glUniform4f); LOAD(G, glEnable); LOAD(G, glDisable); LOAD(G, glBlendFunc); LOAD(G, glViewport); LOAD(G, glScissor); LOAD(G, glGetError); LOAD(G, glGetString); LOAD(G, glActiveTexture);

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
  // 15. T-504: the cheapest 1x1 conversion an engine can do mid-frame (section 14): must the texture
  //     be SAMPLED by a draw, or does a clear of it through a framebuffer count? And without the
  //     glFinish, what does the 1x1 write cost? BENCH_ONLY=15.
  if (getenv("BENCH_ONLY") && atoi(getenv("BENCH_ONLY")) == 15) {
    enum { NV = 8 };
    uint8_t* src = malloc(256 * 256 * 4); memset(src, 0x80, 256 * 256 * 4);
    glDisable(GL_BLEND); glUniform4f(um, 1, 1, 1, 1);
    GLuint fb; glGenFramebuffers(1, &fb);
    const char* names[5] = { "no use (control)", "a clear through a framebuffer, glFinish", "a clear through a framebuffer, no finish",
                             "a draw sampling it, no finish", "a draw sampling it, batch: all draws then all writes" };
    for (int rep = 0; rep < 3; rep++) {
      for (int variant = 0; variant < 5; variant++) {
        GLuint tt[NV]; glGenTextures(NV, tt);
        double e1[NV], e3[NV];
        for (int k = 0; k < NV; k++) {
          glBindTexture(GL_TEXTURE_2D, tt[k]);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
          glTexParameteri(GL_TEXTURE_2D, 0x2802, 0x812F); glTexParameteri(GL_TEXTURE_2D, 0x2803, 0x812F);
          glTexImage2D(GL_TEXTURE_2D, 0, GL_BGRA_EXT, 1, 1, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, src);
        }
        const double b0 = now_ms();
        if (variant == 4) {
          for (int k = 0; k < NV; k++) { glBindTexture(GL_TEXTURE_2D, tt[k]); glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); }
        }
        for (int k = 0; k < NV; k++) {
          if (variant == 1 || variant == 2) {
            glBindFramebuffer(GL_FRAMEBUFFER, fb); glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tt[k], 0);
            glClear(GL_COLOR_BUFFER_BIT);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0); glBindFramebuffer(GL_FRAMEBUFFER, 0);
            if (variant == 1) glFinish();
          } else if (variant == 3) {
            glBindTexture(GL_TEXTURE_2D, tt[k]); glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
          }
          glBindTexture(GL_TEXTURE_2D, tt[k]);
          const double a = now_ms();
          if (variant > 0) glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_BGRA_EXT, GL_UNSIGNED_BYTE, src);
          e1[k] = now_ms() - a;
        }
        const double conv = now_ms() - b0;
        for (int k = 0; k < NV; k++) { glBindTexture(GL_TEXTURE_2D, tt[k]); glTexImage2D(GL_TEXTURE_2D, 0, GL_BGRA_EXT, 256, 256, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, src); }
        for (int k = 0; k < NV; k++) { glBindTexture(GL_TEXTURE_2D, tt[k]); glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); }
        glFinish();
        for (int k = 0; k < NV; k++) { glBindTexture(GL_TEXTURE_2D, tt[k]); const double a = now_ms(); glTexImage2D(GL_TEXTURE_2D, 0, GL_BGRA_EXT, 256, 256, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, src); e3[k] = now_ms() - a; }
        double s1 = 0, s3 = 0, m1 = 0; for (int k = 0; k < NV; k++) { s1 += e1[k]; s3 += e3[k]; if (e1[k] > m1) m1 = e1[k]; }
        printf("section 15 rep %d %s: the conversion of %d took %.1f ms (1x1 writes avg %.1f, max %.1f), the 256x256 re-upload after a draw %.1f ms avg\n",
               rep, names[variant], NV, conv, s1 / NV, m1, s3 / NV);
        glDeleteTextures(NV, tt);
      }
    }
    glDeleteFramebuffers(1, &fb);
    glFinish();
    return 0;
  }
  // 14. T-504: is the conversion (section 12) kept across a resize? If a texture converted while it
  //     is 1x1 (a 1x1 upload, a draw, a 1x1 write) stays converted when re-specified at 256x256, a
  //     new tile texture can be converted for ~1 ms instead of ~18. BENCH_ONLY=14.
  if (getenv("BENCH_ONLY") && atoi(getenv("BENCH_ONLY")) == 14) {
    enum { NV = 8 };
    uint8_t* src = malloc(256 * 256 * 4); memset(src, 0x80, 256 * 256 * 4);
    glDisable(GL_BLEND); glUniform4f(um, 1, 1, 1, 1);
    for (int rep = 0; rep < 3; rep++) {
      for (int variant = 0; variant < 3; variant++) {
        GLuint tt[NV]; glGenTextures(NV, tt);
        double e1[NV], e2[NV], e3[NV];
        for (int k = 0; k < NV; k++) {
          glBindTexture(GL_TEXTURE_2D, tt[k]);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
          glTexParameteri(GL_TEXTURE_2D, 0x2802, 0x812F); glTexParameteri(GL_TEXTURE_2D, 0x2803, 0x812F);
          glTexImage2D(GL_TEXTURE_2D, 0, GL_BGRA_EXT, 1, 1, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, src);
        }
        if (variant > 0) {   // the 1x1 conversion: a draw, a finish, then a 1x1 write
          for (int k = 0; k < NV; k++) { glBindTexture(GL_TEXTURE_2D, tt[k]); glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); }
          glFinish();
        }
        for (int k = 0; k < NV; k++) {
          glBindTexture(GL_TEXTURE_2D, tt[k]); const double a = now_ms();
          if (variant == 2) glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_BGRA_EXT, GL_UNSIGNED_BYTE, src);
          else if (variant == 1) glTexImage2D(GL_TEXTURE_2D, 0, GL_BGRA_EXT, 1, 1, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, src);
          e1[k] = now_ms() - a;
        }
        for (int k = 0; k < NV; k++) { glBindTexture(GL_TEXTURE_2D, tt[k]); const double a = now_ms(); glTexImage2D(GL_TEXTURE_2D, 0, GL_BGRA_EXT, 256, 256, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, src); e2[k] = now_ms() - a; }
        for (int k = 0; k < NV; k++) { glBindTexture(GL_TEXTURE_2D, tt[k]); glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); }
        glFinish();
        for (int k = 0; k < NV; k++) { glBindTexture(GL_TEXTURE_2D, tt[k]); const double a = now_ms(); glTexImage2D(GL_TEXTURE_2D, 0, GL_BGRA_EXT, 256, 256, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, src); e3[k] = now_ms() - a; }
        double s1 = 0, s2 = 0, s3 = 0; for (int k = 0; k < NV; k++) { s1 += e1[k]; s2 += e2[k]; s3 += e3[k]; }
        const char* names[3] = { "no 1x1 conversion", "1x1 drawn, then a 1x1 image", "1x1 drawn, then a 1x1 sub-image" };
        printf("section 14 rep %d %s: the 1x1 write %.1f ms avg, the 256x256 image %.1f ms avg, its re-upload after a draw %.1f ms avg (max %.1f)\n",
               rep, names[variant], s1 / NV, s2 / NV, s3 / NV, ({ double m = 0; for (int k = 0; k < NV; k++) if (e3[k] > m) m = e3[k]; m; }));
        glDeleteTextures(NV, tt);
      }
    }
    glFinish();
    return 0;
  }
  // 13. T-504: does any texture setup escape section 12's one-time cost (the first write after the
  //     first draw)? Per variant: 8 new textures, first upload, a draw, the timed re-upload. And
  //     T6: converted textures re-uploaded while a queued draw still reads them. BENCH_ONLY=13.
  if (getenv("BENCH_ONLY") && atoi(getenv("BENCH_ONLY")) == 13) {
    enum { NV = 8 };
    uint8_t* src = malloc(1024 * 1024 * 4); memset(src, 0x80, 1024 * 1024 * 4);
    glDisable(GL_BLEND); glUniform4f(um, 1, 1, 1, 1);
    struct { const char* name; int w, h; GLenum fmt, type; GLint filter, wrap; } v[] = {
      { "256x256 BGRA nearest repeat", 256, 256, GL_BGRA_EXT, GL_UNSIGNED_BYTE, GL_NEAREST, 0x2901 },
      { "256x256 BGRA linear clamp", 256, 256, GL_BGRA_EXT, GL_UNSIGNED_BYTE, GL_LINEAR, 0x812F },
      { "256x256 RGBA linear clamp", 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, GL_LINEAR, 0x812F },
      { "256x255 BGRA linear clamp (NPOT)", 256, 255, GL_BGRA_EXT, GL_UNSIGNED_BYTE, GL_LINEAR, 0x812F },
      { "255x256 BGRA linear clamp (NPOT)", 255, 256, GL_BGRA_EXT, GL_UNSIGNED_BYTE, GL_LINEAR, 0x812F },
      { "256x256 RGB565 linear clamp", 256, 256, 0x1907 /*RGB*/, 0x8363 /*UNSIGNED_SHORT_5_6_5*/, GL_LINEAR, 0x812F },
      { "512x512 BGRA linear clamp", 512, 512, GL_BGRA_EXT, GL_UNSIGNED_BYTE, GL_LINEAR, 0x812F },
      { "1024x1024 BGRA linear clamp", 1024, 1024, GL_BGRA_EXT, GL_UNSIGNED_BYTE, GL_LINEAR, 0x812F },
      { "64x64 BGRA linear clamp", 64, 64, GL_BGRA_EXT, GL_UNSIGNED_BYTE, GL_LINEAR, 0x812F },
    };
    for (int rep = 0; rep < 2; rep++) {
      for (unsigned vi = 0; vi < sizeof v / sizeof v[0]; vi++) {
        GLuint tt[NV]; glGenTextures(NV, tt);
        for (int k = 0; k < NV; k++) {
          glBindTexture(GL_TEXTURE_2D, tt[k]);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, v[vi].filter); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, v[vi].filter);
          glTexParameteri(GL_TEXTURE_2D, 0x2802 /*WRAP_S*/, v[vi].wrap); glTexParameteri(GL_TEXTURE_2D, 0x2803 /*WRAP_T*/, v[vi].wrap);
          glTexImage2D(GL_TEXTURE_2D, 0, v[vi].fmt, v[vi].w, v[vi].h, 0, v[vi].fmt, v[vi].type, src);
        }
        for (int k = 0; k < NV; k++) { glBindTexture(GL_TEXTURE_2D, tt[k]); glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); }
        glFinish();
        double e[NV], sum = 0;
        for (int k = 0; k < NV; k++) { glBindTexture(GL_TEXTURE_2D, tt[k]); const double a = now_ms(); glTexImage2D(GL_TEXTURE_2D, 0, v[vi].fmt, v[vi].w, v[vi].h, 0, v[vi].fmt, v[vi].type, src); e[k] = now_ms() - a; sum += e[k]; }
        const GLenum err = glGetError();
        char line[300]; int o = 0; for (int k = 0; k < NV && o < 280; k++) o += snprintf(line + o, sizeof line - o, "%s%.1f", k ? " " : "", e[k]);
        printf("section 13 rep %d %s: first re-upload after the first draw %s ms (avg %.1f)%s\n", rep, v[vi].name, line, sum / NV, err ? " GL ERROR" : "");
        if (vi == 1) {
          // T6: these are converted now; draw them (queued, not finished) and re-upload each at once
          for (int k = 0; k < NV; k++) { glBindTexture(GL_TEXTURE_2D, tt[k]); glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); }
          sum = 0; o = 0;
          for (int k = 0; k < NV; k++) { glBindTexture(GL_TEXTURE_2D, tt[k]); const double a = now_ms(); glTexImage2D(GL_TEXTURE_2D, 0, v[vi].fmt, v[vi].w, v[vi].h, 0, v[vi].fmt, v[vi].type, src); e[k] = now_ms() - a; sum += e[k]; }
          for (int k = 0; k < NV && o < 280; k++) o += snprintf(line + o, sizeof line - o, "%s%.1f", k ? " " : "", e[k]);
          printf("section 13 rep %d T6 converted textures re-uploaded under a queued draw that reads them: %s ms (avg %.1f)\n", rep, line, sum / NV);
          glFinish();
          // T7: converted, then deleted and their names regenerated: is the new name converted too?
          glDeleteTextures(NV, tt); glGenTextures(NV, tt);
          for (int k = 0; k < NV; k++) { glBindTexture(GL_TEXTURE_2D, tt[k]); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); glTexParameteri(GL_TEXTURE_2D, 0x2802, 0x812F); glTexParameteri(GL_TEXTURE_2D, 0x2803, 0x812F); glTexImage2D(GL_TEXTURE_2D, 0, GL_BGRA_EXT, 256, 256, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, src); }
          for (int k = 0; k < NV; k++) { glBindTexture(GL_TEXTURE_2D, tt[k]); glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); }
          glFinish(); sum = 0; o = 0;
          for (int k = 0; k < NV; k++) { glBindTexture(GL_TEXTURE_2D, tt[k]); const double a = now_ms(); glTexImage2D(GL_TEXTURE_2D, 0, GL_BGRA_EXT, 256, 256, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, src); e[k] = now_ms() - a; sum += e[k]; }
          for (int k = 0; k < NV && o < 280; k++) o += snprintf(line + o, sizeof line - o, "%s%.1f", k ? " " : "", e[k]);
          printf("section 13 rep %d T7 recycled names (deleted, regenerated): first re-upload after the first draw %s ms (avg %.1f)\n", rep, line, sum / NV);
        }
        glDeleteTextures(NV, tt);
      }
    }
    glFinish();
    return 0;
  }
  // 12. T-504: which re-upload waits. A texture's FIRST re-upload after its first draw cost 17-31 ms in
  //     section 11 (rep 0) and later ones ~0.7 ms; the daemon's re-uploads into kept tile textures
  //     averaged 37 ms, its first uploads into new ones 1.5 ms. Each step below on 12 new 256x256
  //     BGRA textures, the costs per call; BENCH_ONLY=12.
  if (getenv("BENCH_ONLY") && atoi(getenv("BENCH_ONLY")) == 12) {
    enum { NT = 12 };
    uint8_t* src = malloc(256 * 256 * 4); memset(src, 0x80, 256 * 256 * 4);
    glDisable(GL_BLEND); glUniform4f(um, 1, 1, 1, 1);
    GLuint tt[NT];
    #define T12_GEN() do { glGenTextures(NT, tt); for (int k = 0; k < NT; k++) { glBindTexture(GL_TEXTURE_2D, tt[k]); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); } } while (0)
    #define T12_DRAW() do { for (int k = 0; k < NT; k++) { glBindTexture(GL_TEXTURE_2D, tt[k]); glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); } glFinish(); } while (0)
    #define T12_UP(label, call) do { double e[NT], sum = 0; for (int k = 0; k < NT; k++) { glBindTexture(GL_TEXTURE_2D, tt[k]); const double a = now_ms(); call; e[k] = now_ms() - a; sum += e[k]; } \
      char line[400]; int o = 0; for (int k = 0; k < NT && o < 380; k++) o += snprintf(line + o, sizeof line - o, "%s%.1f", k ? " " : "", e[k]); \
      printf("section 12 %s: %s ms (sum %.1f)\n", label, line, sum); } while (0)
    #define T12_IMG() glTexImage2D(GL_TEXTURE_2D, 0, GL_BGRA_EXT, 256, 256, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, src)
    for (int rep = 0; rep < 2; rep++) {
      printf("section 12 --- rep %d\n", rep);
      T12_GEN(); T12_UP("T1 first upload", T12_IMG()); T12_DRAW();
      T12_UP("T1 re-upload after the first draw", T12_IMG()); T12_DRAW();
      T12_UP("T1 re-upload after the second draw", T12_IMG()); T12_DRAW();
      T12_UP("T1 re-upload after the third draw", T12_IMG());
      { const double a = now_ms(); glDeleteTextures(NT, tt); glFinish(); printf("section 12 T1 delete 12 drawn textures: %.1f ms\n", now_ms() - a); }
      T12_GEN(); T12_UP("T2 first upload", T12_IMG()); T12_UP("T2 re-upload before any draw", T12_IMG()); T12_DRAW();
      T12_UP("T2 re-upload after the first draw", T12_IMG());
      glDeleteTextures(NT, tt);
      T12_GEN(); T12_UP("T3 allocate (NULL)", glTexImage2D(GL_TEXTURE_2D, 0, GL_BGRA_EXT, 256, 256, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, NULL));
      T12_UP("T3 sub-image fill", glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 256, GL_BGRA_EXT, GL_UNSIGNED_BYTE, src)); T12_DRAW();
      T12_UP("T3 sub-image re-fill after the first draw", glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 256, GL_BGRA_EXT, GL_UNSIGNED_BYTE, src)); T12_DRAW();
      T12_UP("T3 re-upload (image) after the second draw", T12_IMG());
      glDeleteTextures(NT, tt);
      T12_GEN(); T12_UP("T4 first upload", T12_IMG()); T12_DRAW();
      { struct timespec ts = { 0, 300 * 1000000L }; nanosleep(&ts, NULL); }
      T12_UP("T4 re-upload after the first draw and 300 ms idle", T12_IMG()); T12_DRAW();
      { struct timespec ts = { 0, 300 * 1000000L }; nanosleep(&ts, NULL); }
      T12_UP("T4 re-upload after the second draw and 300 ms idle", T12_IMG());
      glDeleteTextures(NT, tt);
      T12_GEN(); T12_UP("T5 first upload", T12_IMG()); T12_DRAW();
      T12_UP("T5 a 1x1 sub-image after the first draw", glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_BGRA_EXT, GL_UNSIGNED_BYTE, src)); T12_DRAW();
      T12_UP("T5 re-upload after that", T12_IMG());
      glDeleteTextures(NT, tt);
    }
    glFinish();
    return 0;
  }
  // 11. T-504: back-to-back tile uploads, as the compositor makes them when a transaction brings new
  //     tiles (each a recycled 256x256 texture, BGRA, glTexImage2D: the Adreno takes no sub-image
  //     updates). The daemon logs 18-31 ms per upload in runs and ~1 ms in others: what does each
  //     wait for? Run with the screen still and with a card animating (LunaSysMgr on the GPU too).
  //     BENCH_ONLY=11 runs this section alone.
  if (getenv("BENCH_ONLY") && atoi(getenv("BENCH_ONLY")) == 11) {
    enum { NT = 12 };
    GLuint tt[NT]; glGenTextures(NT, tt);
    uint8_t* src = malloc(512 * 512 * 4); memset(src, 0x80, 512 * 512 * 4);
    for (int k = 0; k < NT; k++) {
      glBindTexture(GL_TEXTURE_2D, tt[k]); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_BGRA_EXT, 256, 256, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, src);
    }
    for (int i = 0; i < 4; i++) { glBindTexture(GL_TEXTURE_2D, tex[i]); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, W, H, 0, GL_RGBA, GL_UNSIGNED_BYTE, pix); }
    glFinish();
    const char* cases[7] = { "GPU idle", "after a queued 4-layer composite", "after a composite and a 160-row read",
                             "each after glFlush", "fresh texture names, GPU idle", "512x512 tiles, GPU idle", "512x512 tiles after a queued composite" };
    for (int c = 0; c < 7; c++) {
      for (int rep = 0; rep < 3; rep++) {
        glDisable(GL_BLEND); glUniform4f(um, 1, 1, 1, 1);
        for (int k = 0; k < NT; k++) { glBindTexture(GL_TEXTURE_2D, tt[k]); glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); }
        glFinish();
        if (c == 1 || c == 2 || c == 6) {
          glClear(GL_COLOR_BUFFER_BIT);
          for (int k = 0; k < 4; k++) { glBindTexture(GL_TEXTURE_2D, tex[k]); glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); if (k == 0) { glEnable(GL_BLEND); glUniform4f(um, 1, 1, 1, 0.5f); } }
          glDisable(GL_BLEND); glUniform4f(um, 1, 1, 1, 1);
        }
        if (c == 2) glReadPixels(0, 0, W, 160, GL_RGBA, GL_UNSIGNED_BYTE, back);
        const int sz = c >= 5 ? 512 : 256, nt = c >= 5 ? 3 : NT;
        double each[NT]; const double a0 = now_ms();
        for (int k = 0; k < nt; k++) {
          if (c == 3) glFlush();
          if (c == 4) { glDeleteTextures(1, &tt[k]); glGenTextures(1, &tt[k]); glBindTexture(GL_TEXTURE_2D, tt[k]); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); }
          else glBindTexture(GL_TEXTURE_2D, tt[k]);
          const double a = now_ms();
          glTexImage2D(GL_TEXTURE_2D, 0, GL_BGRA_EXT, sz, sz, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, src);
          each[k] = now_ms() - a;
        }
        const double calls = now_ms() - a0; glFinish(); const double fin = now_ms() - a0;
        char line[512]; int o = 0;
        for (int k = 0; k < nt && o < 480; k++) o += snprintf(line + o, sizeof line - o, "%s%.1f", k ? " " : "", each[k]);
        printf("section 11 %s, rep %d: %d uploads %dx%d: %s ms (calls %.1f, to glFinish %.1f)\n", cases[c], rep, nt, sz, sz, line, calls, fin);
        if (sz == 512) for (int k = 0; k < NT; k++) { glBindTexture(GL_TEXTURE_2D, tt[k]); glTexImage2D(GL_TEXTURE_2D, 0, GL_BGRA_EXT, 256, 256, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, src); }
      }
    }
    glFinish();
    return 0;
  }
  // 1. one full-frame texture upload (RGBA 1024x768 = 3 MB)
  double t = now_ms();
  for (int i = 0; i < 4; i++) { glBindTexture(GL_TEXTURE_2D, tex[i]); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, W, H, 0, GL_RGBA, GL_UNSIGNED_BYTE, pix); }
  glFinish(); printf("upload 4 x 1024x768 RGBA textures: %.1f ms (%.1f ms each)\n", now_ms() - t, (now_ms() - t) / 4);
  // 1b. a plugin-sized texture (a Flash movie's own frame): whole-texture uploads at 640x480 and 320x240
  t = now_ms(); for (int f = 0; f < frames; f++) { glBindTexture(GL_TEXTURE_2D, tex[3]); glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 640, 480, 0, GL_RGBA, GL_UNSIGNED_BYTE, pix); } glFinish();
  printf("upload 640x480 RGBA texture: %.2f ms/frame\n", (now_ms() - t) / frames);
  t = now_ms(); for (int f = 0; f < frames; f++) { glBindTexture(GL_TEXTURE_2D, tex[3]); glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 320, 240, 0, GL_RGBA, GL_UNSIGNED_BYTE, pix); } glFinish();
  printf("upload 320x240 RGBA texture: %.2f ms/frame\n", (now_ms() - t) / frames);
  glBindTexture(GL_TEXTURE_2D, tex[3]); glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, W, H, 0, GL_RGBA, GL_UNSIGNED_BYTE, pix); glFinish();
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
  // 7. the cost of ONE upload call by size, texture freshness, format and the GPU's state
  //    (T-502 run 70: the engine's uploads cost ~17 ms whatever their size, so a 256x256 tile
  //    costs what a 2.9 MB layer does and tiled layers lose)
  {
    const int sizes[3][2] = { {256, 256}, {64, 942}, {768, 942} };
    unsigned char* big = (unsigned char*)malloc(768 * 942 * 4); memset(big, 0x80, 768 * 942 * 4);
    const char* modes[5] = { "same texture, GPU idle", "fresh texture each call, GPU idle", "same texture, after a 4-layer composite (GPU busy)", "same texture, BGRA_EXT, GPU idle", "allocate (NULL) then glTexSubImage2D, GPU idle" };
    for (int mode = 0; mode < 5; mode++) {
      for (int si = 0; si < 3; si++) {
        const int w = sizes[si][0], h = sizes[si][1];
        GLuint t1; glGenTextures(1, &t1); glBindTexture(GL_TEXTURE_2D, t1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        const GLenum fmt = (mode == 3) ? GL_BGRA_EXT : GL_RGBA;
        double callSum = 0, callMax = 0, finSum = 0, finMax = 0; int n = 0;
        for (int i2 = 0; i2 < 8; i2++) {
          if (mode == 2) {
            glClear(GL_COLOR_BUFFER_BIT); glDisable(GL_BLEND); glUniform4f(um, 1, 1, 1, 1);
            for (int k = 0; k < 4; k++) { glBindTexture(GL_TEXTURE_2D, tex[k]); glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); if (k == 0) { glEnable(GL_BLEND); glUniform4f(um, 1, 1, 1, 0.5f); } }
            glDisable(GL_BLEND); glUniform4f(um, 1, 1, 1, 1);
          }
          if (mode == 1) { glDeleteTextures(1, &t1); glGenTextures(1, &t1); glBindTexture(GL_TEXTURE_2D, t1); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); }
          else glBindTexture(GL_TEXTURE_2D, t1);
          const double a = now_ms();
          if (mode == 4) { glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL); glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, big); }
          else glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, big);
          const double c = now_ms() - a;
          glFinish();
          const double f = now_ms() - a;
          if (i2 >= 2) { callSum += c; finSum += f; if (c > callMax) callMax = c; if (f > finMax) finMax = f; n++; }
        }
        const GLenum err = glGetError();
        printf("upload %dx%d (%s): call %.1f ms avg / %.1f max, to glFinish %.1f avg / %.1f max%s\n", w, h, modes[mode],
               callSum / n, callMax, finSum / n, finMax, err ? " (GL error)" : "");
        glDeleteTextures(1, &t1);
      }
    }
    free(big);
  }
  // 8. a partial composite: the 4-layer frame drawn whole, under a scissor of 160 rows, and
  //    with the viewport shrunk to those rows (T-502: does the Adreno 220 skip the rest?)
  {
    double tFull = 0, tSc = 0, tVp = 0; const int reps = 8;
    for (int rep = 0; rep < reps; rep++) {
      for (int mode = 0; mode < 3; mode++) {
        if (mode == 1) { glEnable(0x0C11 /* GL_SCISSOR_TEST */); glScissor(0, 0, W, 160); }
        if (mode == 2) { glViewport(0, 0, W, 160); }
        double a = now_ms();
        glClear(GL_COLOR_BUFFER_BIT); glDisable(GL_BLEND); glUniform4f(um, 1, 1, 1, 1);
        for (int k2 = 0; k2 < 4; k2++) { glBindTexture(GL_TEXTURE_2D, tex[k2]); glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); if (k2 == 0) { glEnable(GL_BLEND); glUniform4f(um, 1, 1, 1, 0.5f); } }
        glFinish();
        double d = now_ms() - a;
        if (mode == 0) tFull += d; else if (mode == 1) tSc += d; else tVp += d;
        glDisable(GL_BLEND); glUniform4f(um, 1, 1, 1, 1); glDisable(0x0C11); glViewport(0, 0, W, H);
      }
    }
    printf("4-layer composite to glFinish: whole %dx%d %.1f ms, scissored to 160 rows %.1f ms, viewport 160 rows %.1f ms\n", W, H, tFull / reps, tSc / reps, tVp / reps);
  }
  // 5. readback: the cost every GPU route pays until LunaSysMgr takes GPU buffers
  t = now_ms(); for (int f = 0; f < frames; f++) { glReadPixels(0, 0, W, H, GL_RGBA, GL_UNSIGNED_BYTE, back); }
  printf("glReadPixels 1024x768 RGBA: %.2f ms/frame\n", (now_ms() - t) / frames);
  // a plugin-sized readback (the damage rectangle of a Flash movie, not the whole frame)
  t = now_ms(); for (int f = 0; f < frames; f++) { glReadPixels(0, 0, 640, 480, GL_RGBA, GL_UNSIGNED_BYTE, back); }
  printf("glReadPixels 640x480 RGBA: %.2f ms/frame\n", (now_ms() - t) / frames);
  t = now_ms(); for (int f = 0; f < frames; f++) { glReadPixels(0, 0, 320, 240, GL_RGBA, GL_UNSIGNED_BYTE, back); }
  printf("glReadPixels 320x240 RGBA: %.2f ms/frame\n", (now_ms() - t) / frames);
  // T-502: the compositor renders into a TEXTURE-backed FBO (GLScreenBuffer), not the pbuffer; and
  // Gecko asks for BGRA. Read back from such an FBO, at the card's size and at the layout size the
  // spike composed (980x1202), as RGBA and as BGRA_EXT, to tell the driver's cost from Gecko's.
  {
    GLuint fbo = 0, ftex = 0; glGenFramebuffers(1, &fbo); glGenTextures(1, &ftex);
    const int sizes[][2] = { {1024, 768}, {768, 942}, {980, 1202} };
    uint8_t* big = malloc((size_t)980 * 1202 * 4);
    for (int si = 0; si < 3; si++) {
      int fw = sizes[si][0], fh = sizes[si][1];
      glBindTexture(GL_TEXTURE_2D, ftex); glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fw, fh, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glBindFramebuffer(GL_FRAMEBUFFER, fbo); glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ftex, 0);
      GLenum st = glCheckFramebufferStatus(GL_FRAMEBUFFER);
      glViewport(0, 0, fw, fh); glBindTexture(GL_TEXTURE_2D, tex[0]); glClear(GL_COLOR_BUFFER_BIT); glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); glFinish();
      t = now_ms(); for (int f = 0; f < frames; f++) { glReadPixels(0, 0, fw, fh, GL_RGBA, GL_UNSIGNED_BYTE, big); }
      printf("FBO(texture) %dx%d status=0x%x: glReadPixels RGBA %.2f ms/frame\n", fw, fh, st, (now_ms() - t) / frames);
      t = now_ms(); for (int f = 0; f < frames; f++) { glReadPixels(0, 0, fw, fh, 0x80E1 /*GL_BGRA_EXT*/, GL_UNSIGNED_BYTE, big); }
      printf("FBO(texture) %dx%d: glReadPixels BGRA_EXT %.2f ms/frame (GL error 0x%x)\n", fw, fh, (now_ms() - t) / frames, glGetError());
      t = now_ms(); for (int f = 0; f < frames; f++) { glClear(GL_COLOR_BUFFER_BIT); glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); glReadPixels(0, 0, fw, fh, GL_RGBA, GL_UNSIGNED_BYTE, big); }
      printf("FBO(texture) %dx%d: draw 1 layer + glReadPixels RGBA %.2f ms/frame\n", fw, fh, (now_ms() - t) / frames);
      // the CPU side of a direct readback: swap R/B and flip rows into a second buffer
      t = now_ms(); for (int f = 0; f < frames; f++) { for (int y = 0; y < fh; y++) { uint32_t* s = (uint32_t*)(big + (size_t)y * fw * 4); uint32_t* d = (uint32_t*)(back + (size_t)(fh - 1 - y) * fw * 4); for (int x = 0; x < fw; x++) { uint32_t v = s[x]; d[x] = (v & 0xff00ff00u) | ((v & 0xffu) << 16) | ((v >> 16) & 0xffu); } } }
      printf("CPU swizzle+flip %dx%d (plain C): %.2f ms\n", fw, fh, (now_ms() - t) / frames);
    }
    // and a RENDERBUFFER-backed FBO (no texture): the other surface a compositor could draw into
    {
      GLuint rb = 0; glGenRenderbuffers(1, &rb); glBindRenderbuffer(GL_RENDERBUFFER, rb);
      glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA4, W, H);
      GLenum e1 = glGetError();
      glRenderbufferStorage(GL_RENDERBUFFER, 0x8058 /*GL_RGBA8_OES*/, W, H);
      GLenum e2 = glGetError();
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rb);
      GLenum st = glCheckFramebufferStatus(GL_FRAMEBUFFER);
      glViewport(0, 0, W, H); glBindTexture(GL_TEXTURE_2D, tex[0]); glClear(GL_COLOR_BUFFER_BIT); glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); glFinish();
      t = now_ms(); for (int f = 0; f < frames; f++) { glReadPixels(0, 0, W, H, GL_RGBA, GL_UNSIGNED_BYTE, back); }
      printf("FBO(renderbuffer RGBA8_OES; rgba4 err 0x%x rgba8 err 0x%x) 1024x768 status=0x%x: glReadPixels RGBA %.2f ms/frame\n", e1, e2, st, (now_ms() - t) / frames);
      glDeleteRenderbuffers(1, &rb);
    }
    // T-502 round 14: does a renderbuffer FBO READ BACK what was drawn into it? (the compositor's
    // frames came back all black from one). Per format: clear to a colour, read, check; draw the
    // textured quad, read, check; and the extension that makes RGBA8 renderbuffers legal.
    {
      const char* ext = (const char*)glGetString(0x1F03 /*GL_EXTENSIONS*/);
      printf("GL_OES_rgb8_rgba8=%d GL_OES_depth24=%d\n", ext && strstr(ext, "GL_OES_rgb8_rgba8") ? 1 : 0, ext && strstr(ext, "GL_OES_depth24") ? 1 : 0);
      const struct { GLenum fmt; const char* name; } fmts[] = { {0x8058, "RGBA8_OES"}, {0x8D62, "RGB565"}, {0x8056, "RGBA4"}, {0x8057, "RGB5_A1"} };
      for (int fi = 0; fi < 4; fi++) {
        GLuint rb = 0; glGenRenderbuffers(1, &rb); glBindRenderbuffer(GL_RENDERBUFFER, rb);
        glRenderbufferStorage(GL_RENDERBUFFER, fmts[fi].fmt, W, H);
        GLenum e = glGetError();
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rb);
        GLenum st2 = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        glViewport(0, 0, W, H);
        glClearColor(1.0f, 0.5f, 0.0f, 1.0f); glClear(GL_COLOR_BUFFER_BIT); glFinish();
        memset(back, 0x11, 16);
        glReadPixels(0, 0, 4, 1, GL_RGBA, GL_UNSIGNED_BYTE, back);
        GLenum e3 = glGetError();
        int clearOk = back[0] > 200 && back[1] > 100 && back[1] < 160 && back[2] < 30;
        glBindTexture(GL_TEXTURE_2D, tex[0]); glClear(GL_COLOR_BUFFER_BIT); glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); glFinish();
        glReadPixels(W / 2, H / 2, 4, 1, GL_RGBA, GL_UNSIGNED_BYTE, back);
        int drawNonZero = (back[0] | back[1] | back[2]) != 0;
        t = now_ms(); for (int f = 0; f < 10; f++) { glReadPixels(0, 0, W, H, GL_RGBA, GL_UNSIGNED_BYTE, back); }
        printf("FBO(renderbuffer %s) storage err 0x%x status 0x%x: clear-read %s (%d,%d,%d,%d err 0x%x), textured draw read %s, read %.2f ms/frame\n",
               fmts[fi].name, e, st2, clearOk ? "OK" : "WRONG", back[0], back[1], back[2], back[3], e3, drawNonZero ? "non-zero" : "ZERO", (now_ms() - t) / 10);
        glDeleteRenderbuffers(1, &rb);
      }
      glClearColor(0, 0, 0, 1);
    }
    free(big);
    glBindFramebuffer(GL_FRAMEBUFFER, 0); glViewport(0, 0, W, H); glDeleteFramebuffers(1, &fbo); glDeleteTextures(1, &ftex);
  }
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

  // 9. T-502 round 91 (the compositor-side scroll reads ~100 rows a frame, and the first read
  //    waits 18-24 ms for the GPU): the 4-layer composite into a BAND-sized FBO (768x96) with the
  //    viewport offset so each layer's band rows land in it and nothing else is binned or
  //    sampled, against the same into a frame-sized FBO (768x942); then all four opaque (no
  //    blend state change between the draws), and one layer alone. Is a band-sized target the
  //    way past the full-frame floor?
  {
    const int BW = 768, BH = 96, FW = 768, FH = 942;
    GLuint fbo9[2], ftex9[2]; glGenFramebuffers(2, fbo9); glGenTextures(2, ftex9);
    const int sz[2][2] = { {FW, FH}, {BW, BH} };
    for (int i = 0; i < 2; i++) {
      glBindTexture(GL_TEXTURE_2D, ftex9[i]); glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sz[i][0], sz[i][1], 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glBindFramebuffer(GL_FRAMEBUFFER, fbo9[i]); glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ftex9[i], 0);
      printf("section 9 FBO %dx%d status=0x%x\n", sz[i][0], sz[i][1], glCheckFramebufferStatus(GL_FRAMEBUFFER));
    }
    uint8_t* band = malloc((size_t)FW * FH * 4);
    const int reps = 8;
    for (int variant = 0; variant < 3; variant++) {
      for (int i = 0; i < 2; i++) {
        double tDraw = 0, tRead = 0;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo9[i]);
        // the band is rows FH-BH..FH of the frame: on the band target the viewport keeps the
        // frame's size and is shifted up, so only those rows fall inside the FBO
        if (i == 0) glViewport(0, 0, FW, FH); else glViewport(0, -(FH - BH), FW, FH);
        for (int rep = 0; rep < reps + 1; rep++) {
          double a = now_ms();
          glDisable(GL_BLEND); glUniform4f(um, 1, 1, 1, 1);
          const int nl = variant == 2 ? 1 : 4;
          for (int k2 = 0; k2 < nl; k2++) {
            glBindTexture(GL_TEXTURE_2D, tex[k2]); glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            if (variant == 0 && k2 == 0) { glEnable(GL_BLEND); glUniform4f(um, 1, 1, 1, 0.5f); }
          }
          glFinish();
          double b = now_ms();
          glReadPixels(0, 0, BW, BH, GL_RGBA, GL_UNSIGNED_BYTE, band);
          double c = now_ms();
          if (rep > 0) { tDraw += b - a; tRead += c - b; }
        }
        glDisable(GL_BLEND); glUniform4f(um, 1, 1, 1, 1);
        printf("section 9 %s into %dx%d FBO (%s): draw+finish %.1f ms, read %d rows %.1f ms (err 0x%x)\n",
               variant == 0 ? "4 layers (3 blended)" : variant == 1 ? "4 opaque layers" : "1 opaque layer",
               sz[i][0], sz[i][1], i == 0 ? "whole" : "viewport offset to the band", tDraw / reps, BH, tRead / reps, glGetError());
      }
    }
    free(band); glBindFramebuffer(GL_FRAMEBUFFER, 0); glViewport(0, 0, W, H);
    glDeleteFramebuffers(2, fbo9); glDeleteTextures(2, ftex9);
  }
  // 10. T-502 round 138 (a real page's stuck sidebars are ~540 rows re-read every frame): is a
  //     narrow column cheaper to read than the full rows? A 768x942 texture FBO drawn with one
  //     opaque layer, then glReadPixels of 542 full rows, of a 170-wide column, of two 170-wide
  //     columns, and of 30 full rows, each after a glFinish
  {
    const int FW = 768, FH = 942;
    GLuint fbo10, tex10; glGenFramebuffers(1, &fbo10); glGenTextures(1, &tex10);
    glBindTexture(GL_TEXTURE_2D, tex10); glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FW, FH, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo10); glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex10, 0);
    glViewport(0, 0, FW, FH);
    uint8_t* buf = malloc((size_t)FW * FH * 4);
    const int shapes[6][4] = { {0, 400, 768, 542}, {0, 400, 170, 542}, {598, 400, 170, 542}, {0, 400, 384, 542}, {0, 0, 768, 30}, {0, 400, 768, 271} };
    const char* names[6] = { "542 full rows", "a 170x542 column at x 0", "a 170x542 column at x 598", "a 384x542 half", "30 full rows", "271 full rows" };
    for (int k = 0; k < 6; k++) {
      double tRead = 0; const int reps = 8;
      for (int rep = 0; rep < reps + 1; rep++) {
        glDisable(GL_BLEND); glBindTexture(GL_TEXTURE_2D, tex[0]); glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); glFinish();
        double a = now_ms();
        glReadPixels(shapes[k][0], shapes[k][1], shapes[k][2], shapes[k][3], GL_RGBA, GL_UNSIGNED_BYTE, buf);
        if (rep > 0) tRead += now_ms() - a;
      }
      printf("section 10 read %s: %.2f ms (err 0x%x)\n", names[k], tRead / reps, glGetError());
    }
    // the two sidebars of one frame read back to back
    double tTwo = 0;
    for (int rep = 0; rep < 9; rep++) {
      glBindTexture(GL_TEXTURE_2D, tex[0]); glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); glFinish();
      double a = now_ms();
      glReadPixels(0, 400, 170, 542, GL_RGBA, GL_UNSIGNED_BYTE, buf);
      glReadPixels(598, 400, 170, 542, GL_RGBA, GL_UNSIGNED_BYTE, buf + (size_t)170 * 542 * 4);
      if (rep > 0) tTwo += now_ms() - a;
    }
    printf("section 10 read two 170x542 columns back to back: %.2f ms\n", tTwo / 8);
    free(buf); glBindFramebuffer(GL_FRAMEBUFFER, 0); glViewport(0, 0, W, H);
    glDeleteFramebuffers(1, &fbo10); glDeleteTextures(1, &tex10);
  }
  // T-502: EGL_KHR_lock_surface (the device lists it): a lockable pbuffer can be MAPPED after
  // rendering, so the frame is read with memcpy instead of glReadPixels. Costed here with a
  // second context on a lock-capable config: render one layer, unbind, lock, copy, unlock.
  {
    typedef EGLBoolean (*PFNLOCK)(EGLDisplay, EGLSurface, const EGLint*);
    typedef EGLBoolean (*PFNUNLOCK)(EGLDisplay, EGLSurface);
    PFNLOCK lockf = (PFNLOCK)eglGetProcAddress("eglLockSurfaceKHR");
    PFNUNLOCK unlockf = (PFNUNLOCK)eglGetProcAddress("eglUnlockSurfaceKHR");
    if (!lockf || !unlockf) {
      printf("lock_surface: eglLockSurfaceKHR not exported\n");
    } else {
      const EGLint lcfg[] = { 0x3033 /*SURFACE_TYPE*/, 0x0001 /*PBUFFER_BIT*/ | 0x0080 /*LOCK_SURFACE_BIT_KHR*/,
                              0x3040 /*RENDERABLE_TYPE*/, 0x0004 /*ES2_BIT*/,
                              0x3024, 8, 0x3023, 8, 0x3022, 8, 0x3021, 8, /*R,G,B,A*/
                              0x3043 /*MATCH_FORMAT_KHR*/, 0x30C3 /*FORMAT_RGBA_8888_KHR*/, 0x3038 /*NONE*/ };
      EGLConfig lc; EGLint ln = 0;
      if (!eglChooseConfig(dpy, lcfg, &lc, 1, &ln) || ln < 1) {
        const EGLint lcfg2[] = { 0x3033, 0x0001 | 0x0080, 0x3040, 0x0004, 0x3024, 8, 0x3023, 8, 0x3022, 8, 0x3021, 8, 0x3038 };
        if (!eglChooseConfig(dpy, lcfg2, &lc, 1, &ln) || ln < 1) { printf("lock_surface: no lockable RGBA8888 pbuffer config (egl error 0x%x)\n", eglGetError()); ln = 0; }
        else printf("lock_surface: config without MATCH_FORMAT\n");
      }
      if (ln >= 1) {
        const EGLint pba[] = { 0x3057 /*WIDTH*/, W, 0x3056 /*HEIGHT*/, H, 0x3038 };
        EGLSurface ls = eglCreatePbufferSurface(dpy, lc, pba);
        const EGLint cattr[] = { 0x3098 /*CONTEXT_CLIENT_VERSION*/, 2, 0x3038 };
        EGLContext lctx = eglCreateContext(dpy, lc, EGL_NO_CONTEXT, cattr);
        if (ls == EGL_NO_SURFACE || lctx == EGL_NO_CONTEXT || !eglMakeCurrent(dpy, ls, ls, lctx)) {
          printf("lock_surface: surface/context failed (egl error 0x%x)\n", eglGetError());
        } else {
          // the same program and quad as above, in the new context
          GLuint prog2 = glCreateProgram(); glAttachShader(prog2, shader(GL_VERTEX_SHADER, vs)); glAttachShader(prog2, shader(GL_FRAGMENT_SHADER, fs)); glLinkProgram(prog2); glUseProgram(prog2);
          GLint ap2 = glGetAttribLocation(prog2, "p"), at2 = glGetAttribLocation(prog2, "t"), um2 = glGetUniformLocation(prog2, "m");
          glVertexAttribPointer(ap2, 2, GL_FLOAT, GL_FALSE, 16, quad); glEnableVertexAttribArray(ap2);
          glVertexAttribPointer(at2, 2, GL_FLOAT, GL_FALSE, 16, quad + 2); glEnableVertexAttribArray(at2);
          glUniform4f(um2, 1, 1, 1, 1); glViewport(0, 0, W, H);
          GLuint ltex; glGenTextures(1, &ltex); glBindTexture(GL_TEXTURE_2D, ltex); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
          glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, W, H, 0, GL_RGBA, GL_UNSIGNED_BYTE, pix);
          glClear(GL_COLOR_BUFFER_BIT); glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); glFinish();
          t = now_ms(); for (int f = 0; f < frames; f++) { glReadPixels(0, 0, W, H, GL_RGBA, GL_UNSIGNED_BYTE, back); }
          printf("lock_surface config: glReadPixels 1024x768 RGBA %.2f ms/frame (for comparison)\n", (now_ms() - t) / frames);
          const EGLint lattr[] = { 0x30C4 /*MAP_PRESERVE_PIXELS_KHR*/, 1, 0x30C5 /*LOCK_USAGE_HINT_KHR*/, 0x0001 /*READ_SURFACE_BIT_KHR*/, 0x3038 };
          int okc = 0; double tlock = 0, tcopy = 0, tunlock = 0; EGLint ptrv = 0, pitch = 0, origin = -1, roff = -1, boff = -1, psize = -1;
          for (int f = 0; f < frames; f++) {
            double a = now_ms();
            eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
            if (!lockf(dpy, ls, lattr)) { printf("lock_surface: eglLockSurfaceKHR failed (egl error 0x%x)\n", eglGetError()); break; }
            eglQuerySurface(dpy, ls, 0x30C6 /*BITMAP_POINTER_KHR*/, &ptrv); eglQuerySurface(dpy, ls, 0x30C7 /*BITMAP_PITCH_KHR*/, &pitch);
            eglQuerySurface(dpy, ls, 0x30C8 /*BITMAP_ORIGIN_KHR*/, &origin); eglQuerySurface(dpy, ls, 0x30C9 /*PIXEL_RED_OFFSET_KHR*/, &roff); eglQuerySurface(dpy, ls, 0x30CB /*PIXEL_BLUE_OFFSET_KHR*/, &boff); eglQuerySurface(dpy, ls, 0x3110 /*PIXEL_SIZE_KHR (lock_surface2)*/, &psize);
            double b = now_ms();
            if (ptrv && pitch > 0) { for (int y = 0; y < H; y++) memcpy(back + (size_t)y * W * 4, (uint8_t*)(intptr_t)ptrv + (size_t)y * pitch, (size_t)W * 4); }
            double c = now_ms();
            unlockf(dpy, ls); eglMakeCurrent(dpy, ls, ls, lctx);
            double d = now_ms();
            tlock += b - a; tcopy += c - b; tunlock += d - c; okc++;
          }
          if (okc) printf("lock_surface 1024x768: unbind+lock %.2f ms, memcpy rows %.2f ms, unlock+rebind %.2f ms per frame (pointer %s, pitch %d, origin 0x%x, red offset %d, blue offset %d, pixel size %d)\n",
                          tlock / okc, tcopy / okc, tunlock / okc, ptrv ? "set" : "NULL", pitch, origin, roff, boff, psize);
          // a lock after a fresh draw each time (what a frame loop would do)
          okc = 0; t = now_ms();
          for (int f = 0; f < frames; f++) {
            glClear(GL_COLOR_BUFFER_BIT); glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); glFinish();
            eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
            if (!lockf(dpy, ls, lattr)) break;
            eglQuerySurface(dpy, ls, 0x30C6, &ptrv); eglQuerySurface(dpy, ls, 0x30C7, &pitch);
            if (ptrv && pitch > 0) { for (int y = 0; y < H; y++) memcpy(back + (size_t)y * W * 4, (uint8_t*)(intptr_t)ptrv + (size_t)y * pitch, (size_t)W * 4); }
            unlockf(dpy, ls); eglMakeCurrent(dpy, ls, ls, lctx); okc++;
          }
          if (okc) printf("lock_surface 1024x768: draw 1 layer + lock + copy + unlock: %.2f ms/frame\n", (now_ms() - t) / okc);
          eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
          eglDestroyContext(dpy, lctx); eglDestroySurface(dpy, ls);
        }
      }
    }
  }
  // and a NEON row copy with the fit scale folded in as a plain per-row memcpy of the scaled row (upper bound for a NEON scaler)
#endif
  return 0;
}
