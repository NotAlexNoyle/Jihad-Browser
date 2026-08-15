#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdint.h>
static void hex(const char*t, void*p, int n){ unsigned char*b=p; printf("%-26s",t);
  for(int i=0;i<n;i++) printf("%02x%s", b[i], (i%4==3)?" ":""); printf("\n"); }
int main(void){
  pthread_mutex_t m; pthread_mutexattr_t a;
  printf("sizeof(pthread_mutex_t)=%u\n",(unsigned)sizeof m);
  memset(&m,0xEE,sizeof m);
  pthread_mutexattr_init(&a);
  pthread_mutex_init(&m,&a); hex("default-attr init",&m,sizeof m);
  memset(&m,0xEE,sizeof m);
  pthread_mutexattr_init(&a);
  pthread_mutexattr_setpshared(&a,PTHREAD_PROCESS_SHARED);
  pthread_mutex_init(&m,&a); hex("PROCESS_SHARED init",&m,sizeof m);
  /* __kind lives at byte offset 12 on 32-bit ARM */
  printf("__kind after PROCESS_SHARED init = %d (0x%x)\n",
         *(int*)((char*)&m+12), *(unsigned*)((char*)&m+12));
  pthread_mutex_lock(&m); hex("after lock",&m,sizeof m);
  pthread_mutex_unlock(&m);
  /* what does this libc do with a foreign __kind of 128 ? */
  memset(&m,0,sizeof m); *(int*)((char*)&m+12)=128;
  int r = pthread_mutex_lock(&m);
  printf("lock(mutex with __kind=128) = %d\n", r);
  if(!r) pthread_mutex_unlock(&m);
  memset(&m,0,sizeof m); *(int*)((char*)&m+12)=0;
  r = pthread_mutex_lock(&m);
  printf("lock(mutex with __kind=0)   = %d\n", r);
  if(!r) pthread_mutex_unlock(&m);
  return 0;
}
