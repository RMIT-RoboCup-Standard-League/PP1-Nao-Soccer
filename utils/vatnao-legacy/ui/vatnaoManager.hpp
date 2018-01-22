#pragma once

#include <QObject>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QRadioButton>

#include "../app/appAdaptor.hpp"

#include "../../robot/perception/vision/detector/BallDetector.hpp"

#include "frameTools/readImage.hpp"
#include "frameTools/readBoundingBoxes.hpp"

class VatnaoManager:public QObject{
    Q_OBJECT
    public:
        VatnaoManager(AppAdaptor *a);
        ~VatnaoManager();

        QImage *topImage;
        QImage *botImage;

        QImage *regionRawImage;
        QImage *regionSaliencyImage;

        QRadioButton *radioCircleCandidate;
        QRadioButton *radioOtsuThreshold;
        QRadioButton *radioColourSaliency;
        QRadioButton *radioAll;
        QRadioButton *radioYHist;

        QLineEdit *YHistThresh;

        QRadioButton *radioNaive;
        QRadioButton *radioCircle;
        QRadioButton *radioBlack;
        QRadioButton *radioCombo;

        QCheckBox *checkBoxDownsampled;
        QCheckBox *checkBoxBallMetaData;

        QLabel *resultInfo;

    signals:
        void topImageChanged(QPixmap topImage);
        void botImageChanged(QPixmap botImage);
        void regionImageChanged(QPixmap regionRawImage);
        void regionImageSaliencyChanged(QPixmap regionRawImage);
    public slots:
        void nextFrame();
        void nextTenFrames();
        void prevFrame();
        void prevTenFrames();
        void nextRegion();
        void nextTenRegions();
        void prevRegion();
        void prevTenRegions();

        void nextROI();
        void nextTenROIs();
        void prevROI();
        void prevTenROIs();
 
        void saliencySelectionMade();
        void saliencyROISelectionMade();
        void showMetaData();

    private:

        int prev_region_counter_;
        int region_counter_;
        int ball_region_counter_;

        void refreshImages();
        void refreshRegionImages();
        void refreshRegionROIImages();

        AppAdaptor *appAdaptor;

        std::vector <BallDetectorVisionBundle> ball_regions_;

        QPainter *painter;

        SaliencyType t;
        bool downSampled;
};
