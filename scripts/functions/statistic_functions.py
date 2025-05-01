""""
This module contains the functions used to process the raw data and obtain results with it

"""
import numpy as np

def delete_d_jackknife(data, d, statistic=np.mean, *args, **kwargs):
    n = len(data)
    if d <= 0 or d >= n:
        raise ValueError("d must be between 1 and len(data) - 1")

    num_blocks = n // d
    estimates = []

    for i in range(num_blocks):
        start = i * d
        end = start + d
        jack_sample = np.delete(data, slice(start, end))
        estimates.append(statistic(jack_sample, *args, **kwargs))

    estimates = np.array(estimates)
    mean_estimate = np.mean(estimates)
    error = np.sqrt((num_blocks - 1) / num_blocks * np.sum((estimates - mean_estimate) ** 2))

    return mean_estimate, error

def density(data, volume):
    return np.mean(data) / volume

def susceptibility(data, volume, factor):
    data_squared = np.square(data)
    return factor * (np.mean(data_squared) - np.mean(data)**2) / volume

def binder_cumulant(data, volume):
    data_squared = np.square(data)
    data_fourth = np.square(data_squared)
    return 1 - np.mean(data_fourth) / (3 * np.mean(data_squared)**2)