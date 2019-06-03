//****************************************************************************
//*  Copyright (c) Faculty of Engineering, University of Porto, Portugal     *
//*  All rights reserved                                                     *
//*                                                                          *
//*  Module: Mac ARounD Approach for Castalia Simulator                      *
//*  File: Basic802154Around.ned                                             *
//*  Date: 2015-03-05                                                        *
//*  Version:  0.1                                                           *
//*  Author(s): Erico Leão <ericoleao@gmail.com>                             *
//****************************************************************************/

package node.communication.mac.mac802154Around;

simple Basic802154Around like node.communication.mac.iMac {
 parameters:
    //debug parameters
	bool collectTraceInfo = default (false);
	bool printStateTransitions = default (false);

	//mac layer parameters
	int macMaxPacketSize = default (142);	//128 (Maximum payload for 802.15.4) + 14 (Data packet overhead)
	int macPacketOverhead = default (14);
	int macBufferSize = default (32);

	bool enableSlottedCSMA = default (true);
	bool isFFD = default (false);
	bool isPANCoordinator = default (false);
	bool macBattLifeExt = default (false);

    int superframeOrder = default (2);
    int beaconOrder = default (8);
	int aUnitBackoffPeriod = default (20);  //default = 20
	int aBaseSlotDuration = default (60); //como consta no padrão

	int aNumSuperframeSlots = default (16); //como consta no padrão
    //melhores combinacoes de min e max até agora: 6-7 e 7-8
    int macMinBE = default (3);
    int macMaxBE = default (5);
	int macMaxCSMABackoffs = default (4); //como consta no padrão
    int macMaxFrameRetries = default (3);
	int aMaxLostBeacons = default (4);
	int aMinCAPLength = default (440);


	// parameters dependent on physical layer
	// some are essential and are not defined as default
	double phyDelayForValidCS = default (0.128);
	double phyDataRate;
    double phyDelaySleep2Tx = default (0.2); //in ms
    double phyDelayRx2Tx = default (0.01);	//in ms
	double phyDelayRx2Sleep = default (0.05);
	int phyFrameOverhead = default (6);
	int phyBitsPerSymbol;

	//reception guard time, in ms
	double guardTime = default (1);

    //AROUND APPROACH
    bool isCH = default (false); // Se o nó é Cluster-head
    //cluster-tree formation parameters
    int hops = default (1);
    int ttl = default (1);
    int maxFormationFrame = default (2);
    double formationTime = default (0.12); //in ms
    double ackWaitFormation = default (0.02); //in ms
    bool nowFormationACK = default (true);
    int maxCCH = default(3);
    int maxChildren = default(6); //define o número máximo de filhos para cada cluster
    int baseDepth = default(3);
    double maxformationtime = default (20.0);  //tempo máximo para o procedimento de formação de clusters
    bool fixedPacketRate = default (true);  //por padrão true, todos os nós geram a mesma taxa de pacote basePacketRate
                                             //quando false, todos os nós geram pacotes com base na quant de nós do cluster
    bool withMACBuffer = default (false);   //usar ou não o buffer do MAC. Quando não for para usar, um novo pacote de background
                                            // não será bufferizado e irá substituir o currentpack
    double basePacketRate = default (0);  //Padrão é não gerar dados de background
    int macLIFSPeriod = default (40);

    int maxNumPendingAddr = default(7);  //número máximo de mensagens (endereços) pendentes. O padrão especifica para 7

    int maxConfirmTimer = default(3);

	int difference_neighbors = default(0); 
	double rssi_threshold = default(-90);
	double packet_rate_prio = default(0.01); 	// daniel
	double packet_rate_noprio  = default(0.1);	//daniel
	bool isPrio = default (false); 				//prioridade do nodo - daniel	
	double prob_t = default(0.01);
	double prob_l = default(0.1);
	int nSlotsDiscovery = default(2000);	

 gates:
	output toNetworkModule;
	output toRadioModule;
	input fromNetworkModule;
	input fromRadioModule;
	input fromCommModuleResourceMgr;
}

