#ifndef MEASURE_CLASS_TEMPLATE
#define MEASURE_CLASS_TEMPLATE

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <span>
#include "lattice.h"

using namespace std;

template <typename T, typename P>
class measure {
private:
    lattice<T, P>& spin_lattice;
    vector<T> energy;
    vector<T> magnetization;
    unordered_map<string, vector<double>*> data_map;
    vector<T> configurations;

public:
    // Constructor
    measure(lattice<T, P>& lattice_instance): spin_lattice(lattice_instance) {
        data_map["energy"] = &energy;
        data_map["magnetization"] = &magnetization;
        data_map["configurations"] = &configurations;
        energy.reserve(1000);
        magnetization.reserve(1000);
    }
    
    void measure_energy() {
        T energy_configuration = 0;

        for (size_t i = 0; i < spin_lattice.total_spin_entries(); i += spin_lattice.get_spin_dimensions())
        {
            vector<P> spatial_coords = spin_lattice.get_spatial_coordinates(i);
            vector<P> coordinates(spin_lattice.get_total_dimensions());
            copy(spatial_coords.begin(), spatial_coords.end(), coordinates.begin() + 1);

            for (P j = 0; j < spin_lattice.get_spin_dimensions(); j++)
            {
                coordinates[0] = j;
                for (P k = 0; k < spin_lattice.get_spatial_dimensions(); k++) {
                    vector<int> neighbor_coordinates = coordinates;
                    neighbor_coordinates[1 + k] += 1;
                    // cout << "Par energía: ";
                    // spin_lattice.print_spin(spin_lattice.get_spatial_coordinates(i + j));
                    // spin_lattice.print_spin(spin_lattice.get_spatial_coordinates(spin_lattice.get_flat_index(neighbor_coordinates)));
                    // cout << "Cambios de coordenada k: " << k << ", cambio: " << coordinates[1 + k] << ", por: " << neighbor_coordinates[1 + k] << endl;
                    // cout << "Energy contribution: " << -spin_lattice.get_spin_flat_index(i + j) * spin_lattice.get_spin(neighbor_coordinates) << ", ";
                    energy_configuration -= spin_lattice.get_spin_flat_index(i + j) * spin_lattice.get_spin(neighbor_coordinates);
                    // cout << "Energy total: " << energy_configuration << endl;
                    // neighbor_coordinates[1 + k] = coordinates[1 + k];
                }
            }
        }
        // cout << "Energy: " << energy_configuration << endl;
        energy.push_back(energy_configuration);
    }

    void measure_magnetization() {
        vector<T> magnetization_vector(spin_lattice.get_spin_dimensions(), 0);
        T norm = 0;

        for (size_t i = 0; i < spin_lattice.total_spin_entries(); i++)
        {
            magnetization_vector[i % spin_lattice.get_spin_dimensions()] += spin_lattice.get_spin_flat_index(i);
        }
        
        for (P i = 0; i < spin_lattice.get_spin_dimensions(); i++)
        {
            norm += magnetization_vector[i] * magnetization_vector[i];
        }
        // cout << "Magnetization: " << sqrt(norm) << endl;
        magnetization.push_back(sqrt(norm));
    }

    void save_configuration() {
        for (size_t i = 0; i < spin_lattice.total_spin_entries(); i++)
        {
            configurations.push_back(spin_lattice.get_spin_flat_index(i));
        }
    }

    void perform_measurements() {
        measure_energy();
        measure_magnetization();
    }

    span<const T> get_data(const string& key) const {
        auto it = data_map.find(key);
        const auto& vec = *(it->second);
        return span<const T>(vec);
    }

    void clear_all_data() {
        for (auto& pair : data_map) {
            pair.second->clear();
        }
    }
};

#endif