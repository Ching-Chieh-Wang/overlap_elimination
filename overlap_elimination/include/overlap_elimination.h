#ifndef _OVERLAP_TACKLE_H
#define _OVERLAP_TACKLE_H

#include "../pch.h"
#include "types.h"
#include "graph.h"
#include "rtree.h"

//重疊消除，並減少切重建範圍所造成的邊界
class OverlapElimination {
    class Distance {
    public:
        double distance = 0;
        inline double w() {
            return distance;
        }
    };
    class OverlapRegion {
    public:
        RTree<int> originalIntersectPolyRTree;
        Point_xys crossingPoints;
        std::vector<std::pair<PointAt, PointAt>> diffsCrossPointsPairs;
        double oldPolyDistance = 0;
        double newPolyDistance = 0;
        OverlapRegion(const MyPolygonSmartPtr& intersectPoly);
        OverlapRegion() {}
    };
    typedef std::unordered_map<MyPolygon*, OverlapRegion> OverlapRegions;
    WeightedGraph<Distance> createGraph(const MyPolygon& oldPoly);
    //得到newPoly與oldPoly的交"點"
    void getCrossingPoints(const MyPolygon& oldPoly, const MyPolygon& newPoly, MyPolygonSmartPtrs& intersects, OverlapRegions& overlapRegions);
    //進行重疊消除
    void pairOverlapElimination(MyPolygon& oldPoly, MyPolygon& newPoly, MyPolygonSmartPtrs& intersectPolys, MyPolygonSmartPtrs& modOldPolys, MyPolygonSmartPtrs& modNewPolys);
    //得到
    void getDiffCrossPairs(MyPolygonSmartPtrs& oldPolyDiffs, MyPolygonSmartPtrs& newPolyDiffs, OverlapRegions& overlapRegions);
    //若crop intersect之後，intersect仍然有面積，將intersect交由其中一個poly
    void assignIntersectToDiff(MyPolygon& intersectPoly, OverlapRegion& overlapRegion, MyPolygonSmartPtrs& oldPolyDiffs, MyPolygonSmartPtrs& newPolyDiffs);
    //視覺化重疊消除結果
    void showPolySets(const std::vector<std::vector<const MyPolygon*>>& polysSets)const;
    //縮減intersectPoly，並將縮減範圍給予diffPolys
    void cropBoundaries(MyPolygon& intersectPoly, OverlapRegion& overlapRegion, std::unordered_set<MyPolygon*>& oldPolySets, MyPolygonSmartPtrs& modOldPolys, MyPolygonSmartPtrs& modNewPolys);
    //引進新的多邊形並與現已存在之多邊形進行重疊消除
    void addPoly(const FrameNumber& frameNumber, MyPolygonSmartPtr& newPoly, std::unordered_map<MyPolygon*, MyPolygonSmartPtr>& overlapEliminatedPolys, std::unordered_map<const MyPolygon*, int>& polyFrameNumberTable, boost::geometry::index::rtree<std::pair<Box, MyPolygon*>, boost::geometry::index::quadratic<16>>& rTree, std::unordered_set<MyPolygon*> visitedPolys = std::unordered_set<MyPolygon*>());
    //是否要在乎此poly
    bool isValidPoly(const MyPolygon& poly);
public:
    //執行重疊消除
    std::unordered_map<int, MyPolygonSmartPtrs> run(MyPolygonSmartPtrs& polys);
    //視覺化polys
    void showPolys(const MyPolygonSmartPtrs& polys) const;
    //展示重疊消除的結果
    void showResults(const std::unordered_map<int, MyPolygonSmartPtrs> &results) const;
};

#endif // !_OVERLAP_TACKLE_H
