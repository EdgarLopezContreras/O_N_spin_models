"""
This script processes the raw data obtained in the simulations. The results
are saved in the ../results folder

"""


import argparse
import numpy as np
import h5py
import functions.routes as routes
from functions.read_h5 import get_data, get_list_of_params, get_beta, file_name
from functions.statistic_functions import delete_d_jackknife, density, susceptibility, binder_cumulant
from functions.plots import plot_data
from multiprocessing import Pool

def model_name(spin_dims):
    if spin_dims == 1:
        return "Ising"
    else:
        return f"O({spin_dims})"

def route_suffix(spatial_dims, spin_dims):
    return f"{spatial_dims}D_" + model_name(spin_dims)

def beta_processing(args):
    l, b, spatial_dims = args
    volume = l**spatial_dims

    energy = get_data(routes.RAW_DATA_FOLDER, "energy", l, b)
    magnetization = get_data(routes.RAW_DATA_FOLDER, "magnetization", l, b)
    inv_temp = get_beta(routes.RAW_DATA_FOLDER, l, b)

    energy_density = np.array([[inv_temp, *delete_d_jackknife(energy, 25, density, volume)]])
    magnetization_density = np.array([[inv_temp, *delete_d_jackknife(magnetization, 25, density, volume)]])
    specific_ht = np.array([[inv_temp, *delete_d_jackknife(energy, 25, susceptibility, volume, inv_temp)]])
    magnetic_sscpblt = np.array([[inv_temp, *delete_d_jackknife(magnetization, 25, susceptibility, volume, inv_temp**2)]])
    binder = np.array([[inv_temp, *delete_d_jackknife(magnetization, 25, binder_cumulant, volume)]])

    return {
        "energy": energy_density,
        "magnetization": magnetization_density,
        "specific_heat": specific_ht,
        "susceptibility": magnetic_sscpblt,
        "binder": binder,
        "sample_size": len(energy)
    }

def length_processing(l, beta, spatial_dims, spin_dims):
    leng, betas, runs = get_list_of_params(routes.RAW_DATA_FOLDER, l, beta)
    min_size = float('inf')
    max_size = 0

    with Pool() as pool:
        results = pool.map(beta_processing, [(l, b, spatial_dims) for b in betas])

    energy_results = np.vstack([r["energy"] for r in results])
    magnetization_results = np.vstack([r["magnetization"] for r in results])
    specific_heat = np.vstack([r["specific_heat"] for r in results])
    magnetic_susceptibility = np.vstack([r["susceptibility"] for r in results])
    binder_cumulant_vec = np.vstack([r["binder"] for r in results])
    sample_sizes = [r["sample_size"] for r in results]
    min_size = min(sample_sizes)
    max_size = max(sample_sizes)

    with h5py.File(routes.PROCESSED_DATA_FOLDER / file_name(l), "w") as f:
        f.create_dataset("energy density", data=energy_results)
        f.create_dataset("magnetization density", data=magnetization_results)
        f.create_dataset("specific heat", data=specific_heat)
        f.create_dataset("magnetic susceptibility", data=magnetic_susceptibility)
        f.create_dataset("binder cumulant", data=binder_cumulant_vec)

        f.attrs["author"] = "Edgar Lopez-Contreras"
        f.attrs["length"] = l
        f.attrs["model"] = model_name(spin_dims)
        f.attrs["spatial dimensions"] = spatial_dims
        f.attrs["spin dimensions"] = spin_dims
        f.attrs["min sample size"] = min_size
        f.attrs["max sample size"] = max_size

def main():
    parser = argparse.ArgumentParser(description="Processer of raw data")

    parser.add_argument('--spatial_dims', type=int, required=True, help="Spatial dimensions of the spin system")
    parser.add_argument('--spin_dims', type=int, required=True, help="Spin dimensions of the spin system")
    parser.add_argument('--length', type=int, default=None, help="Number of spins per spatial dimension of the lattice (not required)")
    parser.add_argument('--beta', type=float, default=None, help="Inverse temperature in the simulation")
    parser.add_argument('--plot_results', type=bool, default=False, help="Should the results be ploted?")
    parser.add_argument('--min_l_plot', type=int, default=0, help='Min length to plot')
    parser.add_argument('--max_l_plot', type=int, default=float('inf'), help='Max length to plot')

    args = parser.parse_args()

    spatial_dims = args.spatial_dims
    spin_dims = args.spin_dims
    length = args.length
    beta = args.beta
    plot_results = args.plot_results
    min_l_plot = args.min_l_plot
    max_l_plot = args.max_l_plot

    routes.RAW_DATA_FOLDER = routes.DATA_ROUTE / ("Raw_Data_" + route_suffix(spatial_dims, spin_dims))
    routes.PROCESSED_DATA_FOLDER = routes.RESULTS_ROUTE / ("Data_" + route_suffix(spatial_dims, spin_dims))
    routes.PLOTS_FOLDER = routes.PLOTS_ROUTE / route_suffix(spatial_dims, spin_dims)

    if not routes.RAW_DATA_FOLDER.exists():
        raise FileNotFoundError(f"The folder {routes.RAW_DATA_FOLDER} does not exists")
    
    routes.PROCESSED_DATA_FOLDER.mkdir(parents=True, exist_ok=True) #Creates folders if doesn't exist

    lengths, betas, runs = get_list_of_params(routes.RAW_DATA_FOLDER, length, beta)

    for l in lengths:
        length_processing(l, beta, spatial_dims, spin_dims)

    if plot_results:
        routes.PLOTS_FOLDER.mkdir(parents=True, exist_ok=True)
        lengths, betas, runs = get_list_of_params(routes.RAW_DATA_FOLDER, length, beta)
        lengths = [i for i in lengths if (i >= min_l_plot and i <= max_l_plot)]

        plot_data("binder cumulant", lengths, "binder_cumulant.pdf", r"$\beta$", r"$U_4$")
        plot_data("energy density", lengths, "energy_density.pdf", r"$\beta$", r"$e$")
        plot_data("magnetization density", lengths, "magnetic_density.pdf", r"$\beta$", r"$m$")
        plot_data("specific heat", lengths, "specific_heat.pdf", r"$\beta$", r"$c_V$")
        plot_data("magnetic susceptibility", lengths, "magnetic_susceptibility.pdf", r"$\beta$", r"$\chi_m$")

if __name__ == '__main__':
    main()