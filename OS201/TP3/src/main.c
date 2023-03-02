#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#define STACK_SIZE 4096

#define READY 1000
#define WAITING 1001
#define DEAD 1002

typedef void* coroutine_t;

struct thread {
	coroutine_t routine;
	unsigned int status;
	char* stack;
};


coroutine_t schedulingCoroutine;
struct thread* currentThread;
struct thread t1;
struct thread t2;

char stack1[STACK_SIZE];
char stack2[STACK_SIZE];
char stack3[STACK_SIZE];

void enter_coroutine(coroutine_t cr);
void switch_coroutine(coroutine_t *p_from, coroutine_t to);
coroutine_t init_coroutine(void *stack_begin, unsigned int stack_size, void (*initial_pc)(void)) {
	char *stack_end = ((char *)stack_begin)	+ stack_size;
	void **ptr = stack_end;
	ptr--;
	*ptr = 0;
	ptr--;
	*ptr = initial_pc;
	ptr--;
	*ptr = 0; //rbp
	ptr--;
	*ptr = 0; //rbx
	ptr--;
	*ptr = 0; //r12
	ptr--;
	*ptr = 0; //r13
	ptr--;
	*ptr = 0; //r14
	ptr--;
	*ptr = 0; //r15
	return ptr;
}

void yield() {
	printf("YIELD\n");
	//currentThread->status = WAITING;
	switch_coroutine(&currentThread->routine,schedulingCoroutine);
}

void testFunction1() {
	int i = 0;
	while(1==1) {
		printf("Compteur 1 : %d\n",i);
		i++;
		if (i%10==0) {
			t1.status = WAITING;
			t2.status = READY;
			yield();
		}
	}
}

void testFunction2() {
	int i = 0;
	while(1==1) {
		if (getchar() != -1) {i=0; printf("-----------------------\n");}
		printf("Compteur 2 : %d\n",i);
		i++;
		if (i%10==0) {
			t1.status = READY;
			t2.status = WAITING;
			yield();
		}
	}
}

void scheduler() {
	while (1==1) {
		if (t1.status==READY) {
			printf("Thread 1\n");
			currentThread = &t1;
			switch_coroutine(&schedulingCoroutine,currentThread->routine);
		}
		else if (t2.status==READY) {
			printf("Thread 2\n");
			currentThread = &t2;
			switch_coroutine(&schedulingCoroutine,currentThread->routine);
		} else {
			printf("Oups\n");
		}
	}
}

void thread_create(struct thread* t, void (*function)(void)) {
	t->routine = init_coroutine(t->stack,STACK_SIZE,function);
	t->status = READY;	
}

int main() {
	fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);
	schedulingCoroutine = init_coroutine(stack1,STACK_SIZE,&scheduler);
	t1.stack = stack2;
	t2.stack = stack3;
	thread_create(&t1,&testFunction1);
	thread_create(&t2,&testFunction2);
	enter_coroutine(schedulingCoroutine);
	return 0;
}