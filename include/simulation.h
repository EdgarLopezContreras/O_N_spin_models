#ifndef SIMULATION_CLASS_TEMPLATE
#define SIMULATION_CLASS_TEMPLATE

#include <iostream>
#include "lattice.h"
#include "montecarlo.h"
#include "measure.h"
#include "data_saver.h"

using namespace std;

template <typename T, typename P>
class simulation {
private:
    lattice<T, P>& spin_lattice;
    montecarlo<T,P>& montecarlo_update;
    measure<T,P>& measure_procedure;
    data_saver<T,P>& saver_in_file;
    const bool save_history;

    const size_t thermalization;           // Number of sweeps to be performed in the thermalization
    const size_t sweeps;                   // Number of sweeps to be performed in the simulation after thermalization
    const size_t skip;                     // Measurements are taken each spik sweeps
    

public:
    // Constructor
    simulation(lattice<T, P>& lattice_instance, montecarlo<T,P>& montecarlo_instance, measure<T,P>& measure_instance, data_saver<T,P>& saver_instance, size_t n_thermalization, const size_t n_sweeps, const size_t n_skip, const bool save_hist): spin_lattice(lattice_instance), montecarlo_update(montecarlo_instance), measure_procedure(measure_instance), saver_in_file(saver_instance), thermalization(n_thermalization), sweeps(n_sweeps), skip(n_skip), save_history(save_hist) {}

    void update(const size_t sweeps_update, const size_t skiped) {
        if (skiped >= 1) {
            for (size_t i = 0; i < sweeps_update; i++)
            {
                montecarlo_update.checkerboard_update();
                if(i % skiped == 0) {
                    measure_procedure.perform_measurements();
                    measure_procedure.save_configuration();
                    if (save_history)
                    {   
                        measure_procedure.save_configuration();
                    }
                }
                if (i % 1000 == 0 || i == sweeps_update - 1) {
                    cout << "Sweep " << i << " out of " << sweeps_update << endl;

                    saver_in_file.save_data("energy", measure_procedure.get_data("energy"));
                    saver_in_file.save_data("magnetization", measure_procedure.get_data("magnetization"));
                    if (save_history)
                    {
                        saver_in_file.save_data("history", measure_procedure.get_data("configurations"));
                    }
                    
                    measure_procedure.clear_all_data();
                }
            }
        }
        else {
            for (size_t i = 0; i < sweeps_update; i++)
            {
                montecarlo_update.checkerboard_update();

                if (i % 1000 == 0 || i == sweeps_update - 1) {
                    cout << "Sweep " << i << " out of " << sweeps_update << endl;
                }
            }
        }
    }

    void simulate() {
        cout << "Starting thermalization" << endl;
        update(thermalization, 0);
        cout << "Thermaliztion finished" << endl;

        measure_procedure.save_configuration();
        saver_in_file.save_data("first", measure_procedure.get_data("configurations"));
        measure_procedure.clear_all_data();

        cout << "Starting simulation" << endl;
        update(sweeps, skip);
        cout << "Simulation finished" << endl;

        measure_procedure.save_configuration();
        saver_in_file.save_data("last", measure_procedure.get_data("configurations"));
        measure_procedure.clear_all_data();
    }

    void print_info() {
        cout << "Thermalization sweeps: " << thermalization << endl; 
        cout << "Sweeps simulation: " << sweeps  << endl;
        cout << "Measurments each [sweeps]: " << skip << endl;
    }
    
};

#endif