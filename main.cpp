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
    using polygon =  bg::model::polygon<point>;

	std::string test_num = "01";
	std::string test_name = std::string("../tests/") + test_num + ".txt";
	std::string res_name = std::string("../") + test_num + ".txt";
	std::freopen(test_name.data(), "r", stdin);
    std::freopen(res_name.data(), "w", stdout);

	int n;
	double c1;
	double c2;
	std::cin >> n >> c1 >> c2;

	polygon poly;
	for (int i = 0; i < n; ++i) {
		double x, y;
		std::cin >> x >> y;
		poly.outer().push_back(point(x, y));
	}

    for (point p : result) {
        std::cout << bg::wkt<point>(p) << std::endl;
    }

    return 0;
}
