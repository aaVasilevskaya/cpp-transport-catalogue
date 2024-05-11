#pragma once

#include <string>
#include <cmath>

// namespace Catalogue {
namespace geo {

struct Coordinates {
    double lat;
    double lng;
    bool operator==(const Coordinates& other) const {
        return lat == other.lat && lng == other.lng;
    }
    bool operator!=(const Coordinates& other) const {
        return !(*this == other);
    }
};

struct Distance{
    unsigned int dist;
    std::string name_location;
};

inline double ComputeDistance(Coordinates from, Coordinates to){
    using namespace std;
    if (from == to) {
        return 0;
    }
    static const double dr = 3.1415926535 / 180.;
    static const int earth_radius = 6371000;
    return acos(sin(from.lat * dr) * sin(to.lat * dr)
                + cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr))
        * earth_radius;
}

}  // namespace geo
// } // namespace Catalogue