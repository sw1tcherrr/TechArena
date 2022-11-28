#include <iostream>
#include <boost/geometry/geometry.hpp>

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

int main() {
    using point = bg::model::point<double, 2, bg::cs::cartesian>;
	using box = bg::model::box<point>;
    using polygon =  bg::model::polygon<point>;
    using linestring = bg::model::linestring<point>;

	std::string test_num = "01";
	std::string test_name = std::string("../tests/") + test_num + ".txt";
	std::string res_name = std::string("../") + test_num + ".txt";
	std::freopen(test_name.data(), "r", stdin);
    std::freopen(res_name.data(), "w", stdout);

	int n;
	double c1;
	double c2;
	std::cin >> n >> c1 >> c2;

    std::set<double> x_points;
    std::set<double> y_points;

	boost::geometry::model::polygon<boost::geometry::model::point<double, 2, boost::geometry::cs::cartesian>> poly;
    boost::geometry::model::linestring<boost::geometry::model::point<double, 2, boost::geometry::cs::cartesian>> border;
	for (int i = 0; i < n; ++i) {
		double x, y;
		std::cin >> x >> y;

		poly.outer().push_back(point(x, y));
        border.push_back(point(x, y));

        x_points.insert(x);
        y_points.insert(y);
	}
    border.push_back(border[0]);

    bg::correct(poly);
    bg::correct(border);

    box bbox = bg::return_envelope<box>(poly);
    double bbox_val = 1 + bg::area(bbox) * c2;

    double bbox_h = bbox.max_corner().get<1>() - bbox.min_corner().get<1>();
    double bbox_w = bbox.max_corner().get<0>() - bbox.min_corner().get<0>();

    bgi::rtree<box, bgi::rstar<16, 4>> rtree;

    std::set<double>::iterator x_it;
    std::set<double>::iterator y_it;

    std::vector<boost::geometry::model::box<boost::geometry::model::point<double, 2, boost::geometry::cs::cartesian>>> boxes;

    for (y_it = y_points.begin(); y_it != y_points.end(); ++y_it) {
        for (x_it = x_points.begin(); x_it != x_points.end(); ++x_it) {
            std::set<double>::iterator next_x_it = std::next(x_it);
            std::set<double>::iterator next_y_it = std::next(y_it);

            if (next_x_it == x_points.end() || next_y_it== y_points.end()) {
                continue;
            }

            box b(point(*x_it, *y_it), point(*next_x_it, *next_y_it));
            rtree.insert(b);
            boxes.push_back(b);
        }
    }

    std::vector<boost::geometry::model::box<boost::geometry::model::point<double, 2, boost::geometry::cs::cartesian>>> result_i;
    size_t intersects_cnt = rtree.query(bgi::intersects<polygon>(poly), std::back_inserter(result_i));

    std::vector<boost::geometry::model::box<boost::geometry::model::point<double, 2, boost::geometry::cs::cartesian>>> result_ii;
    size_t intersects_b_cnt = rtree.query(bgi::intersects<linestring>(border), std::back_inserter(result_ii));

    auto res_it = std::begin(result_ii);
    while(res_it != std::end(result_ii)) {
        box b = *res_it;

        std::vector<polygon> p;
        bg::intersection(poly, b, p);

        if (p.size() == 0) {
            result_ii.erase(res_it);
            continue;
        }

        if (p.size() > 0) {
            box bbox_p = bg::return_envelope<box>(p[0]);

            double p_area = bg::area(p[0]);
            double bbp_area = bg::area(bbox_p);

            if (std::abs(p_area - bbp_area) < 0.000000001) {
                result_ii.erase(res_it);
                continue;
            }

            *res_it = std::move(bbox_p);
        }

        ++res_it;
    }

	// std::cout << min_vec.size() << "\n";
	// for (auto&& b : min_vec) {
	// 	std::cout << std::fixed << std::setprecision(9) << b.min_corner().get<0>() << " " << b.min_corner().get<1>() << " " << b.max_corner().get<0>() << " " << b.max_corner().get<1>() << "\n";
	// }

    return 0;
}
