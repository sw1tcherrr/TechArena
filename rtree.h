#pragma once
#include <variant>
#include <array>
#include "box.h"

template <class Point, int M>
struct rtree {
private:
    struct node;
    node sentinel{};
};

template <class Point, int M>
struct rtree<Point, M>::node {
private:
    box<Point> rect{};
    std::variant<std::array<node*, M>, std::array<Point, M>> content{};
};
