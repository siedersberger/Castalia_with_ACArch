//****************************************************************************
//*  Copyright (c) Faculty of Engineering, University of Porto, Portugal     *
//*  All rights reserved                                                     *
//*                                                                          *
//*  Module: Mac ARounD Approach for Castalia Simulator                      *
//*  File: StaticGTS802154Around.cc                                          *
//*  Date: 2015-03-05                                                        *
//*  Version:  0.1                                                           *
//*  Author(s): Erico Leão <ericoleao@gmail.com>                             *
//****************************************************************************/

#include "StaticGTS802154Around.h"

Define_Module(StaticGTS802154Around);

/***
 * Initialising some parameters, specific to Static GTS module
 * by overriding the startup() method. Important to call startup()
 * of the parent module in the end, otherwise it will not initialize
 ***/
void StaticGTS802154Around::startup() {
	// initialise GTS-specific parameters
	GTSlist.clear(); totalGTS = 0; assignedGTS = 0;
	requestGTS = par("requestGTS");
	gtsOnly = par("gtsOnly");

	// other parameters are from Basic802154, need to read them for GTS scheduling
	totalSlots = par("aNumSuperframeSlots"); 	
	baseSlot = par("aBaseSlotDuration");
	minCap = par("aMinCAPLength");
	superframeOrder = par("superframeOrder");
	return Basic802154Around::startup();
}

/***
 * GTS request received by hub, need to return the number of 
 * slots to be granted. Can return 0 to indicate request denial
 ***/
int StaticGTS802154Around::gtsRequest_hub(Basic802154AroundPacket *gtsPkt) {
	//Length of CAP after lengths of all GTS slots are subtracted
	int CAPlength = totalSlots;
	
	//check if the node already exists in the GTS list
	vector<Basic802154AroundGTSspec>::iterator i;
	int total = 0;
	for (i = GTSlist.begin(); i != GTSlist.end(); i++) {
		total++;
		if (i->owner == gtsPkt->getSrcID()) {
			if (i->length == gtsPkt->getGTSlength()) {
				return i->length;
			} else {
				totalGTS -= i->length;
				GTSlist.erase(i);
				total--;
			}
		} else {
			CAPlength -= i->length;
		}
	}
	
	//node not found, or requested slots changed
	if (total >= 7 || (CAPlength - gtsPkt->getGTSlength()) *
	    baseSlot * (1 << superframeOrder) < minCap) {
		trace() << "GTS request from " << gtsPkt->getSrcID() <<
		    " cannot be acocmodated";
		return 0;
	}
	
	Basic802154AroundGTSspec newGTSspec;
	newGTSspec.length = gtsPkt->getGTSlength();
	totalGTS += newGTSspec.length;
	newGTSspec.owner = gtsPkt->getSrcID();
	GTSlist.push_back(newGTSspec);
	return newGTSspec.length;
}

/***
 * Hub can alter the beacon before broadcasting it
 * In particular, assign GTS slots and set CAP length
 ***/
void StaticGTS802154Around::prepareBeacon_hub(Basic802154AroundPacket *beaconPacket) {
	int CAPlength = totalSlots;
	beaconPacket->setGTSlistArraySize(GTSlist.size());
	for (int i = 0; i < (int)GTSlist.size(); i++) {
		if (CAPlength > GTSlist[i].length) {
			CAPlength -= GTSlist[i].length;
			GTSlist[i].start = CAPlength + 1;
			beaconPacket->setGTSlist(i, GTSlist[i]);
		} else {
			trace() << "Internal ERROR: GTS list corrupted";
			GTSlist.clear(); totalGTS = 0;
			beaconPacket->setGTSlistArraySize(0);	
			CAPlength = totalSlots;
			break;
		}
	}
	beaconPacket->setCAPlength(CAPlength);
}

/***
 * If disconnected from PAN, also need to reset GTS slots
 ***/
void StaticGTS802154Around::disconnectedFromPAN_node() {
	assignedGTS = 0;
}

/***
 * GTS request was successful
 ***/
void StaticGTS802154Around::assignedGTS_node(int slots) {
	assignedGTS = slots;
}

/***
 * Transmission of data packet requested earlier is complete
 * status string holds comma separated list of outcomes
 * for each transmission attempt
 ***/
void StaticGTS802154Around::transmissionOutcome(Basic802154AroundPacket *pkt, bool success, string status) {
	if (getAssociatedPAN() != -1) {
		if (assignedGTS == 0 && requestGTS > 0) {
			transmitPacket(newGtsRequest(getAssociatedPAN(), requestGTS));
		} else if (TXBuffer.size()) {
			Basic802154AroundPacket *packet = check_and_cast<Basic802154AroundPacket*>(TXBuffer.front());
			TXBuffer.pop();
			transmitPacket(packet,0,gtsOnly);
		}
	}
}

bool StaticGTS802154Around::acceptNewPacket(Basic802154AroundPacket *newPacket)
{
	if (getAssociatedPAN() != -1 && getCurrentPacket() == NULL) {
		transmitPacket(newPacket,0,gtsOnly);
		return true;
	}
	return bufferPacket(newPacket);
}

/***
 * Timers can be accessed by overwriting timerFiredCallback
 **/
/*
void StaticGTS802154Around::timerFiredCallback(int index) {
	switch(index) {
		case NEW_TIMER: {
			//do something
			break;
		}
		
		default: {
			//important to call the function of the parent module
			Basic802154::timerFiredCallback(index);
		}
	}
}
*/
