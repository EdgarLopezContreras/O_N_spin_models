#ifndef DATA_SAVER_CLASS_TEMPLATE
#define DATA_SAVER_CLASS_TEMPLATE

#include <iostream>
#include <vector>
#include <sstream>
#include <filesystem>
#include <string>
#include <ctime>
#include <span>
#include <unordered_map>
#include <H5Cpp.h>

using namespace std;
namespace fs = std::filesystem;
using namespace H5;

template <typename T, typename P>
class data_saver {
private:
    vector<P> dimensions; // First entry are the spin dimensions and second entry are the spatial dimensions
    const P batch; // Batch number to label the results
    const bool save_history;  // When true, the spin configurations will be sved each time measures are taken
    H5std_string file_name;
    unique_ptr<H5File> file;
    unordered_map<string, DataSet> datasets; // Map of routes and DataSets to save data in hdf5 format
    T beta; // inverse temperature
    const P L, sweeps, skip, thermalization; // length, total number of sweeps, sweeps between measurments
    const bool thermalization_skiped;
    unordered_map<string, string> labels_map;

    string get_date() {
        time_t now = time(0);
        char buffer[80];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&now));
        return string(buffer);
    }

    void create_dataset(const string& route, const PredType& data_type, const unordered_map<string, string>& attributes) {
        hsize_t dim = 0;
        hsize_t maxdim = H5S_UNLIMITED;
        DataSpace dataspace(1, &dim, &maxdim);

        DSetCreatPropList props;
        hsize_t chunk_size = 1000; 
        props.setChunk(1, &chunk_size);

        DataSet dataset = file->createDataSet(route, data_type, dataspace, props);

        // Create atributes (metadata)
        for (const auto& attribute : attributes) {
            dataset.createAttribute(attribute.first, StrType(PredType::C_S1, H5T_VARIABLE), DataSpace()).write(StrType(PredType::C_S1, H5T_VARIABLE), &attribute.second);
        }
        
        datasets[route] = dataset;
    }

    void create_dataset(const string& route, const StrType& data_type, const unordered_map<string, string>& attributes) {
        hsize_t dim = 0;
        hsize_t maxdim = H5S_UNLIMITED;
        DataSpace dataspace(1, &dim, &maxdim);

        DSetCreatPropList props;
        hsize_t chunk_size = 1000; 
        props.setChunk(1, &chunk_size);

        DataSet dataset = file->createDataSet(route, data_type, dataspace, props);

        // Create atributes (metadata)
        for (const auto& attribute : attributes) {
            dataset.createAttribute(attribute.first, StrType(PredType::C_S1, H5T_VARIABLE), DataSpace()).write(StrType(PredType::C_S1, H5T_VARIABLE), &attribute.second);
        }
        
        datasets[route] = dataset;
    }

    static string get_model_name(vector<P>& dims) {
        if (dims[0] == 1) {
            return "Ising";
        }
        return ("O(" + to_string(dims[0]) + ")");
    }

    void prepare_hdf5_dataset(const string& nombre, hsize_t count, DataSet& dataset, DataSpace& new_dataspace, DataSpace& memspace) {
        hsize_t old_size, new_size;
        dataset.getSpace().getSimpleExtentDims(&old_size, NULL);
        new_size = old_size + count;
        dataset.extend(&new_size);

        new_dataspace = dataset.getSpace();
        hsize_t start = old_size;
        new_dataspace.selectHyperslab(H5S_SELECT_SET, &count, &start);

        memspace = DataSpace(1, &count);
    }

    void save_data_hdf5(const string& nombre, const span<const T>& datos, const PredType& data_type) {
        DataSet& dataset = datasets[nombre];
        DataSpace new_dataspace, memspace;
        hsize_t count = datos.size();
        
        prepare_hdf5_dataset(nombre, count, dataset, new_dataspace, memspace);

        dataset.write(datos.data(), data_type, memspace, new_dataspace);
    }

    void save_string_hdf5(const string& nombre, const span<const string>& content) {
        DataSet& dataset = datasets[nombre];
        DataSpace new_dataspace, memspace;
        hsize_t count = content.size();
        
        prepare_hdf5_dataset(nombre, count, dataset, new_dataspace, memspace);

        dataset.write(content.data(), StrType(PredType::C_S1, H5T_VARIABLE), memspace, new_dataspace);
    }

    void save_bool_hdf5(const string& nombre, const span<const uint8_t>& content) {
        DataSet& dataset = datasets[nombre];
        DataSpace new_dataspace, memspace;
        hsize_t count = content.size();
        
        prepare_hdf5_dataset(nombre, count, dataset, new_dataspace, memspace);

        dataset.write(content.data(), PredType::NATIVE_UINT8, memspace, new_dataspace);
    }

    static string create_folder(vector<P>& dims) {
        ostringstream folder_name;

        folder_name << "../raw_data/Raw_Data_" << to_string(dims[1]) << "D_";
        
        folder_name << get_model_name(dims);

        string base_folder = folder_name.str();

        fs::create_directories(base_folder);

        return base_folder;
    }

