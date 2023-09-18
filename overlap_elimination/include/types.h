#ifndef _TYPES_H
#define _TYPES_H
#include "../pch.h"

class Frame;
class KeyFrame;
class ReconstructFrame;
class Marker;
class KeyMarker;
class SamplingMarker;
class MyPolygon;
class MyPolygonSmartPtr;

typedef int MarkerId;
typedef int FrameNumber;
typedef int VertexId;
typedef std::unordered_map<FrameNumber, KeyFrame> KeyFrames;
typedef std::unordered_map<FrameNumber, KeyFrame* > KeyFramesPtr;
typedef std::unordered_map<MarkerId, KeyMarker> KeyMarkers;
typedef std::unordered_map<MarkerId, KeyMarker* > KeyMarkersPtr;
typedef std::unordered_map<MarkerId, std::shared_ptr<Marker>> Markers;
typedef std::unordered_map<MarkerId, Marker*> MarkersPtr;
typedef std::unordered_map<MarkerId, SamplingMarker> SamplingMarkers;
typedef std::unordered_map<MarkerId, SamplingMarker*> SamplingMarkersPtr;
typedef std::unordered_map<FrameNumber, std::shared_ptr<Frame>>Frames;
typedef std::unordered_map<FrameNumber, Frame*>FramesPtr;
typedef std::unordered_map<FrameNumber, ReconstructFrame> ReconstructFrames;


typedef boost::geometry::model::d2::point_xy<double> Point_xy;
typedef std::vector<Point_xy> Point_xys;
typedef boost::geometry::model::polygon<Point_xy> Polygon_xy;
typedef boost::geometry::model::linestring<Point_xy> Line;
typedef boost::geometry::model::multi_linestring<Line>Lines;
typedef boost::geometry::model::multi_polygon<Polygon_xy> Polygon_xys;
typedef boost::geometry::model::box<Point_xy> Box;
typedef boost::geometry::model::ring<Point_xy> Ring;
typedef std::vector<Ring> Rings;
typedef std::pair<MyPolygon*, int> PointAt;
typedef std::vector<PointAt> PointAts;
typedef std::vector<MyPolygonSmartPtr> MyPolygonSmartPtrs;
typedef std::vector<MyPolygon> MyPolygons;

#endif