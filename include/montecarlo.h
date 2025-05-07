#ifndef MONTECARLO_CLASS_TEMPLATE
#define MONTECARLO_CLASS_TEMPLATE

#include <iostream>
#include <vector>
#include <random>
#include "lattice.h"

using namespace std;

template <typename T, typename P>
class montecarlo {
private:
    lattice<T, P>& spin_lattice;
    random_device rd;
    mt19937 gen;                // Random number generator
    uniform_real_distribution<double> prob_dist;  // Used in the accept/reject step of the updates
    normal_distribution<double> dist;   // Used in to generate random spins
    uniform_int_distribution<int> int_dist;      // For Ising
    const P L;                   // Spatial length of the lattice
    const P total_dimensions;    // Spatial dimensions + spin dimensions
    vector<P> dimensions;        // N = dimensions[0] is the dimension of the spin, d = dimensions[1] is the spatial dimension of the lattice
    const T beta;                      // inverse temperature
    const bool is_ising;

public:
    // Constructor
    montecarlo(lattice<T, P>& lattice_instance, const vector<int>& dims, const P length, const T inverse_temperature): spin_lattice(lattice_instance), dimensions(dims), L(length), beta(inverse_temperature), total_dimensions(dims[1] + 1), gen(rd()), prob_dist(0.0, 1.0), dist(0.0, 1.0), int_dist(0, 1), is_ising(dims[0] == 1) {}

    bool accept_metropolis(T& delta) {
        return (prob_dist(gen) <= min(1.0, exp(- beta * delta)));
    }

    void metropolis_update(const vector<int>& spatial_indices) {
        vector<T> proposed_spin(dimensions[0]);
        T norm = 0, delta_energy = 0;
        
        // Generates a random vector where each component has normal distribution
        for(P i = 0; i < dimensions[0]; i++) {
            proposed_spin[i] = dist(gen);
            norm += proposed_spin[i] * proposed_spin[i];
        }
        // Normalization
        norm = sqrt(norm);
        for (P i = 0; i < dimensions[0]; i++) {
            proposed_spin[i] /= norm;
        }

        vector<int> indices(total_dimensions, 0);
        copy(spatial_indices.begin(), spatial_indices.end(), indices.begin() + 1);

        // Calculates the change of energy by changing the spin
        for (P i = 0; i < dimensions[0]; i++)
        {
            indices[0] = i;
            T current_spin = spin_lattice.get_spin(indices);
            T new_spin = proposed_spin[i];
            for (P j = 0; j < dimensions[1]; j++) {
                for (int k : {-1, 1})
                {
                    vector<int> neighbor_indices = indices;
                    neighbor_indices[1 + j] += k;
                    T neighbor_spin = spin_lattice.get_spin(neighbor_indices);
                    delta_energy += (current_spin - new_spin) * neighbor_spin;
                }
            }
        }
        
        // Modify the current spin if accepted by the Metropolis algorithm
        if (accept_metropolis(delta_energy)) {
            for (P i = 0; i < dimensions[0]; i++) {
                indices[0] = i; 
                spin_lattice.set_spin(indices, proposed_spin[i]);
            }
        }
    }

    int random_int() {
        return (int_dist(gen) * 2) - 1;
    }

    void metropolis_update_ising(const vector<int>& spatial_indices) {
        vector<T> proposed_spin(dimensions[0]);
        T delta_energy = 0;
        
        // Generates a random vector where each component has normal distribution
        for(P i = 0; i < dimensions[0]; i++) {
            proposed_spin[i] = random_int();
        }

        vector<int> indices(total_dimensions, 0);
        copy(spatial_indices.begin(), spatial_indices.end(), indices.begin() + 1);

        // Calculates the change of energy by changing the spin
        for (P i = 0; i < dimensions[0]; i++)
        {
            indices[0] = i;
            T current_spin = spin_lattice.get_spin(indices);
            T new_spin = proposed_spin[i];
            for (P j = 0; j < dimensions[1]; j++) {
                for (int k : {-1, 1})
                {
                    vector<int> neighbor_indices = indices;
                    neighbor_indices[1 + j] += k;
                    T neighbor_spin = spin_lattice.get_spin(neighbor_indices);
                    delta_energy += (current_spin - new_spin) * neighbor_spin;
                }
            }
        }
        
        // Modify the current spin if accepted by the Metropolis algorithm
        if (accept_metropolis(delta_energy)) {
            for (P i = 0; i < dimensions[0]; i++) {
                indices[0] = i; 
                spin_lattice.set_spin(indices, proposed_spin[i]);
            }
        }
    }

    // Function to diferenciate the black and white sites in the checkerboard actualization procedure
    bool is_black_site(const vector<int>& indices) {
        int sum = 0;
        for (size_t i = 0; i < indices.size(); i++) {
            sum += indices[i];  
        }
        return (sum % 2) == 0;
    }

    void checkerboard_update() {
        if (is_ising) {
            // black sites update
            for (size_t flat_index = 0; flat_index < spin_lattice.total_spin_entries(); flat_index += dimensions[0]) {
                double norm = 0.0;
                vector<P> spatial_indices = spin_lattice.get_spatial_coordinates(flat_index);
                if (is_black_site(spatial_indices))
                {
                    metropolis_update_ising(spatial_indices);
                }
                
            }

            // white sites update
            for (size_t flat_index = 0; flat_index < spin_lattice.total_spin_entries(); flat_index += dimensions[0]) {
                double norm = 0.0;
                vector<P> spatial_indices = spin_lattice.get_spatial_coordinates(flat_index);
                if (!(is_black_site(spatial_indices)))
                {
                    metropolis_update_ising(spatial_indices);
                }
            }
        }
        else {
            // black sites update
            for (size_t flat_index = 0; flat_index < spin_lattice.total_spin_entries(); flat_index += dimensions[0]) {
                double norm = 0.0;
                vector<P> spatial_indices = spin_lattice.get_spatial_coordinates(flat_index);
                if (is_black_site(spatial_indices))
                {
                    metropolis_update(spatial_indices);
                }
                
            }

            // white sites update
            for (size_t flat_index = 0; flat_index < spin_lattice.total_spin_entries(); flat_index += dimensions[0]) {
                double norm = 0.0;
                vector<P> spatial_indices = spin_lattice.get_spatial_coordinates(flat_index);
                if (!(is_black_site(spatial_indices)))
                {
                    metropolis_update(spatial_indices);
                }
            }
        }
    }
};

#endif