public:
    // Constructor
    data_saver(const vector<P>& dims, const P batch_number, const bool save_hist, const P len, const T inv_temp, const P swee, const P ski, const P therm, const bool therm_skip): dimensions(dims), batch(batch_number), save_history(save_hist), L(len), beta(inv_temp), sweeps(swee), skip(ski), thermalization(therm), thermalization_skiped(therm_skip){

        // Creates folder to store data
        string base_folder = create_folder(dimensions);

        file_name = base_folder + "/" + "L_" + to_string(L) +"_beta_" + to_string(static_cast<int>(beta * 1000000)) + "_run_" + to_string(batch) + ".h5";

        file = make_unique<H5File>(file_name, H5F_ACC_TRUNC);

        //Create groups
        file->createGroup("/measurements");
        Group conf_group = file->createGroup("/configurations");

        // Create global atributes (metadata)
        const string date = get_date();
        const double value = 1.0, neg_value = -1.0;
        const uint8_t is_finished = 0;
        const string algorithm = "metropolis", boundary_conditions = "periodic", geometry_lat = "cubic", author_name = "Edgar Lopez-Contreras", version_number = "1.0.0";
        string model_name = get_model_name(dimensions);
        
        file->createAttribute("model", StrType(PredType::C_S1, H5T_VARIABLE), DataSpace()).write(StrType(PredType::C_S1, H5T_VARIABLE), &model_name);
        file->createAttribute("spin dimensions", PredType::NATIVE_INT, DataSpace()).write(PredType::NATIVE_INT, &dimensions[0]);
        file->createAttribute("spatial dimensions", PredType::NATIVE_INT, DataSpace()).write(PredType::NATIVE_INT, &dimensions[1]);
        file->createAttribute("length", PredType::NATIVE_INT, DataSpace()).write(PredType::NATIVE_INT, &L);
        file->createAttribute("algorithm", StrType(PredType::C_S1, H5T_VARIABLE), DataSpace()).write(StrType(PredType::C_S1, H5T_VARIABLE), &algorithm);
        file->createAttribute("boundary conditions", StrType(PredType::C_S1, H5T_VARIABLE), DataSpace()).write(StrType(PredType::C_S1, H5T_VARIABLE), &boundary_conditions);
        file->createAttribute("lattice geometry", StrType(PredType::C_S1, H5T_VARIABLE), DataSpace()).write(StrType(PredType::C_S1, H5T_VARIABLE), &geometry_lat);
        file->createAttribute("lattice spacing", PredType::NATIVE_DOUBLE, DataSpace()).write(PredType::NATIVE_DOUBLE, &value);
        file->createAttribute("coupling constant", PredType::NATIVE_DOUBLE, DataSpace()).write(PredType::NATIVE_DOUBLE, &neg_value);
        file->createAttribute("boltzmann constant", PredType::NATIVE_DOUBLE, DataSpace()).write(PredType::NATIVE_DOUBLE, &value);
        file->createAttribute("target iterations[sweep]", PredType::NATIVE_INT, DataSpace()).write(PredType::NATIVE_INT, &sweeps);
        file->createAttribute("measurments each[sweep]", PredType::NATIVE_INT, DataSpace()).write(PredType::NATIVE_INT, &skip);
        file->createAttribute("start", StrType(PredType::C_S1, H5T_VARIABLE), DataSpace()).write(StrType(PredType::C_S1, H5T_VARIABLE), &date);
        file->createAttribute("inverse temperature", PredType::NATIVE_DOUBLE, DataSpace()).write(PredType::NATIVE_DOUBLE, &beta);
        file->createAttribute("run", PredType::NATIVE_INT, DataSpace()).write(PredType::NATIVE_INT, &batch);
        file->createAttribute("author", StrType(PredType::C_S1, H5T_VARIABLE), DataSpace()).write(StrType(PredType::C_S1, H5T_VARIABLE), &author_name);
        file->createAttribute("code version", StrType(PredType::C_S1, H5T_VARIABLE), DataSpace()).write(StrType(PredType::C_S1, H5T_VARIABLE), &version_number);
        file->createAttribute("has finished", PredType::NATIVE_UINT8, DataSpace()).write(PredType::NATIVE_DOUBLE, &is_finished);
        file->createAttribute("has skiped thermalization", PredType::NATIVE_UINT8, DataSpace()).write(PredType::NATIVE_DOUBLE, &thermalization_skiped);
        if (thermalization_skiped) {
            file->createAttribute("thermalization[sweeps]", PredType::NATIVE_INT, DataSpace()).write(PredType::NATIVE_INT, &is_finished);
        }
        else {
            file->createAttribute("thermalization[sweeps]", PredType::NATIVE_INT, DataSpace()).write(PredType::NATIVE_INT, &thermalization);
        }
        

        const string desc = "In 'first': the first configuration after thermalization is stored. In 'last': the last configuration from the simulation is stored. If 'history' is available, a field configuration is saved each time measurements are taken.";
        conf_group.createAttribute("description", StrType(PredType::C_S1, H5T_VARIABLE), DataSpace()).write(StrType(PredType::C_S1, H5T_VARIABLE), &desc);
        // Datasets atributes, where J is the coupling constant and \\langle x y \\rangle \\in \\Lambda is notation for nearest neighboor coordinates x and y in the lattice Lambda
        unordered_map<string, string> attributes_energy = {{"name", "energy"},{"type", "extensive"},{"method", "E = J\\sum_{\\langle x y \\rangle \\in \\Lambda} \\vec{s}_x \\cdot \\vec{s}_y"}};
        unordered_map<string, string> attributes_magnetization = {{"name", "magnetization"}, {"type", "extensive"}, {"method", "|\\vec{M}| = \\sum_{x \\in \\Lambda} \\vec{s}_x"}};
        unordered_map<string, string> attributes_configurations;

        // routes map
        labels_map["energy"] = "/measurements/energy";
        labels_map["magnetization"] = "/measurements/magnetization";
        labels_map["first"] = "/configurations/first";
        labels_map["last"] = "/configurations/last";

        // Creates datasets
        create_dataset("/measurements/energy", PredType::NATIVE_DOUBLE, attributes_energy);
        create_dataset("/measurements/magnetization", PredType::NATIVE_DOUBLE, attributes_magnetization);
        create_dataset("/configurations/first", PredType::NATIVE_DOUBLE, attributes_configurations);
        create_dataset("/configurations/last", PredType::NATIVE_DOUBLE, attributes_configurations);
        if (save_history)
        {
            labels_map["history"] = "/configurations/history";
            create_dataset("/configurations/history", PredType::NATIVE_DOUBLE, attributes_configurations);
        }
    }

    ~data_saver() {
        const string date = get_date();
        const uint8_t is_finished = 1;
        file->createAttribute("end", StrType(PredType::C_S1, H5T_VARIABLE), DataSpace()).write(StrType(PredType::C_S1, H5T_VARIABLE), &date);
        file->openAttribute("has finished").write(PredType::NATIVE_UINT8, &is_finished);
    }

    void save_data(const string& nombre, const span<const T>& datos) {
        const string route = labels_map[nombre];
        save_data_hdf5(route, datos, PredType::NATIVE_DOUBLE);
    }

    // void save_log(const span<const string>& messages, const span<const string>& timestams, const span<const uint8_t>& is_error) {
    //     if (!messages.empty()) {
    //         const string route_timestamp = "/logs/timestamp";
    //         const string route_message = "/logs/message";
    //         const string route_errors = "/logs/is_error";
            
    //         save_string_hdf5(route_message, messages);
    //         save_string_hdf5(route_timestamp, timestams);
    //         save_bool_hdf5(route_errors, is_error);
    //     }
    // }
};

#endif