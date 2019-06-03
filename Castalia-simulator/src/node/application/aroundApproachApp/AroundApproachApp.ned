//****************************************************************************
//*  Copyright (c) Faculty of Engineering, University of Porto, Portugal     *
//*  All rights reserved                                                     *
//*                                                                          *
//*  Module:   ARounD Approach for Castalia Simulator                        *
//*  File: AroundApproachApp.ned                                             *
//*  Date: 2015-03-05                                                        *
//*  Version:  0.1                                                           *
//*  Author(s): Erico Leão <ericoleao@gmail.com>                             *
//****************************************************************************/

package node.application.aroundApproachApp;

// The sensor node module. Connects to the wireless channel in order to communicate 
// with other nodes. Connects to psysical processes so it can sample them.

simple AroundApproachApp like node.application.iApplication {
 parameters:
 	string applicationID = default ("AroundApproachApp");
	bool collectTraceInfo = default (false);
	int priority = default (1);
	int packetHeaderOverhead = default (5);		// in bytes
	int constantDataPayload = default (100);	// in bytes
	double delayLimit = default (0); 			// application delay limit (0 - no limit)
											// discard packets arriving after the limit

	string sink = default ("0");	// Destination for packets received in this node. 
											// These parameters can be used to create an
											// application-level static routing. This way we can
											// have a multi-hop throughput test   

	double packet_rate = default (0);	// packets per second, by default we transmit no packets
	double startupDelay = default (0);	// delay in seconds before the app stars producing packets

	double latencyHistogramMax = default (200);
	int latencyHistogramBuckets = default (10);

    // Comunicação source to sink
    string source2sink = default ("-1");
    double s2sStartupDelay = default (0);
    double s2sDuration = default (0);
    double s2sRatePacket = default (0);

    // Quantidade máxima de pacotes de background enviados por cada nó
    int sentPacketMAX = default(10);

    // Variável para permitir geração de dados de background
    // Se configurado como false, o node não gera dados de background
    bool dataGen = default(true);



 gates:
 	output toCommunicationModule;
	output toSensorDeviceManager;
	input fromCommunicationModule;
	input fromSensorDeviceManager;
	input fromResourceManager;
}
