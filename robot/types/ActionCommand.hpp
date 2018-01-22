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

/**
 * ActionCommand.hpp
 * Modified: 2009-11-08
 * Description: Commands which are accepted by the Locomotion Module
 * This is the new interface between the Behaviour and Locomotion
 * Body are for the walks and special actions which use the body
 * Head are for the head yaw and pitch
 * LED are for the ear, face, chest and foot LEDs
 * Tidied up from 2009 code
 */

#pragma once

#include <stdint.h>

#include <iostream>

#include "utils/body.hpp"

/* Remember to update your constants in python/wrappers/ActionCommand */

namespace ActionCommand {

/**
 * Command for controlling the body
 * Note: Some ActionType Commands WILL disable the head
 */
   struct Body {
      // Predefined actions. These take precedence over walk parameters
      enum ActionType {
         NONE = 0,

         // Stand - common starting pose for all other actions
         STAND,   //1

         // Walk
         WALK,    //2
         DRIBBLE, //3
         TURN_DRIBBLE, //4

         // Actions
         GETUP_FRONT, GETUP_BACK,         //5,6
         TIP_OVER,                        //7
         KICK,                            //8
         INITIAL,                         //9
         DEAD, REF_PICKUP,                //10, 11
         GOALIE_SIT,                      //12
         GOALIE_DIVE_RIGHT,               //13
         GOALIE_DIVE_LEFT,                //14
         GOALIE_CENTRE,                   //15
         GOALIE_UNCENTRE,                 //16
         GOALIE_INITIAL,                  //17
         GOALIE_AFTERSIT_INITIAL,         //18
         DEFENDER_CENTRE,                 //19
         GOALIE_FAST_SIT,                 //20
         MOTION_CALIBRATE,                //21
         STAND_STRAIGHT,                  //22
         LINE_UP,                         //23
         NUM_ACTION_TYPES
      };
      ActionType actionType;

      // Walk/Kick Parameters
      int forward; // How far forward (negative for backwards)  (mm)
      int left;  // How far to the left (negative for rightwards) (mm)
      float turn; // How much anti-clockwise turn (negative for clockwise) (rad)
      float power; // How much kick power (0.0-1.0)
      float bend;

      // Kick parameters
      float speed;
      float kickDirection;

      enum Foot {
         LEFT = 0,
         RIGHT
      };
      Foot foot;
      bool isFast;

      // Set this to true if you want the robot to do a kick that tries to kick it not straight
      // but angled, primarily to avoid an opponent straight ahead of it.
      bool misalignedKick; 
      bool useShuffle;

      // Set this to true if we want to put our arms back
      bool leftArmLimp;
      bool rightArmLimp;

      /**
       * Constructor for walks and kicks
       * @param at Action Type
       * @param f  How far forward (mm)
       * @param l  How far to the left (mm)
       * @param t  How much counter-clockwise turn (rad)
       * @param p  How much power
       * @param bend  Angle to bend knees (rad)
       * @param s  How much speed
       * @param k  Direction to kick (rad)
       * @param ft  Which foot to use
       * @param fast  go fast or not
       * @param useShuffle use shuffle (low step height) or not
       * @param leftArmLimp make left arm limp
       * @param rightArmLimp make right arm limp
       * @see http://runswift.cse.unsw.edu.au/confluence/display/rc2010/Movement%2C+walk%2C+kicks
       */
      Body(ActionType at, int f = 0, int l = 0, float t = 0.0, float p = 1.0,
           float bend = 15.0, float s = 1.0, float k = 0.0, Foot ft = LEFT, bool fast=false,
           bool misalignedKick=false, bool useShuffle=false, bool leftArmLimp=false, bool rightArmLimp=false)
         : actionType(at),
           forward(f),
           left(l),
           turn(t),
           power(p),
           bend(bend),
           speed(s),
           kickDirection(k),
           foot(ft),
           isFast(fast),
           misalignedKick(misalignedKick),
           useShuffle(useShuffle),
           leftArmLimp(leftArmLimp),
           rightArmLimp(rightArmLimp) {}

      /* Boost python makes using default arguements difficult.
       * Define an arguementless constructor to wrap
       */
      Body()
         : actionType(NONE),
           forward(0),
           left(0),
           turn(0),
           power(0),
           bend(0),
           speed(0),
           kickDirection(0),
           foot(LEFT),
           isFast(false),
           misalignedKick(false),
           useShuffle(false),
           leftArmLimp(false),
           rightArmLimp(false) {}

      template<class Archive>
      void serialize(Archive &ar, const unsigned int file_version) {
         ar & actionType;
         ar & forward;
         ar & left;
         ar & turn;
         ar & power;
      }
   };

   const uint8_t priorities[Body::NUM_ACTION_TYPES] = {
      0, // NONE
      0, // STAND
      0, // WALK
      0, // DRIBBLE
      0, // TURN_DRIBBLE
      3, // GETUP_FRONT
      3, // GETUP_BACK
      3, // TIP_OVER
      0, // KICK
      2, // INITIAL
      1, // DEAD
      0, // REF_PICKUP
      2, // GOALIE_SIT
      2, // GOALIE_DIVE_LEFT
      2, // GOALIE_DIVE_RIGHT
      2, // GOALIE_CENTRE
      2, // GOALIE_UNCENTRE
      0, // GOALIE_INITIAL
      0, // GOALIE_AFTERSIT_INITIAL
      2, // DEFENDER_CENTRE
      2, // GOALIE FAST SIT
      0, // MOTION_CALIBRATE
      0, // STAND_STRAIGHT
      0, // LINE_UP
   };

/**
 * Command for controlling the head
 */
   struct Head {
      float yaw;      // LEFT-RIGHT motion. Positive is LEFT
      float pitch;    // UP-DOWN angle. Positive is DOWN
      bool isRelative; // TRUE to add to current head angles [DEFAULT]
      float yawSpeed; // Speed of the yaw [0.0, 1.0]
      float pitchSpeed; // Speed of the pitch [0.0, 1.0]

