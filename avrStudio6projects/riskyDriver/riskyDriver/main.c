/*
    FreeRTOS V7.4.0 - Copyright (C) 2013 Real Time Engineers Ltd.

    FEATURES AND PORTS ARE ADDED TO FREERTOS ALL THE TIME.  PLEASE VISIT
    http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS tutorial books are available in pdf and paperback.        *
     *    Complete, revised, and edited pdf reference manuals are also       *
     *    available.                                                         *
     *                                                                       *
     *    Purchasing FreeRTOS documentation will not only help you, by       *
     *    ensuring you get running as quickly as possible and with an        *
     *    in-depth knowledge of how to use FreeRTOS, it will also help       *
     *    the FreeRTOS project to continue with its mission of providing     *
     *    professional grade, cross platform, de facto standard solutions    *
     *    for microcontrollers - completely free of charge!                  *
     *                                                                       *
     *    >>> See http://www.FreeRTOS.org/Documentation for details. <<<     *
     *                                                                       *
     *    Thank you for using FreeRTOS, and thank you for your support!      *
     *                                                                       *
    ***************************************************************************


    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation AND MODIFIED BY the FreeRTOS exception.

    >>>>>>NOTE<<<<<< The modification to the GPL is included to allow you to
    distribute a combined work that includes FreeRTOS without being obliged to
    provide the source code for proprietary components outside of the FreeRTOS
    kernel.

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
    details. You should have received a copy of the GNU General Public License
    and the FreeRTOS license exception along with FreeRTOS; if not itcan be
    viewed here: http://www.freertos.org/a00114.html and also obtained by
    writing to Real Time Engineers Ltd., contact details for whom are available
    on the FreeRTOS WEB site.

    1 tab == 4 spaces!

    ***************************************************************************
     *                                                                       *
     *    Having a problem?  Start by reading the FAQ "My application does   *
     *    not run, what could be wrong?"                                     *
     *                                                                       *
     *    http://www.FreeRTOS.org/FAQHelp.html                               *
     *                                                                       *
    ***************************************************************************


    http://www.FreeRTOS.org - Documentation, books, training, latest versions, 
    license and Real Time Engineers Ltd. contact details.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, and our new
    fully thread aware and reentrant UDP/IP stack.

    http://www.OpenRTOS.com - Real Time Engineers ltd license FreeRTOS to High 
    Integrity Systems, who sell the code with commercial support, 
    indemnification and middleware, under the OpenRTOS brand.
    
    http://www.SafeRTOS.com - High Integrity Systems also provide a safety 
    engineered and independently SIL3 certified version for use in safety and 
    mission critical applications that require provable dependability.
*/

/*
 * Creates all the demo application tasks, then starts the scheduler.  The WEB
 * documentation provides more details of the demo application tasks.
 * 
 * Main. c also creates a task called "Check".  This only executes every three 
 * seconds but has the highest priority so is guaranteed to get processor time.  
 * Its main function is to check that all the other tasks are still operational.  
 * Each task that does not flash an LED maintains a unique count that is 
 * incremented each time the task successfully completes its function.  Should 
 * any error occur within such a task the count is permanently halted.  The 
 * check task inspects the count of each task to ensure it has changed since
 * the last time the check task executed.  If all the count variables have 
 * changed all the tasks are still executing error free, and the check task
 * toggles an LED.  Should any task contain an error at any time the LED toggle
 * will stop.
 *
 * The LED flash and communications test tasks do not maintain a count.
 */

/*
Changes from V1.2.0
	
	+ Changed the baud rate for the serial test from 19200 to 57600.

Changes from V1.2.3

	+ The integer and comtest tasks are now used when the cooperative scheduler 
	  is being used.  Previously they were only used with the preemptive
	  scheduler.

Changes from V1.2.5

	+ Set the baud rate to 38400.  This has a smaller error percentage with an
	  8MHz clock (according to the manual).

Changes from V2.0.0

	+ Delay periods are now specified using variables and constants of
	  uint32_t rather than unsigned long.

Changes from V2.6.1

	+ The IAR and WinAVR AVR ports are now maintained separately.

Changes from V4.0.5

	+ Modified to demonstrate the use of co-routines.

*/

#define F_CPU   8000000UL

#include "portmacro.h"
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include <stdlib.h>
#include <string.h>

#define GCC_MEGA_AVR

