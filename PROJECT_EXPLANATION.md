# Philosophers Project - Complete Implementation Explanation

## Table of Contents
1. [Project Overview](#project-overview)
2. [Data Structures](#data-structures)
3. [Program Flow](#program-flow)
4. [File-by-File Breakdown](#file-by-file-breakdown)
5. [Thread Synchronization](#thread-synchronization)
6. [Key Algorithms](#key-algorithms)
7. [Complete Execution Flow](#complete-execution-flow)

---

## Project Overview

### The Problem
The **Dining Philosophers Problem** is a classic synchronization problem where:
- N philosophers sit around a circular table
- There are N forks (one between each pair of philosophers)
- Each philosopher needs **2 forks** to eat (left and right)
- Philosophers alternate between: **Thinking → Eating → Sleeping**
- If a philosopher doesn't eat within `time_to_die` milliseconds, they die
- The goal: prevent deadlocks and starvation while ensuring philosophers can eat

### Our Solution
- **One thread per philosopher** (each runs `philosopher_life`)
- **One mutex per fork** (mutex = fork ownership)
- **One monitor thread** to check for deaths
- **Mutex-protected shared data** to prevent race conditions

---

## Data Structures

### `t_data` (Main Shared Data)
```c
typedef struct s_data {
    int num_philos;              // Number of philosophers
    int time_to_die;             // Max time (ms) without eating before death
    int time_to_eat;             // Time (ms) to eat
    int time_to_sleep;           // Time (ms) to sleep
    int num_times_to_eat;        // Optional: stop after N meals (-1 = infinite)
    long start_time;             // Simulation start timestamp
    int simulation_end;          // Flag: 1 = stop simulation
    pthread_mutex_t *forks;      // Array of fork mutexes
    pthread_mutex_t print_mutex; // Protects printf output
    pthread_mutex_t death_mutex; // Protects simulation_end flag
} t_data;
```

**Purpose**: Shared data accessible by all threads. Protected by mutexes.

### `t_philo` (Individual Philosopher)
```c
typedef struct s_philo {
    int id;                      // Philosopher ID (1 to num_philos)
    int left_fork;               // Index of left fork
    int right_fork;              // Index of right fork
    int meals_eaten;             // Counter for meals
    long last_meal_time;         // Timestamp of last meal start
    t_state state;                // Current state (THINKING/EATING/SLEEPING)
    t_data *data;                // Pointer to shared data
    pthread_t thread;             // Thread handle
    pthread_mutex_t meal_mutex;  // Protects meals_eaten & last_meal_time
} t_philo;
```

**Purpose**: Each philosopher has their own structure with thread-specific data.

### `t_state` (Philosopher States)
```c
typedef enum e_state {
    THINKING,  // Contemplating
    EATING,    // Currently eating (has both forks)
    SLEEPING,  // Resting after eating
    DEAD       // Died (not actively used)
} t_state;
```

---

## Program Flow

### High-Level Execution
```
main()
  ├─> parse_args()          // Parse command-line arguments
  ├─> validate_args()        // Check argument validity
  ├─> init_data()            // Initialize mutexes, philosophers
  ├─> start_simulation()     // Create threads, run simulation
  └─> cleanup()              // Destroy mutexes, free memory
```

---

## File-by-File Breakdown

### 1. `main.c` - Entry Point

#### `main(int argc, char **argv)`
**Purpose**: Program entry point, orchestrates the entire simulation.

**Flow**:
1. **Initialize data structure** with `memset` (zero out)
2. **Check argument count** (must be 5 or 6)
3. **Parse arguments** → `parse_args()`
4. **Validate arguments** → `validate_args()`
   - Philosophers: 1-200
   - All times must be positive
5. **Initialize simulation** → `init_data()`
   - Creates mutexes, philosophers
6. **Start simulation** → `start_simulation()`
   - Creates threads, runs simulation
7. **Cleanup** → `cleanup()`
   - Destroys mutexes, frees memory

#### `validate_args(t_data *data)`
**Purpose**: Validate parsed arguments meet requirements.

**Checks**:
- `num_philos` between 1 and 200
- All time values are positive

**Returns**: 1 if valid, 0 if invalid

---

### 2. `parse.c` - Argument Parsing

#### `ft_atoi(const char *str)`
**Purpose**: Custom atoi implementation with error handling.

**Logic**:
1. Skip whitespace
2. Handle optional sign (+/-)
3. Convert digits to integer
4. Check for overflow (INT_MAX/INT_MIN)
5. Check for non-digit characters (returns -1 on error)

**Returns**: Integer value or -1 on error

#### `parse_args(int argc, char **argv, t_data *data)`
**Purpose**: Parse command-line arguments into `t_data` structure.

**Arguments parsed**:
- `argv[1]` → `num_philos`
- `argv[2]` → `time_to_die`
- `argv[3]` → `time_to_eat`
- `argv[4]` → `time_to_sleep`
- `argv[5]` (optional) → `num_times_to_eat` (or -1 if not provided)

**Returns**: 1 on success, 0 on error

---

### 3. `init.c` - Initialization

#### `init_mutexes(t_data *data)`
**Purpose**: Initialize all mutexes needed for the simulation.

**Creates**:
1. **Fork mutexes**: Array of `num_philos` mutexes (one per fork)
2. **Print mutex**: Protects `printf` output
3. **Death mutex**: Protects `simulation_end` flag

**Error handling**: If any mutex init fails, destroys previously created mutexes and returns 0.

#### `init_philosophers(t_philo *philos, t_data *data)`
**Purpose**: Initialize philosopher structures.

**For each philosopher**:
- **ID**: 1 to `num_philos`
- **Left fork**: Index `i` (philosopher's own fork)
- **Right fork**: Index `(i + 1) % num_philos` (next philosopher's fork)
- **Meals eaten**: 0
- **Last meal time**: `start_time` (initialized to current time)
- **State**: THINKING
- **Data pointer**: Points to shared `t_data`
- **Meal mutex**: One per philosopher (protects meal data)

**Circular fork assignment**:
- Philosopher 1: forks[0] (left) and forks[1] (right)
- Philosopher 2: forks[1] (left) and forks[2] (right)
- Philosopher N: forks[N-1] (left) and forks[0] (right) ← wraps around

#### `init_data(t_data *data, t_philo **philos)`
**Purpose**: Main initialization function.

**Steps**:
1. Set `start_time` to current time
2. Set `simulation_end` to 0
3. Initialize mutexes → `init_mutexes()`
4. Allocate philosopher array
5. Initialize philosophers → `init_philosophers()`

**Returns**: 1 on success, 0 on failure

#### `cleanup(t_data *data, t_philo *philos)`
**Purpose**: Clean up all resources.

**Destroys**:
- All fork mutexes
- Print mutex
- Death mutex
- All philosopher meal mutexes
- Frees allocated memory

---

### 4. `philosopher.c` - Philosopher Thread Logic

#### `take_forks(t_philo *philo)`
**Purpose**: Acquire both forks needed to eat.

**Deadlock Prevention Strategy**:
- **Even ID philosophers** (2, 4, 6...): Take **right fork first**, then left
- **Odd ID philosophers** (1, 3, 5...): Take **left fork first**, then right

**Why this works**: Prevents circular wait (deadlock condition). If all philosophers try to grab the same fork first, some will succeed while others wait, breaking the cycle.

**Process**:
1. Lock first fork (based on ID parity)
2. Print "has taken a fork"
3. Lock second fork
4. Print "has taken a fork"

**Note**: `pthread_mutex_lock()` blocks until the mutex is available.

#### `eat(t_philo *philo)`
**Purpose**: Complete eating cycle.

**Steps**:
1. **Acquire forks** → `take_forks()`
2. **Set state** to EATING
3. **Update `last_meal_time`** (protected by `meal_mutex`)
   - This is when eating **starts** (after getting forks)
4. **Print** "is eating"
5. **Sleep** for `time_to_eat` milliseconds
6. **Increment `meals_eaten`** (protected by `meal_mutex`)
7. **Release both forks** (unlock mutexes)

**Critical**: `last_meal_time` is updated **after** acquiring forks, so the timer starts when eating actually begins.

#### `sleep_philo(t_philo *philo)`
**Purpose**: Philosopher sleeps after eating.

**Steps**:
1. Set state to SLEEPING
2. Print "is sleeping"
3. Sleep for `time_to_sleep` milliseconds

#### `think(t_philo *philo)`
**Purpose**: Philosopher thinks before trying to eat again.

**Steps**:
1. Set state to THINKING
2. Print "is thinking"
3. **Special case**: If `num_philos` is **odd**, sleep for `time_to_eat` milliseconds
   - **Why**: Prevents starvation in odd-numbered groups
   - Creates desynchronization so philosophers don't all compete at once

#### `philosopher_life(void *arg)`
**Purpose**: Main thread function for each philosopher.

**Argument**: Pointer to `t_philo` structure

**Initial delay**: Even-numbered philosophers wait 1ms before starting
- **Why**: Further desynchronization to prevent all philosophers from starting simultaneously

**Main loop**:
```c
while (1) {
    if (check_simulation_end(philo))  // Check if simulation ended
        break;
    eat(philo);                        // Eat
    if (check_meal_limit(philo))      // Check if eaten enough
        break;
    sleep_philo(philo);                // Sleep
    think(philo);                       // Think
}
```

**Exit conditions**:
1. Simulation ended (someone died or all ate enough)
2. Meal limit reached (if specified)

---

### 5. `monitor.c` - Death Monitoring

#### `is_simulation_ended(t_philo *philo)`
**Purpose**: Check if simulation should stop.

**Process**:
1. Lock `death_mutex`
2. Check `simulation_end` flag
3. Unlock mutex
4. Return 1 if ended, 0 otherwise

**Why mutex**: Prevents race conditions when multiple threads check simultaneously.

#### `handle_philo_death(t_philo *philo)`
**Purpose**: Handle philosopher death.

**Process**:
1. Lock `death_mutex`
2. Check if simulation already ended (double-check pattern)
3. If not ended:
   - Set `simulation_end = 1`
   - Unlock `death_mutex`
   - Lock `print_mutex`
   - Print death message with timestamp
   - Unlock `print_mutex`
4. Return 1

**Critical**: Death message is printed **directly** (not via `print_status`) to ensure it's always printed, even if `simulation_end` is already set.

#### `check_death(t_philo *philo)`
**Purpose**: Check if a philosopher has died.

**Process**:
1. Check if simulation already ended → return early
2. Get current time
3. Lock `meal_mutex`
4. Calculate `time_since_last_meal = current_time - last_meal_time`
5. Unlock `meal_mutex`
6. If `time_since_last_meal >= time_to_die`:
   - Call `handle_philo_death()`
   - Return 1 (death detected)
7. Return 0 (alive)

**Why `meal_mutex`**: Protects `last_meal_time` from being read while it's being updated in `eat()`.

#### `check_all_ate(t_data *data, t_philo *philos)`
**Purpose**: Check if all philosophers have eaten enough (if meal limit specified).

**Process**:
1. If `num_times_to_eat == -1` (no limit) → return 0
2. Check if simulation already ended → return 1
3. Loop through all philosophers:
   - Lock each philosopher's `meal_mutex`
   - Check if `meals_eaten < num_times_to_eat`
   - If any haven't eaten enough → return 0
   - Unlock mutex
4. If all have eaten enough → return 1

**Why individual mutexes**: Each philosopher's `meals_eaten` is protected by their own `meal_mutex` to allow concurrent checking.

#### `monitor_death(void *arg)`
**Purpose**: Monitor thread that continuously checks for deaths.

**Argument**: Pointer to first philosopher (used to access `data` and `philos` array)

**Main loop**:
```c
while (1) {
    // Check each philosopher for death
    for each philosopher:
        if (check_death(philosopher))
            return;  // Death detected, exit
    
    // Check if all have eaten enough
    if (check_all_ate())
        set simulation_end = 1;
        return;
    
    usleep(1000);  // Sleep 1ms between checks
}
```

**Frequency**: Checks every 1ms (1000 microseconds)

**Exit conditions**:
1. A philosopher dies
2. All philosophers have eaten enough (if limit specified)

---

### 6. `simulation.c` - Simulation Management

#### `handle_single_philo(t_data *data, t_philo *philos)`
**Purpose**: Special case for 1 philosopher (can't get 2 forks).

**Process**:
1. Print "has taken a fork" (only one fork available)
2. Wait for `time_to_die` milliseconds
3. Print "died"

**Why needed**: With 1 philosopher, there's only 1 fork, so they can never eat and will always die.

#### `join_philos(t_philo *philos, int count)`
**Purpose**: Wait for all philosopher threads to finish.

**Process**: Loops through all philosophers and calls `pthread_join()` on each thread.

**Why needed**: Ensures all threads complete before program exits.

#### `create_philos(t_data *data, t_philo *philos)`
**Purpose**: Create all philosopher threads.

**Process**:
1. Loop through all philosophers
2. Create thread with `pthread_create()`:
   - Thread function: `philosopher_life`
   - Argument: pointer to philosopher structure
3. If creation fails:
   - Set `simulation_end = 1`
   - Join previously created threads
   - Return 0

**Returns**: 1 on success, 0 on failure

#### `start_simulation(t_data *data, t_philo *philos)`
**Purpose**: Start the entire simulation.

**Process**:
1. **Special case**: If `num_philos == 1`, handle directly → return
2. **Create philosopher threads** → `create_philos()`
3. **Create monitor thread**:
   - Thread function: `monitor_death`
   - Argument: pointer to philosophers array
4. **Wait for monitor thread** to finish (joins it)
   - Monitor exits when death detected or all ate enough
5. **Join all philosopher threads** → `join_philos()`
   - Philosophers exit when `simulation_end` is set

**Returns**: 1 on success, 0 on failure

---

### 7. `utils.c` - Utility Functions

#### `get_time_ms(void)`
**Purpose**: Get current time in milliseconds.

**Implementation**: Uses `gettimeofday()` system call.

**Returns**: Milliseconds since epoch

#### `print_status(t_philo *philo, char *status)`
**Purpose**: Thread-safe printing.

**Process**:
1. Lock `print_mutex`
2. Check if `simulation_end` is set:
   - If yes, unlock and return (don't print after death)
3. Calculate timestamp: `current_time - start_time`
4. Print: `timestamp philosopher_id status`
5. Unlock `print_mutex`

**Why mutex**: Prevents output from multiple threads interleaving.

**Example output**: `200 3 is eating`

#### `check_simulation_end(t_philo *philo)`
**Purpose**: Check if simulation should end (used by philosopher threads).

**Process**:
1. Lock `death_mutex`
2. Check `simulation_end` flag
3. Unlock mutex
4. Return 1 if ended, 0 otherwise

**Used by**: Philosopher threads to exit their loop when simulation ends.

#### `check_meal_limit(t_philo *philo)`
**Purpose**: Check if philosopher has eaten enough meals.

**Process**:
1. Lock philosopher's `meal_mutex`
2. Check: `num_times_to_eat != -1 && meals_eaten >= num_times_to_eat`
3. Unlock mutex
4. Return result

**Used by**: Philosopher threads to exit when they've eaten enough.

---

## Thread Synchronization

### Mutexes Used

1. **Fork Mutexes** (`data->forks[]`)
   - **Purpose**: Represent fork ownership
   - **Locked**: When philosopher takes a fork
   - **Unlocked**: When philosopher releases a fork
   - **Protects**: Fork availability (mutex itself = fork)

2. **Print Mutex** (`data->print_mutex`)
   - **Purpose**: Prevent interleaved output
   - **Locked**: Before any `printf`
   - **Unlocked**: After `printf`
   - **Protects**: Console output

3. **Death Mutex** (`data->death_mutex`)
   - **Purpose**: Protect `simulation_end` flag
   - **Locked**: When reading/writing `simulation_end`
   - **Unlocked**: After operation
   - **Protects**: `simulation_end` flag

4. **Meal Mutexes** (`philo->meal_mutex`)
   - **Purpose**: Protect meal-related data
   - **Locked**: When reading/writing `last_meal_time` or `meals_eaten`
   - **Unlocked**: After operation
   - **Protects**: `last_meal_time`, `meals_eaten`

### Race Condition Prevention

**Problem**: Multiple threads accessing shared data simultaneously.

**Solution**: Mutexes ensure only one thread accesses protected data at a time.

**Example**: When monitor checks `last_meal_time` while philosopher updates it:
- Monitor locks `meal_mutex` → reads `last_meal_time` → unlocks
- Philosopher locks `meal_mutex` → updates `last_meal_time` → unlocks
- No race condition because operations are atomic (protected by mutex)

---

## Key Algorithms

### 1. Deadlock Prevention

**Problem**: If all philosophers grab left fork simultaneously, deadlock occurs.

**Solution**: Different fork order based on ID parity:
- **Even IDs**: Right fork first
- **Odd IDs**: Left fork first

**Why it works**: Breaks circular wait condition. Some philosophers will succeed while others wait, preventing all from being blocked.

### 2. Death Detection

**Algorithm**:
```
For each philosopher:
    current_time = get_time_ms()
    time_since_last_meal = current_time - last_meal_time
    if time_since_last_meal >= time_to_die:
        philosopher died
```

**Timing**: `last_meal_time` is updated when eating **starts** (after acquiring forks).

**Frequency**: Monitor checks every 1ms.

### 3. Meal Limit Detection

**Algorithm**:
```
If num_times_to_eat != -1:
    For each philosopher:
        if meals_eaten < num_times_to_eat:
            return false  // Not all have eaten enough
    return true  // All have eaten enough
```

**Timing**: Checked continuously by monitor thread.

### 4. Starvation Prevention (Odd Number of Philosophers)

**Problem**: With odd number of philosophers, some may starve.

**Solution**: When thinking, if `num_philos % 2 == 1`, sleep for `time_to_eat` milliseconds.

**Why**: Creates desynchronization so philosophers don't all compete for forks simultaneously.

---

## Complete Execution Flow

### Example: `./philo 5 800 200 200`

1. **Initialization** (`main.c` → `init_data()`)
   - Parse arguments: 5 philosophers, 800ms to die, 200ms eat, 200ms sleep
   - Create 5 fork mutexes
   - Create 5 philosopher structures
   - Set `start_time` to current time

2. **Thread Creation** (`simulation.c` → `start_simulation()`)
   - Create 5 philosopher threads (each runs `philosopher_life`)
   - Create 1 monitor thread (runs `monitor_death`)

3. **Philosopher Threads** (run concurrently)
   - Each philosopher:
     - Waits briefly (even IDs wait 1ms)
     - Enters main loop:
       - Check if simulation ended
       - **Eat**: Take forks → update meal time → eat 200ms → release forks
       - Check meal limit
       - **Sleep**: 200ms
       - **Think**: Brief thinking (if odd number, sleep 200ms)

4. **Monitor Thread** (runs continuously)
   - Every 1ms:
     - Check each philosopher's `time_since_last_meal`
     - If >= 800ms → philosopher died → set `simulation_end = 1`
     - Check if all philosophers have eaten enough (if limit specified)

5. **Termination**
   - When `simulation_end = 1`:
     - All philosopher threads exit their loops
     - Monitor thread exits
     - Main thread joins all threads
     - Cleanup destroys mutexes and frees memory

### Example: `./philo 5 800 200 200 7`

Same as above, but:
- Monitor also checks if all philosophers have eaten 7 meals
- When all have eaten 7 times → set `simulation_end = 1` → simulation stops


### Example: `./philo 1 800 200 200`

Special case:
- Only 1 philosopher, only 1 fork
- Can never get 2 forks to eat
- Prints "has taken a fork"
- Waits 800ms
- Prints "died"
- Simulation ends

---

## Key Design Decisions

### 1. Why Update `last_meal_time` After Getting Forks?

**Decision**: Update `last_meal_time` when eating **starts** (after acquiring both forks), not before.

**Reason**: The timer should start when eating actually begins, not when trying to get forks. This prevents philosophers from dying while waiting for forks.

### 2. Why Separate Mutexes for Each Philosopher's Meal Data?

**Decision**: Each philosopher has their own `meal_mutex` for `last_meal_time` and `meals_eaten`.

**Reason**: Allows monitor to check multiple philosophers concurrently without blocking each other. If one mutex protected all philosophers, checking would be sequential and slower.

### 3. Why Different Fork Order for Even/Odd IDs?

**Decision**: Even IDs take right fork first, odd IDs take left fork first.

**Reason**: Prevents deadlock by breaking circular wait condition.

### 4. Why Sleep During Thinking for Odd Number of Philosophers?

**Decision**: If `num_philos % 2 == 1`, sleep for `time_to_eat` during thinking.

**Reason**: Prevents starvation by desynchronizing philosophers so they don't all compete simultaneously.

### 5. Why Direct Print for Death Message?

**Decision**: Death message is printed directly in `handle_philo_death()`, not via `print_status()`.

**Reason**: `print_status()` checks `simulation_end` and returns early if set. Since we set `simulation_end` before printing, we need to print directly to ensure the death message is always shown.

---

## Summary

This implementation solves the Dining Philosophers Problem using:
- **One thread per philosopher** for concurrent execution
- **One mutex per fork** for resource protection
- **Mutex-protected shared data** to prevent race conditions
- **Deadlock prevention** via different fork acquisition order
- **Starvation prevention** via desynchronization for odd numbers
- **Thread-safe printing** via print mutex
- **Continuous monitoring** for death detection

The code follows 42 coding standards (no global variables, proper mutex usage, clean code structure) and handles all edge cases including single philosopher and meal limits.
