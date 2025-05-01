"""
This module contains the functions used to plot the data.

"""

import h5py
import matplotlib.pyplot as plt
from functions.read_h5 import file_name
import functions.routes as routes

def plot_data(name, list_of_lengths, title, xaxis, yaxis):
    plt.figure()
    
    for j in list_of_lengths:
        file = routes.PROCESSED_DATA_FOLDER / file_name(j)
        try:
            with h5py.File(file, 'r') as f:
                beta = f[name][:,0]
                data = f[name][:,1]
                errors = f[name][:,2]
                plt.errorbar(beta, data, yerr=errors, label=str(j), fmt="-o")
        except Exception as e:
            print(f"Failed to read file {file}: {e}")
    
    plt.xlabel(xaxis)
    plt.ylabel(yaxis)
    plt.grid()
    plt.legend()
    plt.savefig(routes.PLOTS_FOLDER / title)