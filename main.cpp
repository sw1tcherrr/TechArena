#include <iostream>
#include <boost/geometry/geometry.hpp>

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

    box bbox = bg::return_envelope<box>(poly);
    double bbox_val = 1 + bg::area(bbox) * c1;

    bgi::rtree<polygon, bgi::rstar<16, 4>> rtree;

    double bbox_h = bbox.max_corner().get<1>() - bbox.max_corner().get<1>();
    double bbox_w = bbox.max_corner().get<0>() - bbox.max_corner().get<0>();

    int w_piece_cnt = 10;
    int h_piece_cnt = 20;

    double w_piece = bbox_w / w_piece_cnt;
    double h_piece = bbox_h / h_piece_cnt;
    double area_piece = w_piece * h_piece;

    point min_corner = bbox.min_corner();
    point max_corner = min_corner;
    bg::add_point(max_corner, point(w_piece, h_piece));

    for (int i = 0; i < h_piece_cnt; ++i) {
        for (int j = 0; j < w_piece_cnt; ++j) {
            point corner2 = min_corner;
            bg::add_point(corner2, point(0, h_piece));
            point corner4 = min_corner;
            bg::add_point(corner4, point(w_piece, 0));
            polygon p = {{min_corner, corner2, max_corner, corner4}};
            rtree.insert(p);

            bg::add_point(min_corner, point(w_piece, 0));
            bg::add_point(max_corner, point(w_piece, 0));
        }

        bg::add_point(min_corner, point(0, h_piece));
        bg::add_point(max_corner, point(0, h_piece));
    }

    std::vector<polygon> result_i;
    size_t intersects_cnt = rtree.query(bgi::intersects<polygon>(poly), std::back_inserter(result_i));

    std::vector<polygon> result_w;
    size_t within_cnt = rtree.query(bgi::within<polygon>(poly), std::back_inserter(result_w));

    double sum = within_cnt * (1 + area_piece * c1) + (intersects_cnt - within_cnt) * (1 + area_piece * c2);

    std::cout << sum << " " << bbox_val << std::endl;

    return 0;
}
