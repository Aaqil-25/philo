/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mabdur-r <mabdur-r@42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/05 15:56:21 by mabdur-r          #+#    #+#             */
/*   Updated: 2025/12/05 15:56:28 by mabdur-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

static int	init_mutexes(t_data *data)
{
	int	i;

	data->forks = malloc(sizeof(pthread_mutex_t) * data->num_philos);
	if (!data->forks)
		return (0);
	i = 0;
	while (i < data->num_philos)
	{
		if (pthread_mutex_init(&data->forks[i], NULL) != 0)
		{
			while (--i >= 0)
				pthread_mutex_destroy(&data->forks[i]);
			free(data->forks);
			return (0);
		}
		i++;
	}
	if (pthread_mutex_init(&data->print_mutex, NULL) != 0)
		return (0);
	if (pthread_mutex_init(&data->death_mutex, NULL) != 0)
		return (0);
	return (1);
}

static void	init_philosophers(t_philo *philos, t_data *data)
{
	int	i;

	i = 0;
	while (i < data->num_philos)
	{
		philos[i].id = i + 1;
		philos[i].left_fork = i;
		philos[i].right_fork = (i + 1) % data->num_philos;
		philos[i].meals_eaten = 0;
		philos[i].last_meal_time = data->start_time;
		philos[i].state = THINKING;
		philos[i].data = data;
		i++;
	}
}

int	init_data(t_data *data, t_philo **philos)
{
	data->start_time = get_time_ms();
	data->simulation_end = 0;
	if (!init_mutexes(data))
	{
		printf("Error: Failed to initialize mutexes\n");
		return (0);
	}
	*philos = malloc(sizeof(t_philo) * data->num_philos);
	if (!*philos)
	{
		cleanup(data, NULL);
		return (0);
	}
	init_philosophers(*philos, data);
	return (1);
}

void	cleanup(t_data *data, t_philo *philos)
{
	int	i;

	if (data->forks)
	{
		i = 0;
		while (i < data->num_philos)
		{
			pthread_mutex_destroy(&data->forks[i]);
			i++;
		}
		free(data->forks);
	}
	pthread_mutex_destroy(&data->print_mutex);
	pthread_mutex_destroy(&data->death_mutex);
	if (philos)
		free(philos);
}

