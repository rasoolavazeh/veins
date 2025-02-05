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
        EV << "Initializing " << par("appName").stringValue() << endl;
        attackDetectorEnabled = par("attackDetectorEnabled").boolValue();
        ddosMessageInterval = par("ddosMessageInterval");
        attacker = (dblrand() <= par("attackerProbability").doubleValue());

        if (attackDetectorEnabled) {
            attackDetectorName = par("attackDetectorName").stdstringValue();
            initInterpreter();
        } else
            initCsvFile();
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

    if (firstMessageIsSendPerVehicle[senderAddress] == false) {
        firstMessageIsSendPerVehicle[senderAddress] = true;
    } else if (!attackDetectorEnabled) {
        appendToCsv(safetyMessage);
    }

    if (attackDetectorEnabled && isAttack(safetyMessage)) {
        EV_DEBUG << "attack" << endl;
        lastReceivedMessageTimePerVehicle[senderAddress] = safetyMessage->getArrivalTime().inUnit(SimTimeUnit::SIMTIME_MS);
        return;
    }

    EV_DEBUG << "normal" << endl;
    lastReceivedMessageTimePerVehicle[senderAddress] = safetyMessage->getArrivalTime().inUnit(SimTimeUnit::SIMTIME_MS);
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

    if (!attackDetectorEnabled)
        saveVehicle(mac->getMACAddress(), attacker);

    switch (msg->getKind()) {
        case SEND_BEACON_EVT: {
            NewSafetyMessage* safetyMessage = new NewSafetyMessage();
            populateWSM(safetyMessage);
            safetyMessage->setSenderAddress(mac->getMACAddress());
            sendDown(safetyMessage);

            if (attacker)
                scheduleAt(simTime() + ddosMessageInterval, sendBeaconEvt);
            else
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
        out_stream << "messageId" << ","
                   << "senderAddress" << ","
                   << "receiverAddress" << ","
                   << "sendingTime" << ","
                   << "receivingTime" << ","
                   << "bitLength" << ","
                   << "byteLength" << ","
                   << "lastReceivedMessageInterval" << ","
                   << "totalMessagesCount" << ","
                   << "totalMessagesLength"
                   << endl;
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
        out_stream << msg->getId() << ","
                   << senderAddress << ","
                   << mac->getMACAddress() << ","
                   << msg->getSendingTime().inUnit(SimTimeUnit::SIMTIME_MS) << ","
                   << msg->getArrivalTime().inUnit(SimTimeUnit::SIMTIME_MS) << ","
                   << msg->getBitLength() << ","
                   << msg->getByteLength() << ","
                   << msg->getArrivalTime().inUnit(SimTimeUnit::SIMTIME_MS) - static_cast<int64_t>(lastReceivedMessageTimePerVehicle[senderAddress]) << ","
                   << static_cast<long>(totalMessagesCountPerVehicle[senderAddress]) << ","
                   << static_cast<long>(totalMessagesLengthPerVehicle[senderAddress])
                   << endl;
    else
        EV_DEBUG << "Warning, logs file stream is closed";
    out_stream.close();
}

void ThesisApp::saveVehicle(LAddress::L2Type vehicleAddress, bool isAttacker)
{
    rapidjson::StringBuffer s;
    rapidjson::Writer<rapidjson::StringBuffer> writer(s);

    writer.StartObject();
    if (isAttacker) {
        writer.Key("attacker");
        writer.Uint(vehicleAddress);
    } else {
        writer.Key("normal");
        writer.Uint(vehicleAddress);
    }
    writer.EndObject();

    std::ostringstream out_json; out_json << "vehicles.json";
    std::ofstream out_stream;
    out_stream.open(out_json.str(), std::ios_base::app);
    if(out_stream.is_open())
        out_stream << s.GetString() << endl;
    else
        EV_DEBUG << "Warning, logs file stream is closed";
    out_stream.close();
}

void ThesisApp::initInterpreter()
{
    if (!Py_IsInitialized()) {
        EV << "INITIALIZE" << endl;
        EV << attackDetectorName << endl;
        EV << attackDetectorName.c_str() << endl;
        guard = new pybind11::scoped_interpreter();
        pybind11::module_ attackDetector = pybind11::module_::import(attackDetectorName.c_str());
        attackDetector.attr("initialize")();
    }
}

bool ThesisApp::isAttack(NewSafetyMessage* msg)
{
    EV << "BSM" << endl;
    EV << attackDetectorName << endl;
    EV << attackDetectorName.c_str() << endl;

    std::string features = getFeatures(msg);
    pybind11::module_ attackDetector = pybind11::module_::import(attackDetectorName.c_str());
    int result = attackDetector.attr("predict")(features).cast<int>();
    
    EV << result << endl;
    if (result)
        return true;
    return false;
}

std::string ThesisApp::getFeatures(NewSafetyMessage* msg)
{
    pybind11::module_ np = pybind11::module_::import("numpy");
    LAddress::L2Type senderAddress = msg->getSenderAddress();
    std::ostringstream oss;
    oss << msg->getId() << ","
        << senderAddress << ","
        << mac->getMACAddress() << ","
        << msg->getSendingTime().inUnit(SimTimeUnit::SIMTIME_MS) << ","
        << msg->getArrivalTime().inUnit(SimTimeUnit::SIMTIME_MS) << ","
        << simTime().inUnit(SimTimeUnit::SIMTIME_MS) - static_cast<int64_t>(lastReceivedMessageTimePerVehicle[senderAddress]) << ","
        << static_cast<long>(totalMessagesCountPerVehicle[senderAddress]) << ","
        << static_cast<long>(totalMessagesLengthPerVehicle[senderAddress]);
    return oss.str();
}
