// Kieth Vo
// A09632897
/* mykernel2.c: your portion of the kernel
 *
 *	Below are procedures that are called by other parts of the kernel. 
 *	Your ability to modify the kernel is via these procedures.  You may
 *	modify the bodies of these procedures any way you wish (however,
 *	you cannot change the interfaces).  
 * 
 */

#include "aux.h"
#include "sys.h"
#include "mykernel2.h"

#define TIMERINTERVAL 1 /* in ticks (tick = 10 msec) */
#define LARGEINTEGER 10000
#define INT_MAX 2147483647

/*	A sample process table.  You may change this any way you wish. 
 */

int cpuSUM = 100;
int shareCPU = 0;

static struct {
	int valid;		/* is this entry valid: 1 = yes, 0 = no */
	int pid;		/* process id (as provided by kernel) */
    int pass;
    int stride;
    int share;
} proctab[MAXPROCS];


/*	InitSched () is called when kernel starts up.  First, set the
 *	scheduling policy (see sys.h). Make sure you follow the rules
 *	below on where and how to set it.  Next, initialize all your data
 *	structures (such as the process table).  Finally, set the timer
 *	to interrupt after a specified number of ticks.  
 */

void InitSched ()
{
	int i;

	/* First, set the scheduling policy. You should only set it
	 * from within this conditional statement.  While you are working
	 * on this assignment, GetSchedPolicy will return NOSCHEDPOLICY,
	 * and so the condition will be true and you may set the scheduling
	 * policy to whatever you choose (i.e., you may replace ARBITRARY). 
	 * After the assignment is over, during the testing phase, we will
	 * have GetSchedPolicy return the policy we wish to test, and so
	 * the condition will be false and SetSchedPolicy will not be
	 * called, thus leaving the policy to whatever we chose to test.  
	 */
	if (GetSchedPolicy () == NOSCHEDPOLICY) {	/* leave as is */
		SetSchedPolicy (PROPORTIONAL);		/* set policy here */
	}
		
	/* Initialize all your data structures here */
	for (i = 0; i < MAXPROCS; i++) {
		proctab[i].valid = 0;
        proctab[i].share = 0;
        proctab[i].pass = 0;
        proctab[i].stride = 0;
	}

	/* Set the timer last */
	SetTimer (TIMERINTERVAL);
}


/*	StartingProc (pid) is called by the kernel when the process
 *	identified by pid is starting.  This allows you to record the
 *	arrival of a new process in the process table, and allocate
 *	any resources (if necessary).  Returns 1 if successful, 0 otherwise. 
 */

int StartingProc (pid)
	int pid;
{
	int i;
	for (i = 0; i < MAXPROCS; i++) {
		if (! proctab[i].valid) {
            shareCPU += 1;
            proctab[i].share = 1;
			proctab[i].valid = 1;
			proctab[i].pid = pid;
            for (i = 0; i < MAXPROCS; i++) {
                if (proctab[i].valid && (proctab[i].share == 1)) {
                    cpuSUM += proctab[i].stride;
                }
            }
            for (i = 0; i < MAXPROCS; i++) {
                if (proctab[i].valid && (proctab[i].share == 1)) {
                    if ((cpuSUM >= shareCPU) && (shareCPU != 0)) {
                        proctab[i].stride = cpuSUM / shareCPU;
                    }
                    else {
                        proctab[i].stride = 1;
                    }
                }
            }
            for (i = 0; i < MAXPROCS; i++) {
                if (proctab[i].valid && (proctab[i].share == 1)) {
                    cpuSUM -= proctab[i].stride;
                }
            }
            return (1);
		}
	}

	Printf ("Error in StartingProc: no free table entries\n");
	return (0);
}
			

/*	EndingProc (pid) is called by the kernel when the process
 *	identified by pid is ending.  This allows you to update the
 *	process table accordingly, and deallocate any resources (if
 *	necessary). Returns 1 if successful, 0 otherwise.  
 */


int EndingProc (pid)
	int pid;
{
	int i;
    int temp;

	for (i = 0; i < MAXPROCS; i++) {
        if (proctab[i].valid && proctab[i].pid == pid) 
        {
            proctab[i].valid = 0;
            proctab[i].share = 0;
            shareCPU -= 1;
            cpuSUM += proctab[i].stride;
            temp = i;
            for (i = temp; i < MAXPROCS; i++) 
            {
                if (proctab[i+1].valid) {
                    proctab[i] = proctab[i+1];
                    proctab[i].valid = 1;
                    proctab[i+1].valid = 0;}
	        }
			return (1);
        }
    }
	Printf ("Error in EndingProc: can't find process %d\n", pid);
	return (0);
}


/*	SchedProc () is called by kernel when it needs a decision for
 *	which process to run next.  It calls the kernel function
 *	GetSchedPolicy () which will return the current scheduling policy
 *	which was previously set via SetSchedPolicy (policy).  SchedProc ()
 *	should return a process id, or 0 if there are no processes to run. 
 */

