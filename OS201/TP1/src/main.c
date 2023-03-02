#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>
#include <math.h>


struct fs_header {
	char c[8];
	uint32_t size;
	uint32_t checksum;
	char name[14];
	char next;
} __attribute__((packed));;

uint32_t read32(uint32_t ptr) {
	uint32_t res = ptr & 0xF000;
	res += ptr & 0x0F00;
	res += ptr & 0x00F0;
	res += ptr & 0x000F;
	return res; 
}

unsigned int roundUp(unsigned int c) {
	while (c % 16 != 0) {
		c++;
	}
	return c;
}

unsigned int findH(struct fs_header *p) {
	printf("%c\n",p[32]);
}

void decode(struct fs_header *p, size_t size) {
	//printf("%s\n",p->c1);
	printf("Header : %s\n",p->c);
	printf("Size : %u\n",read32(p->size));
	printf("Checksum : %u\n",read32(p->checksum));
	printf("Name : %s\n",p->name);
	printf("Next : %c\n",p->next);
	//printf("%d\n",p[0]);
}

int main(void){

  int fd = open("./files/tp1fs.romfs",O_RDONLY);
  assert(fd != -1);
  off_t fsize;
  fsize = lseek(fd,0,SEEK_END);

  printf("Size is %ld\n", fsize);
  
  void *addr = mmap(NULL, fsize, PROT_READ, MAP_SHARED, fd, 0);
  assert(addr != MAP_FAILED);
  decode(addr, fsize);
  //findH(addr);
  
  return 0;
}