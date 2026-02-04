/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mabdur-r <mabdur-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/05 15:57:52 by mabdur-r          #+#    #+#             */
/*   Updated: 2025/12/08 11:23:23 by mabdur-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

long	get_time_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

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

int	check_simulation_end(t_philo *philo)
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

int	check_meal_limit(t_philo *philo)
{
	int	should_stop;

	pthread_mutex_lock(&philo->meal_mutex);
	should_stop = (philo->data->num_times_to_eat != -1
			&& philo->meals_eaten >= philo->data->num_times_to_eat);
	pthread_mutex_unlock(&philo->meal_mutex);
	return (should_stop);
}
