//****************************************************************************
//*  Copyright (c) Faculty of Engineering, University of Porto, Portugal     *
//*  All rights reserved                                                     *
//*                                                                          *
//*  Module: Mac ARounD Approach for Castalia Simulator                      *
//*  File: StaticGTS802154Around.h                                           *
//*  Date: 2015-03-05                                                        *
//*  Version:  0.1                                                           *
//*  Author(s): Erico Leão <ericoleao@gmail.com>                             *
//****************************************************************************/

#ifndef STATIC_GTS_802154_AROUND_H_
#define STATIC_GTS_802154_AROUND_H_

#include <string>
#include <vector>

#include "Basic802154Around.h"
#include "Basic802154AroundPacket_m.h"

class StaticGTS802154Around: public Basic802154Around {
 protected:
 	/*--- 802154Mac GTS list --- */
	vector<Basic802154AroundGTSspec> GTSlist;	// list of GTS specifications (for PAN coordinator)
	int assignedGTS,requestGTS,totalGTS,totalSlots,baseSlot,minCap,superframeOrder;
	bool gtsOnly;

	virtual void startup();
	virtual int gtsRequest_hub(Basic802154AroundPacket *);
	virtual void prepareBeacon_hub(Basic802154AroundPacket *);
	virtual void disconnectedFromPAN_node();
	virtual void assignedGTS_node(int);
	virtual bool acceptNewPacket(Basic802154AroundPacket *);
	virtual void transmissionOutcome(Basic802154AroundPacket *, bool, string);
};

#endif	//STATIC_GTS_802154_AROUND
