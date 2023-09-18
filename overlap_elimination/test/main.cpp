#include "types.h"
#include "my_polygon.h"
#include "overlap_elimination.h"

int main() {
    MyPolygonSmartPtrs polys{
        MyPolygon(Point_xys{ {0, 2}, { 0,4 }, { 6,4 }, { 6,2 },{0, 2} }),
        MyPolygon(Point_xys{ {4,0}, { 4,6}, { 10,6}, { 10,0},{4,0} }),
        MyPolygon(Point_xys{ {2,3}, { 2,8}, { 12,8}, { 12,3},{2,3} })
    };
    OverlapElimination overlapEliminator;
    overlapEliminator.showPolys(polys);
    auto result=overlapEliminator.run(polys);  
    overlapEliminator.showResults(result);
}