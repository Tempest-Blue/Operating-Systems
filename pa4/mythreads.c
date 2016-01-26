/*	User-level thread system
 *
 */

// Kieth Vo
// A09632897
#include <setjmp.h>
#include <string.h>
#include "aux.h"
#include "umix.h"
#include "mythreads.h"
static int MyInitThreadsCalled = 0;	/* 1 if MyInitThreads called, else 0 */

static int currThread;
static int lastSlot;
static int noSlots;
static int numOfThreads;
static int yield;
static int exit;

static struct thread {			/* thread table */
	int valid;			/* 1 if entry is valid, else 0 */
	jmp_buf env;			/* current context */
    jmp_buf envTwo;
    void (*f) ();
    int p;
} thread[MAXTHREADS];

static struct queue {
    int head;
    int tail;
    int pid[MAXTHREADS];
} queue;

#define STACKSIZE	65536		/* maximum size of thread stack */

/*	MyInitThreads () initializes the thread package. Must be the first
 *	function called by any user program that uses the thread package.  
 */

void MyInitThreads ()
{
	if (MyInitThreadsCalled) {                /* run only once */
		Printf ("InitThreads: should be called only once\n");
		Exit ();
	}

	for (int i = 0; i < MAXTHREADS; i++) {	/* initialize thread table */
		thread[i].valid = 0;
        queue.pid[i] = -1;
	}
    
    if (setjmp(thread[0].env) == 0) 
    {
        memcpy(thread[0].envTwo, thread[0].env, sizeof(thread[0].env));
        for (int i = 1; i < MAXTHREADS; i++)
        {
            char s[STACKSIZE * i];
            if (((int) &s[(STACKSIZE) * i - 1]) - ((int) &s[STACKSIZE * (i-1)]) + 1 != STACKSIZE)
            {
                Printf ("Stack space reservation failed\n");
                Exit();
            }
            if (setjmp(thread[i].env) == 0)
            {
                memcpy(thread[i].envTwo, thread[i].env, sizeof(thread[i].env));
                continue;
            }
            else
            {
                (*thread[currThread].f)(thread[currThread].p);
                MyExitThread();
            }
        }
	    thread[0].valid = 1;			/* initialize thread 0 */
	    MyInitThreadsCalled = 1;
    }
    else
    {
        (*thread[0].f)(thread[0].p);
        MyExitThread();
    }
}

/*	MySpawnThread (func, param) spawns a new thread to execute
 *	func (param), where func is a function with no return value and
 *	param is an integer parameter.  The new thread does not begin
 *	executing until another thread yields to it.  
 */

int MySpawnThread (func, param)
	void (*func)();		/* function to be executed */
	int param;		/* integer parameter */
{
	if (! MyInitThreadsCalled) {
		Printf ("SpawnThread: Must call InitThreads first\n");
		Exit ();
	}
    
    if (numOfThreads == MAXTHREADS - 1)
    {
        noSlots = 1;
    }
    int freeSlot = 0;
    int index = lastSlot;
    for (int i = 0; i < MAXTHREADS; i++)
    {
        index = (index + 1) % MAXTHREADS;
        if (! thread[index].valid)
        {
            freeSlot = 1;
            break;
        }
    }
    if (! freeSlot)
    {
        return -1;
    }
    lastSlot = index;
    thread[index].valid = 1;
    thread[index].f = func;
    thread[index].p = param;
    if (noSlots)
    {
        memcpy(thread[index].env, thread[index].envTwo, sizeof(thread[index].envTwo));
    }
    queue.pid[queue.tail] = index;
    queue.tail = (queue.tail + 1) % MAXTHREADS;
    numOfThreads++;
    return index;
}

/*	MyYieldThread (t) causes the running thread, call it T, to yield to
 *	thread t.  Returns the ID of the thread that yielded to the calling
 *	thread T, or -1 if t is an invalid ID.  Example: given two threads
 *	with IDs 1 and 2, if thread 1 calls MyYieldThread (2), then thread 2
 *	will resume, and if thread 2 then calls MyYieldThread (1), thread 1
 *	will resume by returning from its call to MyYieldThread (2), which
 *	will return the value 2.
 */

int MyYieldThread (t)
	int t;				/* thread being yielded to */
{
	if (! MyInitThreadsCalled) {
		Printf ("YieldThread: Must call InitThreads first\n");
		Exit ();
	}

	if (t < 0 || t >= MAXTHREADS) {
		Printf ("YieldThread: %d is not a valid thread ID\n", t);
		return (-1);
	}
	if (! thread[t].valid) {
		Printf ("YieldThread: Thread %d does not exist\n", t);
		return (-1);
	}
    yield = currThread;
    if (t == yield)
    {
        return yield;
    }
    int index;
    for (int i = 0; i < MAXTHREADS; i++)
    {
        if (queue.pid[i] == t)
        {
            index = i;
        }
    }
    queue.pid[index] = -1;

    int loopLimit;
    if (index > queue.tail)
    {
        loopLimit = MAXTHREADS - queue.tail;
    }
    else
    {
        loopLimit = queue.tail - index - 1;
    }
    for (int i = index; i < loopLimit + index; i++)
    {
        queue.pid[i] = queue.pid[(i + 1) % MAXTHREADS];
        queue.pid[(i + 1) % MAXTHREADS] = -1;
    }
    queue.tail = (queue.tail + MAXTHREADS - 1) % MAXTHREADS;

    if (!exit)
    {
        queue.pid[queue.tail] = currThread;
        queue.tail = (queue.tail + 1) % MAXTHREADS;
    }

    exit = 0;

    if (setjmp (thread[currThread].env) == 0)
    {
        currThread = t;
        longjmp (thread[t].env, 1);
    }

    return yield;
}

/*	MyGetThread () returns ID of currently running thread. 
 */

int MyGetThread ()
{
	if (! MyInitThreadsCalled) {
		Printf ("GetThread: Must call InitThreads first\n");
		Exit ();
	}
    return currThread;
}

/*	MySchedThread () causes the running thread to simply give up the
 *	CPU and allow another thread to be scheduled.  Selecting which
 *	thread to run is determined here. Note that the same thread may
 * 	be chosen (as will be the case if there are no other threads).  
 */

void MySchedThread ()
{
	if (! MyInitThreadsCalled) {
		Printf ("SchedThread: Must call InitThreads first\n");
		Exit ();
	}
    if (exit && queue.pid[queue.head] == -1)
    {
        Exit ();
    }
    else if (queue.pid[queue.head] == -1)
    {
        MyYieldThread(currThread);
    }
    else
        MyYieldThread(queue.pid[queue.head]);
}

/*	MyExitThread () causes the currently running thread to exit.  
 */

void MyExitThread ()
{
	if (! MyInitThreadsCalled) {
		Printf ("ExitThread: Must call InitThreads first\n");
		Exit ();
	}

    thread[currThread].valid = 0;
    exit = 1;
    MySchedThread();
}

