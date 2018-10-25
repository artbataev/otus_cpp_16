#include <iostream>
#include <vector>
#include <string>
#include <sstream>
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
        std::cout << "Execute with tow arguments - number of clusters and modelfname, e.g.:" << std::endl;
        std::cout << argv[0] << " 100 modelfname" << std::endl;
        exit(0);
    }

    // reading data
    std::vector<realty_entry_t> points;
    std::string buf;
    while (std::getline(std::cin, buf))
        points.emplace_back(RealtyDataEntry(buf, /*is_eval=*/false).get_vector());


    // construct model
    std::vector<realty_entry_t> initial_centers;
    dlib::kcentroid<realty_kernel_t> model_k_centroids(realty_kernel_t(0.1), 0.01, 100);
    dlib::kkmeans<realty_kernel_t> model(model_k_centroids);

    model.set_number_of_centers(static_cast<size_t>(num_clusters));
    pick_initial_centers(num_clusters, initial_centers, points, model.get_kernel());

    // train model
    model.train(points, initial_centers);

    dlib::serialize(filename) << model;

    return 0;
}
