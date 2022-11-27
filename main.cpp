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

	boost::geometry::model::polygon<boost::geometry::model::point<double, 2, boost::geometry::cs::cartesian>> poly;
    boost::geometry::model::linestring<boost::geometry::model::point<double, 2, boost::geometry::cs::cartesian>> border;
	for (int i = 0; i < n; ++i) {
		double x, y;
		std::cin >> x >> y;
		poly.outer().push_back(point(x, y));
        border.push_back(point(x, y));
	}
    border.push_back(border[0]);

    box bbox = bg::return_envelope<box>(poly);
    double bbox_val = 1 + bg::area(bbox) * c2;

    double bbox_h = bbox.max_corner().get<1>() - bbox.min_corner().get<1>();
    double bbox_w = bbox.max_corner().get<0>() - bbox.min_corner().get<0>();

    double min_sum = bbox_val;
	std::vector<box> min_vec;

    for (int ww = 1; ww <= bbox_w; ++ww) {
        for (int hh = 1; hh <= bbox_h; ++hh) {
            bgi::rtree<box, bgi::rstar<16, 4>> rtree;

            double w_piece = bbox_w / ww;
            double h_piece = bbox_h / hh;
            double area_piece = w_piece * h_piece;

            point min_corner = bbox.min_corner();
            point max_corner = min_corner;
            bg::add_point(max_corner, point(w_piece, h_piece));

            double min_x = min_corner.get<0>();

            std::vector<boost::geometry::model::box<boost::geometry::model::point<double, 2, boost::geometry::cs::cartesian>>> boxes;
            for (int i = 0; i < hh; ++i) {
                for (int j = 0; j < ww; ++j) {
                    box b(min_corner, max_corner);
                    rtree.insert(b);
                    boxes.push_back(b);

                    bg::add_point(min_corner, point(w_piece, 0));
                    bg::add_point(max_corner, point(w_piece, 0));
                }

                min_corner.set<0>(min_x);
                max_corner.set<0>(min_x + w_piece);

                bg::add_point(min_corner, point(0, h_piece));
                bg::add_point(max_corner, point(0, h_piece));
            }

            std::vector<boost::geometry::model::box<boost::geometry::model::point<double, 2, boost::geometry::cs::cartesian>>> result_i;
            size_t intersects_cnt = rtree.query(bgi::intersects<polygon>(poly), std::back_inserter(result_i));

            std::vector<boost::geometry::model::box<boost::geometry::model::point<double, 2, boost::geometry::cs::cartesian>>> result_ii;
            size_t intersects_b_cnt = rtree.query(bgi::intersects<linestring>(border), std::back_inserter(result_ii));

			double cur = (intersects_cnt - intersects_b_cnt) * (1 + area_piece * c1) + intersects_b_cnt * (1 + area_piece * c2);
            if (cur < min_sum) {
				min_sum = cur;
				min_vec = std::move(result_i);
			}
        }
    }

    std::cout << min_sum << " " << bbox_val << std::endl;

	std::cout << min_vec.size() << "\n";
	for (auto&& b : min_vec) {
		std::cout << std::setprecision(9) << b.min_corner().get<0>() << " " << b.min_corner().get<1>() << " " << b.max_corner().get<0>() << " " << b.max_corner().get<1>() << "\n";
	}

    return 0;
}
