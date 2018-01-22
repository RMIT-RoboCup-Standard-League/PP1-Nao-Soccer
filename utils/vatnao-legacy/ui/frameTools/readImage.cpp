#include "readImage.hpp"

#include <QRgb>

#include "readBoundingBoxes.hpp"

#define NUM_OTSU_HISTOGRAM_BUCKETS 256 

// TODO: this is copied from BALLDETECTOR.cpp

#define NORMALISE_PIXEL(val, c) std::min(std::max(((double)val* c) , 0.0), 255.0)

QRgb yuvToQRgb(int y, int u, int v) {
    y -= 16;
    u -= 128;
    v -= 128;

    int r = static_cast<int>((298.082 * y + 0       * u + 408.583 * v) / 256);
    int g = static_cast<int>((298.082 * y - 100.291 * u - 208.120 * v) / 256);
    int b = static_cast<int>((298.082 * y + 516.411 * u + 0       * v) / 256);

    // bound each r, g and b value between 0 and 255
    r = (r < 0) ? 0 : ((r > 255) ? 255 : r);
    g = (g < 0) ? 0 : ((g > 255) ? 255 : g);
    b = (b < 0) ? 0 : ((b > 255) ? 255 : b);

    return qRgb(r, g, b);
}

QRgb getRGB(uint8_t const* frame, int col, int row, int num_cols){
    uint8_t y = gety(frame, row, col, num_cols);
    uint8_t u = getu(frame, row, col, num_cols);
    uint8_t v = getv(frame, row, col, num_cols);

    return yuvToQRgb(y, u, v);
}

void frameToQImage(VatnaoFrameInfo frameInfo, bool top, QImage *image){
    uint8_t const* frame = top ? frameInfo.topFrame : frameInfo.botFrame;

    if (frame != NULL) {
        int const ROWS = top ? TOP_IMAGE_ROWS : BOT_IMAGE_ROWS;
        int const COLS = top ? TOP_IMAGE_COLS : BOT_IMAGE_COLS;

        for (int row = 0; row < ROWS; row++){
            for (int col = 0; col < COLS; col++){
                image->setPixel(col, row, getRGB(frame, col, row, COLS));
            }
        }
    } else {
        // if we can't load the frame, fill with red
        image->fill(qRgb(255, 0, 0));
    }
}

QImage regionToQImage(VatnaoFrameInfo &frameInfo, FrameRect &r, QImage *image){
    image = new QImage(r.width, r.height, QImage::Format_RGB32);

    uint8_t const* frame = r.topCamera ? frameInfo.topFrame : frameInfo.botFrame;
    int num_cols = r.topCamera ? TOP_IMAGE_COLS : BOT_IMAGE_COLS;

    if (frame != NULL) {
        for (int row = 0; row < r.height; row++){
            for (int col = 0; col < r.width; col++){
                image->setPixel(col, row, getRGB(frame, r.x + col, r.y + row, num_cols));
            }
        }
    } else {
        // if we can't load the frame, fill with red
        image->fill(qRgb(255, 0, 0));
    }

    int scaled_size = 1000;
    QImage result = image->scaledToHeight(scaled_size);

    delete image;
    return result;
}

QImage ballROIToSaliencyQImage(VatnaoFrameInfo &frameInfo, BallDetectorVisionBundle &bdvb, QImage *image, SaliencyType t, 
        bool downSampled, GreenYUVClassifier* colour_cal, QLineEdit *YHist){
    FrameRect r = rectFromBoundingBox(bdvb.region->getBoundingBoxRaw(), bdvb.region->isTopCamera(), "region");
    return regionToSaliencyQImage(frameInfo, r, bdvb, image, t, downSampled, colour_cal, YHist);
}

