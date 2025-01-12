//
// Copyright (C) 2016 David Eckhoff <david.eckhoff@fau.de>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
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

#include "veins/modules/application/traci/MyVeinsApp.h"
#include "rapidjson/writer.h"
#include "veins/modules/messages/MyMessage_m.h"

using namespace veins;

Define_Module(veins::MyVeinsApp);

void MyVeinsApp::initialize(int stage)
{
    DemoBaseApplLayer::initialize(stage);
    if (stage == 0) {
        // Initializing members and pointers of your application goes here
        EV << "Initializing " << par("appName").stringValue() << std::endl;

        lastPacketReceiveTime = simTime().inUnit(SimTimeUnit::SIMTIME_NS);
        packetsCountInBatch = par("packetsCountInBatch").intValue();
        attacker = (dblrand() <= par("attackerProbability").doubleValue());

        EV << packetsCountInBatch << endl;
        scheduleAt(simTime() + beaconInterval, sendBeaconEvt);
    } else if (stage == 1) {
        // Initializing members that require initialized other modules goes here
    }
}

void MyVeinsApp::finish()
{
    DemoBaseApplLayer::finish();
    // statistics recording goes here
}

void MyVeinsApp::onBSM(DemoSafetyMessage* bsm)
{
    // Your application has received a beacon message from another car or RSU
    // code for handling the message goes here
    EV_DEBUG << mac->getMACAddress() << ": onBSM" << endl;
    MyMessage* myMessage = check_and_cast<MyMessage*>(bsm);

    rapidjson::StringBuffer s;
    rapidjson::Writer<rapidjson::StringBuffer> writer(s);

    writer.StartObject();
    writer.Key("messageId");
    writer.Uint(myMessage->getId());
    writer.Key("senderAddress");
    writer.Uint(myMessage->getSenderAddress());
    writer.Key("receiverAddress");
    writer.Uint(mac->getMACAddress());
    writer.Key("creationTime");
    writer.Uint(myMessage->getCreationTime().inUnit(SimTimeUnit::SIMTIME_NS));
    writer.Key("sendingTime");
    writer.Uint(myMessage->getSendingTime().inUnit(SimTimeUnit::SIMTIME_NS));
    writer.Key("arrivalTime");
    writer.Uint(myMessage->getArrivalTime().inUnit(SimTimeUnit::SIMTIME_NS));
    writer.Key("duration");
    writer.Uint(myMessage->getDuration().inUnit(SimTimeUnit::SIMTIME_NS));
    writer.Key("bitLength");
    writer.Uint(myMessage->getBitLength());
    writer.Key("byteLength");
    writer.Uint(myMessage->getByteLength());
    writer.Key("consecutivePacketTime");
    writer.Uint(simTime().inUnit(SimTimeUnit::SIMTIME_NS) - lastPacketReceiveTime);
    writer.EndObject();

    std::ostringstream out_json; out_json << par("logsFileName").stdstringValue();
    logsFileName = out_json.str();

    std::ofstream out_stream;
    out_stream.open(logsFileName, std::ios_base::app);
    if(out_stream.is_open())
        out_stream << s.GetString() << std::endl;
    else
        EV_DEBUG << "Warning, logs file stream is closed";
    out_stream.close();
}

void MyVeinsApp::onWSM(BaseFrame1609_4* wsm)
{
    // Your application has received a data message from another car or RSU
    // code for handling the message goes here, see TraciDemo11p.cc for examples
}

void MyVeinsApp::onWSA(DemoServiceAdvertisment* wsa)
{
    // Your application has received a service advertisement from another car or RSU
    // code for handling the message goes here, see TraciDemo11p.cc for examples
}

void MyVeinsApp::handleSelfMsg(cMessage* msg)
{
    EV_DEBUG << mac->getMACAddress() << ": handleSelfMsg" << endl;

    switch (msg->getKind()) {
        case SEND_BEACON_EVT: {
            if (attacker) {
                for (int i = 0; i < packetsCountInBatch; i++) {
                    MyMessage* myMessage = new MyMessage();
                    populateWSM(myMessage);
                    myMessage->setSenderAddress(mac->getMACAddress());
                    myMessage->setMyData("Hello World!");
                    sendDown(myMessage);
                }
            } else {
                MyMessage* myMessage = new MyMessage();
                populateWSM(myMessage);
                myMessage->setSenderAddress(mac->getMACAddress());
                myMessage->setMyData("Hello World!");
                sendDown(myMessage);
            }
            scheduleAt(simTime() + beaconInterval, sendBeaconEvt);
            break;
        }
        case SEND_WSA_EVT: {
            DemoServiceAdvertisment* wsa = new DemoServiceAdvertisment();
            populateWSM(wsa);
            sendDown(wsa);
            scheduleAt(simTime() + wsaInterval, sendWSAEvt);
            break;
        }
        default: {
            if (msg) EV_WARN << "APP: Error: Got Self Message of unknown kind! Name: " << msg->getName() << endl;
            break;
        }
    }
        
}

void MyVeinsApp::handlePositionUpdate(cObject* obj)
{
    DemoBaseApplLayer::handlePositionUpdate(obj);
    // the vehicle has moved. Code that reacts to new positions goes here.
    // member variables such as currentPosition and currentSpeed are updated in the parent class
}
