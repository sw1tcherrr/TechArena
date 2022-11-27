#include <iostream>
#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <simdjson.h>

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

int main() {
    using point = bg::model::point<double, 2, bg::cs::cartesian>;
	using box = bg::model::box<point>;
    using polygon =  bg::model::polygon<point, false, false>; // ccw, open polygon



    std::vector<point> result;
    rtree.query(bgi::intersects<polygon>(poly), std::back_inserter(result));

    for (point p : result) {
        std::cout << bg::wkt<point>(p) << std::endl;
    }

    return 0;
}
