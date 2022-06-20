/****************************************************************/
/* A Small Real Time System for the Real-Time laboratory        */
/* built by: A.Teitelbaum on an idea of H.G.Mendelbaum          */
/* Jerusalem College of Technology, 5759-64 (1999)              */
/* update  Tishrey   5777                                       */
/* SMARTS77.CPP, SMARTS class body                              */
/****************************************************************/
#include "smarts77.h"

/**********    Function     **********/
unsigned getTimerClocks( )
// Gets the remaining clocks of the timer register
{
	unsigned clocks;
		/* latch counter #0 */
	outportb(0x43,0x00);
		/* read counter #0 low byte */
	clocks=inportb(0x40);
		/* read counter #0 high byte */
	clocks += inportb(0x40)<<8;
	return clocks;
}
////////////////////////////////////////////////////
/**********    class body:  Parallelism     **********/
Parallelism::Parallelism()
{
	currentTask = 0;
	sleepTasks  = 0;
	activeTasks = 0;
	totalTasks  = 0;
	deadlock    = false;
	contextSwitchFlag = true;
	endOfTimeSlice = true;
}

void Parallelism::externalFunctions(void interrupt ( *timerInterruptHandler)(...),
				    void far *scheduler, void far *userTaskEnd,
				    int far (*algorithm)( ))
		// Sets the external functions
{
	this->timerInterruptHandler = timerInterruptHandler;
	this->scheduler = scheduler;
	this->userTaskEnd = userTaskEnd;
	this->algorithm = algorithm;
	contextSched.declare(scheduler, userTaskEnd, 'S', 0, 0, 0); // prepare the stack of the scheduler task
	for (int i=MaxStack-1; i >= (MaxStack-14); i--)
		schedCopy[i]=contextSched.stack[i];
}

int Parallelism::declareTask(void far *code, char name, int periodTime, int cycles, int priority)
		// Insert a new task entry in SMARTS context array [ ]
{
	if (totalTasks < MaxTask-1) 
	{
		context[totalTasks++].declare(code, userTaskEnd, name, periodTime, cycles, priority);
		++activeTasks;
		return true;
	}
	else	
		return false;
}

void Parallelism::runTheTasks()
		// Start running all tasks declared in SMARTS.		
{
	context[totalTasks].status = READY;	//for task 'runTheTasks' (IDLE task)
	context[totalTasks].priority = MAXINT;
	context[totalTasks].currentPriority = MAXINT;

	currentTask = totalTasks;

	asm	cli;		// forbids interrupts (clear interrupts) while changing the interrupt vect
		// saves the original BIOS userInt in our variable 'userIntAddress' to be restored at the end
	userIntAddress = getvect(userInt);	// BIOS userInt 0x60  (unused by PC)
				// puts the normal BIOS timerInt into the unused userInt address
	setvect(userInt,getvect(timerInt));	// timerInt 0x08 
	
	// sets our SMARTS external function 'timerInterruptHandler' as the new PC hard interrupt time handler
	setvect(timerInt,timerInterruptHandler);
	asm	sti;	// allows back interrupts (set interrupts)

	    // waits for end of runTheTasks (end of all the tasks)
	while(true)
	{
		if (deadlock)
		{
			textcolor(RED);
			cprintf("\n\n\rExit : deadlock");
			break;
		}
		if (activeTasks==0)
		{
			cprintf("\n\n\rExit : finish");
			break;
		}
		if (timeOutTasks > 0)
		{
			cprintf("\n\n\rExit : time error");
			break;
		}
		
	}

		// restore the original BIOS 'interrupt vector' at the end before returning to regular DOS
	asm	cli;	// no interrupts
	setvect(timerInt, getvect(userInt));	// restore original BIOS time handler
	setvect(userInt, userIntAddress);	// restore original BIOS userInt
	asm	sti;	// yes interrupts
}

void Parallelism::callScheduler( )
	// Return the control to the scheduler, this sets ON the software interrupt ProgInt flag
{
	setProgInt( );
	asm int timerInt;
}

