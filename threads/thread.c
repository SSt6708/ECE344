#include <assert.h>
#include <stdlib.h>
#include <ucontext.h>
#include "thread.h"
#include "interrupt.h"
#include <stdbool.h>





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
	bool yield; // see if it yield twice 
	int kernal;//set it to one when kernal thread is created
	int exit; //if 1, thread should exit 

};


typedef	struct qn
{
	struct qn *next;
	struct thread *node; 
}queue_node; //a node in the linked list


typedef struct queue{  //ready queue, the whole linked list
	queue_node *head;
}thread_queue; 


//helper functions for queue
void enqueue(thread_queue *t, queue_node *q);
struct thread* findThread(thread_queue *t, Tid wantID);
struct thread* queue_pop(thread_queue *t);
void delete_queue_node(queue_node *q);
void delete_queue_node_andthread(queue_node *q);
struct thread* dequeue_thread(thread_queue *t, Tid thread_id);
void deleteQueue (queue_node *q);



struct thread* findThread(thread_queue *t, Tid id){
	queue_node *tmp;

	tmp = t->head;
	while(tmp != NULL){

		if(tmp->node->threadID == id){
			return tmp->node;
		}
		tmp = tmp->next;

	}
	printf("Can't find this thread\n");
	return NULL;
}

void delete_queue_node(queue_node *q){
	free(q);
	
}

void delete_queue_node_andthread(queue_node *q){
	free(q->node);
	free(q);
	
}

void deleteQueue(queue_node *q){
	if(q->next!= NULL){
		deleteQueue(q->next);
	}
	free(q->node);
	free(q);
}

struct thread* queue_pop(thread_queue *t){
	queue_node *tmp = NULL;
	struct thread* first_thread = NULL;
	if(t->head != NULL){
		tmp = t->head;
		t->head = t->head->next; //update head
		first_thread = tmp->node;
		tmp->next = NULL;
	}
	delete_queue_node(tmp);
	return first_thread;
}


struct thread* dequeue_thread(thread_queue *t, Tid thread_id){

	queue_node *tmp = NULL;
	queue_node *prev = NULL;
	struct thread* thread_to_dequeue = NULL;

	if(t->head == NULL){
		return NULL;
	}

	tmp = t->head;
	prev = tmp;
	while (tmp != NULL){
		if(tmp->node->threadID == thread_id){
			thread_to_dequeue = tmp->node;
			prev->next = tmp->next;
			tmp->next = NULL;
			delete_queue_node(tmp);
			return thread_to_dequeue;
			
		}
		prev = tmp;
		tmp = tmp->next;

	}
	return NULL;
	
	
	
	
}

void printQueue(thread_queue *t){
	queue_node * q = t->head;
	while(q != NULL){
		printf("%i ->", (int)q->node->threadID);
	}
	printf("\n");
}

void enqueue(thread_queue *t, queue_node *q){ //push the node back to the last of the linked list

	queue_node *tmp;

	if(t->head == NULL){
		t->head = q;
	}else{
		tmp = t->head;

		while(tmp->next != NULL){
			tmp = tmp->next;
		}

		tmp->next = q;

	}
	q->node->state = READY; //in the ready queue
	q->next = NULL;

}




//global variables
thread_queue *ready_q;
thread_queue *exit_q;
struct thread *curr_thread; //current thread thats running 
int tidArray[THREAD_MAX_THREADS]; //set it to 1 if its taken 





void
thread_init(void)
{
	//allocate the queue
	ready_q = (thread_queue *)malloc(sizeof(thread_queue));
	exit_q = (thread_queue *)malloc(sizeof(thread_queue));

	int i;
	for(i = 0 ; i < THREAD_MAX_THREADS; i++){
		tidArray[i] = 0;
	}

	// Create first thread
	struct thread* kernalThread = (struct thread *)malloc(sizeof(struct thread));

	kernalThread->kernal = 1;
	kernalThread->threadID = 0;
	tidArray[0] = 1;

	kernalThread->yield = false;
	kernalThread->state = RUNNING;
	
	curr_thread = kernalThread;
	printQueue(ready_q);
}

void thread_stub(void (*thread_main)(void *), void *arg){
	//Tid ret;

	thread_main(arg);

	//ret = curr_thread->threadID;

	thread_exit();

}


