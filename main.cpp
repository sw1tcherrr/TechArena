#include <iostream>
#include <boost/geometry/geometry.hpp>
#include "geo.h"

using namespace boost::geometry;

int main() {
    point<int> p(1, 2);
    point<int> q(3, 4);
    point<int> r(2, 1);
    std::cout << p << " " << q << " " << r << "\n";
    std::cout << euclid_sq_metric{}(p, q) << "\n";

    polygon<point<int>> poly = {{p, q, r}, {}};
    std::cout << poly << "\n";

    auto b1 = poly.bounding_box();
    box<point<int>> b2(2, 4, 2, 5);
    std::cout << b1 << " " << b2 << " " << intersect(b1, b2).value() << "\n";

    model::d2::point_xy<int> bp(1, 2), bq(3, 4), br(2, 1);
//    std::cout << bp << " " << bq << " " << br << "\n";
    model::polygon<model::d2::point_xy<int>> bpoly = {{bp, bq, br}};
}
