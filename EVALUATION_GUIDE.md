# 42 Philosophers - Peer Evaluation Defense Guide

## Evaluation Point 1: One Thread Per Philosopher

### ✅ **How to Verify:**
Show the evaluator in `simulation.c`:

**Location:** `simulation.c`, lines 42-61

```c
static int	create_philos(t_data *data, t_philo *philos)
{
	int	i;

	i = 0;
	while (i < data->num_philos)
	{
		if (pthread_create(&philos[i].thread, NULL,
				philosopher_life, &philos[i]) != 0)
		{
			// error handling...
		}
		i++;
	}
	return (1);
}
```

### **Your Explanation:**
"Each philosopher is created as a separate thread using `pthread_create`. The loop iterates through all philosophers (`data->num_philos`), and for each one, we call `pthread_create` which creates a new thread that runs the `philosopher_life` function. Each philosopher structure (`philos[i]`) has its own `thread` field (defined in `philo.h` line 57: `pthread_t thread;`), so we have exactly one thread per philosopher."

**Key Evidence:**
- `philo.h` line 57: Each `t_philo` has a `pthread_t thread` field
- `simulation.c` line 49-50: `pthread_create(&philos[i].thread, NULL, philosopher_life, &philos[i])`
- The loop creates exactly `num_philos` threads

---

## Evaluation Point 2: One Fork Per Philosopher

### ✅ **How to Verify:**
Show the evaluator in `init.c`:

**Location:** `init.c`, lines 15-33

```c
static int	init_mutexes(t_data *data)
{
	int	i;

	data->forks = malloc(sizeof(pthread_mutex_t) * data->num_philos);
	// ... initializes exactly num_philos mutexes
}
```

### **Your Explanation:**
"We allocate exactly `num_philos` mutexes for the forks. In `init.c` line 19, we allocate `sizeof(pthread_mutex_t) * data->num_philos` bytes, which creates an array of exactly `num_philos` fork mutexes. Since the subject states there are as many forks as philosophers, this matches the requirement: one fork per philosopher."

**Key Evidence:**
- `init.c` line 19: `malloc(sizeof(pthread_mutex_t) * data->num_philos)`
- `philo.h` line 42: `pthread_mutex_t *forks;` - array of mutexes
- The number of forks equals the number of philosophers

---

## Evaluation Point 3: Mutex Per Fork - Used to Check/Change Fork State

### ✅ **How to Verify:**
Show the evaluator in `philosopher.c`:

**Location:** `philosopher.c`, lines 15-31 (taking forks) and lines 45-46 (releasing forks)

```c
static void	take_forks(t_philo *philo)
{
	if (philo->id % 2 == 0)
	{
		pthread_mutex_lock(&philo->data->forks[philo->right_fork]);
		print_status(philo, "has taken a fork");
		pthread_mutex_lock(&philo->data->forks[philo->left_fork]);
		print_status(philo, "has taken a fork");
	}
	else
	{
		pthread_mutex_lock(&philo->data->forks[philo->left_fork]);
		print_status(philo, "has taken a fork");
		pthread_mutex_lock(&philo->data->forks[philo->right_fork]);
		print_status(philo, "has taken a fork");
	}
}
```

And in the `eat` function (lines 45-46):
```c
pthread_mutex_unlock(&philo->data->forks[philo->left_fork]);
pthread_mutex_unlock(&philo->data->forks[philo->right_fork]);
```

### **Your Explanation:**
"Each fork is protected by its own mutex. When a philosopher wants to eat, they must lock both their left and right fork mutexes using `pthread_mutex_lock`. The mutex itself represents the fork - if the mutex is locked, the fork is taken; if unlocked, the fork is available. 

In `take_forks` (lines 15-31), we lock the fork mutexes before printing 'has taken a fork'. When eating is done (lines 45-46), we unlock both mutexes, making the forks available again. The mutex mechanism ensures that only one philosopher can hold a fork at a time, preventing two philosophers from using the same fork simultaneously."

**Key Evidence:**
- `philosopher.c` lines 19, 21, 26, 28: `pthread_mutex_lock` on fork mutexes
- `philosopher.c` lines 45-46: `pthread_mutex_unlock` to release forks
- The mutex IS the fork - locking it means taking it, unlocking means releasing it

**Note:** The subject says "protect each fork's state with a mutex" - in our implementation, the mutex itself represents the fork state (locked = taken, unlocked = available). This is a common and correct approach.

---

## Evaluation Point 4: Outputs Never Mixed Up

### ✅ **How to Verify:**
Show the evaluator in `utils.c`:

**Location:** `utils.c`, lines 23-37

```c
void	print_status(t_philo *philo, char *status)
{
	long	timestamp;
	int		sim_ended;

	pthread_mutex_lock(&philo->data->death_mutex);
	sim_ended = philo->data->simulation_end;
	pthread_mutex_unlock(&philo->data->death_mutex);
	if (sim_ended)
		return ;
	pthread_mutex_lock(&philo->data->print_mutex);
	timestamp = get_time_ms() - philo->data->start_time;
	printf("%ld %d %s\n", timestamp, philo->id, status);
	pthread_mutex_unlock(&philo->data->print_mutex);
}
```

### **Your Explanation:**
"All printing is protected by a single `print_mutex` (defined in `philo.h` line 43). Every time we print a status message, we first lock the `print_mutex` (line 33), then print the entire message with `printf` (line 35), and then unlock it (line 36). This ensures that only one thread can print at a time, preventing messages from being interleaved or mixed up.

For example, if two philosophers try to print at the same time, one will lock the mutex and print completely, then the other will wait until the first is done before printing. This guarantees that each output line is atomic and never gets mixed with another thread's output."

