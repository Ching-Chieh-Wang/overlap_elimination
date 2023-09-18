#include "../pch.h"
#include "overlap_elimination.h"
#include "my_polygon.h"





OverlapElimination::OverlapRegion::OverlapRegion(const MyPolygonSmartPtr& intersectPoly) {
    originalIntersectPolyRTree = RTree<int>(*intersectPoly);
}


std::unordered_map<int, MyPolygonSmartPtrs> OverlapElimination::run(MyPolygonSmartPtrs& polys) {
    //重疊消除後修正之3D重建範圍
    std::unordered_map<MyPolygon*, MyPolygonSmartPtr> overlapEliminatedPolys;
    //於overlapEliminatedPolys，與3D重建範圍與frameNumber對照表
    std::unordered_map<const MyPolygon*, int> polyFrameNumberTable;
    //3D空間資料結構
    boost::geometry::index::rtree<std::pair<Box, MyPolygon*>, boost::geometry::index::quadratic<16>> rtree;

    //已拜訪過，不必檢查重疊消除之多邊形
    std::unordered_set<MyPolygon* > visitedPolys;

    //逐一將每個reconstructFrame的重建範圍丟入系統，系統將自動將新來的poly與現存的poly進行重疊消除
    for (int i = 0; i < polys.size();i++) {
        addPoly(i, polys[i], overlapEliminatedPolys, polyFrameNumberTable, rtree, visitedPolys);
    }
    std::unordered_map<int, MyPolygonSmartPtrs> results;
    for (const auto& [polyPtr, poly] : overlapEliminatedPolys) {
        results[polyFrameNumberTable.at(polyPtr)].emplace_back(std::move(overlapEliminatedPolys.at(polyPtr)));
    }
    return results;
}




WeightedGraph<OverlapElimination::Distance> OverlapElimination::createGraph(const MyPolygon& poly) {
    WeightedGraph<Distance> overlapGraph;
    for (int i = 0; i < poly.size() - 2; i++) {
        for (int j = i + 1; j < poly.size() - 1; j++) {
            Line connection{ poly.at(i), poly.at(j) };
            if (boost::geometry::covered_by(connection, poly))
                overlapGraph.addEdge(i, j, Distance(boost::geometry::length(connection)));
        }
    }
    return overlapGraph;
}


