#pragma once

#include <string>

#include "../../../robot/blackboard/Blackboard.hpp"
#include "../../../robot/perception/vision/colour/GreenYUVClassifier.hpp"
#include "../../../robot/perception/vision/VisionAdapter.hpp"
#include "../../../robot/perception/vision/Vision.hpp"

#include "appStatus.hpp"
#include "vatnaoFrameInfo.hpp"
#include "world/world.hpp"
#include "world/dumpParser.hpp"

using namespace std;

class AppAdaptor{
    public:
        AppAdaptor(string path, string colour_cal_top, string colour_cal_bot);

        // Move forward or back the given number of frames
        // Return the number of frames actually moved
        int forward(int numFrames = 1);
        int back(int numFrames = 1);

        void appGenerateFrameInfo();

        AppStatus getStatus();

        GreenYUVClassifier* get_colour_cal(bool top) { 
            if (top) {
                return colour_classifier_top_; 
            }
            else {
                return colour_classifier_bot_;
            }
        };

        VatnaoFrameInfo getFrameInfo() { return frame_info_; };

    private:
        DumpParser dumpParser;
        World world;
        VisionAdapter *runswiftVisionAdapter;
        Vision *runswift_vision_;
        string colour_cal_top_;
        string colour_cal_bot_;

        CameraToRR camera_to_rr_;
        boost::shared_ptr<CombinedFrame> combined_frame_;
        GreenYUVClassifier *colour_classifier_top_;
        GreenYUVClassifier *colour_classifier_bot_;

        // Bounding boxes for full regions
        BBox bbox_top_;
        BBox bbox_bot_;

        CombinedFovea combined_fovea_;

        VatnaoFrameInfo frame_info_;
};
