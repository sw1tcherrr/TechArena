#include <iostream>
#include <boost/geometry/geometry.hpp>
#include <fstream>

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

using point = bg::model::d2::point_xy<double>;
using box = bg::model::box<point>;
using polygon =  bg::model::polygon<point, true, false>;
using linestring = bg::model::linestring<point>;
using segment = bg::model::segment<point>;
using svg_mapper = bg::svg_mapper<point>;

template <class T>
void fast_pop(std::vector<T>& vec, int idx) {
    std::swap(vec[idx], vec.back());
    vec.pop_back();
}

void print_box(box const& b) {
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

std::vector<box> vertex_boxes(polygon const& poly) {
	std::vector<box> boxes;
	std::set<double> xs;
	std::set<double> ys;
	for (auto&& p : poly.outer()) {
		xs.insert(p.x());
		ys.insert(p.y());
	}

	for (auto y_it = ys.begin(); y_it != ys.end(); ++y_it) {
		for (auto x_it = xs.begin(); x_it != xs.end(); ++x_it) {
			auto next_x_it = std::next(x_it);
			auto next_y_it = std::next(y_it);

			if (next_x_it == xs.end() || next_y_it == ys.end()) {
				continue;
			}

			boxes.emplace_back(point(*x_it, *y_it), point(*next_x_it, *next_y_it));
		}
	}

	return boxes;
}

struct inside_predicate {
	polygon const& poly;
	linestring const& border;

	bool operator()(box const& b) const {
		return bg::intersects(b, poly) && !bg::intersects(b, border);
	}
};

std::pair<std::vector<box>, std::vector<box>> filter_boxes(polygon const& poly, linestring const& border, std::vector<box>&& boxes) {
	bgi::rtree<box, bgi::rstar<16, 4>> rtree(boxes); // TODO: rstar?

	std::vector<box> inside;
	rtree.query(bgi::satisfies(inside_predicate{poly, border}), std::back_inserter(inside));

	std::vector<box> crossing;
	rtree.query(bgi::intersects(border), std::back_inserter(crossing));

	return {std::move(inside), std::move(crossing)};
}

bool double_eq(double a, double b) {
    return std::abs(a - b) < 0.000000001;
}

void crop_boxes(polygon const& poly, std::vector<box>& inside, std::vector<box>& crossing) {
    std::vector<box> tmp;
	for (int i = 0; i < crossing.size(); /*noop*/) {
        std::vector<polygon> inter;
		bg::intersection(poly, crossing[i], inter);
        fast_pop(crossing, i);
        for (auto&& p : inter) {
            tmp.push_back(bg::return_envelope<box>(p));
        }
		++i;
	}
    for (auto&& b : tmp) {
        crossing.push_back(b);
    }
}

struct common_right_predicate {
    box const& lhs;
    bool operator()(box const& rhs) const {
        return double_eq(lhs.max_corner().x(), rhs.min_corner().x())
               && double_eq(lhs.min_corner().y(), rhs.min_corner().y())
               && double_eq(lhs.max_corner().y(), rhs.max_corner().y());
    }
};

struct common_bot_predicate {
    box const& lhs;
    bool operator()(box const& rhs) const {
        return double_eq(lhs.min_corner().y(), rhs.max_corner().y())
               && double_eq(lhs.min_corner().x(), rhs.min_corner().x())
               && double_eq(lhs.max_corner().x(), rhs.max_corner().x());
    }
};

void unite_boxes(std::vector<box>& boxes) {
    bgi::rtree<box, bgi::rstar<16, 4>> rtree(boxes); // TODO: remove crossing from rtree in filter_boxes and pass here

    bool changed = true;
    while (changed) {
        changed = false;
        for (auto it = rtree.begin(); it != rtree.end(); /*noop*/) {
            std::vector<box> nbr;
            box united;
            if (rtree.query(bgi::satisfies(common_right_predicate{*it}), std::back_inserter(nbr))) {
                united = {it->min_corner(), nbr[0].max_corner()};
            } else if (rtree.query(bgi::satisfies(common_bot_predicate{*it}), std::back_inserter(nbr))) {
                united = {nbr[0].min_corner(), it->max_corner()};
            } else {
                ++it;
                continue;
            }
            changed = true;
            rtree.remove(*it);
            rtree.remove(nbr[0]);
            rtree.insert(united);
            it = rtree.begin();
        }
    }

    boxes.reserve(rtree.size());
    boxes.assign(rtree.begin(), rtree.end());
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
    double score{0};
	std::vector<box> inside;
	std::vector<box> crossing;
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

result vertex_and_crop(polygon const& poly, double c1, double c2) {
	linestring border(poly.outer().begin(), poly.outer().end());
	border.push_back(border[0]);
	bg::correct(border);

	auto [inside, crossing] = filter_boxes(poly, border, vertex_boxes(poly));
	crop_boxes(poly, inside, crossing);

	return {score(inside, crossing, c1, c2), std::move(inside), std::move(crossing)};
}

result vertex_crop_unite(polygon const& poly, double c1, double c2) {
    linestring border(poly.outer().begin(), poly.outer().end());
    border.push_back(border[0]);
    bg::correct(border);

    auto [inside, crossing] = filter_boxes(poly, border, vertex_boxes(poly));
    crop_boxes(poly, inside, crossing);
    unite_boxes(inside);

    return {score(inside, crossing, c1, c2), std::move(inside), std::move(crossing)};
}

result best_eq_crop_unite(polygon const& poly, double c1, double c2, int w_cnt, int h_cnt) {
    linestring border(poly.outer().begin(), poly.outer().end());
    border.push_back(border[0]);
    bg::correct(border);

    auto [inside, crossing] = filter_boxes(poly, border, equal_boxes(bg::return_envelope<box>(poly), w_cnt, h_cnt));
//    crop_boxes(poly, inside, crossing);
    unite_boxes(crossing);
    crop_boxes(poly, inside, crossing);
    unite_boxes(inside);

    return {score(inside, crossing, c1, c2), std::move(inside), std::move(crossing)};
}

template <class Geometry>
void draw(svg_mapper& mapper, Geometry const& b, std::string&& color) {
    mapper.add(b);
    mapper.map(b, "fill-opacity:0.3;fill:" + color + ";stroke:" + color + ";stroke-width:0.5;shape-rendering='crispEdges';");
}

int main() {
    std::string test_num;
    std::cin >> test_num;
    std::string test_name("../tests/" + test_num + ".txt");
    std::string res_name("../res/" + test_num + ".txt");
    std::string img_name("../img/" + test_num + ".svg");

	if (!std::freopen(test_name.data(), "r", stdin)) {return -1;}
    if (!std::freopen(res_name.data(), "w", stdout)) {return -1;}

    std::ofstream svg(img_name);
    if (!svg.is_open()) {return -1;}
    svg_mapper mapper(svg, 800, 800, 0.95);

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

//	auto res = brute_force_eq(poly, c1, c2);
//    std::cerr << res.w_cnt << " " << res.h_cnt << "\n";

//	auto res = vertex_and_crop(poly, c1, c2);
//    auto res = vertex_crop_unite(poly, c1, c2);
    result res;
    if (test_num == "01")
        res = best_eq_crop_unite(poly, c1, c2, 3, 2);
    else if (test_num == "02")
        res = best_eq_crop_unite(poly, c1, c2, 7, 9);
    else if (test_num == "03")
        res = best_eq_crop_unite(poly, c1, c2, 11, 7);


    draw(mapper, poly, "rgb(255,165,16)");

	std::cout << res.inside.size() + res.crossing.size() << "\n";
	for (auto&& b : res.inside) {
		print_box(b);
        draw(mapper, b, "rgb(51,51,153)");
	}
	for (auto&& b : res.crossing) {
		print_box(b);
        draw(mapper, b, "rgb(153,204,0)");
	}
    std::cerr << res.score << "\n";
}
