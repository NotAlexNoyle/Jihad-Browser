#include <stdio.h>
#include <semaphore.h>
#include <string.h>
#include <stdlib.h>
static void hex(const char*tag, void*p){ unsigned char*b=p; printf("%-22s",tag);
  for(int i=0;i<16;i++) printf("%02x%s", b[i], (i%4==3)?" ":""); printf("\n"); }
int main(int argc, char**argv){
  sem_t s;
  printf("sizeof(sem_t)=%u\n",(unsigned)sizeof(sem_t));
  sem_init(&s,1,0); hex("pshared init 0",&s);
  sem_init(&s,1,1); hex("pshared init 1",&s);
  sem_init(&s,1,2); hex("pshared init 2",&s);
  sem_post(&s);     hex("after post (was 2)",&s);
  sem_init(&s,0,1); hex("private init 1",&s);
  /* cross-read test: bytes given on argv as hex, then trywait */
  if (argc>1){
    unsigned char b[16]; memset(b,0,16);
    for(int i=0;i<16 && argv[1][2*i];i++){ char t[3]={argv[1][2*i],argv[1][2*i+1],0}; b[i]=strtoul(t,0,16); }
    memcpy(&s,b,16); hex("foreign bytes",&s);
    int r=sem_trywait(&s); int v=-1; sem_getvalue(&s,&v);
    printf("foreign: sem_trywait=%d  sem_getvalue=%d\n", r, v);
  }
  return 0;
}
