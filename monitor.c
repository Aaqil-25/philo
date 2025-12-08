/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mabdur-r <mabdur-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/05 15:56:55 by mabdur-r          #+#    #+#             */
/*   Updated: 2025/12/08 11:41:25 by mabdur-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

static int	is_simulation_ended(t_philo *philo)
{
	pthread_mutex_lock(&philo->data->death_mutex);
	if (philo->data->simulation_end)
	{
		pthread_mutex_unlock(&philo->data->death_mutex);
		return (1);
	}
	pthread_mutex_unlock(&philo->data->death_mutex);
	return (0);
}

static int	handle_philo_death(t_philo *philo)
{
	pthread_mutex_lock(&philo->data->death_mutex);
	if (!philo->data->simulation_end)
	{
		philo->data->simulation_end = 1;
		pthread_mutex_unlock(&philo->data->death_mutex);
		print_status(philo, "died");
		return (1);
	}
	pthread_mutex_unlock(&philo->data->death_mutex);
	return (1);
}

static int	check_death(t_philo *philo)
{
	long	current_time;
	long	time_since_last_meal;

	if (is_simulation_ended(philo))
		return (1);
	current_time = get_time_ms();
	time_since_last_meal = current_time - philo->last_meal_time;
	if (time_since_last_meal >= philo->data->time_to_die)
		return (handle_philo_death(philo));
	return (0);
}

static int	check_all_ate(t_data *data, t_philo *philos)
{
	int	i;

	if (data->num_times_to_eat == -1)
		return (0);
	pthread_mutex_lock(&data->death_mutex);
	if (data->simulation_end)
	{
		pthread_mutex_unlock(&data->death_mutex);
		return (1);
	}
	pthread_mutex_unlock(&data->death_mutex);
	i = 0;
	while (i < data->num_philos)
	{
		if (philos[i].meals_eaten < data->num_times_to_eat)
			return (0);
		i++;
	}
	return (1);
}

void	*monitor_death(void *arg)
{
	t_data	*data;
	t_philo	*philos;
	int		i;

	data = ((t_philo *)arg)->data;
	philos = (t_philo *)arg;
	while (1)
	{
		i = 0;
		while (i < data->num_philos)
		{
			if (check_death(&philos[i]))
				return (NULL);
			i++;
		}
		if (check_all_ate(data, philos))
		{
			pthread_mutex_lock(&data->death_mutex);
			data->simulation_end = 1;
			pthread_mutex_unlock(&data->death_mutex);
			return (NULL);
		}
		usleep(1000);
	}
	return (NULL);
}
