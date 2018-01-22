/*
Copyright 2010 The University of New South Wales (UNSW).

This file is part of the 2010 team rUNSWift RoboCup entry. You may
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version as
modified below. As the original licensors, we add the following
conditions to that license:

In paragraph 2.b), the phrase "distribute or publish" should be
interpreted to include entry into a competition, and hence the source
of any derived work entered into a competition must be made available
to all parties involved in that competition under the terms of this
license.

In addition, if the authors of a derived work publish any conference
proceedings, journal articles or other academic papers describing that
derived work, then appropriate academic citations to the original work
must be included in that publication.

This rUNSWift source is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with this source code; if not, write to the Free Software Foundation,
Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#pragma once

#include <boost/thread/mutex.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/function.hpp>
#include <signal.h>
#include <string>
#include <map>
#include <vector>
#include <deque>

#include "perception/behaviour/ReadySkillPositionAllocation.hpp"
#include "utils/body.hpp"
#include "utils/boostSerializationVariablesMap.hpp"
#include "perception/kinematics/Parameters.hpp"
#include "perception/vision/VisionDefinitions.hpp"
#include "perception/vision/other/RobotRegion.hpp"
#include "perception/localisation/LocalisationDefs.hpp"
#include "perception/localisation/LocalisationUtils.hpp"
#include "perception/localisation/SharedLocalisationUpdateBundle.hpp"
#include "perception/kinematics/Pose.hpp"
#include "gamecontroller/RoboCupGameControlData.hpp"
#include "utils/Logger.hpp"
#include "transmitter/TransmitterDefs.hpp"
#include "types/BehaviourRequest.hpp"

#include "types/ActionCommand.hpp"
#include "types/ButtonPresses.hpp"
#include "types/Odometry.hpp"
#include "types/SensorValues.hpp"
#include "types/RRCoord.hpp"
#include "types/AbsCoord.hpp"
#include "types/XYZ_Coord.hpp"
#include "types/SPLStandardMessage.hpp"
#include "types/BroadcastData.hpp"
#include "types/BehaviourSharedData.hpp"

#include "types/FootInfo.hpp"
#include "types/BallInfo.hpp"
#include "types/PostInfo.hpp"
#include "types/RobotInfo.hpp"
#include "types/RobotObstacle.hpp"
#include "types/FieldBoundaryInfo.hpp"
#include "types/FieldFeatureInfo.hpp"
#include "types/Ipoint.hpp"
#include "types/Odometry.hpp"
#include "types/TeamBallInfo.hpp"
#include "types/CameraSettings.hpp"
#include "types/LastSecondInfo.hpp"

#include "perception/vision/Region.hpp"
#include "soccer.hpp"

#include "simulation/SimVisionAdapter.hpp"

namespace VisionTest
{
   class DumpResult;
}

/**
 * Macro to wrap reads to module's blackboard.
 * @param module which module's blackboard to read
 * @param component the component to read
 */
#define readFrom(module, component) \
   blackboard->read(&(blackboard->module.component))

/**
 * Macro to wrap array reads from module's blackboard.
 * Performs a memcpy on the provided arguments.
 * @param module which module's blackboard to read from
 * @param component while component to be written
 * @param dest where to write to
 */
#define readArray(module, component, dest) \
   memcpy(dest, blackboard->module.component, \
          sizeof(blackboard->module.component));

/**
 * Macro to wrap writes to module's blackboard.
 * @param module which module's blackboard to write
 * @param component while component to write
 * @param value the value to be written
 */
#define writeTo(module, component, value) \
   blackboard->write(&(blackboard->module.component), value);

/**
 * Macro to wrap array writes to module's blackboard.
 * Performs a memcpy on the provided arguments.
 * @param module which module's blackboard to write
 * @param component while component to write
 * @param value the value to be written
 */
#define writeArray(module, component, value) \
   memcpy(blackboard->module.component, value, \
          sizeof(blackboard->module.component));

/**
 * Macro to wrap acquiring a mutex on a the blackboard.
 * @param name which module's lock to acquire
 */
#define acquireLock(name) \
   (blackboard->locks.name)->lock();

/**
 * Macro to wrap releasing a mutex on the blackboard.
 * @param name which module's lock to release
 */
#define releaseLock(name) \
   (blackboard->locks.name)->unlock();

/**
 * Blackboard shared memory class, used for inter-module communication.
 * The Blackboard is friends with each of the module adapters
 * so that they can access Blackboard privates (and nobody else can)
 * The safeRun templated function has access to the blackboard
 * to look up thread timings and things like that
 */

class NetworkReader;
class OverviewTab;

struct KinematicsBlackboard {
   explicit KinematicsBlackboard();
   void readOptions(const boost::program_options::variables_map& config);
   // Sonar filter is in kinematics because it needs to be run before vision
   std::vector< std::vector <int> > sonarFiltered;
   bool isCalibrating;
   Parameters<float> parameters;
   SensorValues sensorsLagged;
};

