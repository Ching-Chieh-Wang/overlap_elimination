#ifndef _OVERLAP_TACKLE_H
#define _OVERLAP_TACKLE_H

#include "../pch.h"
#include "types.h"
#include "graph.h"
#include "rtree.h"

//���|�����A�ô�֤����ؽd��ҳy�������
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
    //�o��newPoly�PoldPoly����"�I"
    void getCrossingPoints(const MyPolygon& oldPoly, const MyPolygon& newPoly, MyPolygonSmartPtrs& intersects, OverlapRegions& overlapRegions);
    //�i�歫�|����
    void pairOverlapElimination(MyPolygon& oldPoly, MyPolygon& newPoly, MyPolygonSmartPtrs& intersectPolys, MyPolygonSmartPtrs& modOldPolys, MyPolygonSmartPtrs& modNewPolys);
    //�o��
    void getDiffCrossPairs(MyPolygonSmartPtrs& oldPolyDiffs, MyPolygonSmartPtrs& newPolyDiffs, OverlapRegions& overlapRegions);
    //�Ycrop intersect����Aintersect���M�����n�A�Nintersect��Ѩ䤤�@��poly
    void assignIntersectToDiff(MyPolygon& intersectPoly, OverlapRegion& overlapRegion, MyPolygonSmartPtrs& oldPolyDiffs, MyPolygonSmartPtrs& newPolyDiffs);
    //��ı�ƭ��|�������G
    void showPolySets(const std::vector<std::vector<const MyPolygon*>>& polysSets)const;
    //�Y��intersectPoly�A�ñN�Y��d�򵹤�diffPolys
    void cropBoundaries(MyPolygon& intersectPoly, OverlapRegion& overlapRegion, std::unordered_set<MyPolygon*>& oldPolySets, MyPolygonSmartPtrs& modOldPolys, MyPolygonSmartPtrs& modNewPolys);
    //�޶i�s���h��ΨûP�{�w�s�b���h��ζi�歫�|����
    void addPoly(const FrameNumber& frameNumber, MyPolygonSmartPtr& newPoly, std::unordered_map<MyPolygon*, MyPolygonSmartPtr>& overlapEliminatedPolys, std::unordered_map<const MyPolygon*, int>& polyFrameNumberTable, boost::geometry::index::rtree<std::pair<Box, MyPolygon*>, boost::geometry::index::quadratic<16>>& rTree, std::unordered_set<MyPolygon*> visitedPolys = std::unordered_set<MyPolygon*>());
    //�O�_�n�b�G��poly
    bool isValidPoly(const MyPolygon& poly);
public:
    //���歫�|����
    std::unordered_map<int, MyPolygonSmartPtrs> run(MyPolygonSmartPtrs& polys);
    //��ı��polys
    void showPolys(const MyPolygonSmartPtrs& polys) const;
    //�i�ܭ��|���������G
    void showResults(const std::unordered_map<int, MyPolygonSmartPtrs> &results) const;
};

#endif // !_OVERLAP_TACKLE_H
