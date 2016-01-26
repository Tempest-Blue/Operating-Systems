/**
 * Test case 1: Spawning Threads, and their stacks
 * 
 * This test case will attempt to spawn 10 threads. The first 9 should
 * be successful, but the 10th one should fail.
 * 
 * Afterwards the program will go through each thread, checking if
 * each thread's stack is at least STACKSIZE apart.
 * 
 * Then the program will respawn all the threads again and check
 * if each thread's stack started at the same location as before.
 */
#include "aux.h"
#include "umix.h"
#include "mythreads.h"

#define STACKSIZE	65536		/* maximum size of thread stack */

#define INITTHREADS() my ? MyInitThreads() : InitThreads()
#define YIELDTHREAD(t) (my ? MyYieldThread(t) : YieldThread(t))
#define SPAWNTHREAD(f, t) (my ? MySpawnThread(f, t) : SpawnThread(f, t))
#define GETTHREAD() (my ? MyGetThread() : GetThread())
#define SCHEDTHREAD() my ? MySchedThread() : SchedThread()
#define EXITTHREAD() my ? MyExitThread() : ExitThread()

int my;

/* array of stack locations for each thread*/
int stacklocs[MAXTHREADS];

/* functions */
void aThread(int t);
void bThread(int t);

void Main (argc, argv)
     int argc;
     char** argv;
{
  if(argc != 2 || (argv[1][0] != '0' && argv[1][0] != '1')) {
    Printf ("Usage: ./spawn [0 or 1]\n");
    Printf ("Runs the Spawn test case. This test case checks if your kernel\n");
    Printf ("can spawn at most MAXTHREADS (default: %d) threads.\n", MAXTHREADS);
    Printf ("\nIf you use 0 as the parameter, then the test case will\n");
    Printf ("use your kernel to run the tests.\n");
    Printf ("\nIf you use 1 as the parameter, then the test case will\n");
    Printf ("use the real kernel to run the test case.\n");
    return;
  }

  my = (argv[1][0] == '0' ? 1 : 0);

//  if (my)
//    Printf("USING YOUR KERNEL\n");

  INITTHREADS();
  int i;

  // Spawns MAXTHREADS threads (MAXTHREAD'th one should fail)
  // The parameter passed into the "a" function is the stack
  // location of the current thread.
  for (i = 1; i <= MAXTHREADS; i++) {
    Printf ("T%d spawning T%d\n", GETTHREAD(),
	    SPAWNTHREAD(aThread, (int)&i));
  }

  
  // Go through all threads to check for stack correctness
  SCHEDTHREAD();

  Printf("\n\n");

  // Spawns MAXTHREADS - 1 thread. The parameter passed into
  // the "b" function is the stack locations of each thread.
  // The goal of this check is to ensure that all threads
  // have their own stack location.
  for (i = 1; i < MAXTHREADS; i++) {
    Printf ("T%d respawning T%d\n", GETTHREAD(),
	    SPAWNTHREAD(bThread, stacklocs[i]));
  }
  
  // Exit and schedule to every other thread
  EXITTHREAD();
}


// Checks if the stack location from thread 0 to current thread
// is correctly spaced apart.
void aThread(t)
     int t;
{
  stacklocs[GETTHREAD()] = (int)&t;
  if ((t - (int)&t)/(GETTHREAD()) >= STACKSIZE) {
    Printf ("T%d stack seems correct\n", GETTHREAD());
  } else {
    Printf ("T%d stack does not seems correct\n", GETTHREAD());
  }
}


// Checks if the stack location of the thread is the same
// as what it was last time.
void bThread(t)
     int t;
{
  
  if ((int)&t != t) {
    Printf ("T%d stack does not seems correct\n", GETTHREAD());
  } else {
    Printf ("T%d stack seems correct\n", GETTHREAD());
    
  }
}
