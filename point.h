#pragma once
#include <utility>
#include <iosfwd>

template <class T>
struct point {
    using coord_type = T;
    T const x{0};
    T const y{0};

    point() = default;
    // implicit copy, move, copy=, move=

    point(T x, T y) : x(x), y(y) {}

    friend std::ostream& operator<<(std::ostream& os, point const& p) {
        return os << "(" << p.x << ", " << p.y << ")";
    }
};
