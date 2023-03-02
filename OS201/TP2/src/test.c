#include <assert.h>
#include <stdio.h>
#include <string.h>

#define question2

#ifdef question2

#define tailleTas 4000

  unsigned int tas[tailleTas];
  unsigned int* sp = &tas[0];

  void *memalloc(size_t size) {
    if (sp+size < &tas[0]+tailleTas) {
      sp+=size;
      return sp;
    } else {
      printf("Erreur seg fault");
      return NULL;
    }
  }
  void memfree(void *ptr) {

  }
  void meminit() {
    for (unsigned int i = 0;i<tailleTas;i++) {
      tas[i]=0;
    }
  }

#endif

#ifdef question3

  #define tailleTas 16000

  struct listeChaine {
    struct listeChaine* next;
    char adress;
    int size;
  };

  char tas[tailleTas];
  char* sp = &tas[0];
  struct listeChaine* liste = &tas[0];

  void *memalloc(size_t size) {
    while (liste->size < size) {
      if (liste==NULL)
        return NULL;
      liste=liste->next;
    }
    liste->size-=size;
    liste->adress+=size;
    return liste->adress-size;
  }
  void memfree(void *ptr) {

  }
  void meminit() {
    for (unsigned int i = 0;i<tailleTas;i++) {
      tas[i]=0;
    }
    liste->next=NULL;
    liste->size=tailleTas;
    liste->adress=&tas[0];
  }

#endif

int permutation[10] = {
  7, 4, 8, 1, 2, 5, 9, 3, 0, 6
};

struct slot {
  char* ptr;
  int size;
  int free;
};

void check(struct slot* slots) {
    for (int k = 0; k<10; k++) {
      if (slots[k].free == 0) {
        for (int l=0; l<slots[k].size; l++) {
          assert(slots[k].ptr[l] == (char)slots[k].size);
        }
      }
    }
}

void alloc(struct slot* slot, int j) {
    printf("allocating slot %p of size %d\n", slot, j+1);
    slot->size = j+1;
    slot->ptr = memalloc(slot->size);
    assert(slot->ptr);
    printf("got %p\n", slot->ptr);
    memset(slot->ptr, (char)slot->size, slot->size);
    slot->free = 0;
}

void dealloc(struct slot *slot) {
  printf("freeing slot %p (ptr=%p) of size %d\n", slot, slot->ptr, slot->size);
  memfree(slot->ptr);
  slot->free = 1;
}

int main() {
  meminit();
  struct slot slots[10];
  for (int i = 0; i<10; i++) {
    int j = permutation[i];
    alloc(&slots[i], j);
  };
  printf("Allocation successful\n");
  check(slots);
  printf("Checking 1 successful\n");
  dealloc(slots);
  printf("Desallocation successful\n");
  check(slots);
  printf("Checking 2 successful\n");
  alloc(slots, permutation[0]);
  for (int i = 0; i<10; i++) {
    int j = permutation[permutation[i]];
    dealloc(&slots[j]);
    check(slots);
  }
  for (int i = 0; i<10; i++) {
    int j = permutation[i];
    alloc(&slots[i], j);
    check(slots);
    int k = permutation[j];
    if (slots[j].free) {
      alloc(&slots[j], k);
    };
    check(slots);
    if (!slots[k].free) {
      dealloc(&slots[k]);
    };
    check(slots);
  }
}