int SchedProc ()
{
	int i;

	switch (GetSchedPolicy ()) {

	case ARBITRARY:
        if (proctab[0].valid == 0) {
            return 0;
        }
		for (i = 0; i < MAXPROCS; i++) {
			if (proctab[i].valid) {
				return (proctab[i].pid);
			}
		}
		break;

	case FIFO:

		/* your code here */
        if (proctab[0].valid == 0) {
            return 0;
        }
		for (i = 0; i < MAXPROCS; i++) {
			if (proctab[i].valid) {
                return (proctab[i].pid);
            }
        }
		break;

	case LIFO:

		/* your code here */
        DoSched();
        if (proctab[0].valid == 0) {
            return 0;
        }
        for (i = MAXPROCS; i >= 0; i--) {
            if (proctab[i].valid) {
                return (proctab[i].pid);
            }
        }
		break;

	case ROUNDROBIN:

		/* your code here */
        if (proctab[0].valid == 0) {
            return 0;
        }
        int min = proctab[0].pass;
        int proc = 0;
        for (i = 0; i < MAXPROCS; i++)  {
            if (proctab[i].valid && proctab[i].pass <= min) {
                    min = proctab[i].pass;
                    proc = i;
            }
        }
        proctab[proc].pass += 1;
        return (proctab[proc].pid);
		break;

	case PROPORTIONAL:

		/* your code here */
        if (proctab[0].valid == 0) {
            return 0;
        }
        int min2 = proctab[0].pass;
        int proc2 = 0;
        for (i = 0; i < MAXPROCS; i++) {
            if (proctab[i].valid && proctab[i].pass <= min2) {
                min2 = proctab[i].pass;
                proc2 = i;
            }
        }
        if ((proctab[proc2].pass += LARGEINTEGER / (proctab[proc2].stride)) >= INT_MAX) {
            int min3 = proctab[0].pass;
            for (int i = 0; i < MAXPROCS; i++) {
                if (proctab[i].valid && (proctab[i].pass <= min))
                    min3 = proctab[i].pass;
            }
            while ((proctab[proc2].pass += LARGEINTEGER / (proctab[proc2].stride)) >= INT_MAX) {
                for (int i = 0; i < MAXPROCS; i++) {
                    if (proctab[i].valid) {
                        proctab[i].pass -= min;
                    }
                }
            }
        }
        proctab[proc2].pass += LARGEINTEGER / (proctab[proc2].stride);
        return (proctab[proc2].pid);
		break;

	}
	
	return (0);
}


/*	HandleTimerIntr () is called by the kernel whenever a timer
 *	interrupt occurs.  
 */

void HandleTimerIntr ()
{
    int i = 0;
	SetTimer (TIMERINTERVAL);

	switch (GetSchedPolicy ()) {	/* is policy preemptive? */

	case ROUNDROBIN:		/* ROUNDROBIN is preemptive */
        DoSched ();
        break;
	case PROPORTIONAL:		/* PROPORTIONAL is preemptive */
		DoSched ();		/* make scheduling decision */
		break;
	default:			/* if non-preemptive, do nothing */
		break;
	}
}

/*	MyRequestCPUrate (pid, m, n) is called by the kernel whenever a process
 *	identified by pid calls RequestCPUrate (m, n). This is a request for
 *	a fraction m/n of CPU time, effectively running on a CPU that is m/n
 *	of the rate of the actual CPU speed.  m of every n quantums should
 *	be allocated to the calling process.  Both m and n must be greater
 *	than zero, and m must be less than or equal to n.  MyRequestCPUrate
 *	should return 0 if successful, i.e., if such a request can be
 *	satisfied, otherwise it should return -1, i.e., error (including if
 *	m < 1, or n < 1, or m > n). If MyRequestCPUrate fails, it should
 *	have no effect on scheduling of this or any other process, i.e., as
 *	if it were never called.  
 */

int MyRequestCPUrate (pid, m, n)
	int pid;
	int m;
	int n;
{
	/* your code here */

    if (m < 1 || n < 1 || m > n || m > 100 || n > 100) {
        return (-1);
    }
    else {
        int i;
        for (i = 0; i < MAXPROCS; i++) {
            if (proctab[i].valid && proctab[i].share) {
                cpuSUM += proctab[i].stride;
            }
        }
        int temp = (m * 100) / n;
        if ((cpuSUM - temp) >= 0) {
            shareCPU -= 1;
            for (i = 0; i < MAXPROCS; i++) {
                if (proctab[i].pid == pid) {
                    proctab[i].share = 0;
                    proctab[i].stride = temp;
                    cpuSUM -= temp;
                }
            }
            for (i = 0; i < MAXPROCS; i++) {
                if (proctab[i].valid && (proctab[i].share == 1)) {
                    if ((cpuSUM >= shareCPU) && (shareCPU != 0)) {
                        proctab[i].stride = cpuSUM / shareCPU;
                    }
                    else {
                        proctab[i].stride = 1;
                    }
                }
            }
            for (i = 0; i < MAXPROCS; i++) {
                if (proctab[i].valid && (proctab[i].share == 1)) {
                    cpuSUM -= proctab[i].stride;
                }
            }
            return (0);
        }
        else {
            for (i = 0; i < MAXPROCS; i++) {
                if (proctab[i].valid && proctab[i].share) {
                    cpuSUM -= proctab[i].stride;
                }
            }
            return (-1);
        }
    }
}
