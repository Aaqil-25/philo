# Philosophers - 42 Project

## Overview
This project implements the classic Dining Philosophers problem using threads and mutexes in C.

## Project Structure
```
philo/
├── philo.h          # Header file with data structures and function prototypes
├── main.c           # Main entry point
├── parse.c          # Argument parsing
├── utils.c          # Utility functions (time, printing)
├── init.c           # Initialization and cleanup
├── philosopher.c    # Philosopher thread logic (eat, sleep, think)
├── monitor.c        # Death detection and monitoring
├── simulation.c     # Simulation control (thread creation/joining)
└── Makefile         # Build configuration
```

## Compilation
```bash
make
```

## Usage
```bash
./philo number_of_philosophers time_to_die time_to_eat time_to_sleep [number_of_times_each_philosopher_must_eat]
```

### Arguments
- `number_of_philosophers`: Number of philosophers (and forks) - must be between 1 and 200
- `time_to_die`: Time in milliseconds before a philosopher dies if not eating
- `time_to_eat`: Time in milliseconds a philosopher spends eating
- `time_to_sleep`: Time in milliseconds a philosopher spends sleeping
- `number_of_times_each_philosopher_must_eat`: (Optional) If all philosophers eat this many times, simulation stops

### Example
```bash
./philo 5 800 200 200
# 5 philosophers, die after 800ms without eating, eat for 200ms, sleep for 200ms

./philo 5 800 200 200 7
# Same as above, but simulation stops when all philosophers have eaten 7 times
```

## Key Features
1. **Deadlock Prevention**: Uses even/odd fork ordering strategy to prevent deadlocks
2. **Death Detection**: Monitor thread checks if any philosopher dies
3. **Thread Safety**: All shared resources protected with mutexes
4. **Proper Logging**: Status messages with timestamps (no scrambling)
5. **Optional Meal Limit**: Can stop simulation when all philosophers eat enough times

## Implementation Details
- Each philosopher is a separate thread
- Each fork is protected by a mutex
- Even-numbered philosophers take right fork first, odd-numbered take left fork first (prevents deadlock)
- Monitor thread continuously checks for deaths and meal completion
- All status messages are protected by a print mutex to prevent scrambling

## Testing
Test with various scenarios:
```bash
# Basic test
./philo 4 410 200 200

# Test with meal limit
./philo 5 800 200 200 5

# Test with single philosopher (should die immediately)
./philo 1 800 200 200
```

## Notes
- The program follows 42 Norm coding standards
- No memory leaks (use valgrind to verify)
- All mutexes are properly initialized and destroyed
- Threads are properly joined before program exit

