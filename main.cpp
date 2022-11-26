#include <iostream>
#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/polygon.hpp>

#include <boost/geometry/index/rtree.hpp>

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

int main() {
    typedef bg::model::point<double, 2, bg::cs::cartesian> point;
    typedef bg::model::box<point> box;
    typedef bg::model::polygon<point, false, false> polygon; // ccw, open polygon

    std::freopen("input.txt", "r", stdin);
    std::freopen("output.txt", "w", stdout);

    bgi::rtree< point, bgi::rstar<16, 4> > rtree;

    int n_points;
    std::cin >> n_points;

    // for vizualization
    // boost::geometry::model::multi_point<boost::geometry::model::point<double, 2, boost::geometry::cs::cartesian>> points;

    for (int i = 0; i < n_points; ++i) {
        double x, y;
        std::cin >> x >> y;

        rtree.insert(point(x, y));
        // points.push_back(point(x, y));
    }

    // boost::geometry::model::polygon<point, false, false> poly;
    bg::model::polygon<point, false, false> poly; // ccw, open polygon

    poly.outer().push_back(point(30000, 40000));
    poly.outer().push_back(point(-20000, 60000));
    poly.outer().push_back(point(-50000, 30000));
    poly.outer().push_back(point(-30000, -20000));
    poly.outer().push_back(point(-10000, 20000));
    poly.outer().push_back(point(20000, 10000));

    std::vector<point> result;
    rtree.query(bgi::intersects<polygon>(poly), std::back_inserter(result));

    // for vizualization
    // boost::geometry::model::multi_point<boost::geometry::model::point<double, 2, boost::geometry::cs::cartesian>> result_points;

    for (point p : result) {
        std::cout << bg::wkt<point>(p) << std::endl;

        // result_points.push_back(p);
    }

    return 0;
}
