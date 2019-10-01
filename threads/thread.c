#include <assert.h>
#include <stdlib.h>
#include <ucontext.h>
#include "thread.h"
#include "interrupt.h"

/* This is the wait queue structure */
struct wait_queue {
	
};


enum{
	READY = 0,
	RUNNING = 1,
	EXIT = 2
};
/* This is the thread control block */
struct thread {
	Tid threadID;
	int state; //ready, running or exited
	ucontext_t context;
	void* stack_address;
	int kernal;//set it to one when kernal thread is created
	int exit; //if 1, thread should exit 

};


typedef	struct qn
{
	struct qn *next;
	struct thread *node; 
}queue_node;


typedef struct queue{  //ready queue
	queue_node *head;
}thread_queue;

void enqueue(thread_queue *t, queue_node *q){ //push the node back to the queue

	queue_node *tmp;

	if(t->head == NULL){
		t->head = q;
	}

	

}







void
thread_init(void)
{
	/* your optional code here */
}

Tid
thread_id()
{
	TBD();
	return THREAD_INVALID;
}

Tid
thread_create(void (*fn) (void *), void *parg)
{
	TBD();
	return THREAD_FAILED;
}

Tid
thread_yield(Tid want_tid)
{
	TBD();
	return THREAD_FAILED;
}

void
thread_exit()
{
	TBD();
}

Tid
thread_kill(Tid tid)
{
	TBD();
	return THREAD_FAILED;
}

/*******************************************************************
 * Important: The rest of the code should be implemented in Lab 3. *
 *******************************************************************/

/* make sure to fill the wait_queue structure defined above */
struct wait_queue *
wait_queue_create()
{
	struct wait_queue *wq;

	wq = malloc(sizeof(struct wait_queue));
	assert(wq);

	TBD();

	return wq;
}

void
wait_queue_destroy(struct wait_queue *wq)
{
	TBD();
	free(wq);
}

Tid
thread_sleep(struct wait_queue *queue)
{
	TBD();
	return THREAD_FAILED;
}

/* when the 'all' parameter is 1, wakeup all threads waiting in the queue.
 * returns whether a thread was woken up on not. */
int
thread_wakeup(struct wait_queue *queue, int all)
{
	TBD();
	return 0;
}

/* suspend current thread until Thread tid exits */
Tid
thread_wait(Tid tid)
{
	TBD();
	return 0;
}

struct lock {
	/* ... Fill this in ... */
};

struct lock *
lock_create()
{
	struct lock *lock;

	lock = malloc(sizeof(struct lock));
	assert(lock);

	TBD();

	return lock;
}

void
lock_destroy(struct lock *lock)
{
	assert(lock != NULL);

	TBD();

	free(lock);
}

void
lock_acquire(struct lock *lock)
{
	assert(lock != NULL);

	TBD();
}

void
lock_release(struct lock *lock)
{
	assert(lock != NULL);

	TBD();
}

struct cv {
	/* ... Fill this in ... */
};

struct cv *
cv_create()
{
	struct cv *cv;

	cv = malloc(sizeof(struct cv));
	assert(cv);

	TBD();

	return cv;
}

void
cv_destroy(struct cv *cv)
{
	assert(cv != NULL);

	TBD();

	free(cv);
}

void
cv_wait(struct cv *cv, struct lock *lock)
{
	assert(cv != NULL);
	assert(lock != NULL);

	TBD();
}

void
cv_signal(struct cv *cv, struct lock *lock)
{
	assert(cv != NULL);
	assert(lock != NULL);

	TBD();
}

void
cv_broadcast(struct cv *cv, struct lock *lock)
{
	assert(cv != NULL);
	assert(lock != NULL);

	TBD();
}
