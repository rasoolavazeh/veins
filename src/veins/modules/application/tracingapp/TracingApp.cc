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

#include "TracingApp.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace rapidjson;

Define_Module(TracingApp);

const void TracingApp::traceJSON(std::string file, std::string JSONObject) const {
    std::ofstream out_stream;
    out_stream.open(file, std::ios_base::app);
    if(out_stream.is_open())
        out_stream << JSONObject << std::endl;
    else
        EV_DEBUG << "Warning, tracing stream is closed";
    out_stream.close();
}

const int TracingApp::getMyID() const {
    return getParentModule()->getIndex(); //Car.ned's index.
}

const veins::Coord TracingApp::getMyPosition() const {
    return mobility->getPositionAt(simTime());
}

const double TracingApp::getMySpeed() const {
    return mobility->getSpeed();
}

const double TracingApp::getAngle() const {
    return mobility->getHeading().getRad();
}

void TracingApp::initialize(int stage) {
    DemoBaseApplLayer::initialize(stage);
    if (stage == 0) {
        //Initializing members and pointers of your application goes here
        EV << "Initializing " << par("appName").stringValue() << std::endl;
        std::ostringstream out_json; out_json << par("traceJSONFile").stdstringValue() << getMyID() << ".json";
        traceJSONFile = out_json.str();

        std::ostringstream out_gt_json; out_gt_json << par("traceGroundTruthJSONFile").stdstringValue() << getMyID() << ".json";
        traceGroundTruthJSONFile = out_gt_json.str();
    }
    else if (stage == 1) {
        //Initializing members that require initialized other modules goes here

    }
}

void TracingApp::setFileNames(std::string traceJSONFile, std::string traceGroundTruthJSONFile) {
    this->traceJSONFile = traceJSONFile;
    this->traceGroundTruthJSONFile = traceGroundTruthJSONFile;
}

void TracingApp::finish() {
    DemoBaseApplLayer::finish();
    //statistics recording goes here

}

void TracingApp::onBSM(veins::DemoSafetyMessage* bsm) {
    //Your application has received a beacon message from another car or RSU
    //code for handling the message goes here
    veins::Coord pos = bsm->getSenderPos();
    veins::Coord spd = bsm->getSenderSpeed();

    StringBuffer s;
    Writer<StringBuffer> writer(s);

    writer.StartObject();

    writer.Key("type");
    writer.Uint(TYPE_BEACON);
    writer.Key("rcvTime");
    writer.Double(simTime().dbl());
    writer.Key("sendTime");
    writer.Double(bsm->getTimestamp().dbl());
    writer.Key("sender");
    writer.Uint(bsm->getSenderModule()->getIndex());
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

//    writer.Key("RSSI");
//    writer.Double(bsm->getRSSI());

    writer.EndObject();

    traceJSON(traceJSONFile, s.GetString());
}

void TracingApp::onWSM(veins::BaseFrame1609_4* wsm) {
    //Your application has received a data message from another car or RSU
    //code for handling the message goes here, see TraciDemo11p.cc for examples
    EV << "onWSM" << endl;
}

void TracingApp::onWSA(veins::DemoServiceAdvertisment* wsa) {
    //Your application has received a service advertisement from another car or RSU
    //code for handling the message goes here, see TraciDemo11p.cc for examples
    EV << "onWSA" << endl;
}

void TracingApp::handleSelfMsg(cMessage* msg) {
    DemoBaseApplLayer::handleSelfMsg(msg);
    //this method is for self messages (mostly timers)
    //it is important to call the BaseWaveApplLayer function for BSM and WSM transmission
}

void TracingApp::populateWSM(veins::BaseFrame1609_4* wsm, veins::LAddress::L2Type rcvId, int serial){
    DemoBaseApplLayer::populateWSM(wsm, rcvId, serial);
    if(veins::DemoSafetyMessage* bsm = dynamic_cast<veins::DemoSafetyMessage*>(wsm)){
        veins::Coord pos = bsm->getSenderPos();
        veins::Coord spd = bsm->getSenderSpeed();
        
        StringBuffer s;
        Writer<StringBuffer> writer(s);

        writer.StartObject();

        writer.Key("type");
        writer.Uint(TYPE_TRUTH_BEACON);
        writer.Key("sendTime");
        writer.Double(bsm->getTimestamp().dbl());
        writer.Key("sender");
        writer.Uint(bsm->getSenderModuleId());
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

void TracingApp::handlePositionUpdate(cObject* obj) {
    DemoBaseApplLayer::handlePositionUpdate(obj);
    //the vehicle has moved. Code that reacts to new positions goes here.
    //member variables such as currentPosition and currentSpeed are updated in the parent class

    StringBuffer s;
    Writer<StringBuffer> writer(s);

    writer.StartObject();

    writer.Key("type");
    writer.Uint(TYPE_GPS);
    writer.Key("rcvTime");
    writer.Double(simTime().dbl());

    writer.Key("pos");
    writer.StartArray();
    writer.Double(curPosition.x);
    writer.Double(curPosition.y);
    writer.Double(curPosition.z);
    writer.EndArray();

    writer.Key("noise");
    writer.StartArray();
    writer.Double(0.0);
    writer.Double(0.0);
    writer.Double(0.0);
    writer.EndArray();

    writer.Key("spd");
    writer.StartArray();
    writer.Double(curSpeed.x);
    writer.Double(curSpeed.y);
    writer.Double(curSpeed.z);
    writer.EndArray();

    writer.Key("spd_noise");
    writer.StartArray();
    writer.Double(0.0);
    writer.Double(0.0);
    writer.Double(0.0);
    writer.EndArray();

    writer.EndObject();

    traceJSON(traceJSONFile, s.GetString());
}
