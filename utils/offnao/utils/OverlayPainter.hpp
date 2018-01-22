#pragma once

#include <QPainter>

#include "perception/vision/VisionDefinitions.hpp"
#include "perception/vision/Region.hpp"

#include "types/BBox.hpp"
#include "types/PostInfo.hpp"
#include "types/Point.hpp"

class BallInfo;
class GoalInfo;
class RobotInfo;
class FieldBoundaryInfo;
class FieldFeatureInfo;
class FootInfo;
class Ipoint;

class ImageRegion;
class RobotRegion;

namespace VisionTest
{
   class Feature;

   class Shape;
   class Circle;
   class Ellipse;

   class Intersection;
}

class OverlayPainter : private QPainter
{
   public:

      using QPainter::begin;
      using QPainter::end;
      using QPainter::scale;
      using QPainter::translate;
      using QPainter::setTransform;
      using QPainter::drawImage;

      using QPainter::setPen;

      void setPen(const QPen & pen);

      OverlayPainter();
      OverlayPainter(QPaintDevice * device);
      virtual ~OverlayPainter();

      void drawHorizon(std::pair<int, int> horizon);

      void drawRobotBox(const BBox &region, const QBrush &colour);
      // Draws a region's bounding box on the image.
      void drawRegionOverlay (const RegionI &region);
      void drawRegionBox(const RegionI &region, const QBrush &colour);

      void drawFootOverlay(const FootInfo &foot);
      void drawBallOverlay(const BallInfo &ball);
      void drawPostOverlay(const PostInfo &post);
      void drawRobotOverlay(const RobotInfo &robot);
      void drawFieldBoundaryOverlay(const FieldBoundaryInfo &fieldBoundary);
      void drawFieldFeatureOverlay(const FieldFeatureInfo &fieldFeature);
      void drawFieldFeatureOverlay(const FieldFeatureInfo &fieldFeature,
                                   QColor colour);
      void drawLandmarkOverlay(const Ipoint &landmark, std::pair<int, int> horizon);

      void drawImageRegion(ImageRegion *region, QColor colour);
      void drawRobotRegion(RobotRegion *region, QColor colour);
      void drawPoint(Point p, QColor colour);
      void drawRectangle(Point a, Point b);
      void drawFeature(const VisionTest::Feature &feature, bool isSelected);
      void drawIntersection(const VisionTest::Intersection &intersection);

      void drawLinePath(const QPainterPath &path, QColor colour);
      void drawPolygon(QPolygon poly, QBrush fill);
   private:
      void drawShape(const VisionTest::Shape &shape);
      void drawShapeGrabs(const VisionTest::Shape &shape);
      void drawCircleShape(const VisionTest::Circle &circle);
      void drawEllipseShape(const VisionTest::Ellipse &ellipse);
      void drawPolyShape(const VisionTest::Shape &shape);

      /* Mapping between features and colours */
      typedef std::pair<feature_type, QColor> colour_pair;
      static const colour_pair colour_arr[fNUM_FEATURES];
      static const std::vector<colour_pair> colour_vector;
      static const std::map<feature_type, QColor> colour_map;

};