QImage regionToSaliencyQImage(VatnaoFrameInfo &frameInfo, FrameRect &r, BallDetectorVisionBundle &bdvb, QImage *image, SaliencyType t, 
        bool downSampled, GreenYUVClassifier* colour_cal, QLineEdit *YHist){
   
    uint8_t const* frame = r.topCamera ? frameInfo.topFrame : frameInfo.botFrame;
    if (frame != NULL) {
        BallDetector bd = BallDetector();

        int image_rows = r.height;
        int image_cols = r.width;
        int density = 1;

        if (downSampled) {
            density = bdvb.region->getDensity();
            image_rows /= density;
            image_cols /= density;
        }

        image = new QImage(image_cols, image_rows, QImage::Format_RGB32);

        int num_cols = r.topCamera ? TOP_IMAGE_COLS : BOT_IMAGE_COLS;

        switch (t) {
            case CANDIDATEPOINTS:
                {
                    RegionI::iterator_raw it = bdvb.region->begin_raw();

                    for (int row = 0; row < image_rows; row++){
                        for (int col = 0; col < image_cols; col++){
                            image->setPixel(col, row, getRGB(frame, r.x + col * density, r.y + row * density, num_cols));
                        }
                    }

                    //bd.getCircleCandidatePoints(*bdvb.region, bdvb);
                    bd.checkPartialRegion(bdvb);
                    bd.preProcess(bdvb); 
                    bd.processCircleFit(*bdvb.region, bdvb); 

                    for (std::vector<Point>::iterator it = bdvb.circle_fit_points.begin(); it != bdvb.circle_fit_points.end(); it++) {
                        if (downSampled) {
                            image->setPixel(it->x(), it->y(), QColor("red").rgb());
                        }
                        else {
                            image->setPixel(it->x() * bdvb.region->getDensity(), it->y() * bdvb.region->getDensity(), QColor("red").rgb());
                        }
                    }

                    QColor circle_pen_colour;

                    if (bdvb.circle_fit.circle_found) {
                        circle_pen_colour = QColor("white");
                    }
                    else {
                        circle_pen_colour = HIGHLIGHT_PINK;
                    }

                    QPainter painter(image);
                    QPoint centre;

                    if (downSampled) {
                        centre.setX(bdvb.circle_fit.result_circle.centre.x()); 
                        centre.setY(bdvb.circle_fit.result_circle.centre.y()); 
                        painter.setPen(QPen(circle_pen_colour, 1));
                        painter.drawEllipse(centre, (int) bdvb.circle_fit.result_circle.radius, (int) bdvb.circle_fit.result_circle.radius);
                    }
                    else {
                        centre.setX(bdvb.circle_fit.result_circle.centre.x() * bdvb.region->getDensity()); 
                        centre.setY(bdvb.circle_fit.result_circle.centre.y() * bdvb.region->getDensity()); 
                        painter.setPen(QPen(circle_pen_colour, 1));
                        painter.drawEllipse(centre, 
                            (int) bdvb.circle_fit.result_circle.radius * bdvb.region->getDensity(), 
                            (int) bdvb.circle_fit.result_circle.radius * bdvb.region->getDensity());
                    }
                }
                break;
            case OTSU:
                {
                    bd.preProcess(bdvb); 
                    int white_black_threshold = bdvb.otsu_top_threshold_;

                    for (int row = 0; row < image_rows; row++){
                        // If we drop to the second half of the image, use the bottom threshold. 
                        if (row > image_rows / 2){
                            white_black_threshold = bdvb.otsu_bot_threshold_;
                        }                

                        for (int col = 0; col < image_cols; col++){
                            if (gety(frame, r.y + row * density, r.x + col * density, num_cols) > white_black_threshold) {
                                image->setPixel(col, row, QColor("white").rgb());
                            }
                            else {
                                image->setPixel(col, row, QColor("black").rgb());
                            }
                        }
                    }
                }
                break; 
            case COLOURSALIENCY:
                {
                    for (int row = 0; row < image_rows; row++){
                        for (int col = 0; col < image_cols; col++){
                            uint8_t* pixel = getpixelpair(frame, r.y + row * density, r.x + col * density, num_cols);
                            image->setPixel(col, row, 
                            CPLANE_COLOURS[colour_cal->classifyTop(pixel)].rgb());
                        }
                    }                  
                }
                break;
            case ALL:
                {
                    bd.checkPartialRegion(bdvb);
                    bd.preProcess(bdvb); 
                    bd.processCircleFit(*bdvb.region, bdvb); 
                    int white_black_threshold = bdvb.otsu_top_threshold_;

                    // Draw internal regions

                    bd.processInCircleOtsu(bdvb);

                    bd.processInternalRegions(*bdvb.region, bdvb, bdvb.circle_fit.result_circle, bdvb.internal_regions);
                    QColor internal_region_pen_colour = QColor(51, 255, 51);


                    for (int row = 0; row < image_rows; row++){
                        double contrast = bdvb.contrast_row_multiplier[row];
                        
                        // If we drop to the second half of the image, use the bottom threshold. 
                        if (row > image_rows / 2){
                            white_black_threshold = bdvb.otsu_bot_threshold_;
                        }                

                        for (int col = 0; col < image_cols; col++){
                            int y = gety(frame, r.y + row * density, r.x + col * density, num_cols);
                            y = NORMALISE_PIXEL(y, contrast);
                            if (y > white_black_threshold) {
                                image->setPixel(col, row, QColor("white").rgb());
                            }
                            else {
                                image->setPixel(col, row, QColor("black").rgb());
                            }
                        }
                    } 

                    //std::cout << "Internalregions: " << internal_regions.groups.size() << "\n";

                    QPainter painter(image);

                    for (unsigned int i = 0; i < bdvb.internal_regions.groups.size(); i++) {
                        painter.setPen(QPen(internal_region_pen_colour, 1));
                        QPointF topLeft = QPointF(bdvb.internal_regions.groups[i].min_x, bdvb.internal_regions.groups[i].min_y);
                        QPointF botRight = QPointF(bdvb.internal_regions.groups[i].max_x, bdvb.internal_regions.groups[i].max_y);
                        QRectF int_region = QRectF(topLeft, botRight);

                        painter.drawRect(int_region);
                    }

// Draw circle candidate points

                    //bd.getCircleCandidatePoints(*bdvb.region, bdvb);

                    float radius = bdvb.region->getRows() * 0.5;
                    float e = radius * 0.1;
                    const int e2 = e * e;

                    unsigned int n_concensus_points = 0;

                    for (std::vector<Point>::iterator it = bdvb.circle_fit_points.begin(); 
                            it != bdvb.circle_fit_points.end(); it++) {
//std::cout << "x: " << it->x() << " y: " << it->y() << "\n";
                        QColor pen_colour;

                        PointF d;

                        d.x() = bdvb.circle_fit.result_circle.centre.x() - it->x();
                        d.y() = bdvb.circle_fit.result_circle.centre.y() - it->y();
                       
                        float dist = d.norm() - bdvb.circle_fit.result_circle.radius;
                        float dist2 = dist * dist;

                        if (dist2 < e2) {
                            pen_colour = QColor("red");
                            //std::cout << "Concensus point: " << it->x() << ", " << it->y() << " num: " << 
                                //n_concensus_points << "\n";
                            ++ n_concensus_points;
                        }
                        else {
                            pen_colour = QColor("orange");
                        } 

                        if (downSampled) {
                            image->setPixel(it->x(), it->y(), pen_colour.rgb());
                        }
                        else {
                            image->setPixel(it->x() * bdvb.region->getDensity(), it->y() * bdvb.region->getDensity(), pen_colour.rgb());
                        }
                    }

                    //std::cout << "N_Concensus: " << n_concensus_points << "\n";
                    //std::cout << "Circle found: " << bdvb.circle_fit.result_circle.centre.x() << " , " << 
                        //bdvb.circle_fit.result_circle.centre.y() << " radius: " << bdvb.circle_fit.result_circle.radius << "\n";

                    //std::cout << "Rows: " << bdvb.region->getRows() << " Cols: " << bdvb.region->getCols() << "\n";

                    // Draw circle fit

                    //bd.processCircleFit(*bdvb.region, bdvb); 
                    QColor circle_pen_colour;

                    if (bdvb.circle_fit.circle_found) {
                        circle_pen_colour = HIGHLIGHT_PINK;
                    }
                    else {
                        circle_pen_colour = QColor(175, 238, 238);
                    }

                    QPoint centre;

                    if (downSampled) {
                        centre.setX(bdvb.circle_fit.result_circle.centre.x()); 
                        centre.setY(bdvb.circle_fit.result_circle.centre.y()); 
                        painter.setPen(QPen(circle_pen_colour, 1));
                        painter.drawEllipse(centre, (int) bdvb.circle_fit.result_circle.radius, (int) bdvb.circle_fit.result_circle.radius);
                    }
                    else {
                        centre.setX(bdvb.circle_fit.result_circle.centre.x() * bdvb.region->getDensity()); 
                        centre.setY(bdvb.circle_fit.result_circle.centre.y() * bdvb.region->getDensity()); 
                        painter.setPen(QPen(circle_pen_colour, 1));
                        painter.drawEllipse(centre, 
                            (int) bdvb.circle_fit.result_circle.radius * bdvb.region->getDensity(), 
                            (int) bdvb.circle_fit.result_circle.radius * bdvb.region->getDensity());
                    }



                    // Draw internal circle regions

                    /* Broken
                    std::vector <CircleFitFeatures> internal_region_circles = 
                        bd.processInternalRegionCircles(regionCopy, bdvb); 

                    painter.setPen(QPen(QColor("yellow"), 1));

                    for (std::vector <CircleFitFeatures>::iterator irc = internal_region_circles.begin();
                        irc != internal_region_circles.end(); irc++) {

                        QPoint centre;

                        if (downSampled) {
                            centre.setX(irc->result_circle.centre.x()); 
                            centre.setY(irc->result_circle.centre.y()); 
                            painter.drawEllipse(centre, (int) irc->result_circle.radius, (int) irc->result_circle.radius);
                        }
                        else {
                            centre.setX(irc->result_circle.centre.x() * bdvb.region->getDensity()); 
                            centre.setY(irc->result_circle.centre.y() * bdvb.region->getDensity()); 
                            painter.drawEllipse(centre, 
                                (int) irc->result_circle.radius * bdvb.region->getDensity(), 
                                (int) irc->result_circle.radius * bdvb.region->getDensity());
                        }                         
                    }
                    */
                }
                break;
            case YHIST:
                {
                    bd.checkPartialRegion(bdvb);
                    bd.preProcess(bdvb); 

                    bd.processCircleFit(*bdvb.region, bdvb);

                    bd.processInCircleOtsu(bdvb);

                    for (int row = 0; row < image_rows; row++){
                        double contrast = bdvb.contrast_row_multiplier[row];

                        for (int col = 0; col < image_cols; col++){
                            int y = gety(frame, r.y + row * density, r.x + col * density, num_cols);
                            y = NORMALISE_PIXEL(y, contrast);

                            image->setPixel(col, row, QColor(y, y, y).rgb());
                            /*
                            if (y >thresh) {
                                image->setPixel(col, row, QColor("white").rgb());
                            }
                            else {
                                image->setPixel(col, row, QColor("black").rgb());
                            }
                            */
                            //image->setPixel(col, row, QColor(y, y, y).rgb());
                        }
                    }
                }
                break;
        }
    } else {
        // if we can't load the frame, fill with red
        image->fill(qRgb(255, 0, 0));
    }

    QImage result = image->copy();

    delete image;
    return result;
}
