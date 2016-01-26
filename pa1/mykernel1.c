// Kieth Vo
// A09632897

/* mykernel1.c: your portion of the kernel
 *
 *	Below are functions that are called by other parts of the kernel. 
 *	Your ability to modify the kernel is via these functions.  You may
 *	modify the bodies of these functions, and add code outside or them,
 *	in any way you wish (however, you cannot change their interfaces).  
 * 
 */

#include "aux.h"
#include "sys.h"
#include "mykernel1.h"

/* MySwitchContext (p) should cause a context switch from the calling
 * process to process p.  It should return the id of the calling process. 
 * The id of the calling process can be determined by calling GetCurProc (),
 * which returns the currently running process's id.  The routine
 * SwitchContext (p) is an internal working version of context switching. 
 * This is provided so that the kernel works without modification, to
 * allow the other exercises to execute and to illustrate proper behavior.  
 * For Exercise F, the call to SwitchContext (p) must be removed.  
 */
 
int MySwitchContext (p)
{
	/*return SwitchContext(p);*/	//Original function for testing purposes.
	static int me;
	if ((me = GetCurProc()) == p)	//Checking to see if a process is
	{				//switching to itself and setting
		return me;		//the variable me at the same time.
	}
	int magic = 0;
	SaveContext (me);
	if (magic == 1)
	{
		return me;
	}
	else
	{
		magic = 1;	
	}
	RestoreContext (p);
	
}
