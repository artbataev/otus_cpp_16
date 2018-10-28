#pragma once

#include <string>
#include <vector>
#include <type_traits>
#include <cmath>
#include <math.h>
#include <set>
#include <dlib/clustering.h>

typedef dlib::matrix<double, 7, 1> realty_entry_t;
typedef dlib::matrix<double, 8, 1> realty_entry_full_t;
typedef dlib::linear_kernel<realty_entry_t> realty_kernel_t;

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

// parsing realty entry from string - for train and eval mode
template<typename RE>
typename std::enable_if<
        std::is_same<RE, realty_entry_t>::value || std::is_same<RE, realty_entry_full_t>::value,
        void
>::type
parse_entry_from_string(const std::string& raw_data, RE& entry) {
    int num_params;
    if constexpr (std::is_same<RE, realty_entry_full_t>::value) {
        num_params = 8;
    } else {
        num_params = 7;
    }
    auto raw_data_splitted = split_string(raw_data);
    if (raw_data_splitted.size() != num_params)
        throw std::invalid_argument("unrecongized data: " + raw_data);

    // replace unknown values with -1
    for (int i = 0; i < num_params; i++)
        entry(i) = raw_data_splitted[i].empty() ? -1.0 : std::stod(raw_data_splitted[i]);
}

// output realty entry (full version)
std::ostream& operator<<(std::ostream& stream, const realty_entry_full_t& entry) {
    std::cout << std::setprecision(10);
    for (int i = 0; i < 8; i++) {
        const std::set<int> int_indices{2, 6, 7};
        if (i != 0)
            stream << ";";
        if (entry(i) == -1.0)
            stream << "<unknown>";
        else {
            if (int_indices.count(i) > 0)
                std::cout << int(entry(i));
            else
                std::cout << entry(i);
        }
    }
    return stream;
}

// instead of using floor and total floors, use flag (first/last) floor
realty_entry_t convert_realty_entry_full(const realty_entry_full_t& realty_entry_full) {
    realty_entry_t result;
    for (int i = 0; i < 6; i++)
        result(i) = realty_entry_full(i);
    if (realty_entry_full(6) == -1.0 || realty_entry_full(7) == -1.0)
        result(6) = -1.0;
    else
        result(6) = (realty_entry_full(6) == 0.0 || realty_entry_full(6) == realty_entry_full(7)) ? 0.0 : 1.0;
    return result;
}

// getting distance between points
template<typename RE1, typename RE2>
typename std::enable_if<
        (std::is_same<RE1, realty_entry_t>::value || std::is_same<RE1, realty_entry_full_t>::value) &&
        (std::is_same<RE2, realty_entry_t>::value || std::is_same<RE2, realty_entry_full_t>::value),
        double
>::type
calculate_haversine_distance(const RE1& re1, const RE2& re2) {
    constexpr double degrees2radians_coeff = M_PI / 180.0;
    constexpr double earth_radius_km = 6367;
    double dlong = (re1(0) - re2(0)) * degrees2radians_coeff;
    double dlat = (re1(1) - re2(1)) * degrees2radians_coeff;
    double a = std::pow(std::sin(dlat / 2.0), 2) +
               std::cos(re1(1) * degrees2radians_coeff) * std::cos(re2(1) * degrees2radians_coeff) *
               std::pow(std::sin(dlong / 2.0), 2);
    double result = 2 * atan2(std::sqrt(a), std::sqrt(1 - a)) * earth_radius_km;

    return result;
}