void OverlapElimination::addPoly(const FrameNumber& frameNumber, MyPolygonSmartPtr& newPoly, std::unordered_map<MyPolygon*, MyPolygonSmartPtr>& overlapEliminatedPolys, std::unordered_map<const MyPolygon*, int>& polyFrameNumberTable, boost::geometry::index::rtree<std::pair<Box, MyPolygon*>, boost::geometry::index::quadratic<16>>& rTree, std::unordered_set<MyPolygon*> visitedPolys) {

    ///
    ///首先進行粗略重疊候選人評估
    ///


    Box box = boost::geometry::return_envelope<Box>(*newPoly);
    //舊poly重疊候選人
    std::vector < std::pair<Box, MyPolygon*> >intersectBoxes;
    rTree.query(boost::geometry::index::intersects(box), back_inserter(intersectBoxes));//粗略看新舊有無重疊
    for (const auto& [oldPolyBox, oldPoly] : intersectBoxes) { //對於每個舊poly粗略重疊候選人
        if (visitedPolys.find(oldPoly) != visitedPolys.end()) continue;
        //細看1們與2是否真有重疊
        MyPolygonSmartPtrs intersectPolys;
        boost::geometry::intersection(*oldPoly, *newPoly, intersectPolys);
        if (!intersectPolys.empty() && boost::geometry::area(intersectPolys) > 0.00001) {//若1們與2真的有重疊，而且重疊面積是可觀的
            if (boost::geometry::covered_by(*oldPoly, *newPoly)) {//若舊poly完全在新poly裡面
                //移除所有舊poly的紀錄
                overlapEliminatedPolys.erase(oldPoly);
                rTree.remove(std::make_pair(oldPolyBox, oldPoly));
            }
            else if (boost::geometry::covered_by(*newPoly, *oldPoly)) {//若新poly完全在舊poly裡面
                return;//放棄新的poly，不把他加入系統
            }
            else {//新舊poly有重疊發生但沒有一方是完全包在對方
                //舊poly的兒子們
                MyPolygonSmartPtrs modOldPolys;
                //新poly的兒子們
                MyPolygonSmartPtrs modNewPolys;
                pairOverlapElimination(*oldPoly, *newPoly, intersectPolys, modOldPolys, modNewPolys); //poly1與poly2互相傷害並生下小孩
                //處理舊poly的屍體，把舊poly逐出系統
                rTree.remove(std::make_pair(oldPolyBox, oldPoly));
                int oldPolyFrameNumber = polyFrameNumberTable.at(oldPoly);
                overlapEliminatedPolys.erase(oldPoly);
                for (auto& modOldPoly : modOldPolys) { //舊poly交棒給兒子
                    visitedPolys.emplace(modOldPoly.get());// 舊poly已經打過新poly了，所以新poly的兒子不必再與舊poly的血脈進行任何互相傷害
                    //註冊舊poly的兒子
                    Box modOldPolyBox = boost::geometry::return_envelope<Box>(*modOldPoly);
                    rTree.insert(std::make_pair(modOldPolyBox, modOldPoly.get()));
                    polyFrameNumberTable.emplace(modOldPoly.get(), oldPolyFrameNumber);
                    overlapEliminatedPolys.emplace(modOldPoly.get(), std::move(modOldPoly));
                }
                for (auto& modNewPoly : modNewPolys) {
                    visitedPolys.emplace(modNewPoly.get());//避免兄弟姊妹互打
                    addPoly(frameNumber, modNewPoly, overlapEliminatedPolys, polyFrameNumberTable, rTree, visitedPolys); //新poly交給新poly的兒子繼續與其他舊poly互相傷害
                }
                return; //重疊的部分放心交給兒子們來處理，父親(新poly)安息吧
            }
        }
    }
    //以下是新poly沒有與任何舊poly有任何重疊


    //直接將新poly完好的加入系統
    rTree.insert(std::make_pair(box, newPoly.get()));
    polyFrameNumberTable.emplace(newPoly.get(), frameNumber);
    overlapEliminatedPolys.emplace(newPoly.get(), std::move(newPoly));
}

void OverlapElimination::getCrossingPoints(const MyPolygon& oldPoly, const MyPolygon& newPoly, MyPolygonSmartPtrs& intersects, OverlapRegions& overlapRegions) {
    Point_xys crossingPoints;
    boost::geometry::intersection(oldPoly, newPoly, crossingPoints);
    std::unordered_map<MyPolygon*, std::map<int, Point_xy>> orderedPolysCrossingPointAts;
    RTree<PointAt> intersectsRTree(intersects);
    for (const auto& crossingPoint : crossingPoints) {
        PointAt pointAtIntersect = intersectsRTree.find(crossingPoint);
        assert(pointAtIntersect.first != nullptr);
        orderedPolysCrossingPointAts[pointAtIntersect.first].emplace(pointAtIntersect.second, crossingPoint);
    }
    for (const auto& [intersectPoly, orderedCrossingPoints] : orderedPolysCrossingPointAts) {
        for (const auto& [id, crossingPoint] : orderedCrossingPoints) {
            overlapRegions[intersectPoly].crossingPoints.emplace_back(crossingPoint);
        }
    }
}




