#ifndef LATTICE_CLASS_TEMPLATE
#define LATTICE_CLASS_TEMPLATE

#include <iostream>
#include <vector>
#include <random>
#include <stdexcept>

using namespace std;

template <typename T, typename P>
class lattice {
private:
    const P L;                   // Spatial length of the lattice
    const P total_dimensions;    // Spatial dimensions + spin dimensions
    vector<P> dimensions;        // N = dimensions[0] is the dimension of the spin, d = dimensions[1] is the spatial dimension of the lattice
    vector<P> strides;          // Indexing strides
    vector<T> spins;            // 1D array storing the spin components of the lattice

public:
    // Constructor
    lattice(const vector<int>& dims, const P length) : dimensions(dims), L(length), total_dimensions(dims[1] + 1) {
        // Once all is tested to be correct this part should be commented
        // if (dims.empty()) {
        //     throw invalid_argument("Dimensions are empty, something went wrong.");
        // }

        strides.resize(total_dimensions - 1);
        // Fill strides
        strides[0] = 1;  // Spins in the first dimension
        strides[1] = dimensions[0]; 
        for (int i = 2; i <= total_dimensions - 1; i++) {
            strides[i] = strides[i - 1] * L;
        }

        // Creates 1D vector that will store the spins
        spins.resize(static_cast<size_t>(pow(L, dimensions[1]) * dimensions[0]), 0);
    }

    P periodic_index(int index) const {
        static bool is_power_of_2 = (L & (L - 1)) == 0; // in the first call of get_spin verify if L is power of 2 to improve % operation
        if (is_power_of_2)
        {
            return index & (L - 1);
        }
        else {
            return (index == -1) ? L - 1 : (index == L ? 0 : index);
        }
    }

    size_t get_flat_index(const vector<P>& indices) const {
        size_t flat_index = indices[0] * strides[0];
        for (P i = 1; i < total_dimensions; i++) {
            flat_index += periodic_index(indices[i]) * strides[i];
        }
        return flat_index;
    }

    // Operator to read spins as a multidimensional array with periodic boundary conditions
    const T& get_spin(const vector<P>& indices) const {
        // Once all is tested to be correct this part should be commented
        // if (indices.size() != total_dimensions) {
        //     throw out_of_range("Incorrect number of indices.");

        //     for (P i = 0; i < total_dimensions; i++)
        //     {
        //         if (indices[i] < 0 || 0 < i && indices[i] >= L || i == 0 && indices[i] >= dimensions[1]) {
        //             throw out_of_range("Indices out of range.");
        //         }
        //     }
            
        // }
        return spins[get_flat_index(indices)];
    }

    // Operator to read spins as linear array
    const T& get_spin_flat_index(const size_t& flat_index) const {
        return spins[flat_index];
    }

    // Operator to set spins as a multidimensional array with periodic boundary conditions
    void set_spin(const vector<P>& indices, T value) {
        // Once all is tested to be correct this part should be commented
        // if (indices.size() != total_dimensions) {
        //     throw out_of_range("Incorrect number of indices.");

        //     for (P i = 0; i < total_dimensions; i++)
        //     {
        //         if (indices[i] < 0 || 0 < i && indices[i] >= L || i == 0 && indices[i] >= dimensions[1]) {
        //             throw out_of_range("Indices out of range.");
        //         }
        //     }
            
        // }
        spins[get_flat_index(indices)] = value;
    }

    vector<P> get_spatial_coordinates(size_t flat_index) const {
        static bool is_power_of_2 = (L & (L - 1)) == 0;
        vector<int> indices(total_dimensions - 1); 
        for (size_t i = total_dimensions; i-- > 1;) {
            size_t coord = flat_index / strides[i];
            indices[i - 1] = is_power_of_2 ? (coord & (L - 1)) : (coord % L);
        }
        return indices;
    }
    

    size_t total_spin_entries() const {
        return spins.size();
    }

    P get_total_dimensions() const {
        return total_dimensions;
    }

    P get_spin_dimensions() const {
        return dimensions[0];
    }

    P get_spatial_dimensions() const {
        return dimensions[1];
    }

