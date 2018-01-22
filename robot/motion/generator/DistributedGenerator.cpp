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

#include "motion/generator/DistributedGenerator.hpp"
#include "motion/generator/ActionGenerator.hpp"
#include "motion/generator/DeadGenerator.hpp"
#include "motion/generator/HeadGenerator.hpp"
#include "motion/generator/NullGenerator.hpp"
#include "motion/generator/RefPickupGenerator.hpp"
#include "motion/generator/WalkEnginePreProcessor.hpp"
#ifdef SIMULATION
    typedef ActionGenerator GetupGenerator;
#else
    #include "motion/generator/GetupGenerator.hpp"
#endif

#include "utils/body.hpp"
#include "utils/Logger.hpp"

using ActionCommand::Body;
using boost::program_options::variables_map;

/*-----------------------------------------------------------------------------
 * Distributed Generator
 * ---------------------
 * This generator switches between all required generators as requested.
 *---------------------------------------------------------------------------*/
DistributedGenerator::DistributedGenerator(Blackboard *bb)
   : isStopping(false),
     current_generator(Body::NONE),
     prev_generator(Body::NONE),
     requestedDive(Body::NONE) {

   headGenerator = (Generator*)(new HeadGenerator());
   if (!headGenerator)
      llog(FATAL) << "headGenerator is NULL!" << std::endl;

   // TODO(dpad): Rewrite these ugly llogs to simply loop through bodyGenerators
   // and print out the string name
   bodyGenerators[Body::NONE] = (Generator*)(new NullGenerator());
   if (!bodyGenerators[Body::NONE])
      llog(FATAL) << "bodyGenerators[NONE] is NULL!" << std::endl;

   bodyGenerators[Body::STAND] = (Generator*)(new ActionGenerator("stand"));
   if (!bodyGenerators[Body::STAND])
      llog(FATAL) << "bodyGenerators[STAND] is NULL!" << std::endl;

   bodyGenerators[Body::MOTION_CALIBRATE] =
           (Generator*)(new ActionGenerator("standStraight"));
   if (!bodyGenerators[Body::MOTION_CALIBRATE])
      llog(FATAL) << "bodyGenerators[MOTION_CALIBRATE] is NULL!" << std::endl;

   bodyGenerators[Body::STAND_STRAIGHT] =
           (Generator*)(new ActionGenerator("standStraight"));
   if (!bodyGenerators[Body::STAND_STRAIGHT])
      llog(FATAL) << "bodyGenerators[STAND_STRAIGHT] is NULL!" << std::endl;

   bodyGenerators[Body::WALK] = (Generator*)(new WalkEnginePreProcessor(bb));
   if (!bodyGenerators[Body::WALK])
      llog(FATAL) << "bodyGenerators[WALK] is NULL!" << std::endl;

   bodyGenerators[Body::KICK] = bodyGenerators[Body::WALK];

   bodyGenerators[Body::LINE_UP] = bodyGenerators[Body::WALK];

   bodyGenerators[Body::DRIBBLE] = bodyGenerators[Body::WALK];

   bodyGenerators[Body::TURN_DRIBBLE] = bodyGenerators[Body::WALK];

   bodyGenerators[Body::GETUP_FRONT] = (Generator*)
                                       (new GetupGenerator("FRONT"));
   if (!bodyGenerators[Body::GETUP_FRONT])
      llog(FATAL) << "bodyGenerators[GETUP_FRONT] is NULL!" << std::endl;

   bodyGenerators[Body::GETUP_BACK] = (Generator*)
                                       (new GetupGenerator("BACK"));
   if (!bodyGenerators[Body::GETUP_BACK])
      llog(FATAL) << "bodyGenerators[GETUP_BACK] is NULL!" << std::endl;

   bodyGenerators[Body::TIP_OVER] = (Generator*)
                                       (new ActionGenerator("tipOver"));
   if (!bodyGenerators[Body::TIP_OVER])
      llog(FATAL) << "bodyGenerators[TIP_OVER] is NULL!" << std::endl;

   bodyGenerators[Body::INITIAL] = (Generator*)
                                   (new ActionGenerator("initial"));
   if (!bodyGenerators[Body::INITIAL])
      llog(FATAL) << "bodyGenerators[INITIAL] is NULL!" << std::endl;
   bodyGenerators[Body::DEAD] = (Generator*)(new DeadGenerator());
   if (!bodyGenerators[Body::DEAD])
      llog(FATAL) << "bodyGenerators[DEAD] is NULL!" << std::endl;

   bodyGenerators[Body::REF_PICKUP] = (Generator*)(new RefPickupGenerator());
   if (!bodyGenerators[Body::REF_PICKUP])
      llog(FATAL) << "bodyGenerators[REF_PICKUP] is NULL!" << std::endl;

   bodyGenerators[Body::GOALIE_SIT] = (Generator*)
                                      (new ActionGenerator("goalieSit"));
   if (!bodyGenerators[Body::GOALIE_SIT])
      llog(FATAL) << "bodyGenerators[GOALIE_SIT] is NULL!" << std::endl;

   bodyGenerators[Body::GOALIE_DIVE_LEFT] = (Generator*)
                                            (new ActionGenerator("goalieDiveLeft"));
   if (!bodyGenerators[Body::GOALIE_DIVE_LEFT])
      llog(FATAL) << "bodyGenerators[GOALIE_DIVE_LEFT] is NULL!" << std::endl;

   bodyGenerators[Body::GOALIE_DIVE_RIGHT] = (Generator*)
                                             (new ActionGenerator("goalieDiveRight"));
   if (!bodyGenerators[Body::GOALIE_DIVE_RIGHT])
      llog(FATAL) << "bodyGenerators[GOALIE_DIVE_RIGHT] is NULL!" << std::endl;

   bodyGenerators[Body::GOALIE_CENTRE] = (Generator*)
                                             (new ActionGenerator("defenderSquat")); //goalieCentre
   if (!bodyGenerators[Body::GOALIE_CENTRE])
      llog(FATAL) << "bodyGenerators[GOALIE_CENTRE] is NULL!" << std::endl;

   bodyGenerators[Body::GOALIE_UNCENTRE] = (Generator*)
                                             (new ActionGenerator("goalieUncentre"));
   if (!bodyGenerators[Body::GOALIE_UNCENTRE])
      llog(FATAL) << "bodyGenerators[GOALIE_UNCENTRE] is NULL!" << std::endl;

   bodyGenerators[Body::GOALIE_INITIAL] = (Generator*)
                                             (new ActionGenerator("goalieInitial"));
   if (!bodyGenerators[Body::GOALIE_INITIAL])
      llog(FATAL) << "bodyGenerators[GOALIE_INITIAL] is NULL!" << std::endl;

   bodyGenerators[Body::GOALIE_AFTERSIT_INITIAL] = (Generator*)
                                             (new ActionGenerator("goalieInitial"));
   if (!bodyGenerators[Body::GOALIE_AFTERSIT_INITIAL])
      llog(FATAL) << "bodyGenerators[GOALIE_AFTERSIT_INITIAL] is NULL!" << std::endl;

   bodyGenerators[Body::DEFENDER_CENTRE] = (Generator*)
                                          (new ActionGenerator("defenderCentre"));
   if (!bodyGenerators[Body::DEFENDER_CENTRE])
     llog(FATAL) << "bodyGenerators[DEFENDER_CENTRE] is NULL!" << std::endl;

   bodyGenerators[Body::GOALIE_FAST_SIT] = (Generator*)
                                          (new ActionGenerator("goalieFastSit"));
   if (!bodyGenerators[Body::GOALIE_FAST_SIT])
     llog(FATAL) << "bodyGenerators[GOALIE_FAST_SIT] is NULL!" << std::endl;

   

   llog(INFO) << "DistributedGenerator constructed" << std::endl;
}

