/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   simulation.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mabdur-r <mabdur-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/05 15:57:44 by mabdur-r          #+#    #+#             */
/*   Updated: 2025/12/08 11:23:07 by mabdur-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

static void	join_philos(t_philo *philos, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		pthread_join(philos[i].thread, NULL);
		i++;
	}
}

static int	create_philos(t_data *data, t_philo *philos)
{
	int	i;

	i = 0;
	while (i < data->num_philos)
	{
		if (pthread_create(&philos[i].thread, NULL,
				philosopher_life, &philos[i]) != 0)
		{
			data->simulation_end = 1;
			while (--i >= 0)
				pthread_join(philos[i].thread, NULL);
			return (0);
		}
		i++;
	}
	return (1);
}

int	start_simulation(t_data *data, t_philo *philos)
{
	pthread_t	monitor_thread;

	if (!create_philos(data, philos))
		return (0);
	if (pthread_create(&monitor_thread, NULL, monitor_death, philos) != 0)
	{
		data->simulation_end = 1;
		join_philos(philos, data->num_philos);
		return (0);
	}
	pthread_join(monitor_thread, NULL);
	join_philos(philos, data->num_philos);
	return (1);
}
