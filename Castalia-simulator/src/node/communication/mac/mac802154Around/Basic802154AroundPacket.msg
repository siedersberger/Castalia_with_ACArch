//****************************************************************************
//*  Copyright (c) Faculty of Engineering, University of Porto, Portugal     *
//*  All rights reserved                                                     *
//*                                                                          *
//*  Module: Mac ARounD Approach for Castalia Simulator                      *
//*  File: Basic802154AroundPacket.msg                                       *
//*  Date: 2015-03-05                                                        *
//*  Version:  0.1                                                           *
//*  Author(s): Erico Leão <ericoleao@gmail.com>                             *
//****************************************************************************/

cplusplus {{
#include "MacPacket_m.h"
}}

class MacPacket;

enum Mac802154Packet_type {
	MAC_802154_BEACON_PACKET = 1001;
	MAC_802154_ASSOCIATE_PACKET = 1002;
	MAC_802154_DATA_PACKET = 1003;
	MAC_802154_ACK_PACKET = 1004;
	MAC_802154_GTS_REQUEST_PACKET = 1005;
    MAC_802154_FORMATION_PACKET = 1006;
    MAC_802154_CCH_PACKET = 1007;
    MAC_802154_COLLISIONDOMAIN_PACKET = 1008;
    MAC_802154_BEACONSCHEDULE_PACKET = 1009;
    MAC_802154_CH_RESPONSE_PACKET = 1010;
    MAC_802154_ADDRESSING_REQUEST_PACKET = 1011;
    MAC_802154_ADDRESSING_RESPONSE_PACKET = 1012;
    MAC_802154_ADDRESSING_ASSIGNMENT_PACKET = 1013;
    MAC_802154_RATE_ADJUSTMENT_PACKET = 1014;
    MAC_802154_DATA_REQUEST_PACKET = 1015;
    MAC_802154_AGAINCCH_PACKET = 1019;
    MAC_802154_BEGINBEACON_PACKET = 1020;
	MAC_802154_KNOWLEDGE_PACKET = 1025;
 }

struct Basic802154AroundGTSspec {
	int owner;
	int start;
	int length;
}

packet Basic802154AroundPacket extends MacPacket {
	int Mac802154AroundPacketType enum (Mac802154Packet_type);
	int PANid;
	int srcID;
	int dstID;
	int seqNum;

	string TableNH; //Nodo1:rel1,rel2;Nodo2:rel1,rel2;..... - relacoes do nodo dentro de um determinado range
    string TableCH;

    //Opções de campos para os quadros
	int beaconOrder;        //original do modelo
	int superframeOrder;    //original do modelo
	int BSN;                //original do modelo
	int CAPlength;          //original do modelo
	int GTSlength;          //original do modelo
	Basic802154AroundGTSspec GTSlist[];     //original do modelo

    int numAddrPending; //Around: Comunicação Indireta
    int AddrList[];     //Around: Comunicação Indireta

    int formBSN;
    int hops;  //Around
    int ttl;   //Around
    int depth; //Around

    bool newCH;             //Around
    bool isCH;             //Around
    double startTime;       //Around
    int position;           //Around
    //int quadrante;        //Around
    int childrenCont;       //Around
    int startAddressBlock;  //Around
    int endAddressBlock;    //Around
    double rate;            //Around
    bool associationAllow;  //Around
	bool priority;			//daniel - prioridade de grupos (taxa de dados maior)

    float rssi_threshold;   //daniel - evita que nodos fora do limite requisitem associacao


    // those fields belong to beacon packet (MAC_802154_BEACON_PACKET)
    //Type  = 2 bytes
    //PANid = 2 bytes
    //DstID = 2 bytes
    //beaconOrder = 2 bytes
    //superframeOrder = 2 bytes
    //BSN   = 2 bytes
    //Possível ter GTSlist

    // those fields belong to cluster formation packet (MAC_802154_FORMATION_PACKET)
    //Type  = 2 bytes
    //PANid = 2 bytes
    //DstID = 2 bytes
    //BSN   = 2 bytes
    //Depth = 2 bytes

    // those fields belong to PAN Associate packet (MAC_802154_ASSOCIATE_PACKET)
    //Type  = 2 bytes
    //PANid = 2 bytes
    //DstID = 2 bytes
    //SrcID = 2 bytes

    // those fields belong to several ACK packet (MAC_802154_ACK_PACKET)
    //Type = 2 bytes
    //PANid = 2 bytes
    //DstID = 2 bytes

    // those fields belong to isCH Response packet (MAC_802154_CH_RESPONSE_PACKET)
    //Type  = 2 bytes
    //DstID = 2 bytes
    //SrcID = 2 bytes

    // those fields belong to CCH Select packet (MAC_802154_CCH_PACKET)
    //Type  = 2 bytes
    //PANid = 2 bytes
    //DstID = 2 bytes
    //NewCH = 2 bytes
    //StartTime = 2 bytes ou 4??
    //Position = 2 bytes

    // those fields belong to Collision Domain Definition packet (MAC_802154_COLLISIONDOMAIN_PACKET)
    //Type  = 2 bytes
    //PANid = 2 bytes
    //SrcID = 2 bytes
    //DstID = 2 bytes
    //isCH = 2 bytes
    //associationAllow = 2 bytes

    // those fields belong to Beacon Schedule packet (MAC_802154_BEACONSCHEDULE_PACKET)
    //Type  = 2 bytes
    //PANid = 2 bytes
    //SrcID = 2 bytes
    //DstID = 2 bytes

    // those fields belong to Children Request packet (MAC_802154_ADDRESSING_REQUEST_PACKET)
    //Type  = 2 bytes
    //PANid = 2 bytes
    //DstID = 2 bytes

    // those fields belong to Children Response packet (MAC_802154_ADDRESSING_RESPONSE_PACKET)
    //Type  = 2 bytes
    //SrcID = 2 bytes
    //DstID = 2 bytes
    //ChildrenCont = 2 bytes

    // those fields belong to Children Assignment packet (MAC_802154_ADDRESSING_ASSIGNMENT_PACKET)
    //Type  = 2 bytes
    //SrcID = 2 bytes
    //DstID = 2 bytes
    //StartAddressBloci = 2 bytes
    //EndAddressBloci = 2 bytes

    // those fields belong to rate adjustment packet (MAC_802154_RATE_ADJUSTMENT_PACKET)
    //Type  = 2 bytes
    //PANid = 2 bytes
    //DstID = 2 bytes
    //rate = 4 bytes

    // those fields belong to DATA packet (MAC_802154_DATA_PACKET)
    int Sink;  // 2 bytes
    //int Source;

    // those fields belong to GTS Request packet (MAC_802154_GTS_REQUEST_PACKET)


    //PACOTES AROUND
    //MAC_802154_AROUND_CONFIG_PACKET
    //MAC_802154_AROUND_AUTHOR_PACKET
    //Type  = 2 bytes
    //DstID = 2 bytes
    int Source;
    int Destination;
    int flowID;
    int offRX;
    int offTX;
    int duration;

}