void OverlapElimination::getDiffCrossPairs(MyPolygonSmartPtrs& oldPolyDiffs, MyPolygonSmartPtrs& newPolyDiffs, OverlapRegions& overlapRegions) {
    RTree<PointAt> diffsRTree;
    for (auto& poly : oldPolyDiffs) {
        diffsRTree.insertPoly(poly);
    }
    for (auto& poly : newPolyDiffs) {
        diffsRTree.insertPoly(poly);
    }

    for (auto& [intersectPoly, overlapRegion] : overlapRegions) {
        const RTree<int>& originalIntersectRTree = overlapRegion.originalIntersectPolyRTree;
        const Point_xys& crossingPoints = overlapRegion.crossingPoints;
        for (auto it = crossingPoints.begin(); it != crossingPoints.end(); it++) {
            const Point_xy& startPt = *it;
            const Point_xy& endPt = it != std::prev(crossingPoints.end()) ? *std::next(it) : crossingPoints.front();
            PointAts startPtAts = diffsRTree.finds(endPt);
            PointAts endPtAts = diffsRTree.finds(startPt);
            assert(startPtAts.size() == 2);
            assert(endPtAts.size() == 2);
            for (int i = 0; i < startPtAts.size(); i++) {
                for (int j = 0; j < endPtAts.size(); j++) {
                    if (startPtAts[i].first != endPtAts[j].first) continue;
                    PointAt startPtAt = startPtAts[i];
                    PointAt endPtAt = endPtAts[j];
                    MyPolygon* nowDiffPoly = startPtAt.first;
                    //濾掉順序不對者
                    if (endPtAt.second == 0) {
                        if (originalIntersectRTree.find(nowDiffPoly->at(nowDiffPoly->size() - 2)) == -1) {
                            continue;
                        }
                    }
                    else if (startPtAt.second == 0) {
                        if (originalIntersectRTree.find(nowDiffPoly->at(1)) == -1) {
                            continue;
                        }
                    }
                    else if (originalIntersectRTree.find(startPtAt.first->at(0)) == -1) {
                        if (endPtAt.second < startPtAt.second) {
                            continue;
                        }
                    }
                    else {
                        if (endPtAt.second < startPtAt.second) {
                            continue;
                        }
                    }
                    overlapRegion.diffsCrossPointsPairs.emplace_back(startPtAt, endPtAt);
                    i = INT_MAX;
                    break;
                }
            }
        }
    }
}


void OverlapElimination::cropBoundaries(MyPolygon& intersectPoly, OverlapRegion& overlapRegion, std::unordered_set<MyPolygon*>& oldPolySets, MyPolygonSmartPtrs& modOldPolys, MyPolygonSmartPtrs& modNewPolys) {
    for (auto& diffsCrossPointsPair : overlapRegion.diffsCrossPointsPairs) {
        const PointAt& pointAtStart = diffsCrossPointsPair.first;
        const PointAt& pointAtEnd = diffsCrossPointsPair.second;
        assert(pointAtStart.first == pointAtEnd.first);
        MyPolygon* diffPoly = pointAtStart.first;
        int diffStartPointId = pointAtStart.second;
        int diffEndPointId = pointAtEnd.second;
        Point_xy startPoint = pointAtStart.first->at(diffStartPointId);
        Point_xy endPoint = pointAtStart.first->at(diffEndPointId);

        WeightedGraph<Distance> polyGraph = createGraph(intersectPoly);
        RTree<int> intersectPolyRTree(intersectPoly);

        //找到intersectPoly的邊界線兩端
        int startId = intersectPolyRTree.find(endPoint);
        int endId = intersectPolyRTree.find(startPoint);
        assert(startId != -1);
        assert(endId != -1);

        //縮短邊界的路徑
        std::vector<int> shortestPathIdxs;
        double distance = polyGraph.shortestPath(startId, endId, shortestPathIdxs);
        if (oldPolySets.find(diffPoly) != oldPolySets.end()) {
            overlapRegion.oldPolyDistance += distance;
        }
        else overlapRegion.newPolyDistance += distance;
        Point_xys shortestPathPoints;
        for (int idx : shortestPathIdxs) {
            shortestPathPoints.emplace_back(intersectPoly.at(idx));
        }
        if (endId == 0) endId = intersectPoly.size() - 1;
        if (startId < endId) {
            intersectPoly.erase(startId + 1, endId);
            intersectPoly.insert(startId + 1, shortestPathPoints.begin() + 1, shortestPathPoints.end() - 1);
        }
        else {
            intersectPoly.erase(startId + 1, intersectPoly.end());
            intersectPoly.erase(intersectPoly.begin(), endId);
            intersectPoly.insert(intersectPoly.end(), shortestPathPoints.begin() + 1, shortestPathPoints.end() - 1);
            intersectPoly.insertBack(intersectPoly.at(0));
        }
#ifdef DEBUG_OVERLAP_TACKLE
        intersectPoly.print();
#endif
        if (diffStartPointId < diffEndPointId) {
            diffPoly->erase(diffStartPointId + 1, diffEndPointId);
            diffPoly->insert(diffStartPointId + 1, shortestPathPoints.rbegin() + 1, shortestPathPoints.rend() - 1);
        }
        else {
            diffPoly->erase(diffStartPointId + 1, diffPoly->end());
            diffPoly->erase(diffPoly->begin(), diffEndPointId);
            diffPoly->insert(diffPoly->end(), shortestPathPoints.rbegin() + 1, shortestPathPoints.rend() - 1);
            diffPoly->insertBack(diffPoly->at(0));
        }
#ifdef DEBUG_OVERLAP_TACKLE
        diffPoly->print();
#endif
    }
}


