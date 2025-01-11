#include "AttackerApp.h"

#include <cmath> // std::abs

#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace rapidjson;

Define_Module(AttackerApp);

void AttackerApp::initialize(int stage) {
    TracingApp::initialize(stage);
    if (stage == 0) {
        // initialize
        EV << "Initializing attacker" << std::endl;

        attacker = false;
        attackerType = ATTACKER_TYPE_NO_ATTACKER;
        positionInitialized = false;
        stayAtPositionProbability = 0.0;

        attackerPosRangeMin = par("attackerPosRangeMin").doubleValue();
        attackerPosRangeMax = par("attackerPosRangeMax").doubleValue();

        attackerSpeedRangeMin = par("attackerSpeedRangeMin").doubleValue();
        attackerSpeedRangeMax = par("attackerSpeedRangeMax").doubleValue();

        double attackerProbability = par("attackerProbability").doubleValue();
        if((attackerProbability < 0 && currentAttackerCount == 0) // < 0 means *exactly one* attacker.
                    || (attackerProbability > 0)) {
            attacker = (dblrand() <= std::abs(attackerProbability));
            if(attacker) {
                attackerType = par("attackerType");
                currentAttackerCount++;
            }
        }

        std::ostringstream out_json; out_json << par("traceJSONFile").stdstringValue() << getMyID() << "-" << getParentModule()->getId()<< "-A" << attackerType << ".json";
        traceJSONFile = out_json.str();
        std::ostringstream out_gt_json; out_gt_json << par("traceGroundTruthJSONFile").stdstringValue() << ".json";
        traceGroundTruthJSONFile = out_gt_json.str();
        TracingApp::setFileNames(traceJSONFile, traceGroundTruthJSONFile);
    }
}

void AttackerApp::handleSelfMsg(cMessage* msg) {
    // code is based on veins::DemoBaseApplLayer::handleSelfMsg(cMessage* msg)
    switch (msg->getKind()) {
        case SEND_BEACON_EVT: {
            veins::DemoSafetyMessage* bsm = new veins::DemoSafetyMessage();
            populateWSM(bsm);

            // attacker
            if (attacker) {
                attackBSM(bsm);
            }

            sendDown(bsm);
            scheduleAt(simTime() + beaconInterval, sendBeaconEvt);
            break;
        }
        case SEND_WSA_EVT:   {
            veins::DemoServiceAdvertisment* wsa = new veins::DemoServiceAdvertisment();
            populateWSM(wsa);

            // add attacks for WSA messages here (currently no attacks)

            sendDown(wsa);
            scheduleAt(simTime() + wsaInterval, sendWSAEvt);
            break;
        }
        default: {
            if (msg)
                EV_DEBUG << "APP: Error: Got Self Message of unknown kind! Name: " << msg->getName() << endl;
            break;
        }
    }
}

// code is based on TracingApp::populateWSM(veins::BaseFrame1609_4* wsm, int rcvId, int serial)
void AttackerApp::populateWSM(veins::BaseFrame1609_4* wsm, veins::LAddress::L2Type rcvId, int serial) {
    veins::DemoBaseApplLayer::populateWSM(wsm, rcvId, serial);
    if (veins::DemoSafetyMessage* bsm = dynamic_cast<veins::DemoSafetyMessage*>(wsm)) {
        veins::Coord pos = bsm->getSenderPos();
        veins::Coord spd = bsm->getSenderSpeed();

        StringBuffer s;
        Writer<StringBuffer> writer(s);

        writer.StartObject();

        writer.Key("type");
        writer.Uint(TYPE_TRUTH_BEACON);
        writer.Key("time");
        writer.Double(simTime().dbl());
//        writer.Key("sender");
//        writer.Uint(bsm->getSenderModule()->getId());
        writer.Key("attackerType");
        writer.Uint(attackerType);
        writer.Key("messageID");
        writer.Uint(bsm->getTreeId());

        writer.Key("pos");
        writer.StartArray();
        writer.Double(pos.x);
        writer.Double(pos.y);
        writer.Double(pos.z);
        writer.EndArray();

        writer.Key("pos_noise");
        writer.StartArray();
        writer.Double(0.0);
        writer.Double(0.0);
        writer.Double(0.0);
        writer.EndArray();

        writer.Key("spd");
        writer.StartArray();
        writer.Double(spd.x);
        writer.Double(spd.y);
        writer.Double(spd.z);
        writer.EndArray();

        writer.Key("spd_noise");
        writer.StartArray();
        writer.Double(0.0);
        writer.Double(0.0);
        writer.Double(0.0);
        writer.EndArray();

        writer.EndObject();

        traceJSON(traceGroundTruthJSONFile, s.GetString());
    }
}

void AttackerApp::handlePositionUpdate(cObject* obj) {
    TracingApp::handlePositionUpdate(obj);
    if((attackerType == ATTACKER_TYPE_STAY_AT_POSITION) && !positionInitialized) {
        stayAtPositionProbability += par("stayAtPositionIncrement").doubleValue();
    }
}