#ifdef GCC_MEGA_AVR
	/* EEPROM routines used only with the WinAVR compiler. */
	#include <avr/eeprom.h> 
#endif

/* Scheduler include files. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

//#include "croutine.h"

/* Demo file headers. */
//#include "PollQ.h"
//#include "integer.h"
//#include "serial.h"
//#include "comtest.h"
//#include "crflash.h"
//#include "print.h"
//#include "partest.h"
//#include "regtest.h"

#include "communicationTask.h"
#include "navigatorTask.h"
#include "operatorTask.h"
#include "sensorTask.h"


/* Priority definitions for most of the tasks in the demo application.  Some
tasks just use the idle priority. */
#define mainCOMMUNICATION_TASK_PRIORITY ( tskIDLE_PRIORITY + 1 )
#define mainNAVIGATOR_TASK_PRIORITY ( tskIDLE_PRIORITY + 2 )
#define mainOPERATOR_TASK_PRIORITY ( tskIDLE_PRIORITY + 3 )
#define mainSENSOR_TASK_PRIORITY ( tskIDLE_PRIORITY + 3 )


/* Baud rate used by the serial port tasks. */
//#define mainCOM_TEST_BAUD_RATE			( ( unsigned long ) 38400 )

/* LED used by the serial port tasks.  This is toggled on each character Tx,
and mainCOM_TEST_LED + 1 is toggles on each character Rx. */
//#define mainCOM_TEST_LED				( 4 )

/* LED that is toggled by the check task.  The check task periodically checks
that all the other tasks are operating without error.  If no errors are found
the LED is toggled.  If an error is found at any time the LED is never toggles
again. */
//#define mainCHECK_TASK_LED				( 7 )

/* The period between executions of the check task. */
//#define mainCHECK_PERIOD				( ( uint32_t ) 3000 / portTICK_RATE_MS  )

/* An address in the EEPROM used to count resets.  This is used to check that
the demo application is not unexpectedly resetting. */
#define mainRESET_COUNT_ADDRESS			( ( void * ) 0x50 )

/* The number of coroutines to create. */
//#define mainNUM_FLASH_COROUTINES		( 3 )

/*
 * The task function for the "Check" task.
 */
//static void vErrorChecks( void *pvParameters );

/*
 * Called on boot to increment a count stored in the EEPROM.  This is used to 
 * ensure the CPU does not reset unexpectedly.
 */
static void prvIncrementResetCount( void );

/*
 * The idle hook is used to scheduler co-routines.
 */
void vApplicationIdleHook( void );

/*-----------------------------------------------------------*/

int main( void )
{  
    //set the length of the queues -> how many items the queue can hold
    const unsigned portBASE_TYPE measurementQueueLength = 5;
    //const unsigned portBASE_TYPE statusQueueLength = 5;
    
    xQueueHandle measurementQueue;
    //xQueueHandle statusQueue;
    
    //configure time measurment pin as output
    DDRD |= (1<<PB7);

	//prvIncrementResetCount();
    
    //create queues for intertask communication
    measurementQueue = xQueueCreate( measurementQueueLength, ( unsigned portBASE_TYPE ) sizeof( measurementQueueMsg * ) );
    //statusQueue = xQueueCreate( statusQueueLength, ( unsigned portBASE_TYPE ) sizeof( statusQueueMsg * ) );
        
	/* Create the tasks. */
    startCommunicationTask( mainCOMMUNICATION_TASK_PRIORITY, &measurementQueue );
    startNavigatorTask( mainNAVIGATOR_TASK_PRIORITY, &measurementQueue );
    startOperatorTask( mainOPERATOR_TASK_PRIORITY, &measurementQueue );
	startSensorTask( mainSENSOR_TASK_PRIORITY, &measurementQueue);
	
	/* In this port, to use preemptive scheduler define configUSE_PREEMPTION 
	as 1 in portmacro.h.  To use the cooperative scheduler define 
	configUSE_PREEMPTION as 0. */
	vTaskStartScheduler();

	return 0;
}
/*-----------------------------------------------------------*/

static void prvIncrementResetCount( void )
{
unsigned char ucCount;

	eeprom_read_block( &ucCount, mainRESET_COUNT_ADDRESS, sizeof( ucCount ) );
	ucCount++;
	eeprom_write_byte( mainRESET_COUNT_ADDRESS, ucCount );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
	//vCoRoutineSchedule();
}