void OverlapElimination::assignIntersectToDiff(MyPolygon& intersectPoly, OverlapRegion& overlapRegion, MyPolygonSmartPtrs& modOldPolys, MyPolygonSmartPtrs& modNewPolys) {
    Polygon_xys correctedIntersectPolys;
    geometry::correct(*intersectPoly.cast(), correctedIntersectPolys, 1);
    if (correctedIntersectPolys.empty()) {
        intersectPoly.clear();
    }
    bool ismodOldPolysAdded = false;
    bool ismodNewPolysAdded = false;
    for (auto& correctedIntersectPoly : correctedIntersectPolys) {
        MyPolygonSmartPtr addingIntersect(correctedIntersectPoly);
        if (!isValidPoly(*addingIntersect)) continue;
        //分配poly1與poly2的重疊區域給Poly1還是poly2
        if (overlapRegion.oldPolyDistance < overlapRegion.newPolyDistance) { //重疊處若交由poly1所形成的邊界大於交由poly2所形成得邊界
            modNewPolys.emplace_back(std::move(addingIntersect));
            ismodNewPolysAdded = true;
        }
        else {//重疊處若交由poly2所形成的邊界大於交由poly1所形成得邊界
            modOldPolys.emplace_back(std::move(addingIntersect));
            ismodOldPolysAdded = true;
        }
    }
    if (ismodOldPolysAdded) {
        MyPolygonSmartPtrs mergedOldPolys;
        for (const auto& oldPolyDiff : modOldPolys) {
            if (oldPolyDiff->empty()) continue;
            if (mergedOldPolys.empty()) {
                mergedOldPolys.emplace_back(oldPolyDiff);
            }
            else {
                MyPolygonSmartPtrs mergedOldPolysTemps;
                boost::geometry::union_(mergedOldPolys, oldPolyDiff, mergedOldPolysTemps);
                mergedOldPolys = std::move(mergedOldPolysTemps);
            }
        }
        modOldPolys = std::move(mergedOldPolys);
    }
    if (ismodNewPolysAdded) {
        MyPolygonSmartPtrs mergedNewPolys;
        for (const auto& newPolyDiff : modNewPolys) {
            if (newPolyDiff->empty()) continue;
            if (mergedNewPolys.empty()) {
                mergedNewPolys.emplace_back(newPolyDiff);
            }
            else {
                MyPolygonSmartPtrs mergedNewPolysTemps;
                boost::geometry::union_(mergedNewPolys, newPolyDiff, mergedNewPolysTemps);
                mergedNewPolys = std::move(mergedNewPolysTemps);
            }
        }
        modNewPolys = std::move(mergedNewPolys);
    }

#ifdef DEBUG_OVERLAP_TACKLE
    std::vector < const MyPolygon* > showOldPolys;
    for (const auto& poly : modOldPolys) {
        showOldPolys.emplace_back(poly.get());
    }
    std::vector < const MyPolygon* > showNewPolys;
    for (const auto& poly : modNewPolys) {
        showNewPolys.emplace_back(poly.get());
    }
    try {
        showPolys(std::vector<std::vector < const MyPolygon* >> { showOldPolys, showNewPolys});
    }
    catch (...) {

    }
    cout << "----------------------" << endl;
#endif
}



