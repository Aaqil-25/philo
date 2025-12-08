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

int	start_simulation(t_data *data, t_philo *philos)
{
	int			i;
	pthread_t	monitor_thread;

	i = 0;
	while (i < data->num_philos)
	{
		if (pthread_create(&philos[i].thread, NULL, philosopher_life, &philos[i]) != 0)
		{
			printf("Error: Failed to create thread for philosopher %d\n", i + 1);
			data->simulation_end = 1;
			while (--i >= 0)
				pthread_join(philos[i].thread, NULL);
			return (0);
		}
		i++;
	}
	if (pthread_create(&monitor_thread, NULL, monitor_death, philos) != 0)
	{
		printf("Error: Failed to create monitor thread\n");
		data->simulation_end = 1;
		i = 0;
		while (i < data->num_philos)
		{
			pthread_join(philos[i].thread, NULL);
			i++;
		}
		return (0);
	}
	pthread_join(monitor_thread, NULL);
	i = 0;
	while (i < data->num_philos)
	{
		pthread_join(philos[i].thread, NULL);
		i++;
	}
	return (1);
}