void Parallelism::restoreSchedStack( )
	// Restore the scheduler stack
{
	for (int i=MaxStack-1; i >= (MaxStack-14); i--)
		contextSched.stack[i] = schedCopy[i];
}

int Parallelism::getCurrentTask( )
{	
	return currentTask;
}

void Parallelism::setCurrentTask(int taskNum)
	// Sets the next task to be run
{	
	if (taskNum <= totalTasks)
		currentTask = taskNum;
}

int Parallelism::getTotalTasks( )
	// Gets total tasks declared
{	
	return totalTasks;
}

int Parallelism::getDeadlock( )
{	
	return deadlock;
}

void Parallelism::setDeadlock( )
{	
	deadlock = true;
}

int Parallelism::contextSwitchOn( )
				// flag which enables context switch
{
	if (endOfTimeSlice) //is current time slice finished ?
	{	
		endOfTimeSlice = false;
		//contextSwitchFlag = true;
		context[currentTask].isContextSwitch = true;
		callScheduler();	// return control to the scheduler
		return 1;
	}
	//contextSwitchFlag = true;
	context[currentTask].isContextSwitch = true;
	return 0;
}

void Parallelism::contextSwitchOff( )
				// Disable context switch
{	
	//contextSwitchFlag = false;
	context[currentTask].isContextSwitch = false;
}

int Parallelism::getContextSwitch( )
{	
	//return contextSwitchFlag;
	return context[currentTask].isContextSwitch;
}

void Parallelism::setProgInt( )
			// flag indicates to the extern function 'timerInterruptHandler' 
			// that this is an internal SMARTS software interrupt call, 
			// and the original BIOS function has no to be called.
{	
	progInt = true;
}

void Parallelism::resetProgInt()
{	
	progInt = false;
}

int Parallelism::getProgInt( )
{
	return progInt;
}

void Parallelism::setEndOfTimeSlice( )
		// flag indicates that when 'context switch' will be enabled, 
		// it must also return the control to the scheduler.
{	
	endOfTimeSlice = true;
}

char Parallelism::getName(int taskNum)	// returns name found or ' ' if not
{	
	return (taskNum <= totalTasks)? context[taskNum].name : ' ';
}

int Parallelism::getPeriodTime(int taskNum)	// returns periodTime found or ' ' if not
{
	return (taskNum <= totalTasks) ? context[taskNum].periodTime : -1;
}

int Parallelism::getPeriodTimeLast(int taskNum)	// returns periodTime found or ' ' if not
{
	return (taskNum <= totalTasks) ? context[taskNum].periodTimeLast : -1;
}

int Parallelism::decPeriodTime(int taskNum)
{
	//cprintf("in decPeriodTime() ");
	//cout << context[taskNum].periodTimeLast << endl;
	if (taskNum <= totalTasks - 1)	// without scheduler.
	{
		context[taskNum].periodTimeLast--;

		if (context[taskNum].periodTimeLast == 0 && context[taskNum].cycles > 0)
		{		
			context[taskNum].reDeclareTask();
		}
	}
}

int Parallelism::getCycles(int taskNum)	// returns Cycles found or ' ' if not
{
	return (taskNum <= totalTasks) ? context[taskNum].cycles : ' ';
}

char Parallelism::getCurrentName( )
{	
	return context[currentTask].name;
}

int Parallelism::getCurrentPriority()
{
	return context[currentTask].currentPriority;
}


taskStatus Parallelism::getStatus(int taskNum)  	
	// returns status or undefined if not found
{
	return (taskNum <= totalTasks)? context[taskNum].status : UNDEFINED;
}

taskStatus Parallelism::getCurrentStatus( )
{	
	return context[currentTask].status;
}

void Parallelism::resume(int taskNum)
{
	if (taskNum < totalTasks)
		context[taskNum].status = READY;
}

