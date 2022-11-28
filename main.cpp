#include <iostream>
#include <boost/geometry/geometry.hpp>

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

using point = bg::model::d2::point_xy<double>;
using box = bg::model::box<point>;
using polygon =  bg::model::polygon<point, true, false>;
using linestring = bg::model::linestring<point>;
using segment = bg::model::segment<point>;

void print_box(box const &b) {
	std::cout << std::fixed << std::setprecision(9)
			  << b.min_corner().x() << " " << b.min_corner().y() << " "
			  << b.max_corner().x() << " " << b.max_corner().y() << "\n";
}

double score(std::vector<box> const& inside, std::vector<box> const& crossing, double c1, double c2) {
	double res = 0;
	for (auto&& b : inside) {
		res += 1 + bg::area(b) * c1;
	}
	for (auto&& b : crossing) {
		res += 1 + bg::area(b) * c2;
	}
	return res;
}

std::vector<box> equal_boxes(box const& bbox, int w_cnt, int h_cnt) {
	double bbox_w = bbox.max_corner().x() - bbox.min_corner().x();
	double bbox_h = bbox.max_corner().y() - bbox.min_corner().y();
	double piece_w = bbox_w / w_cnt;
	double piece_h = bbox_h / h_cnt;

	point min_corner = bbox.min_corner();
	point max_corner(min_corner.x() + piece_w, min_corner.y() + piece_h);
	point hor_shift(piece_w, 0);
	point vert_shift(0, piece_h);
	double min_x = bbox.min_corner().x();

	std::vector<box> boxes;
	for (int i = 0; i < h_cnt; ++i) {
		for (int j = 0; j < w_cnt; ++j) {
			boxes.emplace_back(min_corner, max_corner);

			bg::add_point(min_corner, hor_shift);
			bg::add_point(max_corner, hor_shift);
		}
		min_corner.x(min_x);
		bg::add_point(min_corner, vert_shift);
		max_corner.x(min_x + piece_w);
		bg::add_point(max_corner, vert_shift);
	}

	return boxes;
}

//std::vector<box> vertex_boxes() {
//
//}

struct inside_predicate {
	polygon const& poly;
	linestring const& border;

	template <class Value>
	bool operator()(Value const& v) const {
		return bg::intersects(v, poly) && !bg::intersects(v, border);
	}
};

std::pair<std::vector<box>, std::vector<box>> filter_boxes(polygon const& poly, linestring const& border, std::vector<box>&& boxes) {
	bgi::rtree<box, bgi::rstar<16, 4>> rtree(boxes);

	std::vector<box> inside;
	rtree.query(bgi::satisfies(inside_predicate{poly, border}), std::back_inserter(inside));

	std::vector<box> crossing;
	rtree.query(bgi::intersects(border), std::back_inserter(crossing));

	return {std::move(inside), std::move(crossing)};
}

std::vector<polygon> split_poly(polygon poly) {
	auto bbox = bg::return_envelope<box>(poly);
	point top, r, bot, l;
	int ti, ri, bi, li;
	for (int i = 0; i < poly.outer().size(); ++i) {
		auto p = poly.outer()[i];
		if (p.y() == bbox.max_corner().y()) {
			top = p;
			ti = i;
		}
		if (p.y() == bbox.min_corner().x()) {
			bot = p;
			bi = i;
		}
		if (p.x() == bbox.min_corner().x()) {
			l = p;
			li = i;
		}
		if (p.x() == bbox.max_corner().x()) {
			r = p;
			ri = i;
		}
	}
	polygon tr;
	for (int i = ti; i <= ri; ++i) {
		tr.outer().push_back(poly.outer()[i]);
	}
	polygon br;
	for (int i = ri; i <= bi; ++i) {
		br.outer().push_back(poly.outer()[i]);
	}
	polygon bl;
	for (int i = bi; i <= li; ++i) {
		bl.outer().push_back(poly.outer()[i]);
	}
	polygon tl;
	for (int i = li; i <= ti; ++i) {
		tl.outer().push_back(poly.outer()[i]);
	}

	double bbox_h = bbox.max_corner().y() - bbox.min_corner().y();
	double bbox_w = bbox.max_corner().x() - bbox.min_corner().x();

	point p(bbox.min_corner().get<0> ()+ bbox_w / 2, bbox.min_corner().y() + bbox_h / 2);
	tr.outer().push_back(p);
	br.outer().push_back(p);
	bl.outer().push_back(p);
	tl.outer().push_back(p);

	return {tr, br, bl, tl};
}

struct result {
	std::vector<box> inside;
	std::vector<box> crossing;
	double score{0};
};

struct brute_force_eq_result : result {
	int w_cnt{-1};
	int h_cnt{-1};
};

brute_force_eq_result brute_force_eq(polygon const& poly, double c1, double c2, int range = 100, int step = 1) {
	box bbox = bg::return_envelope<box>(poly);
	double bbox_score = 1 + bg::area(bbox) * c2;

	linestring border(poly.outer().begin(), poly.outer().end());
	border.push_back(border[0]);
	bg::correct(border);

	brute_force_eq_result res;
	res.score = bbox_score;

	for (int w_cnt = 1; w_cnt <= range; w_cnt += step) {
		for (int h_cnt = 1; h_cnt <= range; h_cnt += step) {
			auto [inside, crossing] = filter_boxes(poly, border, equal_boxes(bbox, w_cnt, h_cnt));

			double cur_score = score(inside, crossing, c1, c2);
			if (cur_score < res.score) {
				res.inside = std::move(inside);
				res.crossing = std::move(crossing);
				res.score = cur_score;
				res.w_cnt = w_cnt;
				res.h_cnt = h_cnt;
			}
		}
	}

	return res;
}

int main() {
	#define TEST_NUM "02"
	char const* test_name = "../tests/" TEST_NUM ".txt";
	char const* res_name = "../" TEST_NUM ".txt";
	#undef TEST_NUM

	if (!std::freopen(test_name, "r", stdin)) {return -1;}
    if (!std::freopen(res_name, "w", stdout)) {return -1;}

	int n;
	double c1, c2;
	std::cin >> n >> c1 >> c2;

	polygon poly;
	for (int i = 0; i < n; ++i) {
		double x, y;
		std::cin >> x >> y;
		poly.outer().emplace_back(x, y);
	}
	bg::correct(poly);

	auto res = brute_force_eq(poly, c1, c2);

	std::cout << res.inside.size() + res.crossing.size() << "\n";
	for (auto&& b : res.inside) {
		print_box(b);
	}
	for (auto&& b : res.crossing) {
		print_box(b);
	}
}
