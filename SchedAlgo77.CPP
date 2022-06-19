/********************************************************************/
/* A Small Real Time System for the Real-Time laboratory                 */
/* built by: A.Teitelbaum on an idea of H.G.Mendelbaum                  */
/* Jerusalem College of Technology, 5759-64 (1999)                            */
/* SchedAlgo64.CPP, short-term scheduler algorithms                */
/****************************************************************/


#include "smarts77.h"

int roundRobin( )
// Round Robin short-term algorithm 
{
	int nextTask = (SMARTS.getCurrentTask()+1)%SMARTS.getTotalTasks( );

	for (int count=0; SMARTS.getStatus(nextTask)!=READY && count<SMARTS.getTotalTasks( ); count++)
	     nextTask=++nextTask%SMARTS.getTotalTasks( );

	if (count == SMARTS.getTotalTasks( ))	// no found READY task to run
		nextTask = SMARTS.getTotalTasks( );

	return nextTask; 
}

int earliesDeadlineFirst()
{
	//return the task with the earliest deadline.

	int nextTask = -1;
	int min = 999;

	for (int i = 0; i < SMARTS.getTotalTasks(); i++)
	{
		if (SMARTS.getStatus(i) == READY && SMARTS.getCycles(i) > 0)
		{
			int taskPeriodTimeLast = SMARTS.getPeriodTimeLast(i);

			if (taskPeriodTimeLast < min)
			{
				min = taskPeriodTimeLast;
				nextTask = i;
			}
		}
	}

	if (nextTask < 0)	// no found READY task to run
		nextTask = SMARTS.getTotalTasks();	// call main next time.

	return nextTask;
}