void OverlapElimination::pairOverlapElimination(MyPolygon& oldPoly, MyPolygon& newPoly, MyPolygonSmartPtrs& intersectPolys, MyPolygonSmartPtrs& modOldPolys, MyPolygonSmartPtrs& modNewPolys) {
    modOldPolys.clear();
    modNewPolys.clear();
    std::unordered_map<MyPolygon*, OverlapRegion> overlapRegions;
    for (auto& intersectPoly : intersectPolys) {
        overlapRegions.emplace(intersectPoly.get(), intersectPoly);
    }
#ifdef DEBUG_OVERLAP_TACKLE
    showPolys(std::vector<std::vector < const MyPolygon* >> { {&oldPoly}, { &newPoly }});
    std::cout << "OldPoly<" << std::endl;
    oldPoly.print();
    std::cout << "NewPoly<" << std::endl;
    newPoly.print();
#endif
    boost::geometry::difference(oldPoly, newPoly, modOldPolys);
    boost::geometry::difference(newPoly, oldPoly, modNewPolys);
    getCrossingPoints(oldPoly, newPoly, intersectPolys, overlapRegions);
    std::unordered_set<MyPolygon*> oldPolysSets;
    for (const auto& oldPolyDiff : modOldPolys) {
        oldPolysSets.emplace(oldPolyDiff.get());
    }
    getDiffCrossPairs(modOldPolys, modNewPolys, overlapRegions);
    for (auto& [intersectPoly, overlapRegion] : overlapRegions) {
        cropBoundaries(*intersectPoly, overlapRegion, oldPolysSets, modOldPolys, modNewPolys);
        assignIntersectToDiff(*intersectPoly, overlapRegion, modOldPolys, modNewPolys);
    }
}

bool OverlapElimination::isValidPoly(const MyPolygon& poly) {
    return boost::geometry::area(poly) > 0.000001;
}

void OverlapElimination::showPolys(const MyPolygonSmartPtrs& polys) const{
    std::vector<std::vector<const MyPolygon*>> polySets;
    polySets.reserve(polys.size());
    for (const auto& poly : polys) {
        polySets.resize(polySets.size() + 1);
        polySets.back().emplace_back( poly.get() );
    }
    showPolySets(polySets);
}

void OverlapElimination::showPolySets(const std::vector<std::vector<const MyPolygon*>>& polysSets)const {
    // Create the VTK renderer and render window
    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
    vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    renderWindow->AddRenderer(renderer);

    // Create an interactor to handle user interactions
    vtkSmartPointer<vtkRenderWindowInteractor> interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    interactor->SetRenderWindow(renderWindow);
    for (const auto& polysSet : polysSets) {
        double red = vtkMath::Random(0.0, 1.0);
        double green = vtkMath::Random(0.0, 1.0);
        double blue = vtkMath::Random(0.0, 1.0);
        for (const auto& poly : polysSet) {
            // Create the points representing the vertices of the polygon
            vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();

            for (int i = poly->size() - 2; i >= 0; i--) {
                points->InsertNextPoint(poly->at(i).x(), poly->at(i).y(), 0);
            }

            vtkSmartPointer<vtkCellArray> polygons = vtkSmartPointer<vtkCellArray>::New();
            polygons->InsertNextCell(poly->size() - 1);
            for (int i = 0; i < poly->size() - 1; ++i)
            {
                polygons->InsertCellPoint(i);
            }

            // Create a polydata to store the points and cells
            vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();
            polydata->SetPoints(points);
            polydata->SetPolys(polygons);

            // Apply vtkTriangleFilter to convert to triangles
            vtkSmartPointer<vtkTriangleFilter> triangleFilter = vtkSmartPointer<vtkTriangleFilter>::New();
            triangleFilter->SetInputData(polydata);
            triangleFilter->Update();

            // Create a mapper
            vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
            mapper->SetInputConnection(triangleFilter->GetOutputPort());

            // Create an actor
            vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
            actor->SetMapper(mapper);
            actor->GetProperty()->SetColor(red, green, blue);
            renderer->AddActor(actor);
        }
    }
    // Set background color and start the rendering loop
    renderer->SetBackground(1.0, 1.0, 1.0); // White background
    renderWindow->Render();
    interactor->Start();
}

void OverlapElimination::showResults(const std::unordered_map<int, MyPolygonSmartPtrs>& results) const {
    std::vector<std::vector<const MyPolygon* >> polysSets;
    for (const auto& [frameNumber, polys] : results) {
        if (!polys.empty()) {
            std::vector<const MyPolygon* > polySet;
            for (const auto& poly : polys) {
                polySet.emplace_back(poly.get());
            }
            polysSets.emplace_back(polySet);
        }
    }
    showPolySets(polysSets);
}





