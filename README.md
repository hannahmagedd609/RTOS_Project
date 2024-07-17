# Embedded Systems Project: FreeRTOS Implementation

## Overview

This project is an implementation of an embedded system using FreeRTOS on a target emulation board provided via Eclipse CDT Embedded. The system incorporates key RTOS concepts such as tasks, timers, queues, and semaphores to manage and synchronize multiple operations efficiently.

## Objectives

- Apply knowledge learned in embedded programming.
- Gain hands-on experience with RTOS concepts, including:
  - Tasks
  - Timers
  - Queues
  - Semaphores

## Project Specifications

- Implemented using FreeRTOS on the target emulation board provided via Eclipse CDT Embedded.
- **Four Tasks Communication:** Utilize a fixed-size queue for task communication.
- **Three Sender Tasks:** Two with the same priority and one with higher priority. Sender tasks sleep for a random period, send messages to the queue, and increment counters based on success or failure.
- **Receiver Task:** Sleeps for a fixed period, checks for messages in the queue, and processes received messages.
- **Timer and Semaphore Integration:** Manage sleep/wake cycles and synchronization.

## Key Features

- **Tasks:** Efficient management of concurrent operations.
- **Timers:** Precise control over task execution timing.
- **Queues:** Effective task-to-task communication.
- **Semaphores:** Seamless task synchronization.
- **Performance Metrics:** Track and optimize system performance based on various parameters.


## Implementation Details

### Sender Tasks

- **Task Behavior:** Each sender task sleeps for a random period (`Tsender`), sends a message to the queue, and increments appropriate counters based on the success of the operation.
- **Random Period:** Derived from a uniform distribution within specified bounds.

### Receiver Task

- **Task Behavior:** Sleeps for a fixed period (`Treceiver`), checks the queue for messages, and processes one message at a time.
- **Fixed Period:** `Treceiver` is set to 100 ms.

### Timers and Callbacks

- **Sender Timer Callback:** Releases a semaphore to unblock the sender task.
- **Receiver Timer Callback:** Releases a semaphore to unblock the receiver task and calls the `Reset` function after 1000 messages.

### Reset Function

- Prints statistics for successfully sent and blocked messages.
- Prints sender task statistics.
- Resets message counters and clears the queue.
- Adjusts `Tsender` values based on predefined arrays.
- Destroys timers and stops execution after all array values are used.

## Performance Evaluation

- **Metrics:** Total sent messages, blocked messages, and system response.
- **Queue Size:** Tested with sizes of 3 and 10 to observe performance variations.
- **Graphs:** Plotted to visualize the relationship between sender timer periods and message counts.