void Parallelism::resume(char taskName)
{
	for(int i=0;i<totalTasks;++i)
		if (context[i].name == taskName)
			context[i].status = READY;
}


void Parallelism::setCurrentNotActive()
{	
	context[currentTask].status = NOT_ACTIVE;
	if(context[currentTask].cycles == 1)
		--activeTasks;
}
void Parallelism::suspended()
{	
	context[currentTask].status = SUSPENDED;
	callScheduler();
}

void Parallelism::incrPriority(int taskNum )
{
	if (taskNum < totalTasks)
		context[taskNum].incrPriority();
}
void Parallelism::setOriginalPriority(int taskNum )
{
	if (taskNum < totalTasks)
		context[taskNum].setOriginalPriority();
}

void Parallelism::setCurrentOriginalPriority()
{
	context[currentTask].setOriginalPriority();
}

Event *Parallelism::getExpectedEvent(int taskNum)    
// returns *Event  or  NULL  if not found
{	
	return (taskNum <= totalTasks)? context[taskNum].expectedEvent : NULL;
}

Event *Parallelism::getCurrentExpectedEvent( )
{	
	return context[currentTask].expectedEvent;
}

void Parallelism::setCurrentExpectedEvent(Event *expectedEvent)
{	
	context[currentTask].expectedEvent = expectedEvent;
}

void Parallelism::sleep(int t)
// Current task sleeps for 't' milliseconds
{
	if (t < MAXINT) 
	{
		context[currentTask].sleepCount = t/55+1;
		context[currentTask].status = SLEEP;
		++sleepTasks;
		callScheduler();		// return control to scheduler
	}
}

void Parallelism::sleepDecr(int taskNum)
{
	if (taskNum < totalTasks)
		context[taskNum].sleepDecr();
}

void Parallelism::getCurrentStack(unsigned &StackSeg, unsigned &StackPtr)
	// Load current task stack pointer
{   
	if (context[currentTask].is_restore != 0)	// if
	{
		for (int i = MaxStack - 1; i >= (MaxStack - 14); i--)
			context[currentTask].stack[i] = context[currentTask].stack_copy[i];

		StackSeg = context[currentTask].stackSeg_copy;
		StackPtr = context[currentTask].stackPtr_copy;

		context[currentTask].is_restore = 0;
	}
	else
	{
		StackSeg = context[currentTask].stackSeg;
		StackPtr = context[currentTask].stackPtr;
	}	
}

void Parallelism::setCurrentStack(unsigned StackSeg, unsigned StackPtr)
	// Save current task stack pointer
{
	context[currentTask].stackSeg = StackSeg;
	context[currentTask].stackPtr = StackPtr;
}

void Parallelism::getSchedStack(unsigned &StackSeg, unsigned &StackPtr)
					// Load scheduler  stack pointer
{   
	StackSeg = contextSched.stackSeg;
	StackPtr = contextSched.stackPtr;
}

void Parallelism::handleTimers()
	// handling of the sleep status mode
{
	for (int i=totalTasks-1; i >=0 ; --i) 
	{	
		if(getStatus(i)==SLEEP)
		{
			sleepDecr(i);
			if (getStatus(i) == READY)
				--sleepTasks;
		}

	}

}


void Parallelism::handleClockPeriods()
{
	for (int i = totalTasks - 1; i >= 0; --i)
	{
		decPeriodTime(i);
	}
}

void Parallelism::taskEnd( )
	// This function is called after the last operation of a task, instead of function return
{
	SMARTS.setCurrentNotActive();
	SMARTS.callScheduler();	// return the control to the scheduler to run a next task
}

/**********    class body:  Task     **********/
Task::Task()
{
	stack[MaxStack-14]=_BP;
	stack[MaxStack-13]=_DI;
	stack[MaxStack-12]=_SI;
	stack[MaxStack-11]=_DS;
	stack[MaxStack-10]=_ES;
	stack[MaxStack-9]=_DX;
	stack[MaxStack-8]=_CX;
	stack[MaxStack-7]=_BX;
	stack[MaxStack-6]=_AX;
	stackSeg=FP_SEG(&stack[MaxStack-14]);
	stackPtr=FP_OFF(&stack[MaxStack-14]);
	status = NOT_ACTIVE;
	sleepCount = 0;
	currentPriority=priority=0;
	isContextSwitch = true;
}

