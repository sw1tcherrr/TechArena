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

// TODO: insert + range-query
template <class Point, int M>
struct rtree<Point, M>::node {
private:
    box<Point> rect{};
    std::variant<std::vector<node*>, std::vector<Point>> content{};
};