/*-----------------------------------------------------------------------------
 * Destructor
 *---------------------------------------------------------------------------*/
DistributedGenerator::~DistributedGenerator() {
   delete headGenerator;
   for (uint8_t i = 0; i < Body::NUM_ACTION_TYPES; ++i)
      if (bodyGenerators[i]) {
         delete bodyGenerators[i];
         for (uint8_t j = i + 1; j < Body::NUM_ACTION_TYPES; ++j)
            if (bodyGenerators[j] == bodyGenerators[i])
               bodyGenerators[j] = NULL;
      }
   llog(INFO) << "DistributedGenerator destroyed" << std::endl;
}

/*-----------------------------------------------------------------------------
 * makeJoints
 * Returns the joint values requested by whichever generator we're using
 *---------------------------------------------------------------------------*/
JointValues DistributedGenerator::makeJoints(ActionCommand::All* request,
                                             Odometry* odometry,
                                             const SensorValues &sensors,
                                             BodyModel &bodyModel,
                                             float ballX,
                                             float ballY) {

   // If we're requesting a dive, set requestedDive variable
   if(requestedDive == Body::NONE
         && !(
            current_generator == Body::GOALIE_CENTRE ||
            current_generator == Body::GOALIE_DIVE_LEFT ||
            current_generator == Body::GOALIE_DIVE_RIGHT ||
            current_generator == Body::DEFENDER_CENTRE
            )
         && (
            request->body.actionType == Body::GOALIE_CENTRE ||
            request->body.actionType == Body::GOALIE_DIVE_LEFT ||
            request->body.actionType == Body::GOALIE_DIVE_RIGHT ||
            current_generator == Body::DEFENDER_CENTRE
            )) {
      requestedDive = request->body.actionType;
   }

   // If we're in the dead state, and we aren't transitioning into one of the
   // getup/fallen states, ensure that we do not shoot up to standing by first going
   // to a crouching/ref pickup state, else the motors may experience overdrive

   if (prev_generator == Body::DEAD) {
      if (!(request->body.actionType == Body::GETUP_FRONT 
         || request->body.actionType == Body::GETUP_BACK
         || request->body.actionType == Body::TIP_OVER)) {
         request->body.actionType = Body::REF_PICKUP;
      }
   }

   JointValues fromBody;
   bool usesHead = false;

   // Check the priority of the requested action compared to the current action
   if (ActionCommand::priorities[request->body.actionType] >
       ActionCommand::priorities[current_generator]) {
      reset();
      isStopping = false;
   }

   if (!bodyGenerators[current_generator]->isActive()) {
      if (bodyGenerators[current_generator] != bodyGenerators[request->body.actionType]
            || isStopping
            || (current_generator == Body::GETUP_FRONT && request->body.actionType == Body::GETUP_FRONT)
            || (current_generator == Body::GETUP_BACK && request->body.actionType == Body::GETUP_BACK)) {
         bodyGenerators[current_generator]->reset();
      }

      current_generator = request->body.actionType;
      isStopping = false;
   } else if (bodyGenerators[current_generator]->isActive() &&
              bodyGenerators[current_generator] !=
              bodyGenerators[request->body.actionType]) {
      // Special case to let kicks continue instead of being interrupted by stand
      if (current_generator != Body::KICK || request->body.actionType != Body::STAND) {
         bodyGenerators[current_generator]->stop();
         isStopping = true;
      }
   }

   if(current_generator == requestedDive){
      requestedDive = Body::NONE;
   }

   switch (current_generator) {
   case Body::NONE:             usesHead = false; break;
   case Body::STAND:            usesHead = false; break;
   case Body::WALK:             usesHead = false; break;
   case Body::GETUP_FRONT:      usesHead = true;  break;
   case Body::GETUP_BACK:       usesHead = true;  break;
   case Body::TIP_OVER:         usesHead = true;  break;
   case Body::INITIAL:          usesHead = true;  break;
   case Body::KICK:             usesHead = false; break;
   case Body::DRIBBLE:          usesHead = false; break;
   case Body::TURN_DRIBBLE:     usesHead = false; break;
   case Body::DEAD:             usesHead = true;  break;
   case Body::REF_PICKUP:       usesHead = false; break;
   case Body::GOALIE_SIT:       usesHead = true; break;
   case Body::GOALIE_FAST_SIT:  usesHead = true; break;
   case Body::GOALIE_DIVE_LEFT: usesHead = true; break;
   case Body::GOALIE_DIVE_RIGHT: usesHead = true; break;
   case Body::GOALIE_CENTRE:    usesHead = true; break;
   case Body::GOALIE_UNCENTRE:  usesHead = true; break;
   case Body::GOALIE_INITIAL:   usesHead = true; break;
   case Body::GOALIE_AFTERSIT_INITIAL: usesHead = true; break;
   case Body::DEFENDER_CENTRE:  usesHead = false; break;
   case Body::MOTION_CALIBRATE: usesHead = false; break;
   case Body::STAND_STRAIGHT:   usesHead = false; break;
   case Body::LINE_UP:          usesHead = false; break;
   case Body::NUM_ACTION_TYPES: usesHead = false; break;
   }
   //check for dives and update odometry
   float turn = 0;
   int dir = 0;
   if(current_generator == Body::GETUP_FRONT){
      dir = 1;
   } else if(current_generator == Body::GETUP_BACK){
      dir = -1;
   }
   if(prev_generator == Body::GOALIE_DIVE_LEFT){
      turn = DEG2RAD(dir*80);
   } else if (prev_generator == Body::GOALIE_DIVE_RIGHT){
      turn = DEG2RAD(-dir*80);
   }
   *odometry = *odometry + Odometry(0, 0, turn);

   // Robot will not stiffen without this
   fromBody = bodyGenerators[current_generator]->
      makeJoints(request, odometry, sensors, bodyModel, ballX, ballY);

   if(current_generator == Body::KICK && request->body.actionType == Body::WALK) {
      current_generator = Body::WALK;
   }
   if (!usesHead) {
      JointValues fromHead = headGenerator->
                             makeJoints(request, odometry, sensors, bodyModel, ballX, ballY);
      for (uint8_t i = Joints::HeadYaw; i <= Joints::HeadPitch; ++i) {
         fromBody.angles[i] = fromHead.angles[i];
         fromBody.stiffnesses[i] = fromHead.stiffnesses[i];
      }
   }
   prev_generator = current_generator;
   return fromBody;
}

bool DistributedGenerator::isActive() {
   return true;
}

void DistributedGenerator::reset() {
   for (uint8_t i = 0; i < Body::NUM_ACTION_TYPES; ++i) {
      bodyGenerators[i]->reset();
   }
   headGenerator->reset();
   current_generator = ActionCommand::Body::NONE;
}

void DistributedGenerator::readOptions(const boost::program_options::variables_map &config) {
   for (uint8_t i = 0; i < Body::NUM_ACTION_TYPES; ++i) {
      bodyGenerators[i]->readOptions(config);
   }
   headGenerator->readOptions(config);
}
