""""
This module contains the functions used to read the .h5 files and collect its data.

"""

import os
import re
import h5py
import numpy as np

def get_h5_filenames(path, regex_pattern):
    directory_content = os.listdir(path)
    files_with_error = []
    files_without_error = []

    for file in directory_content:
        if re.fullmatch(regex_pattern, file):
            route = path / file
            try:
                with h5py.File(route, 'r') as f:
                    has_finished = f.attrs.get('has finished', None)
                    if has_finished == 1:
                        files_without_error.append(route)
                    elif has_finished == 0:
                        files_with_error.append(route)
                        print(f"Problem in file: {file}")
            except Exception as e:
                print(f"Failed to read file {file}: {e}")
    
    path_problem_file = os.path.join(path, "attention.txt")

    with open(path_problem_file, 'a') as f:
        for file in files_with_error:
            f.write(file + '\n')

    print(f"Conflicting files written to {path_problem_file}")
    
    return files_without_error

def regex_expression(length=None, beta_label=None, run=None):
    regex_pattern = r"L_"
    regex_pattern += rf"({length})" if length is not None else r"(\d+)"
    regex_pattern += r"_beta_"
    regex_pattern += rf"({beta_label})" if beta_label is not None else r"(\d+)"
    regex_pattern += r"_run_"
    regex_pattern += rf"({run})" if run is not None else r"(\d+)"
    regex_pattern += r"\.h5"
    return regex_pattern

def get_list_of_params(path, length=None, beta_label=None, run=None):
    lengths = []
    betas = []
    runs = []
    regex_pattern = regex_expression(length, beta_label, run)
    directory_content = os.listdir(path)

    for file in directory_content:
        matches_regex = re.fullmatch(regex_pattern, file)
        if matches_regex:
            L, beta, run = map(int, matches_regex.groups())
            lengths.append(L)
            betas.append(beta)
            runs.append(run)
            
    return sorted(set(lengths)), sorted(set(betas)), sorted(set(runs))


def get_data(path, dataset_name, length, beta):
    data = np.empty((0,))
    regex_pattern_beta = regex_expression(length, beta)
    list_of_files = get_h5_filenames(path, regex_pattern_beta)
    for file in list_of_files:
        with h5py.File(file, 'r') as f:
            data = np.append(data, f['measurements/' + dataset_name][:])
    return data
    
def get_beta(path, length, beta):
    regex_pattern_beta = regex_expression(length, beta)
    list_of_files = get_h5_filenames(path, regex_pattern_beta)
    for file in list_of_files:
        with h5py.File(file, 'r') as f:
            return f.attrs["inverse temperature"]
        
def file_name(length):
    return f"L_{length}.h5"

# def get_data(path, dataset_name, length=None):
#     if length is not None:
#         list_of_lengths, list_of_betas, list_of_runs = get_list_of_params(path)
#         for length in list_of_lengths:
#             print(f"Processing length {length}")
#             results = np.empty((0,3))
#             list_of_lengths_b, list_of_betas, list_of_runs = get_list_of_params(path, l)
#             for beta in list_of_betas:
#                 data = get_data(path, dataset_name, length, beta)

