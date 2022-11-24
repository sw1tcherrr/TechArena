#include <iostream>
#include "geo.h"

int main() {
    point<int> p(1, 2);
    point<int> q(3, 4);
    point<int> r(2, 1);
    std::cout << p << " " << q << "\n";
    std::cout << euclid_sq_metric{}(p, q) << "\n";

    polygon<point<int>> poly = {{p, q, r}, {}};
    std::cout << poly << "\n";

    auto b1 = poly.bounding_box();
    box<point<int>> b2(2, 4, 2, 5);
    std::cout << b1 << " " << b2 << " " << intersect(b1, b2).value() << "\n";
}
