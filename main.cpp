#include <iostream>
#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <simdjson.h>

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;
namespace js = simdjson;
namespace jso = simdjson::ondemand;

using point = bg::model::point<double, 2, bg::cs::cartesian>;

point parse_point(jso::array_iterator&& it) {
    double x, y;
    (*it).get(x); ++it;
    (*it).get(y); ++it;
    return {x, y};
}

int main() {
    using box = bg::model::box<point>;
    using polygon =  bg::model::polygon<point, false, false>; // ccw, open polygon

    jso::parser parser;
    auto json = js::padded_string::load("../test.json");
    jso::document doc = parser.iterate(json);

    bgi::rtree<point, bgi::rstar<16, 4>> rtree;
    auto j_points = doc["points"].get_array();
    for (auto&& jp : j_points) {
        rtree.insert(parse_point(jp.begin()));
    }

    polygon poly;
    auto j_poly = doc["polygon"].get_object();
    auto j_border = j_poly["border"];
    for (auto&& jp : j_border) {
        poly.outer().push_back(parse_point(jp.begin()));
    }
    auto j_holes = j_poly["holes"];
    for (auto&& jh : j_holes) {
        poly.inners().emplace_back();
        for (auto&& jp : jh) {
            poly.inners().back().push_back(parse_point(jp.begin()));
        }
    }

    std::vector<point> result;
    rtree.query(bgi::intersects<polygon>(poly), std::back_inserter(result));

    for (point p : result) {
        std::cout << bg::wkt<point>(p) << std::endl;
    }

    return 0;
}
