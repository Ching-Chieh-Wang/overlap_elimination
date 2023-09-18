#ifndef _MY_POLYGON_H
#define _MY_POLYGON_H
#include "../pch.h"
#include "types.h"

//自建2D多邊形資料

class MyPolygon :protected Polygon_xy {
public:
    MyPolygon() {}
    MyPolygon(const Polygon_xy& poly) :Polygon_xy(poly) {};
    MyPolygon(int reserveCnt);
    MyPolygon(const Point_xys& points);
    bool isValid();
    inline Point_xy& at(const int idx) {
        return outer().at(idx);
    }
    inline Point_xy at(const int idx) const{
        return outer().at(idx);
    }
    inline const Point_xys& points() const { return outer(); }
    inline Point_xys& points() { return outer(); }
    inline int size() const {
        return outer().size();
    }
    inline int begin()const {
        return 0;
    }
    inline int end()const {
        return size();
    }
    inline void clear() {
        outer().clear();
    }
    inline bool empty() {
        return outer().empty();
    }
    inline void popBack() {
        outer().pop_back();
    }
    Lines getBoundary() const;
    MyPolygons buffer();
    template<class T>
    void insert(int position, const T& start, const T& end);
    void insert(int position, const Point_xy& point);
    void insertBack(const Point_xy& point);
    void erase(int start, int end);
    void print() const;
    Polygon_xy* cast();
    friend boost::geometry::traits::exterior_ring<MyPolygon>;
    friend boost::geometry::traits::interior_rings<MyPolygon>;
    friend boost::geometry::traits::exterior_ring<MyPolygonSmartPtr>;
    friend boost::geometry::traits::interior_rings<MyPolygonSmartPtr>;
};

namespace boost::geometry::traits {
    template <> struct tag<MyPolygon> { using type = polygon_tag; };
    template <> struct ring_mutable_type<MyPolygon> { using type = Ring&; };
    template <> struct ring_const_type<MyPolygon> { using type = const Ring&; };
    template <> struct interior_mutable_type<MyPolygon> { using type = Rings; };
    template <> struct interior_const_type<MyPolygon> { using type = const Rings; };

    template<> struct exterior_ring<MyPolygon> {
        static auto& get(MyPolygon& poly) { return poly.outer(); }
        static auto& get(const MyPolygon& poly) { return poly.outer(); }
    };

    template<> struct interior_rings<MyPolygon> {
        static auto& get(MyPolygon& poly) { return poly.inners(); }
        static auto& get(const MyPolygon& poly) { return poly.inners(); }
    };
} // namespace boost::geometry::traits
namespace boost::geometry::traits {
    template <> struct tag<MyPolygons> { using type = multi_polygon_tag; };
} // namespace boost::geometry::traits


class MyPolygonSmartPtr : public std::unique_ptr<MyPolygon> {
public:
    MyPolygonSmartPtr():std::unique_ptr<MyPolygon>(std::make_unique<MyPolygon>()) {}
    MyPolygonSmartPtr(int reserveCnt) :std::unique_ptr<MyPolygon>(std::make_unique<MyPolygon>(reserveCnt)) {}
    MyPolygonSmartPtr(const MyPolygon& poly) :std::unique_ptr<MyPolygon>(std::make_unique<MyPolygon>(poly)) {}
    MyPolygonSmartPtr(const Polygon_xy& poly) :std::unique_ptr<MyPolygon>(std::make_unique<MyPolygon>(poly)) {}
    MyPolygonSmartPtr(MyPolygonSmartPtr&& poly) noexcept :std::unique_ptr<MyPolygon>(std::move(poly)) {}
    MyPolygonSmartPtr(const MyPolygonSmartPtr& poly) :std::unique_ptr<MyPolygon>(std::make_unique<MyPolygon>(*poly)) {}
};

namespace boost::geometry::traits {
    template <> struct tag<MyPolygonSmartPtr> { using type = polygon_tag; };
    template <> struct ring_mutable_type<MyPolygonSmartPtr> { using type = Ring&; };
    template <> struct ring_const_type<MyPolygonSmartPtr> { using type = const Ring&; };
    template <> struct interior_mutable_type<MyPolygonSmartPtr> { using type = Rings; };
    template <> struct interior_const_type<MyPolygonSmartPtr> { using type = const Rings; };

    template<> struct exterior_ring<MyPolygonSmartPtr> {
        static auto& get(MyPolygonSmartPtr& poly) { return poly->outer(); }
        static auto& get(const MyPolygonSmartPtr& poly) { return poly->outer(); }
    };

    template<> struct interior_rings<MyPolygonSmartPtr> {
        static auto& get(MyPolygonSmartPtr& poly) { return poly->inners(); }
        static auto& get(const MyPolygonSmartPtr& poly) { return poly->inners(); }
    };
} // namespace boost::geometry::traits
namespace boost::geometry::traits {
    template <> struct tag<MyPolygonSmartPtrs> { using type = multi_polygon_tag; };
} // namespace boost::geometry::traits


template<class T>
void MyPolygon::insert(int position, const T& start, const T& end) {
    outer().insert(outer().begin() + position, start, end);
}

#endif