#include <stddef.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>

/* Copies of the cross-process structs, verbatim from the tree. */
struct BrowserOffscreenInfo {
    int bufferWidth; int bufferHeight;
    double contentZoom;
    int renderedX; int renderedY; int renderedWidth; int renderedHeight;
};
struct PMHeader { uint32_t marker1; pthread_mutex_t mutex; uint32_t marker2; };
struct BufferInfo {
    int bufferId; int bufferWidth; int bufferHeight;
    int viewportWidth; int viewportHeight; int contentsWidth; int contentsHeight;
    int scrollX; int scrollY;
    int width; int height; int stride; int xPadding; int yPadding;
};

#define S(x)  char s_##x[sizeof(x)]
#define OFF(t,f,n) char o_##n[offsetof(struct t, f)+1]

/* Emit as sizes of arrays so we can read them out of the object without running it. */
const char *tags[] = {
  "SIZEOF_BrowserOffscreenInfo", "SIZEOF_PMHeader", "SIZEOF_BufferInfo",
  "SIZEOF_pthread_mutex_t", "SIZEOF_pthread_cond_t", "SIZEOF_sem_t",
  "ALIGNOF_pthread_mutex_t", "ALIGNOF_double",
  "OFF_boi_contentZoom", "OFF_boi_renderedX", "OFF_boi_renderedHeight",
  "OFF_pmh_mutex", "OFF_pmh_marker2",
};
const unsigned long vals[] = {
  sizeof(struct BrowserOffscreenInfo), sizeof(struct PMHeader), sizeof(struct BufferInfo),
  sizeof(pthread_mutex_t), sizeof(pthread_cond_t), sizeof(sem_t),
  __alignof__(pthread_mutex_t), __alignof__(double),
  offsetof(struct BrowserOffscreenInfo, contentZoom),
  offsetof(struct BrowserOffscreenInfo, renderedX),
  offsetof(struct BrowserOffscreenInfo, renderedHeight),
  offsetof(struct PMHeader, mutex),
  offsetof(struct PMHeader, marker2),
};
