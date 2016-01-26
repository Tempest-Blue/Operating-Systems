/**
 * Test Case 3: Queue
 *
 * In this test case the behavior is not so easy to determine.
 * The goal is to "randomly":
 * - Spawn threads
 * - Yield to threads
 * - and Schedule threads
 * 
 * to see if you are moving things around your queue structure
 * correctly (in your kernel).
 */
#include "aux.h"
#include "umix.h"
#include "mythreads.h"
#include <stdlib.h>

#define INITTHREADS() my ? MyInitThreads() : InitThreads()
#define YIELDTHREAD(t) (my ? MyYieldThread(t) : YieldThread(t))
#define SPAWNTHREAD(f, t) (my ? MySpawnThread(f, t) : SpawnThread(f, t))
#define GETTHREAD() (my ? MyGetThread() : GetThread())
#define SCHEDTHREAD() my ? MySchedThread() : SchedThread()
#define EXITTHREAD() my ? MyExitThread() : ExitThread()

int my;

// maximum number of thread spawns that should be made.
#define MAXSPAWNS 100

int yieldIdx = 0; // index in the yield thread to return next
int yield[MAXTHREADS]; // list of threads to yield to in a specific order
int spawnCnt = MAXSPAWNS - 1; // thread spawn count down

// The function the threads will use.
void Thread(int t);
void Thread2(int t);


/* Shuffles the yield array */
void shuffle()
{
  int i;
  int j;
  int val;
  for (i = 0; i < MAXTHREADS; i++) {
    j = rand() % MAXTHREADS;
    val = yield[i];
    yield[i] = yield[j];
    yield[j] = val;
  }
}

/* get next yield id, and increment the yield index */
int nextYield()
{
  int y = yieldIdx % MAXTHREADS;
  yieldIdx++;
  return yield[y];
}


/* the main program; sets rng seed, spawns 9 threads, then yields to
 * first thread in yield queue. when it gets returned to, it should
 * exit.
 */
void Main (argc, argv)
     int argc;
     char** argv;
{
  if(argc != 2 || (argv[1][0] != '0' && argv[1][0] != '1')) {
    Printf ("Usage: ./queue [0 or 1]\n");
    Printf ("Runs the Queue test case. This test case checks if\n");
    Printf ("your program sets up its queue correctly.");

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
  int next;

  /* set a seed to get deterministic output
   *
   * feel free to change this to any integer you want
   */
  srand (0xC5E120);

  // fill yield array with 0 to MAXTHREADS - 1
  yieldIdx = 0;
  for (int i = 0; i < MAXTHREADS; i++) {
    yield[i] = i;
  }

  // shuffle yield array
  shuffle();
  
  /* Create all threads */
  for (int i = 1; i < MAXTHREADS; i++) {
    Printf ("Main : T%d created\n", SPAWNTHREAD(Thread, i));
    spawnCnt--;
  }


  // yield to next thread
  next = nextYield ();
  Printf ("Main : T%d yielding to %d\n", GETTHREAD(), next);
  Printf ("Main : T%d yielded back to T%d\n", YIELDTHREAD(next), GETTHREAD());

  // program is done
  Printf ("Main : T%d exiting\n", GETTHREAD());
  EXITTHREAD();
}


/**
 * A thread function; gets the next thread it should yield to.
 * If it turns out that the next thread it should yield to
 * is itself, the thread will schedule so another
 * thread can run.
 * 
 * Then after the thread is returned to, it will (randomly)
 * decide if it will spawn another thread (50/50 chance)
 *
 * To add more 'randomness' if 50 threads have been spawned,
 * then Thread2() will be spawned instead of Thread().
 */
void Thread (t)
     int t;
{
  int next;

  // decide to either schedule or yield to another thread
  while(1) {
    next = nextYield ();
    if (next == GETTHREAD()) {
      shuffle();
      Printf ("Thread : T%d scheduling\n", GETTHREAD ());
      SCHEDTHREAD ();
      Printf ("Thread : T%d has returned from scheduling\n", GETTHREAD());
      break;
    } else {
      int y;
      Printf ("Thread : T%d attempt to yield to T%d\n", GETTHREAD(), next);
      y = YIELDTHREAD(next);
      if (y != -1) {
	Printf ("Thread : T%d yielded back to T%d\n", y, GETTHREAD());
	break;
      }
    }
  }

  void (*f)(int);

  /* if spawn count is less than MAXSPAWNS/2, then spawn Thread2(), else spawn
   * Thread(), both with 50% chance of not spawning.
   */
  f = spawnCnt < MAXSPAWNS/2 ? &Thread2 : &Thread;
  if (spawnCnt > 0 && rand() % 2) {
    if((t=SPAWNTHREAD(f, t)) != -1) {
      Printf ("Thread : T%d spawned T%d\n", GETTHREAD(), t);
      spawnCnt--;
    }
  }

  /* if spawn count is less than 50, then spawn Thread2(), else spawn
   * Thread(), both with 50% chance of not spawning.
   */
  f = spawnCnt < 50 ? &Thread2 : &Thread;
  if (spawnCnt > 0 && rand() % 2) {
    if((t=SPAWNTHREAD(f, t)) != -1) {
      Printf ("Thread : T%d spawned T%d\n", GETTHREAD(), t);
      spawnCnt--;
    }
  }

  Printf ("Thread : T%d exiting\n", GETTHREAD ());
}


/**
 * Very similar to Thread(); main differences:
 * If current thread is NOT the same as the next thread,
 * then the current thread will schedule. Otherwise, it will
 * attempt to yield to another thread.
 *
 * Also, this thread may spawn a Thread() is divisible by 3.
 */
void Thread2 (t)
     int t;
{
  int next;

  // decide to either schedule or yield to another thread
  while(1) {
    next = nextYield ();
    if (next != GETTHREAD()) {
      shuffle();
      Printf ("Thread2 : T%d scheduling\n", GETTHREAD ());
      SCHEDTHREAD ();
      Printf ("Thread2 : T%d has returned from scheduling\n", GETTHREAD());
      break;
    } else {
      int y = 0;
      next = nextYield();
      Printf ("Thread2 : T%d attempting to yield to T%d\n", GETTHREAD(), next);
      y = YIELDTHREAD(next);

      if (y != -1) {
	Printf ("Thread2 : T%d yielded back to T%d\n", y, GETTHREAD());
	break;
      }
    }
  }

  void (*f)(int);

  /* if spawn count not zero and is divisible by 3, then
   * maybe spawn Thread() instead of Thread2() with 4/5 chance
   */
  f = spawnCnt%3 ? &Thread2 : &Thread; 
  if (spawnCnt > 0 && rand() % 5) {
    if((t=SPAWNTHREAD(f, t)) != -1) {
      Printf ("Thread2 : T%d spawned T%d\n", GETTHREAD(), t);
      spawnCnt--;
    }
  }

  /* if spawn count not zero and is divisible by 3, then
   * maybe spawn Thread() instead of Thread2() with 4/5 chance
   */
  f = spawnCnt%3 ? &Thread2 : &Thread; 
  if (spawnCnt > 0 && rand() % 5) {
    if((t=SPAWNTHREAD(f, t)) != -1) {
      Printf ("Thread2 : T%d spawned T%d\n", GETTHREAD(), t);
      spawnCnt--;
    }
  }

  Printf ("Thread2 : T%d exiting\n", GETTHREAD ());
}
