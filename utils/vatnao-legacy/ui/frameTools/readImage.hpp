#pragma once

#include <QtGui/QApplication>
#include <QColor>
#include <QImage>
#include <QLineEdit>
#include <QPainter>
#include "../../app/vatnaoFrameInfo.hpp"
#include "../../app/generateFrameInfo.hpp"

#include "../../../robot/perception/vision/colour/GreenYUVClassifier.hpp"
#include "../../../robot/perception/vision/detector/BallDetector.hpp"
#include "../../../robot/perception/vision/VisionDefinitions.hpp"
#include "../../../robot/perception/vision/other/YUV.hpp"

//TODO: get this to link to offnao library
//#include "utils/CPlaneColours.hpp"

const QColor CPLANE_COLOURS[] = {
   //Corresponds to enums in robot/perception/vision/VisionDefinitions.hpp
   "green",
   "white",
   "black",
   "#c09", // Background pink purple
   "#cccccc",    // Body
   "orange"
};

enum SaliencyType {
	CANDIDATEPOINTS = 1,
	OTSU = 2,
	COLOURSALIENCY = 3,
  ALL = 4,
  YHIST = 5
};

QRgb getRGB(uint8_t const* frame, int col, int row, int num_cols);
void frameToQImage(VatnaoFrameInfo frameInfo, bool top, QImage *image);
QImage regionToQImage(VatnaoFrameInfo &frameInfo, FrameRect &r, QImage *image);
QImage ballROIToSaliencyQImage(VatnaoFrameInfo &frameInfo, BallDetectorVisionBundle &bdvb, QImage *image, SaliencyType t, 
        bool downSampled, GreenYUVClassifier* colour_cal, QLineEdit *YHist);
QImage regionToSaliencyQImage(VatnaoFrameInfo &frameInfo, FrameRect &r, BallDetectorVisionBundle &bdvb, QImage *image, SaliencyType t, 
	bool downSampled, GreenYUVClassifier* colour_cal, QLineEdit *YHist);
