#ifndef SIMULATION_SIMVISIONADAPTER_H_
#define SIMULATION_SIMVISIONADAPTER_H_

#include "blackboard/Blackboard.hpp"
#include "blackboard/Adapter.hpp"
#include "PerceptorInfo.hpp"
#include "types/BallInfo.hpp"
#include "types/FieldBoundaryInfo.hpp"
#include "types/FieldFeatureInfo.hpp"
#include "types/PostInfo.hpp"
#include "types/RobotInfo.hpp"

#include <boost/shared_ptr.hpp>

#include <vector>

namespace Simulation
{
    class SimVisionAdapter : Adapter
    {
    public:
        /**
         * Constructor
         *
         * @param bb The shared Blackboard instance
         */
        SimVisionAdapter(Blackboard* bb);

        /**
         * Ticks the simulated vision adapter with the vision information
         * received from the simulator.
         *
         * @param perceived Information perceived by the simulated robot.
         */
        void tick(const PerceptorInfo& perceived);

        /**
         * Converts an observation position from the robot camera to robot 
         * relative coordinates from the feet.
         *
         * @param o The Observation to find the RRCoord for.
         * @param head_yaw The head yaw joint value at the time of the observation
         * @return RRCoord Robot relative coordinates of the observation.
         */
        static RRCoord getRR(Observation& o, double head_yaw);

    private:
        BallInfo handleBall(Observation& o, RRCoord rr); /**< Convert form observation to BallInfo */
        PostInfo handlePost(Observation& o, RRCoord rr); /**< Convert from observation to PostInfo */
        FieldFeatureInfo handleFieldFeature(Observation& o, RRCoord rr, double head_yaw); /**< Convert from observation to FieldFeatureInfo */
        RobotInfo handleRobotInfo(Observation& o, RRCoord rr); /**< Convert from observation to RobotInfo */
        void findGoalSide(std::vector<PostInfo>& posts); /**< If we find two posts, find which side of them we're on */
    };
}

#endif // SIMULATION_SIMVISIONADAPTER_H_