//TODO: implement more attacker
void AttackerApp::attackBSM(veins::DemoSafetyMessage* bsm) {
    switch(attackerType)
    {
    case ATTACKER_TYPE_CONST_POSITION:
        attackSetConstPosition(bsm);
        break;
    case ATTACKER_TYPE_DYNAMIC_POSITION:
        attackSetDynamicPosition(bsm);
        break;
    case ATTACKER_TYPE_RANDOM_POSITION:
        attackSetRandomPosition(bsm);
        break;
    case ATTACKER_TYPE_RANDOM_DYNAMIC_POSITION:
        attackSetRandomDynamicPosition(bsm);
        break;
    case ATTACKER_TYPE_STAY_AT_POSITION:
        attackSetCurrentPosition(bsm);
        break;
    case ATTACKER_TYPE_CONST_SPEED:
        attackSetConstSpeed(bsm);
        break;
    case ATTACKER_TYPE_RANDOM_DYNAMIC_SPEED:
        attackSetRandomDynamicSpeed(bsm);
        break;
    default:
        EV_DEBUG << "Unknown attacker type! Type: " << attackerType << endl;
    }
}

void AttackerApp::attackSetConstSpeed(veins::DemoSafetyMessage* bsm) {
    attackSetConstSpeed(bsm, par("attackerXSpeed").doubleValue(), par("attackerYSpeed").doubleValue());
}

void AttackerApp::attackSetConstSpeed(veins::DemoSafetyMessage* bsm, double xSpeed, double ySpeed) {
    EV_DEBUG << "Attack: SetConstSpeed (x=" << xSpeed << ", y=" << ySpeed << ")" << std::endl;
    bsm->setSenderSpeed(veins::Coord(xSpeed, ySpeed, (bsm->getSenderSpeed()).z));
}

void AttackerApp::attackSetDynamicSpeed(veins::DemoSafetyMessage* bsm, double xSpeed, double ySpeed) {
    double newXSpeed = (bsm->getSenderSpeed()).x + xSpeed;
    double newYSpeed = (bsm->getSenderSpeed()).y + ySpeed;

    EV_DEBUG << "Attack: SetDynamicSpeed (x=" << newXSpeed << ", y=" << newYSpeed << ")" << std::endl;
    bsm->setSenderSpeed(veins::Coord(newXSpeed, newYSpeed, (bsm->getSenderSpeed()).z));
}

void AttackerApp::attackSetRandomDynamicSpeed(veins::DemoSafetyMessage* bsm) {
    veins::Coord randomSpeedInRange = getRandomSpeedInRange();
    attackSetDynamicSpeed(bsm, randomSpeedInRange.x, randomSpeedInRange.y);
}

void AttackerApp::attackSetConstPosition(veins::DemoSafetyMessage* bsm) {
    attackSetConstPosition(bsm, par("attackerXPos").doubleValue(), par("attackerYPos").doubleValue());
}

void AttackerApp::attackSetConstPosition(veins::DemoSafetyMessage* bsm, double xPos, double yPos) {
    EV << "Attack: SetConstPosition (x=" << xPos << ", y=" << yPos << ") -> " << bsm->getName() << std::endl;
    bsm->setSenderPos(veins::Coord(xPos, yPos, (bsm->getSenderPos()).z));
}

void AttackerApp::attackSetDynamicPosition(veins::DemoSafetyMessage* bsm) {
    attackSetDynamicPosition(bsm, par("attackerXOffset").doubleValue(), par("attackerYOffset").doubleValue());
}

void AttackerApp::attackSetDynamicPosition(veins::DemoSafetyMessage* bsm, double xPos, double yPos) {
    double newXPos = (bsm->getSenderPos()).x + xPos;
    double newYPos = (bsm->getSenderPos()).y + yPos;

    EV_DEBUG << "Attack: SetDynamicPosition (x=" << newXPos << ", y=" << newYPos << ")" << std::endl;
    bsm->setSenderPos(veins::Coord(newXPos, newYPos, (bsm->getSenderPos()).z));
}

void AttackerApp::attackSetRandomPosition(veins::DemoSafetyMessage* bsm){
    veins::Coord randomPosition = getRandomPosition();
    attackSetConstPosition(bsm, randomPosition.x, randomPosition.y);
}

void AttackerApp::attackSetRandomDynamicPosition(veins::DemoSafetyMessage* bsm) {
    veins::Coord randomPositionInRange = getRandomPositionInRange();
    attackSetDynamicPosition(bsm, randomPositionInRange.x, randomPositionInRange.y);
}

void AttackerApp::attackSetCurrentPosition(veins::DemoSafetyMessage* bsm) {
    if(positionInitialized) {
        bsm->setSenderPos(position);
    } else {
        if(dblrand() <= std::abs(stayAtPositionProbability)) {
            position = veins::Coord(bsm->getSenderPos());
            positionInitialized = true;
        }
    }
}

veins::Coord AttackerApp::getRandomSpeedInRange() {
    return veins::Coord(uniform(attackerSpeedRangeMin, attackerSpeedRangeMax), uniform(attackerSpeedRangeMin, attackerSpeedRangeMax));
}

veins::Coord AttackerApp::getRandomPosition() {
    if(world == NULL) {
        world = veins::FindModule<veins::BaseWorldUtility*>::findGlobalModule();
    }
    return world->getRandomPosition();
}

veins::Coord AttackerApp::getRandomPositionInRange() {
    return veins::Coord(uniform(attackerPosRangeMin, attackerPosRangeMax), uniform(attackerPosRangeMin, attackerPosRangeMax));
}
