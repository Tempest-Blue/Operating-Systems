/* Programming Assignment 3: Exercise D
 *
 * Now that you have a working implementation of semaphores, you can
 * implement a more sophisticated synchronization scheme for the car
 * simulation.
 *
 * Study the program below.  Car 1 begins driving over the road, entering
 * from the East at 40 mph.  After 900 seconds, both Car 3 and Car 4 try to
 * enter the road.  Car 1 may or may not have exited by this time (it should
 * exit after 900 seconds, but recall that the times in the simulation are
 * approximate).  If Car 1 has not exited and Car 4 enters, they will crash
 * (why?).  If Car 1 has exited, Car 3 and Car 4 will be able to enter the
 * road, but since they enter from opposite directions, they will eventually
 * crash.  Finally, after 1200 seconds, Car 2 enters the road from the West,
 * and traveling twice as fast as Car 4.  If Car 3 was not coming from the
 * opposite direction, Car 2 would eventually reach Car 4 from the back and
 * crash.  (You may wish to experiment with reducing the initial delay of
 * Car 2, or increase the initial delay of Car 3, to cause Car 2 to crash
 * with Car 4 before Car 3 crashes with Car 4.)
 *
 *
 * Exercises
 *
 * 1. Modify the procedure driveRoad such that the following rules are obeyed:
 *
 *	A. Avoid all collisions.
 *
 *	B. Multiple cars should be allowed on the road, as long as they are
 *	traveling in the same direction.
 *
 *	C. If a car arrives and there are already other cars traveling in the
 *	SAME DIRECTION, the arriving car should be allowed enter as soon as it
 *	can. Two situations might prevent this car from entering immediately:
 *	(1) there is a car immediately in front of it (going in the same
 *	direction), and if it enters it will crash (which would break rule A);
 *	(2) one or more cars have arrived at the other end to travel in the
 *	opposite direction and are waiting for the current cars on the road
 *	to exit, which is covered by the next rule.
 *
 *	D. If a car arrives and there are already other cars traveling in the
 *	OPPOSITE DIRECTION, the arriving car must wait until all these other
 *	cars complete their course over the road and exit.  It should only wait
 *	for the cars already on the road to exit; no new cars traveling in the
 *	same direction as the existing ones should be allowed to enter.
 *
 *	E. This last rule implies that if there are multiple cars at each end
 *	waiting to enter the road, each side will take turns in allowing one
 *	car to enter and exit.  (However, if there are no cars waiting at one
 *	end, then as cars arrive at the other end, they should be allowed to
 *	enter the road immediately.)
 *	
 *	F. If the road is free (no cars), then any car attempting to enter
 *	should not be prevented from doing so.
 *
 *	G. All starvation must be avoided.  For example, any car that is
 *	waiting must eventually be allowed to proceed.
 *
 * This must be achieved ONLY by adding synchronization and making use of
 * shared memory (as described in Exercise C).  You should NOT modify the
 * delays or speeds to solve the problem.  In other words, the delays and
 * speeds are givens, and your goal is to enforce the above rules by making
 * use of only semaphores and shared memory.
 *
 * 2. Devise different tests (using different numbers of cars, speeds,
 * directions) to see whether your improved implementation of driveRoad
 * obeys the rules above.
 *
 * IMPLEMENTATION GUIDELINES
 * 
 * 1. Avoid busy waiting. In class one of the reasons given for using
 * semaphores was to avoid busy waiting in user code and limit it to
 * minimal use in certain parts of the kernel. This is because busy
 * waiting uses up CPU time, but a blocked process does not. You have
 * semaphores available to implement the driveRoad function, so you
 * should not use busy waiting anywhere.
 *
 * 2. Prevent race conditions. One reason for using semaphores is to
 * enforce mutual exclusion on critical sections to avoid race conditions.
 * You will be using shared memory in your driveRoad implementation.
 * Identify the places in your code where there may be race conditions
 * (the result of a computation on shared memory depends on the order
 * that processes execute).  Prevent these race conditions from occurring
 * by using semaphores.
 *
 * 3. Implement semaphores fully and robustly.  It is possible for your
 * driveRoad function to work with an incorrect implementation of
 * semaphores, because controlling cars does not exercise every use of
 * semaphores.  You will be penalized if your semaphores are not correctly
 * implemented, even if your driveRoad works.
 *
 * 4. Avoid starvation.  This is especially relevant when implementing the
 * Signal function.  If there are multiple processes that blocked on the
 * same semaphore, then a good policy is to unblock them in FIFO order.
 *
 * 5. Control cars with semaphores: Semaphores should be the basic
 * mechanism for enforcing the rules on driving cars. You should not
 * force cars to delay in other ways inside driveRoad such as by calling
 * the Delay function or changing the speed of a car. (You can leave in
 * the delay that is already there that represents the car's speed, just
 * don't add any additional delaying).  Also, you should not be making
 * decisions on what cars do using calculations based on car speed (since
 * there are a number of unpredictable factors that can affect the
 * actual cars' progress).
 *
 * GRADING INFORMATION
 *
 * 1. Semaphores: We will run a number of programs that test your
 * semaphores directly (without using cars at all). For example:
 * enforcing mututal exclusion, testing robustness of your list of
 * waiting processes, calling signal and wait many times to make sure
 * the semaphore state is consistent, etc.
 *
 * 2. Cars: We will run some tests of cars arriving in different ways,
 * to make sure that you correctly enforce all the rules for cars given
 * in the assignment.  We will use a correct semaphore implementation for
 * these tests so that even if your semaphores are not correct you could
 * still get full credit on the driving part of the grade.  Think about
 * how your driveRoad might handle different situations and write your
 * own tests with cars arriving in different ways to make sure you handle
 * all cases correctly.
 *
 *
 * WHAT TO TURN IN
 *
 * You must turn in two files: mykernel3.c and p3d.c.  mykernel3.c should
 * contain you implementation of semaphores, and p3d.c should contain
 * your modified version of InitRoad and driveRoad (Main will be ignored).
 * Note that you may set up your static shared memory struct and other
 * functions as you wish. They should be accessed via InitRoad and driveRoad,
 * as those are the functions that we will call to test your code.
 *
 * Your programs will be tested with various Main programs that will exercise
 * your semaphore implementation, AND different numbers of cars, directions,
 * and speeds, to exercise your driveRoad function.  Our Main programs will
 * first call InitRoad before calling driveRoad.  Make sure you do as much
 * rigorous testing yourself to be sure your implementations are robust.
 */

