# Quick Reference Card - 42 Philosophers Evaluation

## 1. One Thread Per Philosopher ✅
**Show:** `simulation.c:42-61` - `create_philos()` function
- Loop creates `pthread_create()` for each philosopher
- Each `t_philo` has its own `pthread_t thread` field

## 2. One Fork Per Philosopher ✅
**Show:** `init.c:19` - `malloc(sizeof(pthread_mutex_t) * data->num_philos)`
- Allocates exactly `num_philos` mutexes (one per fork)

## 3. Mutex Per Fork ✅
**Show:** `philosopher.c:15-31` (take forks) and `philosopher.c:45-46` (release forks)
- `pthread_mutex_lock()` = taking fork
- `pthread_mutex_unlock()` = releasing fork
- Mutex IS the fork state

## 4. No Mixed Outputs ✅
**Show:** `utils.c:23-37` - `print_status()` function
- All prints protected by single `print_mutex`
- Lock → printf → unlock (atomic)

## 5. Death Verification & Protection ✅
**Show:** `monitor.c:46-60` (check) and `monitor.c:27-44` (handle)
- Monitor locks `meal_mutex` to read `last_meal_time`
- Monitor locks `death_mutex` to set `simulation_end`
- Philosophers check `simulation_end` (protected by `death_mutex`) before eating
- Prevents eating while being declared dead

---

## Key Mutexes:
- `forks[i]` - Each fork mutex
- `print_mutex` - All printing
- `death_mutex` - `simulation_end` flag
- `philos[i].meal_mutex` - Each philosopher's meal data

## Key Functions:
- `philosopher_life()` - Thread routine for each philosopher
- `monitor_death()` - Separate thread checking for deaths
- `print_status()` - Thread-safe printing
- `check_simulation_end()` - Thread-safe death check
