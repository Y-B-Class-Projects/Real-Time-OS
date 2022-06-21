/********************************************************************/
/* A Small Real Time System for the Real-Time laboratory            */
/* built by: A.Teitelbaum on an idea of H.G.Mendelbaum              */
/* Jerusalem College of Technology, 5759-64 (1999-03)               */
/* update  Tishrey   5777                                           */
/* APP77.CPP, an application to demonstrate SMARTS77  functioning   */
/********************************************************************/
#include "smarts77.h"

// user Functions
void a( )
{
	MUTEX.acquire();
	cout << "\n *************   A Start    *********************";
	MUTEX.release();
	for (int j=0; j < 3; j++)
	{
		for (long i=0; i <600000; i++);
		MUTEX.acquire();
		cout << "A";
		MUTEX.release();
	}
	MUTEX.acquire();
	cout << "\n *************   A Finish   *********************";
	MUTEX.release();
}

void b( )
{
	MUTEX.acquire();
	cout << "\n *************   B Start    *********************";
	MUTEX.release();
	for (int j=0; j < 2; j++)
	{
		for (long i=0; i < 600000; i++);
		MUTEX.acquire();
		cout << "B";
		MUTEX.release();
	}
	MUTEX.acquire();
	cout << "\n *************   B Finish   *********************";
	MUTEX.release();
}

void c( )
{
	MUTEX.acquire();
	cout << "\n *************   C Start    *********************";
	MUTEX.release();
	for (int j=0; j <10; j++)
	{
		for (long i=0; i < 600000; i++);
		MUTEX.acquire();
		cout << "C";
		MUTEX.release();
	}
	MUTEX.acquire();
	cout << "\n *************   C Finish   *********************";
	MUTEX.release();
}

void a1()
{
	MUTEX.acquire();
	cout << "\n A";
	SMARTS.sleep(60);
	cout << "\n *************   A    *********************";
	MUTEX.release();
}
void a2()
{
	MUTEX.acquire();
	cout << "\n *************   B    *********************";
	MUTEX.release();
}
void a3()
{
	MUTEX.acquire();
	cout << "\n *************   C    *********************";
	MUTEX.release();
}

void b1()
{
	SMARTS.contextSwitchOff();
	cout << "\n A";
	SMARTS.sleep(60);
	cout << "\n *************   A    *********************";
	SMARTS.contextSwitchOn();
}
void b2()
{
	SMARTS.contextSwitchOff();
	cout << "\n *************   B    *********************";
	SMARTS.contextSwitchOn();
}
void b3()
{
	SMARTS.contextSwitchOff();
	cout << "\n *************   C    *********************";
	SMARTS.contextSwitchOn();
}


void main( )
{
	clrscr();
	int option = 0;

	switch (option)
	{
	case 0:	// regular roundRobin.
		SMARTS.externalFunctions(timerInterruptHandler, scheduler, myTaskEnd, roundRobin);
		SMARTS.declareTask(c, 'C');
		SMARTS.declareTask(a, 'A');
		SMARTS.declareTask(b, 'B');
		break;
	case 1:	// earliesDeadlineFirst with time out.
		SMARTS.externalFunctions(timerInterruptHandler, scheduler, myTaskEnd, earliesDeadlineFirst);
		SMARTS.declareTask(a, 'A', 50, 1);
		SMARTS.declareTask(b, 'B', 51, 1);
		SMARTS.declareTask(c, 'C', 1, 1);
		break;
	case 2:	// earliesDeadlineFirst
		SMARTS.externalFunctions(timerInterruptHandler, scheduler, myTaskEnd, earliesDeadlineFirst);
		SMARTS.declareTask(a, 'A', 51, 3);
		SMARTS.declareTask(b, 'B', 50, 2);
		SMARTS.declareTask(c, 'C', 49, 1);
		break;
	case 3:	// atomic section (MUTEX) with rateMonotonicScheduling.
		SMARTS.externalFunctions(timerInterruptHandler, scheduler, myTaskEnd, rateMonotonicScheduling);
		SMARTS.declareTask(a1, 'a', 70, 1, 1);
		SMARTS.declareTask(a2, 'b', 71, 1, 2);
		SMARTS.declareTask(a3, 'c', 72, 1, 3);
		break;
	case 4:	// atomic section (contextSwitch) with rateMonotonicScheduling.
		SMARTS.externalFunctions(timerInterruptHandler, scheduler, myTaskEnd, rateMonotonicScheduling);
		SMARTS.declareTask(b1, 'a', 50, 1, 1);
		SMARTS.declareTask(b2, 'b', 51, 1, 2);
		SMARTS.declareTask(b3, 'c', 52, 1, 3);
		break;
	default:
		break;
	}
 

	SMARTS.runTheTasks();

	char a;
	cin >> a;
}

