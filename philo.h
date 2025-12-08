/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mabdur-r <mabdur-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/05 15:57:26 by mabdur-r          #+#    #+#             */
/*   Updated: 2025/12/08 11:23:33 by mabdur-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PHILO_H
# define PHILO_H

# include <pthread.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/time.h>
# include <string.h>

// Philosopher states
typedef enum e_state
{
	THINKING,
	EATING,
	SLEEPING,
	DEAD
}	t_state;

// Main data structure
typedef struct s_data
{
	int				num_philos;
	int				time_to_die;
	int				time_to_eat;
	int				time_to_sleep;
	int				num_times_to_eat;
	long			start_time;
	int				simulation_end;
	pthread_mutex_t	*forks;
	pthread_mutex_t	print_mutex;
	pthread_mutex_t	death_mutex;
}	t_data;

// Philosopher structure
typedef struct s_philo
{
	int				id;
	int				left_fork;
	int				right_fork;
	int				meals_eaten;
	long			last_meal_time;
	t_state			state;
	t_data			*data;
	pthread_t		thread;
}	t_philo;

// Function prototypes
int		parse_args(int argc, char **argv, t_data *data);
long	get_time_ms(void);
void	print_status(t_philo *philo, char *status);
void	*philosopher_life(void *arg);
void	*monitor_death(void *arg);
int		init_data(t_data *data, t_philo **philos);
void	cleanup(t_data *data, t_philo *philos);
int		start_simulation(t_data *data, t_philo *philos);

#endif