//-----------------------------------------------------
void Task::declare(void far *code, void far *userTaskEnd, char name, int periodTime, int cycles, int priority)
{
	stack[MaxStack-5]=FP_OFF(code);
	stack[MaxStack-4]=FP_SEG(code);
	stack[MaxStack-3]=_FLAGS;
	stack[MaxStack-2]=FP_OFF(userTaskEnd);
	stack[MaxStack-1]=FP_SEG(userTaskEnd);

	for (int i = MaxStack - 1; i >= (MaxStack - 14); i--)	//Save the initial stack values for future recovery
		stack_copy[i] = stack[i];

	stackSeg_copy = stackSeg;
	stackPtr_copy = stackPtr;

	is_restore = 0;

	this->periodTime = periodTime;	// the task should be complete every 'periodTime'
	this->cycles = cycles;			// the task should be run 'cycles' times
	this->name= name;

	periodTimeLast = this->periodTime;

	currentPriority = priority;
	this->priority = priority;

	status = READY;
}

//----------------------------------------------------
void Task::incrPriority( )
{
 	--currentPriority;
}
//----------------------------------------------------
void Task::setOriginalPriority( )
{
	currentPriority = priority;
}
//----------------------------------------------------
void Task::sleepDecr( )
	// Decrements the sleep counter and update the task status accordingly
{	
	if (status==SLEEP)
	{
		if (sleepCount > 0)
			--sleepCount;
		if (!sleepCount)
			status = READY;
	}
}

void Task::reDeclareTask()
{
	periodTimeLast = periodTime;
	cycles--;

	if (status == NOT_ACTIVE)
	{
		if (cycles > 0)
		{
			//MUTEX.acquire();MUTEX.acquire();
			status = READY;
			is_restore = 1;
			//MUTEX.release();
		}
		else
			status = NOT_ACTIVE;
	}
	else
	{
		MUTEX.acquire();
		cprintf("Time error %c ", name);
		status = UNDEFINED;
		SMARTS.timeOutTasks++;
		MUTEX.release();
	}
}

void Mutex::Mutex()
{
	_owner = -1;
	_level = 0;
	_s = 1;
	_waitingList = new priorityQueue();
	_numberSuspended = 0;
}


void Mutex::acquire()
{
	if (_s == 1 || _owner == SMARTS.getCurrentTask())
		_s = 0;
	else
	{
		_numberSuspended++;
		_waitingList->enqueue(SMARTS.getCurrentTask(), SMARTS.getCurrentPriority());
		SMARTS.suspended();		// suspending the current task.
	}
	_owner = SMARTS.getCurrentTask();
	_level++;
}

void Mutex::release()
{
	if (_owner == SMARTS.getCurrentTask())
		if (--_level)
			return;
		else {
			_owner = -1;
			if (_numberSuspended > 0) {
				_numberSuspended--;
				int task = _waitingList->peek();
				SMARTS.resume(task);
			}
			else
				_s = 1;
		}
}

priorityQueue::priorityQueue()
{
	size = 0;
}

void priorityQueue::enqueue(int value, int priority)
{
	pr[size].value = value;
	pr[size].priority = priority;

	size++;
}

int priorityQueue::peek()
{
	int highestPriority = -999;
	int ind = -1;

	for (int i = 0; i < size; i++) 
	{
		if (highestPriority < pr[i].priority) 
		{
			highestPriority = pr[i].priority;
			ind = i;
		}
	}

	int ret = pr[ind].value;
	
	for (i = ind; i < size; i++) 
		pr[i] = pr[i + 1];

	size--;

	return ret;
}