//
// Copyright (C) 2016 David Eckhoff <david.eckhoff@fau.de>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

package org.car2x.veins.modules.application.attackerapp;
import org.car2x.veins.modules.application.tracingapp.TracingApp;

//
// network description file for your Veins Application. Add parameters here
//
simple AttackerApp extends TracingApp
{
    @class(AttackerApp);
    double attackerProbability = default(0);      // probability that a node is an attacker
                                                  // attackerProbability < 0: 1 attacker (with negative value as selection probability)
                                                  
    int attackerType = default(1);				  // specifies the attacker type
                                                  //
                                                  // 1:  sets the position in BSMs to a const value (can be specified in the parameters attackerXPos and attackerYPos)
                                                  // 2:  sets the position in BSMs to a dynamic value (newXPos = xPos + attackerXOffset, newYPos = yPos + attackerYOffset)
                                                  // 4:  calls 1 with a random position (on the playground)
                                                  // 8:  calls 2 with a random position (minOffset = attackerPosRangeMin, maxOffset = attackerPosRangeMax)
                                                  // 16: stays at the nodes first position and does not move
                                                  // 32: sets a constant speed
                                                  // 64: sets dynamic random position (similar to 8)
                                                  
    double attackerXPos = default(0);             // x coordinate of the attacker (in BSMs) (attacker type 1)
    double attackerYPos = default(0);             // y coordinate of the attacker (in BSMs) (attacker type 1)
    double attackerXOffset = default(150);        // x offset for attacker type 2
    double attackerYOffset = default(150);        // y offset for attacker type 2
    double attackerPosRangeMin = default(-300);   // min value for attacker type 8
    double attackerPosRangeMax = default(300);    // max value for attacker type 8
    double attackerXSpeed = default(0);             // x speed component of the attacker (in BSMs) (attacker type 32)
    double attackerYSpeed = default(0);             // y speed of the attacker (in BSMs) (attacker type 32)
    double attackerSpeedRangeMin = default(-300);   // min value for attacker type 8
    double attackerSpeedRangeMax = default(300);    // max value for attacker type 8
    double stayAtPositionIncrement = default(0.025);// increment that decides the probability that attacker 16 will stop (incremented at every local position update)
}
