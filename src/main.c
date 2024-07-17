/*
 * This file is part of the µOS++ distribution.
 *   (https://github.com/micro-os-plus)
 * Copyright (c) 2014 Liviu Ionescu.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include "diag/trace.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"
#include "diag/trace.h"

#define CCM_RAM __attribute__((section(".ccmram")))

void initRandom() {
    srand(time(NULL));
}

int getRandomValue(int lower, int upper) {
    return (rand() % (upper - lower + 1)) + lower;
}

#define QueueSize 3
#define ReceiverThreshold 1000
const int Lower_Bounds[] = {50, 80, 110, 140, 170, 200};
const int Upper_Bounds[] = {150, 200, 250, 300, 350, 400};
int Bound_Index;

QueueHandle_t xQueue;
SemaphoreHandle_t xSemaphoreSender1, xSemaphoreSender2, xSemaphoreSender3, xSemaphoreReceiver;
TimerHandle_t xTimerSender1, xTimerSender2, xTimerSender3, xTimerReceiver;
TickType_t xTimeSender1, xTimeSender2, xTimeSender3;
int Total_Sent, Total_Blocked, Total_Received;
int Sender1_Sent, Sender1_Blocked,
	Sender2_Sent, Sender2_Blocked,
	Sender3_Sent, Sender3_Blocked;
int w, x, y;
int sum1, sum2, sum3;
int c1, c2, c3;

void vSenderTask1(void *pvParameters);
void vSenderTask2(void *pvParameters);
void vSenderTask3(void *pvParameters);
void vReceiverTask(void *pvParameters);

void vSenderTimerCallback1(TimerHandle_t xTimer);
void vSenderTimerCallback2(TimerHandle_t xTimer);
void vSenderTimerCallback3(TimerHandle_t xTimer);
void vReceiverTimerCallback(TimerHandle_t xTimer);

void InitializeSystem();
void Reset();
void GameOver();

int main(void) {
    initRandom();

    xQueue = xQueueCreate(QueueSize, sizeof(char*));
    if (xQueue == NULL) {
        trace_printf("Failed to create queue\n");
        return 1;
    }

    xSemaphoreSender1 = xSemaphoreCreateBinary();
    xSemaphoreSender2 = xSemaphoreCreateBinary();
    xSemaphoreSender3 = xSemaphoreCreateBinary();
    xSemaphoreReceiver = xSemaphoreCreateBinary();

    if (xSemaphoreSender1 == NULL || xSemaphoreSender2 == NULL || xSemaphoreSender3 == NULL || xSemaphoreReceiver == NULL) {
        trace_printf("Failed to create semaphores\n");
        return 1;
    }

    InitializeSystem();

    xTimerSender1 = xTimerCreate("TimerSender1", pdMS_TO_TICKS(w), pdFALSE, (void*)0, vSenderTimerCallback1);
    xTimerSender2 = xTimerCreate("TimerSender2", pdMS_TO_TICKS(x), pdFALSE, (void*)0, vSenderTimerCallback2);
    xTimerSender3 = xTimerCreate("TimerSender3", pdMS_TO_TICKS(y), pdFALSE, (void*)0, vSenderTimerCallback3);
    xTimerReceiver = xTimerCreate("TimerReceiver", pdMS_TO_TICKS(100), pdTRUE, (void*)0, vReceiverTimerCallback);

   if (xTimerSender1 == NULL || xTimerSender2 == NULL || xTimerSender3 == NULL || xTimerReceiver == NULL) {
        trace_printf("Failed to create timers\n");
        return 1;
    }

   BaseType_t xStatusSender1,xStatusSender2,xStatusSender3,xStatusReceiver;
            xStatusSender1 = xTaskCreate(vSenderTask1, "Sender1", 1000, (void*)100, 1, NULL);
            xStatusSender2 = xTaskCreate(vSenderTask2, "Sender2", 1000, (void*)200, 1, NULL);
            xStatusSender3 = xTaskCreate(vSenderTask3, "Sender3", 1000, (void*)300, 2, NULL);
            xStatusReceiver = xTaskCreate(vReceiverTask, "Receiver", 1000, (void*)400, 3, NULL);

             if (xStatusSender1 != pdPASS || xStatusSender2 != pdPASS || xStatusSender3 != pdPASS || xStatusReceiver != pdPASS) {
                 trace_printf("Failed to create tasks\n");
                 return 1;
             }

   BaseType_t timerStarted1, timerStarted2, timerStarted3, timerStartedReceiver;
            timerStarted1 = xTimerStart(xTimerSender1, 0);
            timerStarted2 = xTimerStart(xTimerSender2, 0);
            timerStarted3 = xTimerStart(xTimerSender3, 0);
            timerStartedReceiver = xTimerStart(xTimerReceiver, 0);

             if(timerStarted1 == pdPASS && timerStarted2 == pdPASS && timerStarted3 == pdPASS && timerStartedReceiver == pdPASS)
                vTaskStartScheduler();
             else {
                 trace_printf("Failed to start one or more timers\n");
                 return 1;
                }
    for (;;);
}

void InitializeSystem(){
	Total_Sent = 0, Total_Blocked = 0, Total_Received = 0;
	Sender1_Sent = 0, Sender1_Blocked = 0;
    Sender2_Sent = 0, Sender2_Blocked = 0;
	Sender3_Sent = 0, Sender3_Blocked = 0;
	Bound_Index = 0;
	w = getRandomValue(Lower_Bounds[0], Upper_Bounds[0]);
    x = getRandomValue(Lower_Bounds[0], Upper_Bounds[0]);
	y = getRandomValue(Lower_Bounds[0], Upper_Bounds[0]);
	sum1 = 0, sum2 = 0, sum3 = 0;
	c1 = 0, c2 = 0, c3 = 0;
	xQueueReset(xQueue);
}

void vSenderTask1(void *pvParameters) {
	char Message[100];
	while(1) {
		    	BaseType_t xStatus,xStatusSemaphore1;
		    	xStatusSemaphore1 = xSemaphoreTake(xSemaphoreSender1, portMAX_DELAY);
		        if (xStatusSemaphore1 == pdTRUE) {
		            xTimeSender1 = xTaskGetTickCount();
		            sprintf(Message, "Time is %ld", xTimeSender1);
		            char *PtrToMessage = strdup(Message);
		            xStatus = xQueueSend(xQueue, &PtrToMessage, 0);
		            if (xStatus != pdPASS) {
		                Sender1_Blocked++;Total_Blocked++;
		                free(PtrToMessage);
		            } else {
		                Sender1_Sent++;Total_Sent++;
		            }
            sum1 += w;
            w = getRandomValue(Lower_Bounds[Bound_Index], Upper_Bounds[Bound_Index]);
            c1++;
            xTimerChangePeriod(xTimerSender1, pdMS_TO_TICKS(w), 0);
            xTimerStart(xTimerSender1, 0);
        }
    }
}

void vSenderTask2(void *pvParameters) {
	char Message[100];
	while(1) {
	    	BaseType_t xStatus,xStatusSemaphore2;
	    		  xStatusSemaphore2 = xSemaphoreTake(xSemaphoreSender2, portMAX_DELAY);
	    		  if (xStatusSemaphore2 == pdTRUE) {
	    		    xTimeSender2 = xTaskGetTickCount();
	    		    sprintf(Message, "Time is %ld", xTimeSender2);
	    		    char *PtrToMessage = strdup(Message);
	    		    xStatus = xQueueSend(xQueue, &PtrToMessage, 0);
	    		    if (xStatus != pdPASS) {
	    		          Sender2_Blocked++;Total_Blocked++;
	    		          free(PtrToMessage);
	            } else {
	                Sender2_Sent++;Total_Sent++;
	            }
            sum2 += x;
            x = getRandomValue(Lower_Bounds[Bound_Index], Upper_Bounds[Bound_Index]);
                        c2++;
            xTimerChangePeriod(xTimerSender2, pdMS_TO_TICKS(x), 0);
            xTimerStart(xTimerSender2, 0);
        }
    }
}

void vSenderTask3(void *pvParameters) {
	char Message[100];
	while(1){
		BaseType_t xStatus,xStatusSemaphore3;
			   xStatusSemaphore3 = xSemaphoreTake(xSemaphoreSender3, portMAX_DELAY);
			   if (xStatusSemaphore3 == pdTRUE) {
			   xTimeSender3 = xTaskGetTickCount();
			   sprintf(Message, "Time is %ld", xTimeSender3);
			   char *PtrToMessage = strdup(Message);
			   xStatus = xQueueSend(xQueue, &PtrToMessage, 0);
			   if (xStatus != pdPASS) {
			        Sender3_Blocked++;Total_Blocked++;
			        free(PtrToMessage);
	            } else {
	                Sender3_Sent++;Total_Sent++;
	            }
            sum3 += y;
            y = getRandomValue(Lower_Bounds[Bound_Index], Upper_Bounds[Bound_Index]);
                        c3++;
            xTimerChangePeriod(xTimerSender3, pdMS_TO_TICKS(y), 0);
            xTimerStart(xTimerSender3, 0);
        }
    }
}

void vReceiverTask(void *pvParameters) {
	while(1) {
	    	BaseType_t xStatus,xStatusSemaphoreReceiver;
	    	xStatusSemaphoreReceiver = xSemaphoreTake(xSemaphoreReceiver, portMAX_DELAY);
	        if (xStatusSemaphoreReceiver == pdTRUE) {
	            char *Message;
	            xStatus = xQueueReceive(xQueue, &Message, 0);
	            if (xStatus == pdPASS) {
	                Total_Received++;
	                trace_printf("Received: %s\n", Message);
	                trace_printf("total %d\n", Total_Received);
	                free(Message);
                if (Total_Received == ReceiverThreshold) {
                	 Reset();
            }
        }
    }
}
}

void vSenderTimerCallback1(TimerHandle_t xTimer) {
    xSemaphoreGive(xSemaphoreSender1);
}

void vSenderTimerCallback2(TimerHandle_t xTimer) {
    xSemaphoreGive(xSemaphoreSender2);
}

void vSenderTimerCallback3(TimerHandle_t xTimer) {
    xSemaphoreGive(xSemaphoreSender3);
}

void vReceiverTimerCallback(TimerHandle_t xTimer) {
    xSemaphoreGive(xSemaphoreReceiver);
}

void Reset(){
	trace_printf("Total Sent Messages: %d\n", Total_Sent);
	  trace_printf("Total Blocked Messages: %d\n", Total_Blocked);
	  trace_printf("Sender1 Sent Messages: %d, Blocked: %d\n", Sender1_Sent, Sender1_Blocked);
	  trace_printf("Sender2 Sent Messages: %d, Blocked: %d\n", Sender2_Sent, Sender2_Blocked);
	  trace_printf("Sender3 Sent Messages: %d, Blocked: %d\n", Sender3_Sent, Sender3_Blocked);
	  trace_printf("Sender1 Average Random Values: %d\n", sum1/c1);
	  trace_printf("Sender2 Average Random Values: %d\n", sum2/c2);
	  trace_printf("Sender3 Average Random Values: %d\n", sum3/c3);

	  Total_Sent = 0,Total_Blocked = 0,Total_Received = 0;
	  Sender1_Sent = 0,Sender1_Blocked = 0;
	  Sender2_Sent = 0,Sender2_Blocked = 0;
	  Sender3_Sent = 0,Sender3_Blocked = 0;
	  sum1 = 0, sum2 = 0, sum3 = 0;
	  c1 = 0, c2 = 0, c3 = 0;
	  xQueueReset(xQueue);
	  Bound_Index++;
	  if (Bound_Index == 6) {
	    GameOver();
	                }
}

void GameOver(){
	trace_printf("Game Over\n");
	  xTimerStop(xTimerSender1, 0);
	  xTimerStop(xTimerSender2, 0);
	  xTimerStop(xTimerSender3, 0);
	  exit(0);
}

void vApplicationMallocFailedHook( void )
{
	/* Called if a call to pvPortMalloc() fails because there is insufficient
	free memory available in the FreeRTOS heap.  pvPortMalloc() is called
	internally by FreeRTOS API functions that create tasks, queues, software
	timers, and semaphores.  The size of the FreeRTOS heap is set by the
	configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;

	/* Run time stack overflow checking is performed if
	configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
volatile size_t xFreeStackSpace;

	/* This function is called on each cycle of the idle task.  In this case it
	does nothing useful, other than report the amout of FreeRTOS heap that
	remains unallocated. */
	xFreeStackSpace = xPortGetFreeHeapSize();

	if( xFreeStackSpace > 100 )
	{
		/* By now, the kernel has allocated everything it is going to, so
		if there is a lot of heap remaining unallocated then
		the value of configTOTAL_HEAP_SIZE in FreeRTOSConfig.h can be
		reduced accordingly. */
	}
}

void vApplicationTickHook(void) {
}

StaticTask_t xIdleTaskTCB CCM_RAM;
StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE] CCM_RAM;

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize) {
  /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
  state will be stored. */
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

  /* Pass out the array that will be used as the Idle task's stack. */
  *ppxIdleTaskStackBuffer = uxIdleTaskStack;

  /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
  Note that, as the array is necessarily of type StackType_t,
  configMINIMAL_STACK_SIZE is specified in words, not bytes. */
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

static StaticTask_t xTimerTaskTCB CCM_RAM;
static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH] CCM_RAM;

/* configUSE_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
application must provide an implementation of vApplicationGetTimerTaskMemory()
to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize) {
  *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
  *ppxTimerTaskStackBuffer = uxTimerTaskStack;
  *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

