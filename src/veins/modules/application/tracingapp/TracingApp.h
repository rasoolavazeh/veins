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

#ifndef __VEINS_MYVEINSAPP_H_
#define __VEINS_MYVEINSAPP_H_

#include <omnetpp.h>
#include "veins/modules/application/ieee80211p/BaseWaveApplLayer.h"

#define TRACING_SEPARATOR ','
#define TRACING_QUOTECHAR '\"'

using namespace omnetpp;

/**
 * @brief
 * This is an example file that tracks data and exports it in a format
 * that is compatible with Maat.
 *
 * @author Rens van der Heijden
 *
 */

#define TYPE_GPS 2
#define TYPE_BEACON 3
#define TYPE_TRUTH_BEACON 4

class TracingApp : public BaseWaveApplLayer {
    private:
        std::string traceJSONFile;
        std::string traceGroundTruthJSONFile;

    public:
        virtual void initialize(int stage);
        virtual void setFileNames(std::string traceJSONFile, std::string traceGroundTruthJSONFile);
        virtual void finish();

    protected:
        virtual void onBSM(BasicSafetyMessage* bsm);
        virtual void onWSM(WaveShortMessage* wsm);
        virtual void onWSA(WaveServiceAdvertisment* wsa);

        virtual void handleSelfMsg(cMessage* msg);
        virtual void populateWSM(WaveShortMessage* wsm, int rcvId, int serial);
        virtual void handlePositionUpdate(cObject* obj);

        virtual const int getMyID() const;
        virtual const Coord getMyPosition() const;
        virtual const double getMySpeed() const;
        virtual const double getAngle() const;

        virtual const void traceJSON(std::string file, std::string JSONObject) const;
    };

#endif
