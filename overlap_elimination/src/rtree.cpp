#include "../pch.h"
#include "types.h"
#include "rtree.h"
#include "my_polygon.h"



template<>
int RTree<int>::noFound() const {
    return -1;
}
template<>
PointAt RTree<PointAt>::noFound() const {
    return std::make_pair(nullptr, -1);
}


template<>
RTree<PointAt>::RTree(MyPolygonSmartPtrs& polys) {
    for (auto& poly : polys) {
        insertPoly(poly);
    }
}
template<>
RTree<int>::RTree(const Point_xys& points) {
    for (int i = 0; i < points.size(); i++) {
        rTree.insert(std::make_pair(points[i], i));
    }
}


template<>
RTree<int>::RTree(const MyPolygon& poly) {
    for (int i = 0; i < poly.size() - 1; i++) {
        rTree.insert(std::make_pair(poly.at(i), i));
    }
}

template<>
void RTree<int>::insertPoints(const Point_xys& points) {
    int currectSize = rTree.size();
    for (int i = currectSize; i < currectSize + points.size(); i++) {
        rTree.insert(std::make_pair(points[i], i));
    }
}



template<>
void RTree<int>::insertPoint(const Point_xy& point) {
    int currectSize = rTree.size();
    rTree.insert(std::make_pair(point, currectSize));
}
template<>
void RTree<int>::insertPoints(const MyPolygon& poly) {
    int currentSize = rTree.size();
    for (int i = 0; i < poly.size() - 1; i++) {
        rTree.insert(std::make_pair(poly.at(i), currentSize + i));
    }
}
template<>
void RTree<PointAt>::insertPoly(MyPolygonSmartPtr& poly) {
    insertPoly(*poly);
}

template<>
void RTree<PointAt>::insertPoly(MyPolygon &poly) {
    for (int i = 0; i < poly.size() - 1; i++) {
        rTree.insert(std::make_pair(poly.at(i), std::make_pair(&poly, i)));
    }
}



template<>
void RTree<PointAt>::erasePoint(const PointAt& pointAt) {
    rTree.remove(make_pair(pointAt.first->at(pointAt.second), pointAt));
}


