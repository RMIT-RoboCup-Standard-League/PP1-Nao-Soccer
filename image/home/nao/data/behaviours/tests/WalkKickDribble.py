import robot

import Constants
import Global
import actioncommand

from Task import BehaviourTask
from util import LedOverride


# Sample behaviour to demonstrate how behaviour to motion interface works
# Switches between walk on the spot, kick, and dribble
class WalkKickDribble(BehaviourTask):
    def init(self):
        self.isActive = False
        self.walkFrames = 0
        self.kickOrDribble = 0

    def tick(self):
        # this tells localisation that we are lining up and to give us a more
        # stable ball position
        if Global.ballRelPos().x < 300:
	    #Detect the ball and get its relative position to the robot
	    Log.info('Ball Position x:%s   y:%s',Global.ballRelPos().x,Global.ballRelPos().y)
            self.world.b_request.doingBallLineUp = True

        active = self.world.blackboard.motion.active.body.actionType
        request = self.world.b_request

        # walk on the spot for 2 seconds (behaviour runs at 30fps)
        if self.walkFrames < 60:
            self.walkFrames += 1
            self.walk()
        elif self.walkFrames == 60:
            if self.kickOrDribble == 0:
                if self.isActive and active != robot.ActionType.KICK:
                    # switch back to walk, dribble when walk finishes
                    self.kickOrDribble = 1
                    self.walkFrames = 0
                    self.walk()
                else:
                    request.actions.body = actioncommand.kick(
                        turn=Global.ballHeading()
                    )
                    LedOverride.override(LedOverride.leftEye,
                                         Constants.LEDColour.blue)
            else:
                if self.isActive and active != robot.ActionType.DRIBBLE:
                    # switch back to walk, kick when walk finishes
                    self.kickOrDribble = 0
                    self.walkFrames = 0
                    self.walk()
                else:
                    request.actions.body = actioncommand.dribble(
                        turn=Global.ballHeading()
                    )
                    LedOverride.override(LedOverride.leftEye,
                                         Constants.LEDColour.green)

        # motion will attempt to line up precisely to the ball before
        # performing kick or dribble. The active actionType will be LINE_UP
        # if still lining up.
        # Motion will set active actionType to KICK or DRIBBLE when finished
        # lining up, and performance kick or dribble.
        # When kick or dribble finishes, motion will set active actionType
        # back to WALK.
        if active in (robot.ActionType.KICK, robot.ActionType.DRIBBLE):
            self.isActive = True

    def walk(self):
        request = self.world.b_request
        request.actions.body = actioncommand.walk(turn=Global.ballHeading())
        LedOverride.override(LedOverride.leftEye, Constants.LEDColour.red)
        self.isActive = False