Tid
thread_id()
{
	
	return curr_thread->threadID;
}

Tid
thread_create(void (*fn) (void *), void *parg)
{
	
	//create a new thread;
	struct thread* new_thread = (struct thread*)malloc(sizeof(struct thread));


	if(new_thread == NULL){

		fprintf(stderr, "Threads have no memory\n");
		return THREAD_NOMEMORY; //cant create new threads 
	}

	//check if theres no more thr
	int i = 0;
	while(i < THREAD_MAX_THREADS && tidArray[i] == 1){
		i++;
	}

	if(i == THREAD_MAX_THREADS){
		fprintf(stderr, "No more threads can be created");
		return THREAD_NOMORE;
	}else{ //there's still space
		new_thread->threadID = i;
		tidArray[i] = 1;
	}

	getcontext(&new_thread->context); //sets the new threads contex to the current contex 

	new_thread->context.uc_mcontext.gregs[REG_RIP] = (long long int)&thread_stub;
	//initialize the argument pointer to the new thread 
	new_thread->context.uc_mcontext.gregs[REG_RDI] = (long long int) fn;
	new_thread->context.uc_mcontext.gregs[REG_RSI] = (long long int)parg;

	new_thread->kernal = 0; //not a kernal thread
	new_thread->state = READY;
	new_thread->yield = false;
	//allocate new stack ptr

	void * stack_ptr =  malloc(sizeof(THREAD_MIN_STACK));
	
	if(stack_ptr == NULL){
		free(new_thread);

		return THREAD_NOMEMORY;
	
	}

	new_thread->context.uc_stack.ss_sp = stack_ptr;
	new_thread->context.uc_mcontext.gregs[REG_RSP] = (long long int) stack_ptr + THREAD_MIN_STACK -8;
	new_thread->context.uc_stack.ss_size = THREAD_MIN_STACK - 8;

	new_thread->stack_address = stack_ptr;
	new_thread->exit = 0;

	//puts it in the ready queue
	queue_node *tmp;
	tmp = (queue_node*)malloc(sizeof(queue_node));
	tmp->next = NULL;
	tmp->node = new_thread;

	enqueue(ready_q, tmp);



	return new_thread->threadID;
}

Tid
thread_yield(Tid want_tid)
{
	Tid ret;
	printQueue(ready_q);
	if(want_tid >= -2 && want_tid < THREAD_MAX_THREADS){ // make sure the id is within the range 
		printf("see if want_tid: %d\n", want_tid);
		
		if(ready_q->head == NULL){
			printf("Nothing in ready q\n");
		}else{
			printf("theres something\n");
		}
		
		if(want_tid == THREAD_ANY && ready_q->head == NULL){
			printf("im here\n");
			return THREAD_NONE;
		}	


		if(want_tid > 0 && tidArray[want_tid] == 0){
			return THREAD_INVALID;
		}


	}else{
		return THREAD_INVALID;
	}


	queue_node* curr = (queue_node*)malloc(sizeof(queue_node));
	curr->node = curr_thread;
	printf("the first id is: %d\n", curr_thread->threadID);
	enqueue(ready_q, curr); //push back to the end of the queue
	// put the current 

	getcontext(&curr->node->context); //store the current thread

	if(curr_thread->yield == false){ 

		curr_thread->yield = true;
		if(want_tid == THREAD_ANY){

			curr_thread = queue_pop(ready_q); //first in the queue
		}else if(want_tid == THREAD_SELF){
			curr_thread = dequeue_thread(ready_q, curr->node->threadID);
		}else{
			curr_thread = dequeue_thread(ready_q, want_tid);
		}

		
		ret = curr_thread->threadID;
		printf("the first id is: %d\n", ret);

		curr_thread->state = RUNNING;
		setcontext(&curr_thread->context);

	}


	curr_thread->yield = false; //reset flag
	return ret;
}

void
thread_exit()
{
	//puts the current thread into exit queue
	queue_node* exitNode = (queue_node*)malloc(sizeof(queue_node));
	
	exitNode->node->state = EXIT;
	exitNode->node = curr_thread;
	enqueue(exit_q, exitNode);

	curr_thread = queue_pop(ready_q);
	setcontext(&curr_thread->context);
	curr_thread->yield = false;
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
