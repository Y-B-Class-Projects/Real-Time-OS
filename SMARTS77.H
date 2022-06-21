/********************************************************************/
/* A Small Real Time System for the Real-Time laboratory            */
/* built by: A.Teitelbaum on an idea of H.G.Mendelbaum              */
/* Jerusalem College of Technology, 5759-64 (1999)                  */
/* update  Tishrey   5777                                           */
/* SMARTS77.H, main header file                                     */
/********************************************************************/

#include <conio.h>
#include <stdio.h>
#include <dos.h>
#include <values.h>
#include <iostream.h>

/**********    Constants    **********/
#define true 1
#define false 0
#define timerInt 0x08
#define userInt 0x60
#define MaxTask 15
#define MaxStack 512
#define MaxWaiting 32
enum taskStatus {READY, NOT_ACTIVE, SUSPENDED, SLEEP, UNDEFINED};

/**********    Classes    **********/
struct item {
	int value;
	int priority;
};

class priorityQueue
{
private:
	item pr[MaxWaiting];
	int size;

public:
	priorityQueue();
	void enqueue(int value, int priority);
	int peek();
};

class Mutex
{
private:
	int _owner;
	int _s;
	int _numberSuspended;
	priorityQueue *_waitingList;
	int _level;

public:
	Mutex();
	void acquire();
	void release();
};

class Event
{
private:
	int  flag;       // arrived flag
	void *data;      // the parameter
	char source;     // source task name
	char dest;       // destination task name
	int senderWait;  // tells if the sender has to wait till the reciver receives
	int testAndSet();// test and set 'flag' in an 'atomic transaction'
 public:
	Event();
	// sends an event to 'dest', if 'synch' is true waits until the receiver gets it.
	void send(char dest,void *param, int synch);
	// waits for the event, and returns the parameter. 
	void *wait(char &sourceP); //'sourceP' will contain the task sender ID.
	// check for for an arrived event. 'sourceP' will contain the task sender ID.
	int arrived(char &sourceP);
	// resets the event
	void reset();
};

// SMARTS manages an entry of this type for each task
class Task 
{
public:
	char name;                      // task name
	unsigned stack[MaxStack];       // the stack
	unsigned stack_copy[MaxStack];  // copy of original stack (for restore).
	unsigned stackSeg, stackPtr;    // the stack pointer
	unsigned stackSeg_copy, stackPtr_copy;    // copy of the stack pointer
	int is_restore;
	int currentPriority;            // the priority of the task
	int priority;                   // the original priority
	taskStatus status;              // the status of the task
	Event *expectedEvent;           // the event that this task is waiting for
	int sleepCount;                 // the sleep remainder time
	Task();
	void declare(void far *code, void far *taskEnd, char name, int periodTime = 0, int cycles = 0, int priority = 0);
	void sleepDecr();
	void incrPriority();
	void setOriginalPriority();
	int periodTime;					// the task should be complete every 'periodTime'.
	int cycles;						// the task should be run 'cycles' times.
	int periodTimeLast;				// time remain till the end of the cycle.

	int isContextSwitch;
	//void atomicSectionOn();
	//void atomicSectionOff();

	void reDeclareTask();
};
//                           Main class
class Parallelism
{
private:
	Task context[MaxTask];  // all tasks entries
	Task contextSched;      // scheduler entry
	unsigned schedCopy[MaxStack];  // a copy of the scheduler stack
	int totalTasks;          // total declared tasks
	int currentTask;        // time slice holding task
	int deadlock;           // deadlock flag
	int progInt;            // 'program interrupt' flag
	int contextSwitchFlag;  // enables/disables context switch
	int endOfTimeSlice;     // indicates that has to be a context switch as soon as possible
		// a pointer to the new timer interrupt handler
	void interrupt ( *timerInterruptHandler)(...);
		// a pointer to the original BIOS timer interrupt handler
	void interrupt ( *userIntAddress)(...);
	void far *scheduler;    // a pointer to the scheduler
	void far *userTaskEnd;      // a pointer to the function called at the end of each task
public:
	int sleepTasks;         // indicates how many task is sleeping, and it can't be deadlock
	int activeTasks;        // indicates how many task is activate 
	int timeOutTasks;
	int traceInd;           // debugging index
	long TScount;           // counts the clocks of the current task
	unsigned timerClocksBegin;// time start of the current task
	unsigned timerClocksEnd;// time finish of the current task
	int far (*algorithm)(); // short-term scheduler algorithm function
	Parallelism();
		// sets the addresses of the external functions
	void externalFunctions(void interrupt ( *timerInterruptHandler)(...),
				   void far *scheduler, void far *userTaskEnd,
				   int far (*algorithm)());
	int declareTask(void far *code, char name, int periodTime = 0, int cycles = 0, int priority = 0); // inserts a new task entry
	void runTheTasks();// start running the tasks
	void callScheduler();          // return to the control to the scheduler
	void restoreSchedStack();       // restore the 'scheduler' stack
	int getCurrentTask();           // get the current running task entry number
	void setCurrentTask(int taskNum);// set the next running task number
	int getTotalTasks();             // get total declared tasks
	int getDeadlock();              // get deadlock flag
	void setDeadlock();             // set deadlock flag
	int contextSwitchOn();          // enable context switch
	void contextSwitchOff();        // disable context switch
	int getContextSwitch();         // get 'context switch' flag status
	void setProgInt();              // set the 'program interrupt' flag
	void resetProgInt();            // clear the 'program interrupt' flag
	int getProgInt();               // get the 'program interrupt' flag status
	void setEndOfTimeSlice();       // set the 'end of time slice' flag
	char getName(int taskNum);
	char getCurrentName();
	int getCurrentPriority();
	taskStatus getStatus(int taskNum);
	taskStatus getCurrentStatus();
	void resume(int taskNum);
	void resume(char taskName);
	void setCurrentNotActive();
	void suspended();
	void incrPriority(int taskNum );
	void setOriginalPriority(int taskNum );
	void setCurrentOriginalPriority();
	Event *getExpectedEvent(int taskNum);
	Event *getCurrentExpectedEvent();
	void setCurrentExpectedEvent(Event *expectedEvent);
	void sleep(int t);             // sleep current task for 't' milliseconds
	void sleepDecr(int taskNum);
	void getCurrentStack(unsigned &StackSeg, unsigned &StackPtr);
	void setCurrentStack(unsigned StackSeg, unsigned StackPtr);
	void getSchedStack(unsigned &StackSeg, unsigned &StackPtr);
	void handleTimers();
	void handleClockPeriods();	//function that reduce priodTime by 1 for each task. This function is called every clock cycle.
	void taskEnd();
	int getCycles(int taskNum);
	int getPeriodTime(int taskNum);
	int getPeriodTimeLast(int taskNum);
	int decPeriodTime(int taskNum);		//function that reduce priodTime by 1 for spacific task.
};
//=======================
/**********    Functions    **********/
	// reads the number of the remaining clocks from the timer register
extern unsigned getTimerClocks();
	// external functions for 'externalFunctions' function
void far interrupt timerInterruptHandler(...);
void scheduler();
void myTaskEnd();
	// 'short term' scheduler algorithms 
int roundRobin();
int earliesDeadlineFirst();
int rateMonotonicScheduling();
	// main scheduler object
extern Parallelism SMARTS;
extern Mutex MUTEX;

