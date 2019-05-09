//****************************************************************************
//*  Copyright (c) Faculty of Engineering, University of Porto, Portugal     *
//*  All rights reserved                                                     *
//*                                                                          *
//*  Module:   ARounD Approach for Castalia Simulator                        *
//*  File: AroundApproachApp.h                                               *
//*  Date: 2015-03-05                                                        *
//*  Version:  0.1                                                           *
//*  Author(s): Erico Leão <ericoleao@gmail.com>                             *
//****************************************************************************/

#ifndef _AROUNDAPPROACHAPP_H_
#define _AROUNDAPPROACHAPP_H_

#include "VirtualApplication.h"
#include "AroundDataPacket_m.h"
#include "AroundControlMessage_m.h"
#include <map>
#include <fstream>
#include <string>

#define AROUND_DATA_BACKGROUND_SIZE 12
#define AROUND_DATA_SRC2SNK_SIZE 12
#define AROUND_DATA_AROUND_SIZE 12

using namespace std;

enum AroundApproachAppTimers {
	SEND_PACKET = 1,
    SEND_SRC2SINK_PACKET = 2,
    STOP_SRC2SINK_PACKET = 3,
    SEND_AROUND_PACKET = 4,
    STOP_AROUND_PACKET = 5,
    AROUND_FLOW = 6,
    STOP_PACKET = 7,
    
};

struct aroundconf{
    int src;
    int dst;
    double packetrate;
    int duration;
    double starttime;
};

class AroundApproachApp: public VirtualApplication {
 private:
	
	double packet_rate;
	double startupDelay;
	double delayLimit;
	float packet_spacing;
	int dataSN;     //background packet sequence number
    int src2snkSN;  //s2s packet sequence number
    int aroundSN;   //around packet sequence number
    int sinkId;
	string sinkAddress;
    
    // Configurando variáveis da comunicação Source to sink
    string s2sSink;
    int s2sSinkId;
    double s2sStartupDelay;
    int s2sDuration;
    double s2sRatePacket;
    
    // Configurando variáveis da comunicação Around
    double aroundRate;
    int aroundSentMAX;
    
    //Mapa para guardar fluxos around
    vector <struct aroundconf> aroundFlows;
	
	//variables below are used to determine the packet delivery rates
    //Statistics
	int numNodes;
	//map<int,int> bytesReceived;
	
    //Estatística: Pacotes de background enviado de nós membros aos seus clusters-heads
    int packetsSent;                //quantidade de pacotes de background enviados por um nó
    map<int,int> packetsReceived;   //ID da origem e a quantidade
    //map<int,simtime_t> averageDelay;   //ID da origem e o delay acumulado para cada nó
    map<int,double> averageDelay;
    
    //Estatística: Pacotes de dados source-to-sink enviados de nós membros para um determinado sink
    int s2sPacketsSent;                //quantidade de pacotes de background enviados por um nó
    map<int,int> s2sPacketsReceived;   //ID da origem e a quantidade
    //map<int,simtime_t> s2sAverageDelay;   //ID da origem e o delay acumulado para o fluxo tree
    map<int,double> s2sAverageDelay;
    
    //Estatística: Pacotes de dados around enviados de nós membros para um determinado sink
    int aroundPacketsSent;                //quantidade de pacotes de background enviados por um nó
    map<int,int> aroundPacketsReceived;   //ID da origem e a quantidade
    //map<int,simtime_t> aroundAverageDelay;   //ID da origem e o delay acumulado para o fluxo around
    map<int,double> aroundAverageDelay;
    
    // Para limitar a quantidade de pacotes enviados
    int sentPacketCount;
    int sentPacketMAX;
    
    // Variável para definir se o node gera ou não data!
    bool dataGen;
    
    //variável para contar o numero de pacotes recebidos
    //int contPKT;
    
    

 protected:
	void startup();
	void fromNetworkLayer(ApplicationPacket *, const char *, double, double);
	void handleMacControlMessage(cMessage *);
    void handleRadioControlMessage(RadioControlMessage *);
	void timerFiredCallback(int);
	void finishSpecific();
    
    //Around Communication
    void aroundFlowConfiguration();

 public:
	//int getPacketsSent(int addr) { return packetsSent[addr]; }
    int getPacketsSent() { return packetsSent; }
	int getPacketsReceived(int addr) { return packetsReceived[addr]; }
	//int getBytesReceived(int addr) { return bytesReceived[addr]; }
    
    int getS2SPacketsSent() { return s2sPacketsSent; }
    
    int getAroundPacketsSent() { return aroundPacketsSent; }
    
    //retonar packetrate do node
    double getdataRate() { return packet_rate; }
    
	
};

#endif				// _AROUNDAPPROACHAPP_APPLICATIONMODULE_H_
