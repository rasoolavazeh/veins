/*
 * AttackerApp.h
 *
 *  Created on: 06.03.2017
 *      Author: Florian
 */


#pragma once

#include <omnetpp.h>
#include <veins/veins.h>
#include "veins/modules/application/tracingapp/TracingApp.h"
#include "veins/modules/application/ieee80211p/DemoBaseApplLayer.h"
#include "veins/base/utils/FindModule.h"
#include "veins/base/modules/BaseWorldUtility.h"

#define ATTACKER_TYPE_NO_ATTACKER 0
#define ATTACKER_TYPE_CONST_POSITION 1
#define ATTACKER_TYPE_DYNAMIC_POSITION 2
#define ATTACKER_TYPE_RANDOM_POSITION 4
#define ATTACKER_TYPE_RANDOM_DYNAMIC_POSITION 8
#define ATTACKER_TYPE_STAY_AT_POSITION 16
#define ATTACKER_TYPE_CONST_SPEED 32
#define ATTACKER_TYPE_RANDOM_DYNAMIC_SPEED 64

using namespace omnetpp;

class AttackerApp : public TracingApp
{
    private:
        static int attackerTypesCount;
        static std::vector<double> attackerTypeProbability;
        static long currentAttackerCount;
        static veins::BaseWorldUtility *world;
        bool attacker;
        double attackerPosRangeMin;
        double attackerPosRangeMax;
        double attackerSpeedRangeMin;
        double attackerSpeedRangeMax;
        veins::Coord position;
        bool positionInitialized;
        int attackerType;
        std::string traceJSONFile;
        std::string traceGroundTruthJSONFile;
        double stayAtPositionProbability;

    public:
        virtual void initialize(int stage) override;
    protected:
        virtual void handleSelfMsg(cMessage* msg) override;
        virtual void populateWSM(veins::BaseFrame1609_4* wsm, veins::LAddress::L2Type rcvId=0, int serial=0) override;
        virtual void handlePositionUpdate(cObject* obj) override;
        virtual void attackBSM(veins::DemoSafetyMessage* bsm);
        virtual void attackSetConstPosition(veins::DemoSafetyMessage* bsm);
        virtual void attackSetConstPosition(veins::DemoSafetyMessage* bsm, double xPos, double yPos);
        virtual void attackSetDynamicPosition(veins::DemoSafetyMessage* bsm);
        virtual void attackSetDynamicPosition(veins::DemoSafetyMessage* bsm, double xPos, double yPos);
        virtual void attackSetRandomPosition(veins::DemoSafetyMessage* bsm);
        virtual void attackSetRandomDynamicPosition(veins::DemoSafetyMessage* bsm);
        virtual void attackSetCurrentPosition(veins::DemoSafetyMessage* bsm);
        virtual void attackSetConstSpeed(veins::DemoSafetyMessage* bsm);
        virtual void attackSetConstSpeed(veins::DemoSafetyMessage* bsm, double xSpeed, double ySpeed);
        virtual void attackSetRandomDynamicSpeed(veins::DemoSafetyMessage* bsm);
        virtual void attackSetDynamicSpeed(veins::DemoSafetyMessage* bsm, double xSpeed, double ySpeed);
        virtual veins::Coord getRandomPosition();
        virtual veins::Coord getRandomPositionInRange();
        virtual veins::Coord getRandomSpeedInRange();
};

long AttackerApp::currentAttackerCount = 0;
veins::BaseWorldUtility* AttackerApp::world = NULL;
