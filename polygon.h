#pragma once
#include <vector>
#include <limits>
#include "point.h"
#include "util.h"
#include "box.h"

template <class Point>
using ring_t = std::vector<Point>;

template <class Point>
struct polygon {
    using ring_type = ring_t<Point>;

    polygon() = delete;
    polygon(polygon const& other) = delete;
    polygon& operator=(polygon const&) = delete;
    // implicit move, move=

    polygon(ring_type&& border, std::vector<ring_type>&& holes)
    : border(std::move(border)), holes(std::move(holes)) {}

    box<Point> bounding_box() {
        using T = typename box<Point>::coord_type;
        using lim = typename std::numeric_limits<T>;
        T x_min = lim::max();
        T x_max = lim::min(); // TODO: is correct for float?
        T y_min = lim::max();
        T y_max = lim::min();
        for (auto&& p : border) { // gashnya-TODO: check boost
            x_min = std::min(x_min, p.x); // TODO: is correct for float?
            x_max = std::max(x_max, p.x);
            y_min = std::min(y_min, p.y);
            y_max = std::max(y_max, p.y);
        }
        return {x_min, x_max, y_min, y_max};
    }

    friend std::ostream& operator<<(std::ostream& os, polygon const& poly) {
        os << "{ ";
        util::print(os, poly.border, "BORDER [", "] ", ", ");
        for (auto&& h : poly.holes) {
           util::print(os, h, "HOLE [", "] ", ", ");
        }
        os << "}";
        return os;
    }

private:
    ring_type const border;
    std::vector<ring_type> const holes;
};
