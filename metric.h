#pragma once

#include "point.h"

struct euclid_sq_metric {
    template <class Point>
    typename Point::coord_type operator()(Point const& a, Point const& b) {
        auto dx = (a.x - b.x);
        auto dy = (a.y - b.y);
        return dx * dx + dy * dy;
    }
};
