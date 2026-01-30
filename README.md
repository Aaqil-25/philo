*This project has been created as part of the 42 curriculum by mabdur-r.*

# Philosophers (Philo)

## Description

**Philosophers** is a classic concurrent programming problem implementation that simulates the famous "Dining Philosophers Problem" using POSIX threads (pthreads). The goal of this project is to understand and implement thread synchronization mechanisms, specifically mutexes, to prevent race conditions and deadlocks in a multi-threaded environment.

### Overview

The program simulates a scenario where a number of philosophers sit around a circular table with forks between them. Each philosopher alternates between three states:
- **Thinking**: The philosopher is contemplating
- **Eating**: The philosopher has acquired two forks and is eating
- **Sleeping**: The philosopher is resting after eating

The challenge lies in preventing deadlocks (where all philosophers are waiting for forks) and ensuring that philosophers don't starve while maintaining proper synchronization between threads.

### Key Features

- Multi-threaded simulation using pthreads
- Mutex-based synchronization for fork access
- Death monitoring system to detect philosopher starvation
- Optional meal limit to stop simulation when all philosophers have eaten enough
- Thread-safe printing with mutex protection
- Proper cleanup of resources (mutexes, threads, memory)

## Instructions

### Compilation

To compile the project, use the provided Makefile:

```bash
make
```

This will create an executable named `philo` in the current directory.

### Cleanup

To remove object files:
```bash
make clean
```

To remove object files and the executable:
```bash
make fclean
```

To recompile from scratch:
```bash
make re
```

### Execution

The program requires 4 mandatory arguments and 1 optional argument:

```bash
./philo number_of_philosophers time_to_die time_to_eat time_to_sleep [number_of_times_each_philosopher_must_eat]
```

**Arguments:**
- `number_of_philosophers`: The number of philosophers (and forks). Must be between 1 and 200.
- `time_to_die`: Time in milliseconds. If a philosopher doesn't start eating within this time after their last meal, they die.
- `time_to_eat`: Time in milliseconds that a philosopher takes to eat (during which they hold two forks).
- `time_to_sleep`: Time in milliseconds that a philosopher spends sleeping.
- `number_of_times_each_philosopher_must_eat` (optional): If specified, the simulation stops when all philosophers have eaten at least this many times.

**Example Usage:**

```bash
# Basic simulation with 5 philosophers
./philo 5 800 200 200

# Simulation with meal limit (stops when each philosopher has eaten 7 times)
./philo 5 800 200 200 7

# Edge case: single philosopher (will die after time_to_die)
./philo 1 800 200 200
```

### Output Format

The program prints status messages in the following format:
```
<timestamp_in_ms> <philosopher_id> <status>
```

Example output:
```
0 1 has taken a fork
0 1 has taken a fork
0 1 is eating
200 1 is sleeping
400 1 is thinking
```

## Technical Implementation

### Architecture

The project is organized into several modules:

- **main.c**: Entry point, argument validation, and program orchestration
- **parse.c**: Command-line argument parsing and validation
- **init.c**: Initialization of mutexes, philosophers, and data structures
- **philosopher.c**: Philosopher thread routine implementing the eating/sleeping/thinking cycle
- **monitor.c**: Death monitoring thread that checks for philosopher starvation
- **simulation.c**: Simulation management, thread creation, and coordination
- **utils.c**: Utility functions for time management and thread-safe printing

### Synchronization Mechanisms

- **Fork Mutexes**: Each fork is protected by a mutex to ensure exclusive access
- **Print Mutex**: Ensures thread-safe output (no interleaved messages)
- **Death Mutex**: Protects the `simulation_end` flag to coordinate thread termination

### Special Cases

- **Single Philosopher**: Handled as a special case where the philosopher takes one fork and dies after `time_to_die` milliseconds, as they cannot acquire a second fork.

## Resources

### Classic References

- **Dining Philosophers Problem**: [Wikipedia Article](https://en.wikipedia.org/wiki/Dining_philosophers_problem)
- **POSIX Threads Programming**: [IBM DeveloperWorks Tutorial](https://www.ibm.com/docs/en/aix/7.2?topic=programming-posix-threads-programming)
- **pthread Manual Pages**: `man pthread_create`, `man pthread_join`, `man pthread_mutex_init`
- **Mutex Synchronization**: [GeeksforGeeks - Mutex in C](https://www.geeksforgeeks.org/mutex-lock-for-linux-thread-synchronization/)
- **Thread Safety**: [Linux Programmer's Manual - pthreads](https://man7.org/linux/man-pages/man7/pthreads.7.html)

### AI Usage Disclosure

AI assistance was used in the following parts of this project:

- **Code Review and Global Variable Detection**: AI was used to scan all source files to identify any global variables, as they are not allowed in this project. This helped ensure compliance with project requirements.

- **Edge Case Handling**: AI assistance was used to identify and implement the special case handling for the single philosopher scenario (`num_philos == 1`), ensuring the program correctly handles this edge case where a philosopher cannot acquire two forks.

