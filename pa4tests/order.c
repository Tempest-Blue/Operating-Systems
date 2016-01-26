/**
 * Test case 2: Spawn Ordering
 * 
 * In this test case we explore how thread ordering should work (Next Best Fit).
 *
 * For the first test:
 * - T4 exits.
 * - T4 is spawned.
 * - T5 exits.
 * - T3 exits.
 * - T3 should spawn.
 * - T5 should spawn.
 *
 *
 * For the second test:
 * - T4 exits.
 * - T4 is spawned.
 * - T3 exits.
 * - T5 exits.
 * - T5 should spawned.
 * - T3 should spawned.
 */
#include "aux.h"
#include "umix.h"
#include "mythreads.h"

#define INITTHREADS() my ? MyInitThreads() : InitThreads()
#define YIELDTHREAD(t) (my ? MyYieldThread(t) : YieldThread(t))
#define SPAWNTHREAD(f, t) (my ? MySpawnThread(f, t) : SpawnThread(f, t))
#define GETTHREAD() (my ? MyGetThread() : GetThread())
#define SCHEDTHREAD() my ? MySchedThread() : SchedThread()
#define EXITTHREAD() my ? MyExitThread() : ExitThread()

int my;

// Variable to decide if a thread will exit next time it runs.
int shouldExit = 0;

// Variable to decide if any thread will exit the next time they run.
int continueProgram = 1;

// The function the threads will use.
void Thread(int t);

void Main (argc, argv)
     int argc;
     char** argv;
{
  if(argc != 2 || (argv[1][0] != '0' && argv[1][0] != '1')) {
    Printf ("Usage: ./order [0 or 1]\n");
    Printf ("Runs the Ordering test case. This test case checks if\n");
    Printf ("your program spawns thread in the correct.");

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

  /* Create all threads */
  for (int i = 1; i < MAXTHREADS; i++) {
    Printf ("T%d created\n", SPAWNTHREAD(Thread, i));
  }

  /* First Case
   * - T4 exits.
   * - T4 is spawned.
   * - T5 exits.
   * - T3 exits.
   * - T3 should spawn.
   * - T5 should spawn.
   */
  shouldExit = 4;
  Printf ("T%d is giving up CPU\n", GETTHREAD());

  SCHEDTHREAD();

  Printf ("T%d created\n", SPAWNTHREAD(Thread, 4));

  shouldExit = 5;
  SCHEDTHREAD();

  shouldExit = 3;
  SCHEDTHREAD();

  Printf ("T%d created\n", SPAWNTHREAD(Thread, 5));
  Printf ("T%d created\n", SPAWNTHREAD(Thread, 3));

  /* Let all threads end */
  continueProgram = 0;
  SCHEDTHREAD();

  Printf ("\n\n\n");
  continueProgram = 1;

  /* Recreate all threads */
  for (int i = 1; i < MAXTHREADS; i++) {
    Printf ("T%d created\n", SPAWNTHREAD(Thread, i));
  }

  /* Second case:
   * - T4 exits.
   * - T4 is spawned.
   * - T3 exits.
   * - T5 exits.
   * - T5 should spawned.
   * - T3 should spawned.
   */
  shouldExit = 4;
  Printf ("T%d is giving up CPU\n", GETTHREAD());
  SCHEDTHREAD();

  Printf ("T%d created\n", SPAWNTHREAD(Thread, 1));

  shouldExit = 3;
  SCHEDTHREAD();

  shouldExit = 5;
  SCHEDTHREAD();

  Printf ("T%d created\n", SPAWNTHREAD(Thread, 2));
  Printf ("T%d created\n", SPAWNTHREAD(Thread, 9));


  continueProgram = 0;
  EXITTHREAD();
}


/*
 * A "thread" which either gives up CPU time, or exits.
 * If the global variable "shouldExit" is set to the
 * thread's ID, then the thread will exit at its next
 * opportune time.
 *
 * If the global variable "continueProgram" is set to 0,
 * then the thread will exit.
 */
void Thread (t)
     int t;
{
  while (continueProgram) {
    Printf("t = %d, Thread = %d... ", t, GETTHREAD());
    if (shouldExit == GETTHREAD()) {
      Printf ("T%d exiting\n", GETTHREAD());
      EXITTHREAD();
    }

    Printf ("T%d is giving up CPU\n", GETTHREAD());
    SCHEDTHREAD();
  }

  Printf("t = %d, Thread = %d... ", t, GETTHREAD());
  Printf ("T%d exiting\n", GETTHREAD());
}
