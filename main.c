/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mabdur-r <mabdur-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/05 15:56:41 by mabdur-r          #+#    #+#             */
/*   Updated: 2025/12/08 12:10:47 by mabdur-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

static int	validate_args(t_data *data)
{
	if (data->num_philos < 1 || data->num_philos > 200)
	{
		printf("Error: number_of_philosophers must be between 1 and 200\n");
		return (0);
	}
	if (data->time_to_die < 0
		|| data->time_to_eat < 0
		|| data->time_to_sleep < 0)
	{
		printf("Error: time values must be positive\n");
		return (0);
	}
	return (1);
}

int	main(int argc, char **argv)
{
	t_data	data;
	t_philo	*philos;

	memset(&data, 0, sizeof(t_data));
	if (argc < 5 || argc > 6)
	{
		printf("Usage: %s number_of_philosophers time_to_die ", argv[0]);
		printf("time_to_eat time_to_sleep ");
		printf("[number_of_times_each_philosopher_must_eat]\n");
		return (1);
	}
	if (!parse_args(argc, argv, &data))
		return (1);
	if (!validate_args(&data))
		return (1);
	if (!init_data(&data, &philos))
		return (1);
	if (!start_simulation(&data, philos))
	{
		cleanup(&data, philos);
		return (1);
	}
	cleanup(&data, philos);
	return (0);
}
