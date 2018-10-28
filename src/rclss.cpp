#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <set>
#include <dlib/clustering.h>
#include "realty_data.h"

bool file_exists(const std::string& filename) {
    std::ifstream infile(filename);
    return infile.good();
}

int main(int argc, char *argv[]) {
    bool executed_correctly = true;
    std::stringstream whats_wrong;
    std::string filename;

    if (argc != 2) {
        executed_correctly = false;
        whats_wrong << "Incorrect number of arguments";
    } else {
        filename = argv[1];
        for (const auto& ext: std::set<std::string>{".model", ".data"})
            if (!file_exists(filename + ext)) {
                executed_correctly = false;
                whats_wrong << "No such file: " << filename + ext;
                break;
            }
    }

    if (!executed_correctly) {
        std::cout << "Error: " << whats_wrong.str() << std::endl;
        std::cout << "Execute with one argument - modelfname, e.g.:" << std::endl;
        std::cout << argv[0] << " modelfname" << std::endl;
        exit(0);
    }

    std::cout << "Loading model..." << std::endl;
    dlib::kcentroid<realty_kernel_t> model_k_centroids;
    dlib::kkmeans<realty_kernel_t> model(model_k_centroids);
    dlib::deserialize(filename + ".model") >> model;

    std::cout << "Loading data..." << std::endl;
    std::vector<realty_entry_full_t> realty_entries_full;
    dlib::deserialize(filename + ".data") >> realty_entries_full;
    std::cout << "Done loading model and data" << std::endl;

    std::unordered_map<int, std::vector<realty_entry_full_t>> class2realty_entries;
    for (const auto& realty_entry_full: realty_entries_full)
        class2realty_entries[model(convert_realty_entry_full(realty_entry_full))].emplace_back(realty_entry_full);


    realty_entry_t current_entry;
    std::string buf;
    while (std::cin >> buf) {
        parse_entry_from_string(buf, current_entry);
        auto& suitable_entries = class2realty_entries[model(current_entry)];
        std::cout << "Total entries in same cluster: " << suitable_entries.size() << std::endl;
        std::sort(suitable_entries.begin(), suitable_entries.end(), [current_entry](auto lhs, auto rhs) {
            return calculate_haversine_distance(lhs, current_entry) < calculate_haversine_distance(rhs, current_entry);
        });
        for (const auto& suitable_entry:suitable_entries)
            std::cout << suitable_entry << "\n";
    }
}