/* Data Behaviour module will be sharing with others */
struct BehaviourBlackboard {
   explicit BehaviourBlackboard();
   void readOptions(const boost::program_options::variables_map& config);
   BehaviourRequest request[2]; // double buffer to avoid concurrent access with motion
   int readBuf;
   std::string skill;
   int kickoffSide;
   AbsCoord walkingTo;
   AbsCoord shootingTo;
   BehaviourSharedData behaviourSharedData;
   bool remoteStiffen;
   bool useGetups;
};

/* Data Localisation module will be sharing */
struct LocalisationBlackboard {
   explicit LocalisationBlackboard();
   void readOptions(const boost::program_options::variables_map& config);

   // Global robot position
   AbsCoord robotPos;
   std::vector< AbsCoord > allrobotPos;

   // Number of frames since the ball has been seen.
   // TODO: this doesnt belong in localisation! Move to behaviours or vision.
   uint32_t ballLostCount;
   uint32_t ballSeenCount;

   // Robot relative coords.
   RRCoord ballPosRR;

   // Cartesian robot relative ball coords.
   AbsCoord ballPosRRC;

   // Robot relative ball velocity.
   // TODO: make this in global coords and change behaviours correspondingly.
   // Ball Detection currently assumes this is in Robot Relative coordinates
   AbsCoord ballVelRRC;

   // Global ball velocity
   AbsCoord ballVel;

   double robotPosUncertainty;
   double robotHeadingUncertainty;

   double ballPosUncertainty;
   // The maximal eigenvalue of the velocity covariance matrix.
   double ballVelEigenvalue;

   // Global ball position
   AbsCoord ballPos;

   // TODO: get rid of this variable from behaviours and everywhere else!
   TeamBallInfo teamBall;

   XYZ_Coord ballNeckRelative;

   SharedLocalisationUpdateBundle sharedLocalisationBundle;

   bool havePendingOutgoingSharedBundle;
   std::vector<bool> havePendingIncomingSharedBundle;

   /** filtered positions of visual robots */
   std::vector<RobotObstacle> robotObstacles;

   bool setInitialPose;
   bool setFallen;
   bool getupLost;
};

/* Data Vision module will be sharing with others */
struct VisionBlackboard {

   explicit VisionBlackboard();
   /* Time the frame was captured */
   int64_t timestamp;

   /* Info from the previous second's frames */
   LastSecondInfo lastSecond;

   /* Detected features */
   std::vector<Ipoint>              landmarks;
   std::vector<FootInfo>            feet_boxes;
   std::vector<BallInfo>            balls;
   std::vector<BallInfo>            uncertain_balls;
   BallHint                         ballHint;
   std::vector<PostInfo>            posts;
   std::vector<RobotInfo>           robots;
   std::vector<FieldBoundaryInfo>   fieldBoundaries;
   std::vector<FieldFeatureInfo>    fieldFeatures;
   std::vector<RegionI>             regions;
   unsigned int                     missedFrames;
   std::pair<int, int>              dxdy;
   PostInfo::Type                   goalArea;
   float                            awayGoalProb;
   int                              homeMapSize;
   int                              awayMapSize;

   /** Saliency scan */
   Colour *topSaliency;
   Colour *botSaliency;
   Colour *saliency;

   /** Pointer to the current frame being processed by Vision */
   uint8_t const* currentFrame;
   uint8_t const* topFrame;
   uint8_t const* botFrame;

   /** Current camera settings on the robot */
   CameraSettings topCameraSettings;
   CameraSettings botCameraSettings;

   /**
    * DO NOT USE ANYTHING BELOW HERE
    * Kept here only for compatibility with 2010 filter
    * Will be depricated by daves new filter
    */

   /** Points on field lines */
   int numFieldLinePoints;
   RRCoord fieldLinePoints[MAX_FIELD_LINE_POINTS];
   bool canSeeBottom[MAX_POSTS];
   bool canSeeBottomRobot[MAX_ROBOTS];
};

struct PerceptionBlackboard {
   explicit PerceptionBlackboard();
   uint32_t kinematics;
   uint32_t localisation;
   uint32_t vision;
   uint32_t behaviour;
   uint32_t total;
};

struct GameControllerBlackboard {
   explicit GameControllerBlackboard();
   void readOptions(const boost::program_options::variables_map& config);
   bool connect;
   bool connected;
   RoboCupGameControlData data;
   TeamInfo our_team;
   bool team_red;
   int player_number;
   uint8_t gameState;
   char* lastGameControllerIPAddress;
};


struct MotionBlackboard {
   explicit MotionBlackboard();
   SensorValues sensors;
   // A rolling list of recent observations of range (m) readings to potentially multiple obstacles
   std::vector < std::vector <int> > sonarWindow;
   float uptime;
   ActionCommand::All active;
   Odometry odometry;
   ButtonPresses buttons;
   Pose pose;
   XYZ_Coord com;
};

