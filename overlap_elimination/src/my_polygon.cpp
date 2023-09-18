#include "../pch.h"
#include "types.h"
#include "my_polygon.h"

MyPolygon::MyPolygon(int reserveCnt) {
    outer().reserve(reserveCnt);
}
MyPolygon::MyPolygon(const Point_xys& points) {
    outer().assign(points.begin(), points.end());
}

void MyPolygon::print()const {
    for (const auto& point : outer()) {
        std::cout << point.x() << "," << point.y() << std::endl;
    }
    std::cout << std::endl;
}

//Line MyPolygon::getBoundary() const{
//    Line boundary;
//    for (const auto& point : outer()) {
//        boundary.emplace_back(point);
//    }
//    return boundary;
//
//}

Lines MyPolygon::getBoundary() const {
    Lines boundary;
    for (auto it = outer().begin(); it != outer().end(); it++) {
        auto nextIt = it == prev(outer().end()) ? outer().begin() : next(it);
        Line line;
        line.push_back(*it);
        line.push_back(*nextIt);
        boundary.push_back(line);
    }
    return boundary;

}

MyPolygons MyPolygon::buffer(){
    MyPolygons buffered;
    const double buffer_distance = 1.0;
    const int points_per_circle = 5;
    boost::geometry::strategy::buffer::distance_symmetric<double> distance_strategy(buffer_distance);
    boost::geometry::strategy::buffer::join_round join_strategy(points_per_circle);
    boost::geometry::strategy::buffer::end_round end_strategy(points_per_circle);
    boost::geometry::strategy::buffer::point_circle circle_strategy(points_per_circle);
    boost::geometry::strategy::buffer::side_straight side_strategy;
    boost::geometry::buffer(*this, buffered, distance_strategy, side_strategy,
        join_strategy, end_strategy, circle_strategy);
    return buffered;
}

void MyPolygon::insert(int position, const Point_xy& point) {
    outer().insert(outer().begin(), point);
}
void MyPolygon::erase(int start, int end) {
    outer().erase(outer().begin() + start, outer().begin() + end);
}
bool MyPolygon::isValid() {
    return boost::geometry::area(*this) > 1;
}

void MyPolygon::insertBack(const Point_xy& point) {
    outer().emplace_back(point);
}

Polygon_xy* MyPolygon::cast() {
    return dynamic_cast<Polygon_xy*>(this);
}

