/* Programming Assignment 2: Exercise C
 *
 * In this third and final set of exercises, you will experiment with
 * preemptive scheduling.  We now return to the file mykernel2.c, and study
 * the procedures that were briefly mentioned but not discussed in detail
 * in Exercise B.
 *
 * HandleTimerIntr () is called by the kernel whenever a timer interrupt
 * occurs.  The system has an interval timer that can be set to interrupt
 * after a specified time.  This is done by calling SetTimer.  Notice that
 * the first thing that HandleTimerIntr does is to reset the timer to go off
 * again in the future (otherwise no more timer interrupts would occur).
 * Depending on the policy (something for you to think about), it may
 * then call DoSched, which informs the kernel to make a scheduling
 * decision at the next opportune time, at which point the kernel will
 * generate a call to SchedProc to select the next process to run, and
 * then switch to that process.
 *
 * MyRequestCPUrate (int pid, int m, int n) is called by the kernel whenever
 * a process identified by pid calls RequestCPUrate (int m, int n), which is
 * a system call that allows a process to request that it should be scheduled
 * to run m out of every n quantums.  For example, if a process wants to run
 * at 50% of the CPU's execution rate, it can call RequestCPUrate (1, 2),
 * asking that it run 1 out of every 2 quantums.  When a process calls
 * RequestCPUrate (m, n), the kernel is entered, and the kernel calls your
 * MyRequestCPUrate (pid, m, n), giving you the opportunity to implement
 * how proportional share is to be achieved.
 *
 * MyRequestCPUrate (pid, m, n) should return 0 if successful, and -1 if it
 * fails.  MyRequestCPUrate (pid, m, n) should fail if either m or n are
 * invalid (m < 1, or n < 1, or m > n).  It should also fail if a process
 * calls RequestCPUrate (m, n) such that it would result in over-allocating
 * the CPU.  Over-allocation occurs if the sum of the rates requested by
 * processes exceeds 100%.  If the call fails, it should have no effect, as
 * if the call were never made; thus, it should not affect the scheduling of
 * other processes, nor the calling process.  (The example below illustrates
 * what happens when a request fails.)  Note that when a process exits, its
 * portion of the CPU is released and is available to other processes.
 * A process may change its allocation by again calling RequestCPUrate (m, n)
 * with different values for m and n.
 *
 * IMPORTANT: If the sum of the requested rates does not equal 100%, then
 * the remaining fraction should be allocated to processes that have not
 * made rate requests (or ones that made only failing rate requests).  This
 * is important, as a process needs some CPU time just to be able to execute
 * to be able to actually call RequestCPUrate (m, n).  A good policy for
 * allocating the unrequested portion is to spread it evenly amongst processes
 * that still have not made (or have only made failed) rate requests.
 *
 * Here's an example that should help clarify the above points, including
 * what to do with unrequested CPU time and what happens when requests fail.
 * Consider the following sequence of 5 processes A, B, C, D, E, F entering
 * the system and some making CPU requests:
 *
 *	- A enters the system: A is able to use 100% of the CPU since there
 *	  are no other processes
 *	- B enters the system: B shares the CPU with A; both get an equal
 *	  amount, 50% each
 *	- B requests 40%: since there is at least 40% unrequested (in fact,
 *	  there is 100% unrequested), B gets 40%; A now gets the remaining 60%
 *	- C enters the system: it shares the unrequested 60% with A (both
 *	  get 30%)
 *	- C requests 50%: since there is at least 50% unrequested (in fact,
 *	  there is 60% unrequested), C gets 50%; A now gets the remaining 10%
 *	- D enters the system: it shares the unrequested 10% with A (both
 *	  get 5%)
 *	- D requests 20%: the request fails, and so D is treated as if it
 *	  never made the request; A and D continue to share 10% (both get 5%)
 *	- D requests 10%: since there is at least 10% unrequested (in fact,
 *	  there is exactly 10% unrequested), D gets 10%; A now gets the
 *	  remaining 0%, i.e., it does not get any CPU time
 *	- E enters the system: it shares the unrequested 0% with A (both
 *	  get zero CPU time, i.e., neither can run)
 *	- D exits, freeing up 10%, which A and E now share (A and E both
 *	  get 5%)
 *	- A exits, and so E gets the remaining 10%
 *	- E exits, and now there are only processes B (which is getting 40%)
 *	  and C (which is getting 50%).  These processes have no expectation
 *	  of additional CPU time, so the remaining 10% may be allocated any
 *	  way you want: it can be allocated evenly, proportionally, randomly,
 *	  and even not at all! As long as a process gets (at least) what it
 *	  requested, the kernel considers it satisfied.
 *
 * SetTimer (int t) will cause the timer to interrupt after t timer ticks.
 * A timer tick is a system-dependent time interval (and is 10 msecs in the
 * current implementation).  Once the timer is set, it begins counting down.
 * When it reaches 0, a timer interrupt is generated (and the kernel will
 * automatically call HandleTimerIntr).  The timer is then stopped until a
 * call to SetTimer is made.  Thus, to cause a new interrupt to go off in the
 * future, the timer must be reset by calling SetTimer.
 *
 * GetTimer () will return the current value of the timer.
 *
 *
 * Exercises
 *
 * 1. Set the TIMERINTERVAL to 1, and run the program below using the three
 * existing scheduling policies: ARBITRARY, FIFO, and LIFO.  What is the
 * effect on the outputs, and why?
 *
 * 2. Implement the ROUNDROBIN scheduling policy.  This means that each
 * process should get a turn whenever a scheduling decision is made.  For
 * ROUNDROBIN to be effective, the timer interrupt period must be made small
 * enough.  Set the TIMERINTERVAL to 1 (the smallest possible value).  You
 * should then see that the outputs of the processes will be interleaved,
 * as in: 1234123412341234123412341234123412341234 (not necessarily perfectly
 * ordered as shown.  Why?  Hint: Distinguish between a fixed amount of time
 * and the execution of enough instructions to print out a character).
 *
 * 3. Try larger values for TIMERINTERVAL, such as 10 and 100.  What is the
 * effect on the interleaving of the output, and why?
 * 
 * 4. Implement the PROPORTIONAL scheduling policy.  This allows processes to
 * call RequestCPUrate (m, n) to receive a fraction of CPU time equal to m/n;
 * specifically, within n consecutive quantums, m should be allocated to that
 * process.  For example, consider the four processes in the program below,
 * where process 1 requests 40% of the CPU by calling RequestCPUrate (4, 10),
 * process 2 requests 30% of the CPU by calling RequestCPUrate (3, 10),
 * process 3 requests 20% of the CPU by calling RequestCPUrate (2, 10), and
 * process 4 requests 10% of the CPU by calling RequestCPUrate (1, 10).  With
 * TIMERINTERVAL set to 1, this should produce an interleaving of the
 * processes' outputs where ratio of characters printed by processes 1, 2, 3,
 * and 4, are 4 to 3 to 2 to 1, respectively.  A sample output is as follows:
 * 121312412312131241231213124123121312412312132423232423343343343444444444
 * NOTE: THIS IS JUST A SAMPLE, YOUR OUTPUT MAY DIFFER FROM THIS!
 *
 * Your solution should work with any number of processes (up to MAXPROCS)
 * that have each called RequestCPUrate (m, n).  You should allow any process
 * to call RequestCPUrate (m, n) multiple times, which would change its share.
 * RequestCPUrate should fail if m < 1, or n < 1, or m > n, or if m/n would
 * cause the overall CPU allocation to exceed 100%.  In that case, the call
 * should have no effect (as if it were never called).  For any process that
 * does not call RequestCPUrate, that process should get any left-over cycles
 * (unless 100% were requested, then it would get none).
 *
 * A good solution will have the following:
 *
 * 1. Over the lifetime of each process, that process will get m/n of the CPU
 * time.  Of course, it is not possible to get EXACTLY m/n, so getting within
 * 10% of m/n is acceptable.
 *
 * 2. Unused CPU time will be equally distributed to any remaining processes
 * that have not requested CPU time.
 *
 * 3. It would be good to avoid the repeated use of floating point
 * operations, which are expensive.  For example, using approximations
 * for your calculations that only use integer operations would be good.
 * However, a working solution that uses floating point operations is
 * better than a non-working solution that avoids them, so get something
 * working, and if you have time, then optimize it.)
 *
 *
 * You must turn in your version of mykernel2.c, with all the scheduling
 * policies implemented.  You should set TIMERINTERVAL to 1, which should
 * work with ALL of your policies.
 */

#include <stdio.h>
#include "aux.h"
#include "umix.h"

void Main ()
{
	if (Fork () == 0) {

		if (Fork () == 0) {

			/* Process 4 */
			RequestCPUrate (1, 10);
			SlowPrintf (3, "444444444444444444");
			Exit ();
		}

		/* Process 2 */
		RequestCPUrate (3, 10);
		SlowPrintf (3, "222222222222222222");
		Exit ();
	}

	if (Fork () == 0) {

		/* Process 3 */
		RequestCPUrate (2, 10);
		SlowPrintf (3, "333333333333333333");
		Exit ();
	}

	/* Process 1 */
	RequestCPUrate (4, 10);
	SlowPrintf (3, "111111111111111111");
	Exit ();
}
