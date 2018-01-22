#include "vatnaoManager.hpp"

#include "../app/appStatus.hpp"
#include "../app/vatnaoFrameInfo.hpp"

VatnaoManager::VatnaoManager(AppAdaptor *a){
    appAdaptor = a;
    AppStatus status = appAdaptor->getStatus();
    topImage = new QImage(status.numTopCameraCols, status.numTopCameraRows, QImage::Format_RGB32);
    botImage = new QImage(status.numBotCameraCols, status.numBotCameraRows, QImage::Format_RGB32);

    prev_region_counter_ = -1;
    region_counter_ = 0;
    ball_region_counter_ = 0;

    t = COLOURSALIENCY;
    downSampled = true;
}

VatnaoManager::~VatnaoManager(){}

void VatnaoManager::nextFrame(){
    appAdaptor->forward();
    refreshImages();
}

void VatnaoManager::nextTenFrames(){
    appAdaptor->forward(10);
    refreshImages();
}

void VatnaoManager::prevFrame(){
    appAdaptor->back();
    refreshImages();
}

void VatnaoManager::prevTenFrames(){
    appAdaptor->back(10);
    refreshImages();
}

void VatnaoManager::nextRegion(){
    int max_regions = appAdaptor->getFrameInfo().regions.size();
    prev_region_counter_ = region_counter_;
    region_counter_ = std::min(region_counter_ + 1, max_regions - 1);
    refreshRegionImages();
}

void VatnaoManager::nextTenRegions(){
    int max_regions = appAdaptor->getFrameInfo().regions.size();
    prev_region_counter_ = region_counter_;
    region_counter_ = std::min(region_counter_ + 10, max_regions - 1);
    refreshRegionImages();
}

void VatnaoManager::prevRegion(){
    prev_region_counter_ = region_counter_;
    region_counter_ = std::max(region_counter_ - 1, 0);
    refreshRegionImages();
}

void VatnaoManager::prevTenRegions(){
    prev_region_counter_ = region_counter_;
    region_counter_ = std::max(region_counter_ - 10, 0);
    refreshRegionImages();
}

void VatnaoManager::nextROI(){
    int max_rois = ball_regions_.size();
    ball_region_counter_ = std::min(ball_region_counter_ + 1, max_rois - 1);
    refreshRegionROIImages();
}

void VatnaoManager::nextTenROIs(){
    int max_rois = ball_regions_.size();
    ball_region_counter_ = std::min(ball_region_counter_ + 10, max_rois - 1);
    refreshRegionROIImages();
}

void VatnaoManager::prevROI(){
    ball_region_counter_ = std::max(ball_region_counter_ - 1, 0);
    refreshRegionROIImages();
}

void VatnaoManager::prevTenROIs(){
    ball_region_counter_ = std::max(ball_region_counter_ - 10, 0);
    refreshRegionROIImages();
}

void VatnaoManager::saliencySelectionMade(){
    refreshRegionImages(); 
}

void VatnaoManager::saliencyROISelectionMade(){
    refreshRegionROIImages(); 
}

void VatnaoManager::showMetaData(){
    if (checkBoxBallMetaData->isChecked()) {
        VatnaoFrameInfo frameInfo = appAdaptor->getFrameInfo();

        BallDetector bd = BallDetector();

        VisionInfoOut info_out;
        info_out.cameraToRR = &frameInfo.cameraToRR;

        std::string ballMetaData = bd.getFeatureSummary(info_out, ball_regions_[ball_region_counter_]);
        resultInfo->setText(QString(ballMetaData.c_str()));
    }
    else {
        resultInfo->setText("");
    }
}

void VatnaoManager::refreshImages(){
    prev_region_counter_ = -1;
    region_counter_ = 0;

    appAdaptor->appGenerateFrameInfo();
    VatnaoFrameInfo frameInfo = appAdaptor->getFrameInfo();

    frameToQImage(frameInfo, true, topImage);
    frameToQImage(frameInfo, false, botImage);

    drawRegionBoundingBoxes(frameInfo, topImage, botImage);
    drawBallBoundingBoxes(frameInfo, topImage, botImage);
    drawFieldBoundaries(frameInfo, topImage, botImage);
    drawFieldLines(frameInfo, topImage, botImage);
    drawFieldPoints(frameInfo, topImage, botImage);
    drawGoalPosts(frameInfo, topImage, botImage);
    drawRobots(frameInfo, topImage, botImage);

    refreshRegionImages();

    topImageChanged(QPixmap::fromImage(*topImage));
    botImageChanged(QPixmap::fromImage(*botImage));

    frameInfo.print();
}

