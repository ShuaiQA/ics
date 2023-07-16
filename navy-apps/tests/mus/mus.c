#include <assert.h>
#include <unistd.h>
#define MUSIC_PATH "/share/music/sfx_hit.wav"
#define SAMPLES 4096

#include <NDL.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

int main() {
  NDL_Init(0);
  NDL_OpenAudio(8000, 1, 1024);
  int f = open(MUSIC_PATH, O_RDONLY);
  assert(f);
  char buf[10000];
  int size = read(f, buf, 10000);
  printf("size is %d\n", size);
  NDL_PlayAudio(buf, size);
  while (NDL_QueryAudio()) {
  }
  return 0;
}
