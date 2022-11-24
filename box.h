#pragma once
#include <array>
#include <optional>

template <typename Point>
struct box {
private: using T = typename Point::coord_type;
public:
    using coord_type = T;

    box() = delete;
    box(T x_min, T x_max, T y_min, T y_max)
    : x_min(x_min), x_max(x_max), y_min(y_min), y_max(y_max) {}
    // implicit copy, move, copy=, move=

    friend std::optional<box> intersect(box const& a, box const& b) {
        auto x_min = std::max(a.x_min, b.x_min);
        auto x_max = std::min(a.x_max, b.x_max);
        auto y_min = std::max(a.y_min, b.y_min);
        auto y_max = std::min(a.y_max, b.y_max);

        if (x_min >= x_max || y_min >= y_max) {
            return std::nullopt;
        }
        return std::optional<box>(std::in_place, x_min, x_max, y_min, y_max);
    }

    friend std::ostream& operator<<(std::ostream& os, box const& b) {
        return os << "([" << b.x_min << ", " << b.x_max << "]"
                  << " x "
                  << "[" << b.y_min << ", " << b.y_max << "])";
    }
private:
    T x_min;
    T x_max;
    T y_min;
    T y_max;
};