#include <stdio.h>
#include "aux.h"
#include "umix.h"

void InitRoad ();
void driveRoad (int from, int mph);
struct {
    int wQueue;
    int eQueue;
    int wDoor;
    int eDoor;
    int ticket;
    int wCarCount;
    int eCarCount;
    int road[NUMPOS];
} shm;

void Main ()
{
    InitRoad ();

	/* The following code is specific to this particular simulation,
	 * e.g., number of cars, directions, and speeds.  You should
	 * experiment with different numbers of cars, directions, and
	 * speeds to test your modification of driveRoad.  When your
	 * solution is tested, we will use different Main procedures,
	 * which will first call InitRoad before any calls to driveRoad.
	 * So, you should do any initializations in InitRoad.
	 */

	if (Fork () == 0) {			/* Car 2 */
		Delay (1162);
		driveRoad (WEST, 60);
		Exit ();
	}

	if (Fork () == 0) {			/* Car 3 */
		Delay (900);
		driveRoad (EAST, 50);
		Exit ();
	}

	if (Fork () == 0) {			/* Car 4 */
		Delay (900);
		driveRoad (WEST, 30);
		Exit ();
	}

	driveRoad (EAST, 40);			/* Car 1 */

	Exit ();
    
}

/* Our tests will call your versions of InitRoad and driveRoad, so your
 * solution to the car simulation should be limited to modifying the code
 * below.  This is in addition to your implementation of semaphores
 * contained in mykernel3.c.
 */

void InitRoad ()
{
	/* do any initializations here */
    Regshm ((char *) &shm, sizeof (shm));
    shm.wQueue = Seminit(1);
    shm.eQueue = Seminit(1);
    shm.ticket = Seminit(1);
    shm.wDoor = Seminit(1);
    shm.eDoor = Seminit(1);
    shm.wCarCount = 0;
    shm.eCarCount = 0;
    for (int i = 0; i < NUMPOS; i++) {
        shm.road[i] = Seminit(1);
    }
}

#define IPOS(FROM)	(((FROM) == WEST) ? 1 : NUMPOS)

void driveRoad (from, mph)
	int from, mph;
{
	int c;					/* car id c = process id */
	int p, np, i;				/* positions */

	c = Getpid ();				/* learn this car's id */
    
    if (from == WEST)
    {
        Wait(shm.wQueue);
        Wait(shm.ticket);
        Wait(shm.wDoor);
        if (shm.wCarCount == 0)
        {
            Wait(shm.eDoor);
        }
        Signal(shm.wDoor);
        Wait(shm.road[0]);
        Signal(shm.ticket);
        EnterRoad(from);
        shm.wCarCount++;
        PrintRoad();
        Printf ("Car %d enters at %d at %d mph\n", c, IPOS(from), mph);
    }
    else
    {
        Wait(shm.eQueue);
        Wait(shm.ticket);
        Wait(shm.eDoor);
        if (shm.eCarCount == 0)
        {
            Wait(shm.wDoor);
        }
        Signal(shm.eDoor);
        Wait(shm.road[NUMPOS - 1]);
        Signal(shm.ticket);
        EnterRoad(from);
        shm.eCarCount++;
        PrintRoad();
        Printf ("Car %d enters at %d at %d mph\n", c, IPOS(from), mph);
    }

	for (i = 1; i < NUMPOS; i++) {
		if (from == WEST) {
			p = i;
			np = i + 1;
		} else {
			p = NUMPOS + 1 - i;
			np = p - 1;
		}

		Delay (3600/mph);
        if (from == WEST)
        {
            if (p == 2) 
            {
                Signal(shm.wQueue);
            }
            Wait(shm.road[np - 1]);
            ProceedRoad();
            Signal(shm.road[p-1]);
            PrintRoad();
            Printf ("Car %d moves from %d to %d\n", c, p, np);
        }
        else
        {
            if (p == (NUMPOS - 1))
            {
                Signal(shm.eQueue);
            }
            Wait(shm.road[np - 1]);
            ProceedRoad();
            Signal(shm.road[p-1]);
            PrintRoad();
            Printf ("Car %d moves from %d to %d\n", c, p, np);
        }
	}

	Delay (3600/mph);
    if (from == WEST)
    {
        ProceedRoad();
        PrintRoad();
        Printf("Car %d exits road\n", c);
        shm.wCarCount--;
        Signal(shm.road[9]);
        if (shm.wCarCount == 0)
        {
            Signal(shm.eDoor);
        }
    }
    else
    {
        ProceedRoad();
        PrintRoad();
        Printf("Car %d exits road\n", c);
        shm.eCarCount--;
        Signal(shm.road[0]);
        if (shm.eCarCount == 0)
        {
            Signal(shm.wDoor);
        }
    }
}