    // Sets a uniform configuration of spins
    bool cold_start() {
        cout << "Cold start" << endl;
        for (size_t flat_index = 0; flat_index < spins.size(); flat_index++) {
            int spin_index = flat_index % dimensions[0];
            spins[flat_index] = (spin_index == 0) ? 1 : 0; // sets the first component of spins to 1 and the remaining to 0
        }

        return false;
    }

    // Sets a random configuration of spins
    bool hot_start() {
        cout << "Hot start" << endl;
        random_device rd;
        mt19937 gen(rd()); 
        normal_distribution<double> dist(0.0, 1.0);
    
        for (size_t flat_index = 0; flat_index < spins.size(); flat_index += dimensions[0]) {
            double norm = 0.0;
            // Generates a random vector where each component has normal distribution
            for (P i = 0; i < dimensions[0]; i++) {
                spins[flat_index + i] = dist(gen);
                norm += spins[flat_index + i] * spins[flat_index + i];
            }
            // Normalization
            norm = sqrt(norm);
            for (P i = 0; i < dimensions[0]; i++) {
                spins[flat_index + i] /= norm;
            }
        }
        return false;
    }

    // pointer to spins (for MPI)
    int* spins_ptr() { return spins.data(); }

    // Prints the spins vector info (for depuration)
    void print_info() const {
        cout << "Spatial dimensions: " << dimensions[1] << endl; 
        cout << "Lattice length: " << L  << endl;
        cout << "Volume: " << pow(L, dimensions[1]) << endl;
        cout << "Spin dimensions: " << dimensions[0] << endl;
        cout << "Spins array size: " << spins.size() << endl;
    }

    // Prints the spin at the given coordinates (for depuration)
    void print_spin(const vector<P>& coordinates) const {
        for (P index = 0; index < dimensions[0]; index++)
        {
            vector<P> indices(total_dimensions, 0);
            indices[0] = index % dimensions[0];
            for (int i = 0; i < dimensions[1]; i++) {
                indices[i + 1] = coordinates[i];
            }

            // Prints the spins as n-tuples
            if (indices[0] == 0) cout << "(";
            cout << get_spin(indices); // Use the operator to get the spin
            if (indices[0] == dimensions[0] - 1) {
                cout << ")_{";
                // Prints the spatial coordinates
                for (int i = 0; i < dimensions[1]; i++) {
                    cout << coordinates[i];
                    if (i < dimensions[1] - 1) cout << ",";
                }
                cout << "}" << endl;
            } 
            else {
                cout << ", ";
            }
        }
    }

    // Prints the spins array (for depuration)
    void print_spins() const {
        for (size_t index = 0; index < spins.size(); index += dimensions[0]) {
            print_spin(get_spatial_coordinates(index));
        }
    }

    // Check the periodic boundary conditions
    void print_periodic_boundary() const {
        for (size_t index = 0; index < spins.size(); index += dimensions[0]) {
            vector<P> coords = get_spatial_coordinates(index); // Spatial coordinates
            for (P i = 0; i < dimensions[1]; i++) {
                if(coords[i] == 0) {
                    cout << "Boundary pair 0: {"; 
                    for (P j = 0; j < dimensions[1]; j++) {
                        cout << coords[j];
                        if(j < dimensions[1] - 1) cout << ",";
                    }
                    cout << "}" << endl;
                    vector<P> copy_coords = coords;
                    copy_coords[i] -= 1;
                    print_spin(copy_coords);
                    copy_coords[i] = L - 1;
                    print_spin(copy_coords);
                }
                else if (coords[i] == L - 1) {
                    cout << "Boundary pair L: {"; 
                    for (P j = 0; j < dimensions[1]; j++) {
                        cout << coords[j];
                        if(j < dimensions[1] - 1) cout << ",";
                    }
                    cout << "}" << endl;
                    vector<P> copy_coords = coords;
                    copy_coords[i] += 1;
                    print_spin(copy_coords);
                    copy_coords[i] = 0;
                    print_spin(copy_coords);
                }
            }
        }
    }
};

#endif