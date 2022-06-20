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
	//SMARTS.contextSwitchOff();
	cout << "\n A";
	for (long i = 0; i < 6000000; i++);
	cout << "\n *************   A1    *********************";
	MUTEX.release();
	//SMARTS.contextSwitchOn();
}
void a2()
{
	MUTEX.acquire();
	//SMARTS.contextSwitchOff();;
	cout << "\n *************   A2    *********************";
	MUTEX.release();
	//SMARTS.contextSwitchOn();
}
void a3()
{
	MUTEX.acquire();
	//SMARTS.contextSwitchOff();
	cout << "\n *************   A3    *********************";
	MUTEX.release();
	//SMARTS.contextSwitchOn();
}



void main( )
{
	clrscr();
	SMARTS.externalFunctions(timerInterruptHandler, scheduler, myTaskEnd, roundRobin);
	//SMARTS.declareTask(a,'A', 51, 3, 30);
	//SMARTS.declareTask(b,'B', 50, 2, 20);
	//SMARTS.declareTask(c,'C', 49, 1, 10);
	SMARTS.declareTask(a3, 'c', 102, 2, 3);
	SMARTS.declareTask(a1, 'a', 100, 2, 1);
	SMARTS.declareTask(a2, 'b', 101, 2, 2);



	SMARTS.runTheTasks();

	char a;
	cin >> a;
}

