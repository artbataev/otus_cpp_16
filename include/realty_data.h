#pragma once

#include <string>
#include <vector>
#include <dlib/clustering.h>

typedef dlib::matrix<double, 7, 1> realty_entry_t;
typedef dlib::radial_basis_kernel<realty_entry_t> realty_kernel_t;

// split input string - for point reading
std::vector<std::string> split_string(const std::string& str, const char by = ';') {
    std::vector<std::string> result;

    std::string::size_type start = 0;
    std::string::size_type stop = str.find_first_of(by);
    while (stop != std::string::npos) {
        result.push_back(str.substr(start, stop - start));
        start = stop + 1;
        stop = str.find_first_of(';', start);
    }

    result.push_back(str.substr(start));
    return result;
}

class RealtyDataEntry {
public:
    RealtyDataEntry(const std::string& raw_data, bool is_eval) {
        auto raw_data_splitted = split_string(raw_data);

        if (is_eval && raw_data_splitted.size() != num_params)
            throw std::invalid_argument("unrecongized data: " + raw_data);
        if (!is_eval && raw_data_splitted.size() != num_params + 1)
            throw std::invalid_argument("unrecongized data: " + raw_data);

        for (int i = 0; i < num_params; i++) {
            if (is_eval || i < num_params - 1) {
                params(i) = raw_data_splitted[i].empty() ? -1.0 : std::stod(raw_data_splitted[i]);
            } else {
                double floor = raw_data_splitted[num_params - 1].empty() ? -1.0 :
                               std::stod(raw_data_splitted[num_params - 1]);
                double last_floor = raw_data_splitted[num_params].empty() ? -1.0 :
                                    std::stod(raw_data_splitted[num_params]);
                if (floor != -1.0)
                    params(i) = (floor == 0.0 || floor == last_floor) ? 0.0 : 1.0;
                else
                    params(i) = -1.0;
            }
        }
    }

    const realty_entry_t& get_vector() {
        return params;
    }

private:
    realty_entry_t params; // all params for model should be double
    constexpr static int num_params = 7;
};
