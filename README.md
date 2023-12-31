# Overlap Elimination
Remove overlapped regions in polygon sets, while ensuring that the intersection boundaries are minimized. This program is especially suitable for image moisaicing (GIS, SFM).. 

![overlap_elimination](https://github.com/Ching-Chieh-Wang/overlap_elimination/assets/81002444/ab82b94a-f762-4e82-8a52-3749fbbad266)

# Description
1. Polygon assignment: Assinging polygons. As shown in Figure 1(a), assume the user assigns two ranges _F_<sub>1</sub> and _F_<sub>2</sub>, each forms one polygon _R_<sub>1</sub> and _R_<sub>2</sub> respectively.
2. Polygon overlap detection: Using 2-D polygon boolen operations to detect all overlapped polygon pairs. Calculate the intersection polygon, difference polygons and the crossing points for each capture range pairs. As shown in Figure 1(b), the polygons _R_<sub>1</sub> and _R_<sub>2</sub> intersect to each other and form an intersection polygon ( _R_<sub>1</sub> ∩ _R_<sub>2</sub> ) and two difference polygons ( _R_<sub>1</sub> - _R_<sub>2</sub>, _R_<sub>2</sub> - _R_<sub>1</sub> ). The _R_<sub>1</sub> and _R_<sub>2</sub> crossing points ( _P_<sub>1</sub>, _P_<sub>2</sub>, _P_<sub>3</sub>, _P_<sub>4</sub> ) are estimated.
3. Shortest path trimming: For each intersection polygon, provide it with a graph, where vertices are corners of intersection polygon, edges are connected to any vertices if the line between the two vertices is entirely within the intersection polygon. Using Dijkstra algorithm to find the shortest path of each adjacent crossing points pairs sequentially. Finally, adjusting the overlap region according to the shortest path. As shown in Figure 1(c), the shortest paths between _P_<sub>1</sub> to _P_<sub>2</sub>, _P_<sub>2</sub> to _P_<sub>3</sub>, _P_<sub>3</sub> to _P_<sub>4</sub>, and _P_<sub>4</sub> to _P_<sub>1</sub> inside _R_<sub>1</sub> ∩ _R_<sub>2</sub> are _S_<sub>1,2</sub>, _S_<sub>2,3</sub>, _S_<sub>3,4</sub>, and _S_<sub>4,1</sub>, respectively. The overlap region _R_<sub>1</sub> ∩ _R_<sub>2</sub> and the differencies _R_<sub>1</sub> - _R_<sub>2</sub> and _R_<sub>2</sub> - _R_<sub>1</sub> are trimmed and modified to _R_<sub>1</sub> ∩ _R_<sub>2</sub>' , _R_<sub>1</sub> - _R_<sub>2</sub>' , and _R_<sub>2</sub> - _R_<sub>1</sub>' according to these paths (In the figure, the upper lines are correspondent to the mark "'").
4. Polygon Assignment: Assigning modified difference polygons to the corresponding range. Then, assign modified overlap polygons to the range that results in the minimum border distances. As shown in Figure 1(c), the overlap eliminated ranges _F_<sub>1</sub> and _F_<sub>2</sub> are assigned as _R_<sub>1</sub> - _R_<sub>2</sub>' and _R_<sub>2</sub> - _R_<sub>1</sub>' respectively. Now, if the modified overlap polygon _R_<sub>1</sub> ∩ _R_<sub>2</sub>' is assigned to range _F_<sub>1</sub>, the generated border distances between _F_<sub>1</sub> and _F_<sub>2</sub> would be |_S_<sub>1,2</sub>|+|_S_<sub>3,4</sub>|. While, if the modified overlap polygon _R_<sub>1</sub> ∩ _R_<sub>2</sub>' is assigned to range _F_<sub>2</sub>, the generated border distances would be |_S_<sub>2,3</sub>| + |_S_<sub>4,1</sub>|. Since |_S_<sub>2,3</sub>| + |_S_<sub>4,1</sub>| < |_S_<sub>1,3</sub>| + |_S_<sub>4,1</sub>|, the polygolns of overlap eliminated range _F_<sub>2</sub> is _R_<sub>2</sub>' , which is the combination of _R_<sub>1</sub> ∩ _R_<sub>2</sub>' and _R_<sub>2</sub> - _R_<sub>1</sub>', while the polygons of overlap eliminated range _F_<sub>1</sub> is _R_<sub>1</sub>', which is _R_<sub>1</sub> - _R_<sub>2</sub>', as shown in Figure 1(d).

![overlap elimination process](https://github.com/Ching-Chieh-Wang/overlap_elimination/assets/81002444/4f5effb8-5524-4f67-b02c-7b406cbdfa14)

# Library Package Installation
1. [VTK](https://vtk.org/download/) : For polygon visualization
2. [boost](https://www.boost.org/users/download/) : For polygon boolen operation

# How to use
1. Include header files
2. Using MyPolygonSmartPtrs to declare polygons ( Clock-wise points, the first point must be identical to the last point, no self-intersection, no inner ring)
3. Declare OverlapElimination
4. (Optional) Use OverlapElimination::showPolys() function to visualize original polygons
5. Use OverlapElimination::showPolys() function to perform overlap elimination algorithm and get result. The result is an unordered_map type, key: the index of the original polygon, value: the corresponding overlap eliminated polygons
6. (Optional) Use OverlapElimination::showResults() function to visualize overlap eliminated result. 
```cpp
//1
#include "types.h"
#include "my_polygon.h"
#include "overlap_elimination.h"

int main() {
    MyPolygonSmartPtrs polys{
        MyPolygon(Point_xys{ {0, 2}, { 0,4 }, { 6,4 }, { 6,2 },{0, 2} }),
        MyPolygon(Point_xys{ {4,0}, { 4,6}, { 10,6}, { 10,0},{4,0} }),
        MyPolygon(Point_xys{ {2,3}, { 2,8}, { 12,8}, { 12,3},{2,3} })
    }; //2
    OverlapElimination overlapEliminator; //3
    overlapEliminator.showPolys(polys); //4
    std::unordered_map<int, MyPolygonSmartPtrs> result=overlapEliminator.run(polys);  //5
    overlapEliminator.showResults(result); //6
}
```
![OE_demo](https://github.com/Ching-Chieh-Wang/overlap_elimination/assets/81002444/ec2ca987-902c-445c-a44d-586d0b5c8b21)