struct RemoteControlBlackboard {
   explicit RemoteControlBlackboard();
   BehaviourRequest request;
   time_t time_received;
};

struct ReceiverBlackboard {
   explicit ReceiverBlackboard();
   // one for each robot on the team
   SPLStandardMessage message[ROBOTS_PER_TEAM];
   BroadcastData data[ROBOTS_PER_TEAM];
   int team;
   void readOptions(const boost::program_options::variables_map& config);
   time_t lastReceived[ROBOTS_PER_TEAM];
   bool incapacitated[ROBOTS_PER_TEAM];
};


struct SynchronisationBlackboard {
   explicit SynchronisationBlackboard();
   boost::mutex *buttons;
   boost::mutex *serialization;
};

struct ThreadBlackboard {
   explicit ThreadBlackboard();
   std::map<std::string, boost::function<void(const boost::program_options::variables_map &)> > configCallbacks;
};

class Blackboard {
   friend class boost::serialization::access;
   template<class Archive> friend void
   boost::serialization::save_construct_data(Archive &ar, const Blackboard *t,
                                             const unsigned int) {
      // TODO(jayen): move code out of header file - http://stackoverflow.com/questions/4112075/trouble-overriding-save-construct-data-when-serializing-a-pointer-to-a-class-with
      // save data required to construct instance
      ar << t->config;
   }
   // Adapter friends
   friend class LocalisationAdapter;
   friend class VisionAdapter;
   friend class SimVisionAdapter;
   friend class MotionAdapter;
   friend class BehaviourAdapter;
   friend class GameController;
   friend class OffNaoTransmitter;
   friend class NaoTransmitter;
   friend class TeamTransmitter;
   //friend class NaturalLandmarksTransmitter;
   friend class NaoReceiver;
   friend class RemoteControlReceiver;
   friend class TeamReceiver;
   friend class KinematicsAdapter;
   friend class PerceptionThread;
   friend class KinematicsCalibrationSkill;
   friend class ThreadManager;

   // Off-nao friends
   friend class DumpReader;
   friend class NetworkReader;
   friend class LocalisationReader;
   friend class OverviewTab;
   friend class nnmcTab;
   friend class CameraTab;
   friend class CalibrationTab;
   friend class VisionTab;
   friend class Connection;
   friend class VariableView;
   friend class FieldView;
   friend class SensorTab;
   friend class CameraPoseTab;
   friend class GraphTab;
   friend class ZMPTab;
   friend class VisionTestSuiteTab;
   friend class ControlTab;

   // Vision Test friends
   friend class VisionTest::DumpResult;

   // Functions
   template <class T> friend void *safelyRun(void * foo);

   public:
      explicit Blackboard();
      explicit Blackboard(const boost::program_options::variables_map &vm);
      ~Blackboard();

      /* Function to read a component from the Blackboard */
      template<class T> const T& read(const T *component);

      /* Write a component to the Blackboard */
      template<class T> void write(T *component, const T& value);

      /**
       * helper for serialization
       */
      template<class Archive>
      void shallowSerialize(Archive &ar, const unsigned int version);
      /**
       * serialises the blackboard for storing to a file or network
       */
      template<class Archive>
      void save(Archive &ar, const unsigned int version) const;
      /**
       * serialises the blackboard for loading from a file or network
       */
      template<class Archive>
      void load(Archive &ar, const unsigned int version);
  #ifndef SWIG
         BOOST_SERIALIZATION_SPLIT_MEMBER();
  #endif

      /* We now have a private inner-class blackboard for each of the
       * modules. Appropriate constructors should be placed in Blackboard.cpp
       * since there's no guarantee as to which order threads will start in. */

      // TODO(find a better solution) private:

      /* Stores command-line/config-file options
       * should be read by modules on start
       * functionality may be added later to allow change at runtime */
      boost::program_options::variables_map config;

      /**
       * the mask of what is stored/loaded from a file or network
       */
      OffNaoMask_t mask;

      /* Options callback for changes at runtime */
      void readOptions(const boost::program_options::variables_map& config);

      /* Data Kinematics module will be sharing with others */
      KinematicsBlackboard kinematics;

      /* Data Behaviour module will be sharing with others */
      BehaviourBlackboard behaviour;

      /* Data Localisation module will be sharing */
      LocalisationBlackboard localisation;

      /* Data Vision module will be sharing with others */
      VisionBlackboard vision;

      PerceptionBlackboard perception;

      /* Data GameController will be sharing */
      GameControllerBlackboard gameController;

      /* Data Motion module will be sharing with others */
      MotionBlackboard motion;

      /* Data received from friendly robots */
      ReceiverBlackboard receiver;

	  /* Data received from remote-control piece in Off-Nao */
	  RemoteControlBlackboard remoteControl;

      /* Data ThreadWatcher will be sharing with others */
      ThreadBlackboard thread;

      /* Locks used for inter-thread synchronisation */
      SynchronisationBlackboard locks;
};

#include "Blackboard.tcc"
