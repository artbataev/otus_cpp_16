#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <dlib/clustering.h>
#include "realty_data.h"


int main(int argc, char *argv[]) {
    bool executed_correctly = true;
    std::stringstream whats_wrong;
    int num_clusters = 0;
    std::string filename;

    if (argc != 3) {
        executed_correctly = false;
        whats_wrong << "Incorrect number of arguments";
    } else {
        try {
            num_clusters = std::stoi(argv[1]);
            if (num_clusters <= 0) {
                whats_wrong << "Number of clusters must be positive";
                executed_correctly = false;
            }
        } catch (std::exception&) {
            executed_correctly = false;
            whats_wrong << "Number of clusters must be a number";
        }
        filename = argv[2];
    }

    if (!executed_correctly) {
        std::cout << "Error: " << whats_wrong.str() << std::endl;
        std::cout << "Execute with 2 arguments - number of clusters and modelfname, e.g.:" << std::endl;
        std::cout << argv[0] << " 100 modelfname" << std::endl;
        exit(0);
    }

    // reading data
    std::cout << "Reading data..." << std::endl;
    std::vector<realty_entry_full_t> realty_entries_full;
    std::string buf;
    realty_entry_full_t current_entry;
    auto& source_stream = std::cin;
//    std::fstream source_stream("../data/dataset.csv");
    while (source_stream >> buf) {
        parse_entry_from_string(buf, current_entry);
        realty_entries_full.emplace_back(current_entry);
    }

    if(realty_entries_full.empty())
        throw std::logic_error("no data! can't train model");

    // converting data
    std::cout << "Converting data" << std::endl;
    std::vector<realty_entry_t> realty_entries;
    realty_entries.reserve(realty_entries_full.size());
    std::transform(realty_entries_full.cbegin(), realty_entries_full.cend(),
                   std::back_inserter(realty_entries), convert_realty_entry_full);


    // construct model
    std::vector<realty_entry_t> initial_centers;
    dlib::kcentroid<realty_kernel_t> model_k_centroids(realty_kernel_t(0.1), 0.01, 100);
    dlib::kkmeans<realty_kernel_t> model(model_k_centroids);

    model.set_number_of_centers(static_cast<size_t>(num_clusters));
    pick_initial_centers(num_clusters, initial_centers, realty_entries, model.get_kernel());

    std::cout << "Total data size: " << realty_entries.size() << " records" << std::endl;
    // train model
    std::cout << "Starting model training..." << std::endl;
    model.train(realty_entries, initial_centers);
    std::cout << "Done model training" << std::endl;

    dlib::serialize(filename + ".model") << model;
    dlib::serialize(filename + ".data") << realty_entries_full;

    return 0;
}
