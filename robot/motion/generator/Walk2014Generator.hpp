/*
Copyright 2010 The University of New South Wales (UNSW).
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

/**
 * Walk2014Generator.hpp
 * BH 18 Jan 2014
 */

#pragma once

#include <cmath>
#include "motion/generator/Generator.hpp"
#include "motion/generator/BodyModel.hpp"
#include "types/XYZ_Coord.hpp"
#include "types/ActionCommand.hpp"
#include "utils/Timer.hpp"
#include "blackboard/Blackboard.hpp"
#include "motion/MotionOdometry.hpp"

class Walk2014Generator : Generator {
   public:
   explicit Walk2014Generator(Blackboard *bb);
   ~Walk2014Generator();
   JointValues makeJoints(ActionCommand::All* request,
                          Odometry* odometry,
                          const SensorValues &sensors,
                          BodyModel &bodyModel,
                          float ballX,
                          float ballY);
   // once started by makeJoints, will not be stopped, unless called reset
   bool isActive();
   ActionCommand::Body active;

   enum Walk2014Option {
      STAND        = 0, // with knees straight and stiffness set to zero to conserve energy and heat generation in motors
      STANDUP      = 1, // process of moving from WALK crouch to STAND
      CROUCH       = 2, // process of transitioning from STAND to WALK
      WALK         = 3,
      READY        = 4, // stand still ready to walk
      KICK         = 5,
      STEP         = 6,
      NONE         = 7,
      KICK_CROUCH  = 8,
      NUMBER_OF_WALK_OPTIONS
   };

   enum WalkState {
      WALKING        = 0,
      STARTING       = 1,
      STOPPING       = 2,
      NOT_WALKING    = 3,
      NUMBER_OF_WALK_STATES
   };

   Walk2014Option walk2014Option;
   WalkState walkState;
   void readOptions(const boost::program_options::variables_map& config);
   void reset();
   void stop();
   friend class WalkEnginePreProcessor;

   private:
   bool exactStepsRequested;

   // legacy code
   bool stopping;
   bool stopped;
   // time step, timers,
   float dt;
   float t;
   float globalTime;

   float timer;
   float T;                                                // period of half a walk cycle

   const float z;                                          // zero
   const float PI;

   // Nao H25 V4 dimensions - from utils/body.hpp and converted to meters
   float thigh;                                            // thigh length in meters
   float tibia;                                            // tibia length in meters
   float ankle;                                            // height of ankle above ground

   // Walk 2014 parameters in meters and seconds
   float hiph;                                             // variable vertical distance ground to hip in meters
   float hiph0;                                            // some initial hiph
   float foothL;                                           // meters left foot is lifted off the ground
   float foothR;                                           // meters right foot is lifted off the ground
   float nextFootSwitchT;                                  // next time-point at which to change support foot
   float forward;                                          // Omnidirectional walk forward/backward
   float lastForward;                                      // previous forward value accepted
   float forwardL0, forwardL;                              // variable left foot position wrt standing
   float forwardR0, forwardR;                              // variable right foot position wrt standing
   float leftR;                                            // sideways step in meters for right foot
   float leftL;                                            // sideways step in meters for left  foot
   float left, lastLeft;                                   // Omnidirectional walk left/right
   float turn;                                             // Omnidirectional walk CW / CCW
   float power;                                            // Omnidirectional walk - reserved for kicking
   float bend;
   float speed;
   ActionCommand::Body::Foot foot;                         // is this right?
   bool isFast;
   bool useShuffle;
   float stiffness;                                        // global stiffness (poweer to motors)
   float turnRL;                                           // turn variable
   float turnRL0;                                          // turnRL at support foot switch
   float swingAngle;                                       // recovery angle for sideStepping
   bool supportFoothasChanged;                             // Indicates that support foot is deemed to have changed
   bool weightHasShifted;
   float balanceAdjustment;
   float coronalBalanceAdjustment;
   float comOffset;                                        // move in meters of CoM in x-direction when walking to spread weight more evenly over foot

   // Gyro filters
   float filteredGyroX;
   float filteredGyroY;

   // Kicks
   float kickT;
   float rock;
   float kneePitchL, kneePitchR, lastKneePitch;
   float anklePitchL, anklePitchR;
   float lastKickForward;
   float lastSide;
   float lastAnklePitch;
   float lastShoulderRollAmp;
   float lastFooth;
   float lastRock;
   float lastKickTime;
   float shoulderPitchL;                                   // to swing left  arm while walking / kicking
   float shoulderPitchR;                                   // to swing right arm while walking / kicking
   float shoulderRollL;
   float shoulderRollR;
   float dynamicSide;
   float turnAngle;
   float lastTurn;

   bool kick_fast; // config option

   // Kick parameter constants
   float shiftPeriod; // time to shift weight on to one leg
   float shiftEndPeriod; // time to shift weight back from one leg
   float backPhase; // time to move kick foot back
   float kickPhase; // time to swing kick foot
   float throughPhase; // time to hold kick foot
   float endPhase; // time to return kick foot to zero position
   float shoulderRollAmpDivisor; // arm roll to leave room for kicking
   float kickLean;
   float kickLeanOffset; // kick lean offset for different robots

   //for odometry updates
   float prevTurn;
   float prevForwardL;
   float prevForwardR;
   float prevLeftL;
   float prevLeftR;

   int lElbowYawCounter;
   int rElbowYawCounter;

   //To clamp feet using feet
   Blackboard *blackboard;

   void initialise();

   // Use for iterative inverse kinematics for turning (see documentation BH 2010)
   struct Hpr {
      float Hp;
      float Hr;
      // Hpr(): Hp(0.0f), Hr(0.0f) { }
   };

   /**
    * Avoids the feet of nearby robots
    */
   void avoidFeet(float &foward, float& left, float& turn, BodyModel& bodyModel);
   /**
    * Calculates the lean angle given:
    * the commanded left step in meters,
    * time thorough walkStep phase, and total walkStep Phase time
    */
   float leftAngle();

   MotionOdometry motionOdometry;
   Odometry updateOdometry(bool isLeftSwingFoot);

   /**
    * returns smooth values in the range 0 to 1 given time progresses from 0 to period
    */
   float parabolicReturn(float); // step function with deadTimeFraction/2 delay
   float parabolicStep(float time, float period, float deadTimeFraction);                  // same as above, except a step up
   float linearStep(float time, float period);                                             // linear increase from 0 to 1
   float interpolateSmooth(float start, float end, float tCurrent, float tEnd);            // sinusoidal interpolation
   //float moveSin(float start, float finish, float period);
   // sinusoidal step - not used yet

   /**
    * Sets kick settings when starting the kick
    */
   void prepKick(bool isLeft, BodyModel &bodyModel);
   
   bool canAbortKick();

   /**
    * Specifies the joint parameters that form the kick 
    */
   void makeForwardKickJoints(float kickLean, float kickStepH, float &footh, float &forward, float &side, 
    float &kneePitch, float &shoulderRoll, float &anklePitch, float &ballY, ActionCommand::All* request);

   /**
    * Adds kick parameters to final joint values 
    */
   void addKickJoints(JointValues &j);

   // Foot to Body coord transform used to calculate IK foot position and ankle tilts to keep foot in ground plane when turning
   XYZ_Coord mf2b(float Hyp, float Hp, float Hr, float Kp, float Ap,
                  float Ar, float xf, float yf, float zf);
   Hpr hipAngles(float Hyp, float Hp, float Hr, float Kp, float Ap,
                 float Ar, float xf, float yf, float zf, XYZ_Coord e);
};


