#ifndef SIMULATION_SIMULATIONTHREAD_H_
#define SIMULATION_SIMULATIONTHREAD_H_

#include "AngleSensor.hpp"
#include "SimulationConnection.hpp"
#include "SimVisionAdapter.hpp"
#include "SonarSensor.hpp"

#include "blackboard/Adapter.hpp"
#include "libagent/AgentData.hpp"

#include <semaphore.h>

// TODO debug checking time
#include <time.h>

using namespace Simulation;

/* Wrapper class for simulator thread */
class SimulationThread : Adapter {
public:
    /* Constructor */
    SimulationThread(Blackboard *bb);

    /* Destructor */
    ~SimulationThread();

    /* One cycle of this thread */
    void tick();

private:
    int shared_fd_;
    sem_t* semaphore_;
    AgentData* shared_data_;
    SimulationConnection connection_;   /**< Handles TCP connection to simulation server */
    SimVisionAdapter vision_;

    JointValues current_;               // Current joint positions
    JointValues last_command_;
    AngleSensor angle_;
    SonarSensor sonar_;

    float prev_rlj1_;
    int team_;                          // Team 
    int player_number_;                 // Player number
    static const double VEL_PER_TICK = 1.22173; // TODO move this somewhere else

};

#endif   // SIMULATION_SIMULATIONTHREAD_H_