      /**
       * Constructor
       * @param y Yaw amount (Left is positive) (rad)
       * @param p Pitch amount (Down is positive) (rad)
       * @param r Enable relative adjustment (default). False for absolute
       * @param ys Yaw speed [0.0, 1.0]
       * @param ps Pitch speed [0.0, 1.0]
       */
      Head(float y, float p = 0.0, bool r = true,
           float ys = 1.0, float ps = 1.0) : yaw(y),
                                             pitch(p),
                                             isRelative(r),
                                             yawSpeed(ys),
                                             pitchSpeed(ps) {}

      Head()
         : yaw(0.0),
           pitch(0.0),
           isRelative(true),
           yawSpeed(1.0),
           pitchSpeed(1.0) {}

      template<class Archive>
      void serialize(Archive &ar, const unsigned int file_version) {
         ar & yaw;
         ar & pitch;
         ar & isRelative;
         ar & yawSpeed;
         ar & pitchSpeed;
      }
   };

   struct rgb {
      bool red;
      bool green;
      bool blue;

      rgb(bool r = false, bool g = false, bool b = false) : red(r),
                                                            green(g),
                                                            blue(b) {}

      template<class Archive>
      void serialize(Archive &ar, const unsigned int file_version) {
         ar & red;
         ar & green;
         ar & blue;
      }
   };

   struct LED {

      uint16_t leftEar; // Number of left ear segments lit [10-bit field]
      uint16_t rightEar; // Number of right ear segments lit [10-bit field]
      rgb leftEye;     // Colour of left eye (default: white)
      rgb rightEye;    // Colour of right eye (default: white)
      rgb chestButton; // Colour of chest button (default: white)
      rgb leftFoot;    // Colour of left foot (default: off)
      rgb rightFoot;   // Colour of right foot (default: off)

      LED(rgb leye, rgb reye = rgb(true, true, true), rgb cb = rgb(true, true, true),
          rgb lf = rgb(), rgb rf = rgb()) : leftEar(0x3FF),
                                            rightEar(0x3FF),
                                            leftEye(leye),
                                            rightEye(reye),
                                            chestButton(cb),
                                            leftFoot(lf),
                                            rightFoot(rf) {}

      LED()
            : leftEar(0x3FF),
              rightEar(0x3FF),
              leftEye(rgb(true, true, true)),
              rightEye(rgb(true, true, true)),
              chestButton(rgb(true, true, true)),
              leftFoot(rgb()),
              rightFoot(rgb()) {}

      template<class Archive>
      void serialize(Archive &ar, const unsigned int file_version) {
         ar & leftEar;
         ar & rightEar;
         ar & leftEye;
         ar & rightEye;
         ar & chestButton;
         ar & leftFoot;
         ar & rightFoot;
      }
   };

   enum Stiffen
   {
      NONE = 0,
      STIFFEN
   };


/**
 * Wrapper for the other action commands, makes it easier to pass them around
 */
   struct All {
      Head head;
      Body body;
      LED leds;
      float sonar;
      Stiffen stiffen;

      All() : head(), body(Body::NONE), leds(),
               sonar(Sonar::Mode::NO_PING),
               stiffen(NONE)
      { }

      All(Head h, Body b, LED l, float s, Stiffen stf) {
         head = h;
         body = b;
         leds = l;
         sonar = s;
         stiffen = stf;
      }

      template<class Archive>
      void serialize(Archive &ar, const unsigned int file_version) {
         ar & head;
         ar & body;
         ar & leds;
         ar & sonar;
      }
   };

//  These classes support stream output for debugging
   static inline bool operator==(const rgb &a, const rgb &b) {
      return (a.red == b.red) && (a.green == b.green) && (a.blue == b.blue);
   }

   static inline bool operator!=(const rgb &a, const rgb &b) {
      return !(a == b);
   }

   static inline std::ostream & operator<<(std::ostream &out, const rgb &a) {
      out << '{' << a.red << ", " << a.green << ", " << a.blue << '}';
      return out;
   }

   static inline std::ostream & operator<<(std::ostream &out, const Head &a) {
      out << '[' << a.yaw << ", " << a.pitch << ", " << a.isRelative << ']';
      return out;
   }

   static inline std::ostream & operator<<(std::ostream &out, const Body &a) {
      out << '[' << a.actionType << ", " << a.forward << ", " << a.left
      << ", " << a.turn << "," << a.power << ']';
      return out;
   }

   static inline std::ostream & operator<<(std::ostream &out, const LED &a) {
      out << '[' << a.leftEar << ", " << a.rightEar << ", " << a.leftEye << ", "
      << a.rightEye << "," << a.chestButton << ","
      << a.leftFoot << "," << a.rightFoot << ']';
      return out;
   }
};  // namespace ActionCommand
