/* shmdump <key-hex> <out> : attach the SysV shared segment by key (the daemon's "painted shmid"
   value) and write its first 24 bytes of header info plus the pixel area to <out>. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
int main(int argc, char** argv) {
  if (argc < 3) return 2;
  key_t key = (key_t)strtoul(argv[1], 0, 16);
  int id = shmget(key, 0, 0);
  if (id < 0) { id = (int)strtoul(argv[1], 0, 16); printf("shmget by key failed; trying it as an id %d\n", id); }
  struct shmid_ds ds; if (shmctl(id, IPC_STAT, &ds) < 0) { perror("shmctl"); return 3; }
  unsigned char* p = (unsigned char*)shmat(id, 0, SHM_RDONLY);
  if (p == (void*)-1) { perror("shmat"); return 4; }
  printf("segment id %d key 0x%lx size %lu nattch %lu\n", id, (unsigned long)key, (unsigned long)ds.shm_segsz, (unsigned long)ds.shm_nattch);
  FILE* f = fopen(argv[2], "wb"); if (!f) return 5;
  fwrite(p, 1, ds.shm_segsz, f); fclose(f);
  /* a few pixel samples assuming a 96-byte header and 768-px rows */
  int hdr = 96; for (int i = 0; i < 6; i++) { int x = 100 + i * 120, y = 100; unsigned char* o = p + hdr + ((size_t)y * 768 + x) * 4; printf("px(%d,%d)=%u,%u,%u,%u ", x, y, o[0], o[1], o[2], o[3]); }
  printf("\n"); shmdt(p); return 0;
}