void VatnaoManager::refreshRegionROIImages() {
    VatnaoFrameInfo frameInfo = appAdaptor->getFrameInfo();

    BallDetector bd = BallDetector();
     
    VisionInfoIn info_in;
    info_in.latestAngleX = 0;

    VisionInfoMiddle info_middle;
    info_middle.colour_classifier_top_ = appAdaptor->get_colour_cal(true);
    info_middle.colour_classifier_bot_ = appAdaptor->get_colour_cal(false);

    VisionInfoOut info_out;
    info_out.cameraToRR = &frameInfo.cameraToRR;

    for (int i = 0; i < TOP_IMAGE_COLS; i++) {
        info_out.topStartScanCoords[i] = 0;
    }

    for (int i = 0; i < BOT_IMAGE_COLS; i++) {
        info_out.botStartScanCoords[i] = 0;
    }

    std::cout << "__Mock run ball detector\n";
    info_middle.roi.push_back(frameInfo.regionData[region_counter_]); 

    bd.detect(info_in, info_middle, info_out);
    std::cout << "__END Mock run ball detector\n";

    ball_regions_.clear();

    if (radioNaive->isChecked()) {
        bd.naiveROI(info_in, frameInfo.regionData[region_counter_], info_middle, info_out, false, ball_regions_);
    }
    else if (radioCircle->isChecked()) {
        bd.circleROI(info_in, frameInfo.regionData[region_counter_], info_middle, info_out, false, ball_regions_);
    }
    else if (radioCombo->isChecked()) {
        bd.comboROI(info_in, frameInfo.regionData[region_counter_], info_middle, info_out, false, ball_regions_);
    }
    else {
        bd.blackROI(info_in, frameInfo.regionData[region_counter_], info_middle, info_out, false, ball_regions_);
    }

    std::cout << "Ball size: " << ball_regions_.size() << "\n";    
    if (ball_regions_.size() > 0) {
        QImage saliencyROIImage = ballROIToSaliencyQImage(frameInfo, ball_regions_[ball_region_counter_], regionSaliencyImage, 
            t, downSampled, appAdaptor->get_colour_cal(frameInfo.regions[region_counter_].topCamera), YHistThresh);

        regionImageSaliencyChanged(QPixmap::fromImage(saliencyROIImage));

        showMetaData();
    }
    else {
        resultInfo->setText("Invalid ball region");
    }
}

void VatnaoManager::refreshRegionImages() {
    VatnaoFrameInfo frameInfo = appAdaptor->getFrameInfo();

    QImage image = regionToQImage(frameInfo, frameInfo.regions[region_counter_], regionRawImage);

    downSampled = checkBoxDownsampled->isChecked();
    YHistThresh->setEnabled(false);

    if (radioCircleCandidate->isChecked()) {
        t = CANDIDATEPOINTS;
    }
    else if (radioColourSaliency->isChecked()) {
        t = COLOURSALIENCY;
    }
    else if (radioOtsuThreshold->isChecked()) {
        t = OTSU;
    }
    else if (radioAll->isChecked()) {
        t = ALL;
    }
    else {
        t = YHIST;
        YHistThresh->setEnabled(true);
    }

    if (frameInfo.regionData[region_counter_].getRows() != 0 && frameInfo.regionData[region_counter_].getCols() != 0) {
        // Change region highlighting on full size image
        if (prev_region_counter_ != -1) {
            FrameRect &prev = frameInfo.regions[prev_region_counter_];
            QImage *img = (prev.topCamera) ? topImage : botImage;
       
            drawRectangle(prev.x, prev.y, prev.width, prev.height, COLOUR_ROI, THIN_LINE, img);
        }

        FrameRect &r = frameInfo.regions[region_counter_];
        QImage *img = (r.topCamera) ? topImage : botImage;

        drawRectangle(r.x, r.y, r.width, r.height, HIGHLIGHT_PINK, THIN_LINE, img);

        topImageChanged(QPixmap::fromImage(*topImage));
        botImageChanged(QPixmap::fromImage(*botImage));

        regionImageChanged(QPixmap::fromImage(image));

        ball_region_counter_ = 0;

        refreshRegionROIImages();
    }
    else {
        resultInfo->setText("Invalid region");
    }
}

