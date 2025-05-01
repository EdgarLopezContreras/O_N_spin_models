#include <iostream>
#include <vector>
#include <string>
#include "lattice.h"
#include "montecarlo.h"
#include "measure.h"
#include "data_saver.h"
#include "simulation.h"

using namespace std;

int main(int argc, char* argv[]) {

    // Parameters
    const int spin_dimension = (argc > 1) ? stoi(argv[1]) : 1;
    const int spatial_dimension = (argc > 2) ? stoi(argv[2]) : 3;
    const int length = (argc > 3) ? stoi(argv[3]) : 16; // length of the cubic lattice (in this code, lattice spacing is 1)
    const double beta = (argc > 4) ? stod(argv[4]) : 0.455; // inverse temperature
    const size_t thermalization_sweeps = (argc > 5) ? stoi(argv[5]) : 10000; // number of sweeps performed in the thermalization
    const size_t sweeps_simulation = (argc > 6) ? stoi(argv[6]) : 100000;     // number of sweeps performed after thermalization in the simulation
    const size_t skip_simulation = (argc > 7) ? stoi(argv[7]) : 10;         // measurments are taken wach skip_simulation sweeps after thermalization
    const int batch = (argc > 8) ? stoi(argv[8]) : 0;                       // batch number label, must be different each time (otherwise the data is overwritten)
    const bool save_history = (argc > 9) ? !(stoi(argv[9]) != 1) : false;            // When true, the configurations will be saved each skip_simulation configurations

    vector<int> dimensions = {spin_dimension,spatial_dimension};


    // Class instances
    lattice<double,int> spin_lattice(dimensions, length);

    const bool thermalization_skiped = spin_lattice.hot_start();

    data_saver<double,int> saver_in_file(dimensions, batch, save_history, length, beta, sweeps_simulation, skip_simulation, thermalization_sweeps, thermalization_skiped);
    montecarlo<double,int> montecarlo_update(spin_lattice, dimensions, length, beta);
    measure<double,int> measurements(spin_lattice);
    simulation<double,int> simulation_updates(spin_lattice, montecarlo_update, measurements, saver_in_file, thermalization_sweeps, sweeps_simulation, skip_simulation, save_history);

    spin_lattice.print_info();
    simulation_updates.print_info();
    cout << "Beta: " << beta << endl;
    cout << "Performing simulation" << endl;
    simulation_updates.simulate();
}