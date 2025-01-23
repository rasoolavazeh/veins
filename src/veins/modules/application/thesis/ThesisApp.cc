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

#include "veins/modules/application/thesis/ThesisApp.h"
#include "rapidjson/writer.h"
#include "veins/modules/messages/thesis/NewSafetyMessage_m.h"

using namespace veins;

Define_Module(veins::ThesisApp);

void ThesisApp::initialize(int stage)
{
    DemoBaseApplLayer::initialize(stage);
    if (stage == 0) {
        // Initializing members and pointers of your application goes here
        EV << "Initializing " << par("appName").stringValue() << std::endl;

        ddosMessagesCount = par("ddosMessagesCount").intValue();
        attacker = (dblrand() <= par("attackerProbability").doubleValue());

        initCsvFile();
        scheduleAt(simTime() + beaconInterval, sendBeaconEvt);
    }
    else if (stage == 1) {
        // Initializing members that require initialized other modules goes here
    }
}

void ThesisApp::finish()
{
    DemoBaseApplLayer::finish();
    // statistics recording goes here
}

void ThesisApp::onBSM(DemoSafetyMessage* bsm)
{
    // Your application has received a beacon message from another car or RSU
    // code for handling the message goes here

    NewSafetyMessage* safetyMessage = check_and_cast<NewSafetyMessage*>(bsm);
    LAddress::L2Type senderAddress = safetyMessage->getSenderAddress();

    totalMessagesCountPerVehicle[senderAddress]++;
    totalMessagesLengthPerVehicle[senderAddress] += safetyMessage->getBitLength();

    if (lastReceivedMessageIntervalPerVehicle[senderAddress] == 0) {
        lastReceivedMessageIntervalPerVehicle[senderAddress] = simTime().inUnit(SimTimeUnit::SIMTIME_NS);
    } else {
        lastReceivedMessageIntervalPerVehicle[senderAddress] = simTime().inUnit(SimTimeUnit::SIMTIME_NS) - lastReceivedMessageIntervalPerVehicle[senderAddress];
    }

    appendToCsv(safetyMessage);
}

void ThesisApp::onWSM(BaseFrame1609_4* wsm)
{
    // Your application has received a data message from another car or RSU
    // code for handling the message goes here, see TraciDemo11p.cc for examples
}

void ThesisApp::onWSA(DemoServiceAdvertisment* wsa)
{
    // Your application has received a service advertisement from another car or RSU
    // code for handling the message goes here, see TraciDemo11p.cc for examples
}

void ThesisApp::handleSelfMsg(cMessage* msg)
{
    // DemoBaseApplLayer::handleSelfMsg(msg);
    // this method is for self messages (mostly timers)
    // it is important to call the DemoBaseApplLayer function for BSM and WSM transmission

    rapidjson::StringBuffer s;
    rapidjson::Writer<rapidjson::StringBuffer> writer(s);
    writer.StartObject();
    if (attacker) {
        writer.Key("attacker");
        writer.Uint(mac->getMACAddress());
    } else {
        writer.Key("normal");
        writer.Uint(mac->getMACAddress());
    }
    writer.EndObject();

   std::ostringstream out_json; out_json << "vehicleIds.json";

   std::ofstream out_stream;
   out_stream.open(out_json.str(), std::ios_base::app);
   if(out_stream.is_open())
       out_stream << s.GetString() << std::endl;
   else
       EV_DEBUG << "Warning, logs file stream is closed";
   out_stream.close();

    switch (msg->getKind()) {
        case SEND_BEACON_EVT: {
            if (attacker) {
                NewSafetyMessage* safetyMessage = new NewSafetyMessage();
                populateWSM(safetyMessage);
                safetyMessage->setSenderAddress(mac->getMACAddress());
                sendDown(safetyMessage);
                scheduleAt(simTime() + (beaconInterval / ddosMessagesCount), sendBeaconEvt);
            } else {
                NewSafetyMessage* safetyMessage = new NewSafetyMessage();
                populateWSM(safetyMessage);
                safetyMessage->setSenderAddress(mac->getMACAddress());
                sendDown(safetyMessage);
                scheduleAt(simTime() + beaconInterval, sendBeaconEvt);
            }
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

void ThesisApp::handlePositionUpdate(cObject* obj)
{
    DemoBaseApplLayer::handlePositionUpdate(obj);
    // the vehicle has moved. Code that reacts to new positions goes here.
    // member variables such as currentPosition and currentSpeed are updated in the parent class
}

void ThesisApp::initCsvFile()
{
    std::ostringstream out_json; out_json << par("csvFileName").stdstringValue() << ".csv";
    csvFileName = out_json.str();

    if (std::ifstream(csvFileName).good())
        return;

    std::ofstream out_stream;
    out_stream.open(csvFileName, std::ios_base::app);
    if(out_stream.is_open())
        out_stream << "messageId" << ", "
                   << "senderAddress" << ", "
                   << "receiverAddress" << ","
                   << "sendingTime" << ", "
                   << "receivingTime" << ", "
                   << "bitLength" << ", "
                   << "byteLength" << ", "
                   << "lastReceivedMessageInterval" << ", "
                   << "totalMessagesCount" << ", "
                   << "totalMessagesLength"
                   << std::endl;
    else
        EV_DEBUG << "Warning, logs file stream is closed";
    out_stream.close();
}

void ThesisApp::appendToCsv(NewSafetyMessage* msg)
{
    LAddress::L2Type senderAddress = msg->getSenderAddress();

    std::ofstream out_stream;
    out_stream.open(csvFileName, std::ios_base::app);
    if(out_stream.is_open())
        out_stream << msg->getId() << ", "
                   << senderAddress << ", "
                   << mac->getMACAddress() << ","
                   << msg->getSendingTime().inUnit(SimTimeUnit::SIMTIME_NS) << ", "
                   << msg->getArrivalTime().inUnit(SimTimeUnit::SIMTIME_NS) << ", "
                   << msg->getBitLength() << ", "
                   << msg->getByteLength() << ", "
                   << static_cast<long>(lastReceivedMessageIntervalPerVehicle[senderAddress]) << ", "
                   << static_cast<long>(totalMessagesCountPerVehicle[senderAddress]) << ", "
                   << static_cast<long>(totalMessagesLengthPerVehicle[senderAddress])
                   << std::endl;
    else
        EV_DEBUG << "Warning, logs file stream is closed";
    out_stream.close();
}
