#ifndef _BTREE_H
#define _BTREE_H

template<class T>
class RTree {
public:
    boost::geometry::index::rtree<std::pair<Point_xy, T>, boost::geometry::index::quadratic<16>> rTree;
    T find(const Point_xy& point) const;
    std::vector<T> finds(const Point_xy& point) const;
    RTree() {}
    RTree(const Point_xys& points);
    RTree(const MyPolygon& poly);
    RTree(MyPolygonSmartPtrs& polys);
    void insertPoints(const Point_xys& points);
    void insertPoint(const Point_xy& point);
    void insertPoint(const Point_xy& point, const T& t);
    void insertPoints(const MyPolygon& poly);
    void insertPoly(MyPolygon& poly);
    void insertPoly(MyPolygonSmartPtr & poly);
    void erasePoint(const Point_xy& point, const T& t);
    void erasePoint(const PointAt& point);


protected:
    virtual T noFound()const;
};

template<class T>
T RTree<T>::find(const Point_xy& point) const {
    std::vector<std::pair<Point_xy, T>> results;
    rTree.query(boost::geometry::index::satisfies([&](std::pair<Point_xy, T> const& value) {return boost::geometry::distance(value.first, point) < 0.000001; }),
        std::back_inserter(results));
    if (results.empty()) {
        return noFound();
    }
    assert(results.size() == 1);
    return results.front().second;
}
template<class T>
std::vector<T> RTree<T>::finds(const Point_xy& point) const {
    std::vector<std::pair<Point_xy, T>> results;
    rTree.query(boost::geometry::index::satisfies([&](std::pair<Point_xy, T> const& value) {return boost::geometry::distance(value.first, point) < 0.000001; }),
        std::back_inserter(results));
    if (results.empty()) {
        return std::vector<T>();
    }
    std::vector<T> returns;
    returns.reserve(results.size());
    for (const auto& [point, t] : results) {
        returns.emplace_back(t);
    }
    return returns;
}

template<class T>
void RTree<T>::erasePoint(const Point_xy& point, const T& t) {
    rTree.remove(std::make_pair(point, t));
}

template<class T>
void RTree<T>::insertPoint(const Point_xy& point, const T& t) {
    rTree.insert(std::make_pair(point, t));
}



#endif