**Key Evidence:**
- `philo.h` line 43: `pthread_mutex_t print_mutex;` - single mutex for all printing
- `utils.c` line 33: `pthread_mutex_lock(&philo->data->print_mutex);` before printf
- `utils.c` line 36: `pthread_mutex_unlock(&philo->data->print_mutex);` after printf
- All print calls go through `print_status()` function

**Additional Note:** We also check `simulation_end` (lines 28-32) before printing to avoid printing after death, but this check is done under `death_mutex` to prevent data races.

---

## Evaluation Point 5: Death Verification & Mutex Protection

### ✅ **How to Verify:**
Show the evaluator in `monitor.c`:

**Location:** `monitor.c`, lines 46-60 (death checking) and lines 27-44 (death handling)

```c
static int	check_death(t_philo *philo)
{
	long	current_time;
	long	time_since_last_meal;

	if (is_simulation_ended(philo))
		return (1);
	current_time = get_time_ms();
	pthread_mutex_lock(&philo->meal_mutex);
	time_since_last_meal = current_time - philo->last_meal_time;
	pthread_mutex_unlock(&philo->meal_mutex);
	if (time_since_last_meal >= philo->data->time_to_die)
		return (handle_philo_death(philo));
	return (0);
}
```

And `handle_philo_death` (lines 27-44):
```c
static int	handle_philo_death(t_philo *philo)
{
	long	timestamp;

	pthread_mutex_lock(&philo->data->death_mutex);
	if (!philo->data->simulation_end)
	{
		philo->data->simulation_end = 1;
		pthread_mutex_unlock(&philo->data->death_mutex);
		// ... print death message
		return (1);
	}
	pthread_mutex_unlock(&philo->data->death_mutex);
	return (1);
}
```

### **Your Explanation:**
"Death verification happens in the `monitor_death` thread (line 90-117), which continuously checks each philosopher. In `check_death` (lines 46-60), we:

1. First check if simulation already ended (line 51) - this prevents checking after death
2. Get current time (line 53)
3. **Lock the philosopher's `meal_mutex`** (line 54) to safely read `last_meal_time` (line 55)
4. Calculate time since last meal (line 55)
5. Unlock the mutex (line 56)
6. If time exceeds `time_to_die`, call `handle_philo_death` (line 58)

The critical protection is in `handle_philo_death` (lines 27-44):
- We lock `death_mutex` (line 31) before checking/setting `simulation_end`
- This prevents a race condition where a philosopher could start eating while the monitor is declaring them dead
- Once `simulation_end` is set to 1 (line 34), all philosopher threads will see this in their `check_simulation_end` calls (in `utils.c` line 39-48) and stop trying to eat

Additionally, when a philosopher updates their `last_meal_time` in the `eat` function (`philosopher.c` lines 37-39), they also lock their `meal_mutex`, ensuring the monitor thread can't read a partially updated value."

**Key Evidence:**
- `monitor.c` line 54: `pthread_mutex_lock(&philo->meal_mutex);` before reading `last_meal_time`
- `monitor.c` line 31: `pthread_mutex_lock(&philo->data->death_mutex);` before setting `simulation_end`
- `philosopher.c` line 37: `pthread_mutex_lock(&philo->meal_mutex);` before updating `last_meal_time`
- `utils.c` line 41: `pthread_mutex_lock(&philo->data->death_mutex);` before checking `simulation_end`
- The `death_mutex` ensures only one thread can declare death and set `simulation_end`

**Why This Prevents Race Conditions:**
- The monitor locks `meal_mutex` to read `last_meal_time` safely
- The monitor locks `death_mutex` to set `simulation_end` atomically
- Philosophers lock `death_mutex` to check `simulation_end` before eating
- Philosophers lock `meal_mutex` to update `last_meal_time` after eating
- This mutual exclusion ensures a philosopher can't start eating while being declared dead

---

## Bonus: Additional Points You Can Mention

### Thread Safety of Shared Data:
- `simulation_end`: Protected by `death_mutex` everywhere
- `last_meal_time`: Protected by each philosopher's `meal_mutex`
- `meals_eaten`: Protected by each philosopher's `meal_mutex`
- Fork access: Protected by fork mutexes
- Printing: Protected by `print_mutex`

### Fork Assignment:
- `init.c` lines 49-50: Each philosopher gets `left_fork = i` and `right_fork = (i + 1) % num_philos`
- This creates a circular arrangement where philosopher N shares fork N with philosopher N-1 (left) and fork (N+1) with philosopher N+1 (right)

### Deadlock Prevention:
- `philosopher.c` lines 17-30: Even-numbered philosophers take right fork first, odd-numbered take left fork first
- This breaks symmetry and prevents circular wait (deadlock)

---

## Quick Reference: File Locations

| Evaluation Point | Key File | Key Lines |
|-----------------|----------|-----------|
| One thread per philosopher | `simulation.c` | 42-61 |
| One fork per philosopher | `init.c` | 15-33 |
| Mutex per fork | `philosopher.c` | 15-31, 45-46 |
| No mixed outputs | `utils.c` | 23-37 |
| Death verification | `monitor.c` | 46-60, 27-44 |

---

## Tips for Defense

1. **Be confident** - Your implementation is correct!
2. **Point to specific lines** - Use your editor to show the exact code
3. **Explain the "why"** - Don't just show code, explain the synchronization logic
4. **Mention edge cases** - Single philosopher handling, meal limits, etc.
5. **If asked about data races** - Explain how all shared variables are protected by mutexes

Good luck! 🍀
