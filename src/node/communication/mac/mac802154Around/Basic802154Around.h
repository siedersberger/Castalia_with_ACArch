//****************************************************************************
//*  Copyright (c) Faculty of Engineering, University of Porto, Portugal     *
//*  All rights reserved                                                     *
//*                                                                          *
//*  Module: Mac ARounD Approach for Castalia Simulator                      *
//*  File: Basic802154Around.h                                               *
//*  Date: 2015-03-05                                                        *
//*  Version:  0.1                                                           *
//*  Author(s): Erico Leão <ericoleao@gmail.com>                             *
//****************************************************************************/

#ifndef BASIC802154AROUND_H_
#define BASIC802154AROUND_H_

#include <map>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "VirtualMac.h"
#include "AroundApproachApp.h"
#include "Basic802154AroundPacket_m.h"
#include "Basic802154AroundControl_m.h"
#include "graph.h"
#include "acarch.h"

#define ACK_PKT_SIZE 6
#define COMMAND_PKT_SIZE 10
#define GTS_SPEC_FIELD_SIZE 3
#define BASE_BEACON_PKT_SIZE 12
#define FORMATION_PKT_SIZE 10
#define CCH_PKT_SIZE 12
#define CHACK_PKT_SIZE 6
#define KNOWLEDGE_PKT_SIZE 6
#define BEACONSCHEDULE_PKT_SIZE 6
#define ADDRESSING_REQUEST_PKT_SIZE 6
#define ADDRESSING_RESPONSE_PKT_SIZE 8
#define ADDRESSING_ASSIGNMENT_PKT_SIZE 10
#define ADDRESSING_RATE_ADJUSTMENT_PKT_SIZE 10
#define AROUND_CONFIG_ACK_PACKET_SIZE 14
#define ASSOCIATE_PKT_SIZE 102

#define TX_TIME(x) (phyLayerOverhead + x)*1/(1000*phyDataRate/8.0)	//x are in BYTES

using namespace std;

enum MacStates {
	MAC_STATE_SETUP = 1000,
	MAC_STATE_SLEEP = 1001,
	MAC_STATE_CAP = 1002,
	MAC_STATE_GTS = 1003,
    MAC_STATE_FORMATION = 1004,
    MAC_STATE_CAP_CH = 1005,
    MAC_STATE_CAP_CH_SEND = 1006,
    MAC_STATE_AROUND = 1007,
    MAC_STATE_DISCOVERY = 1008,
};

enum Mac802154Timers {
	PERFORM_CCA = 1,
	ATTEMPT_TX = 2,
	BEACON_TIMEOUT = 3,
	ACK_TIMEOUT = 4,
	GTS_START = 5,
	SLEEP_START = 6,
	FRAME_START = 7,
	BACK_TO_SETUP = 8,
    CLUSTER_FORMATION = 9,
    CCH_SELECT = 10,
    COLLISION_DOMAIN = 11,
    FRAME_RECEIVE = 12,
    BEACON_SCHEDULE = 13,
    FORMATION_TIME = 14,
    ADDRESSING_REQUEST = 15,
    ADDRESSING_RESPONSE = 16,
    ADDRESSING_ASSIGNMENT = 17,
    RATE_ADJUSTMENT = 18,
    SPENT_ENERGY = 19,
    RE_SCHEDULING = 21,
    BEGIN_BEACON = 22,
    CHECK_SCHEDULING = 28,
	NEIGHBORHOOD_KNOWLEDGE = 29,
    CHANGE_MODE = 30,
};

struct parChild{
    int ch1;
    int ch2;
    
    bool operator==(const parChild &o)  const {
        return ch1 == o.ch1 && ch2 == o.ch2;
    }
    bool operator<(const parChild &o)  const {
        return ch1 < o.ch1 || (ch1 == o.ch1 && ch2 < o.ch2);
    }
};

struct pendingAddrStruct{
    int node;
    int sink;
    Basic802154AroundPacket *pendingAddrPkt;
};

struct schedNode{
    //vector<int> ch;
    //vector<int> off;
    //vector<int> time;
    int ch;
    int off;
    int time;
    int flow;
    int offRX;
    int offTX;
    double rate;
};

// Estruturas para gerenciamento e definição do escalonamento da abordagem ARounD
static map<int, vector<int> > interfMatrix;
static map<int, vector<double> > RSSIinterfMatrix;
static int mat[503][503];  //matriz de sobreposicao, considerando todos os nós
static map<int,int> ctree; //mapa da relação pai-filho do cluster-tree
static map<int, vector<int> > graph; //para cada CH, a lista de CHs que estão em overlapping

//static map<struct parChild,vector<int> > repnodes; //mapa de repeater nodes, para cada par de CH, os nós repeaters no-CH
//static map<struct parChild,vector<int> > CHrepnodes; // mapa de repeater nodes, para cada par de CH, os nós repeaters CH

static map< pair<int,int> , vector<int> > repnodes; //mapa de repeater nodes, para cada par de CH, os nós repeaters no-CH
static map< pair<int,int> , vector<int> > CHrepnodes; // mapa de repeater nodes, para cada par de CH, os nós repeaters CH
static map< pair<int,int> , vector<int> > treerepnodes; // mapa de "repeater" entre nós da árvore

//Para escalonamento
static vector<int> scheduleCH;   // ordem de CH no escalonamento
static map<int,double> offsetCH; // offset de cada cluster
static map<int,double> offsetFormation; // offset top-down para auxiliar na formação, principalmente escalonamento
static map<int,int> soCH; // guarda os SO de cada cluster
static int globalBO; //guarda o BI global para ser utilizado por todos os CH's
static simtime_t baseBI; //Tempo utilizado como base para cada Cluster Iniciar o envio de Beacons

static map<int,bool> checkSched; // estrutura para os CHs checarem se seus CHs filhos receberam o scheudling

//Para endereçamento
static map<int,int> routingTable;  //tabela de routing global: MAC_Address - Hierarchical_Address
static map<int,int> invroutingTable;  //tabela de routing global invertida: Hierarchical_Address - MAC_Address
static map<int,vector<int> > addressBlock;  //Bloco de endereços hierárquicos de todos os CHs, incluindo o PAN


class Basic802154Around: public VirtualMac {
  private:
    /*--- A map from int value of state to its description (used in debug) ---*/
	map<int,string> stateDescr;

    /*--- A map for packet breakdown statistics ---*/
	map<string,int> packetBreak;
    map<string,int> receivedData;

    /*--- General MAC variable ---*/
	simtime_t phyDelayForValidCS;		// delay for valid CS
	simtime_t phyDelaySleep2Tx;
	simtime_t phyDelayRx2Tx;
    simtime_t phyDelayRx2Sleep;
	int phyLayerOverhead;
	int phyBitsPerSymbol;
	double phyDataRate;

	map<int,bool> associatedDevices;	// map of associated devices (for PAN coordinator)
    map<int,double> associatedDeviceRSSI;	// map of RSSI of associated devices (for PAN coordinator)
    vector<int> associatedCH;  // vector of associated CHs (for all CH, included the PAN coordinator) AROUND
    int nchildren;  // número de filhos do Cluster-Head
    int totalchildren;  // número de filhos acumulado do Cluster-Head = 1 (ele próprio) + nchildren + acumulado dos seus CHs filhos
    bool fixedPacketRate; //Essa variável é para definir se os nós terão a mesma taxa de pacotes, ou com base na quant de nós do cluster
    double basePacketRate; //Taxa de pacote base a ser utilizada pelos CHs para cálculo da taxa de pacote dentro de seu cluster

    /*--- 802154Mac state variables  ---*/
	int macState;		// current MAC state
	int CAPlength;		// duration of CAP interval (in number of superframe slots)
	int macBSN;			// beacon sequence number (unused)
    int recMacBSN;		// Around: identify received beacon sequence number (unused)
	int lostBeacons;	// number of consequitive lost beacon packets
	int superframeDuration;	// duration of active part of the frame (in symbols)
	int beaconInterval;	// duration of the whole frame (in symbols)
	int seqNum;			// sequence number for data packets
    int backgroundSeqNum;  // sequence number for background data packets
    int aroundConfSeqNum;   // sequence number for around config packets
    int aroundSeqNum;   // sequence number for around data packets

	int NB, CW, BE;		// CSMA-CA algorithm parameters (Number of Backoffs, 
						// Contention Window length and Backoff Exponent)

	double currentPacketLimit;	// maximum delay limit for current packet
	int currentPacketRetries;	// number of retries left for next packet to be sent
	bool currentPacketGtsOnly;	// transmission of current packet can be done in GTS only
	bool currentPacketSuccess; 	// outcome of transmission for current packet
	string currentPacketHistory;// history of transmission attempts for current packet

	simtime_t currentPacketResponse;// Duration of timeout for receiving a reply after sending a packet
	simtime_t macAckWaitDuration;		// Duration of timeout for receiving an ACK
	simtime_t symbolLen;			// Duration of transmittion of a single symbol
	simtime_t guardTime;

	// packet statistics
	int sentBeacons;
	int recvBeacons;
	int packetoverflow;
    int replacedPackets;
    
	// syncronisation statistics
	simtime_t desyncTime;
	simtime_t desyncTimeStart;

    /*--- 802154Mac packet pointers (sometimes packet is created not immediately before sending) ---*/
	Basic802154AroundPacket *beaconPacket;
	Basic802154AroundPacket *currentPacket;
    
	void fromNetworkLayer(cPacket *, int);
	void fromRadioLayer(cPacket *, double, double);

	void readIniFileParameters(void);
	void setMacState(int newState);
	void handleAckPacket(Basic802154AroundPacket *);
	void performCSMACA();
	void attemptTransmission(const char *);
	void transmitCurrentPacket();
	void collectPacketHistory(const char * s);
	void clearCurrentPacket(const char * s = NULL, bool success = false);

     /*--- The .ned file's parameters ---*/
	bool printStateTransitions;
	bool isPANCoordinator;
	bool isFFD;
	bool macBattLifeExt;
	bool enableSlottedCSMA;

	int macMinBE;
	int macMaxBE;
	int macMaxCSMABackoffs;
	int macMaxFrameRetries;
	int aMaxLostBeacons;
	int aMinCAPLength;
	int aUnitBackoffPeriod;
	int aBaseSlotDuration;
	int aNumSuperframeSlots;
	int aBaseSuperframeDuration;
	int beaconOrder;
	int superframeOrder;
    int low_DC;
    int high_DC;

	simtime_t CAPend;				// Absolute time of end of CAP period for current frame
	simtime_t currentFrameStart;	// Absolute recorded start time of the current frame
	simtime_t GTSstart;				// Absolute time of the start of GTS period for current node
	simtime_t GTSend;				// Absolute time of the end of GTS period for current node
	simtime_t GTSlength;			// length of the GTS period for current node

	int associatedPAN;	// ID of current PAN (-1 if not associated)
    
    //AROUND APPROACH
    bool isCH; // Se o nó é Cluster-head
    int parentCH; // Guarda o cluster-head cluster-head pai. A principio, nao vou precisar, pois CH pai é o associatedPAN
    int maxFormationFrame; //quantidade de pacotes de formação de cluster a ser enviado
    int formationFrameCount; //contador de pacotes
    Basic802154AroundPacket *formationPacket;
    Basic802154AroundPacket *formationAckPacket;
    Basic802154AroundPacket *chAckPacket;
    Basic802154AroundPacket *collisionDomainPacket;
    Basic802154AroundPacket *schedulePacket;
    Basic802154AroundPacket *addressingRequestPacket;
    Basic802154AroundPacket *addressingResponsePacket;
    Basic802154AroundPacket *addressingAssignmentPacket;
    Basic802154AroundPacket *rateAdjustPacket;
	Basic802154AroundPacket *knowledgePacket;
    
    // Cluster-tree formation parameters
    int hops;   //máxima distancia em hops de um nó para o seu CH
    int ttl;    //número de hops para encaminhar um broadcast de formação de cluster-tree
    int depth;  //profundidade no cluster-tree
    int formationSeqNum; //número de sequencia de quadro de formação
    double formationTime;  //Tempo para formação de um cluster
    double ackWaitFormation; //Duração de timeout para um quadro de formação
    bool nowFormationACK; //Variável que define se os ACKs dos pedidos de associação são enviados de imediato!!!
    int maxCCH; //quantidade máxima de CCHs
    int baseDepth; //profundidade no qual garanto formação de cluster escalonado no tempo. A partir disso, a formação ocorre em paralelo
    int position; //posição de um CCH na largura do seu nível, o que é util para o cálculo do deslocamento no tempo do período de ativação de formação de cluster
    int quadrante; //utilizado a fim de construir um cluster-tree que expanda, sem retroagir (algoritmo do quadrante)
    double maxformationtime;  //tempo máximo para o procedimento de formação de clusters
    int macLIFSPeriod;
    
//#########################################################################################################################################
//#################################################				DANIEL				#######################################################
 	
	map<int, map<int, double>>  nhTable;	    //tabela com as informações da vizinhança												
	map<int, int>               nhCont;         //contador de descobrimento

    map<int, int>			    knn_neighbors;
    
    //id,CH
    map<int, bool>              chTable;        //tabela repassada aos CHs do cluster com informações do mesmo 

	string relationTable, TableCH;
	double rssi_threshold;
	int difference_neighbors;	
	int knowledgeFrameCount, nSlotsDiscovery;					
	double packet_rate_prio;					
	double packet_rate_noprio;					
	bool isPrio;
    double prob_t, prob_l, prob_s;
	
//#################################################				DANIEL				#######################################################
//#########################################################################################################################################

    // Grafo de colisão
    struct parChild relation;
    vector<int> overlap;
    vector<int> repeaters;
    
    // Endereçamento hierárquico
    //double addressingTime;
    int addressCount;  //contador para ajudar na atribuição dos endereços
    int startAddrBlock; // Endereço hierárquico inicial
    int endAddrBlock;  // Endereço hierárquico final
    vector<int> auxBlockVector; //vector auxiliar para armazenar os endereços de início e fim de um bloco
    map<int,int> childAddress;  //Lista de filhos e seus endereços hierarquicos MAC_Address - Hierarchical_Address
    map<int, int> chAddrCount;  // Lista com a quantidade solicitada por cada CH filho
    map<int, vector<int> > chAddrBlock;  // O bloco de endereços dos CHs filhos (Local)

    // Lista de Endereços Pendentes (Mensagens pendentes)
    // Para comunicação Indireta: Coordinator -> member nodes
    int numPendingAddr;
    vector<int> pendingAddrList;
    //map<int, Basic802154AroundPacket *> pendingAddrTxBuffer;
    vector<struct pendingAddrStruct> pendingAddrTxBuffer;
    int maxNumPendingAddr;
    bool hasPendingPkt; // variável para saber se o coordenador pode acessar CSMA para transmitir dados pendentes
                        // essa variável serve para evitar que coordenador acesse CSMA como se fosse um nó membro, ou seja,
                        // não atue como nó membro quando está atuando como coordenador PAN
    int pendingCount;
    bool withMACBuffer;
    
    
    //funções
    void clusterFormation();
    void clusterJoin(Basic802154AroundPacket *);
    void receiveFormation_node(Basic802154AroundPacket *);
    void selectCCH();
    void collisionDomain();
    void beaconSchedule();
    void reScheduling();
	
    
    //Funcoes para verificacao de pacotes duplicados para o Around
    map<pair<int,int>, unsigned int > AroundPktHistory;
    bool DuplicatedPacket(cPacket *);
    
    Graph *g = new Graph(); //Objeto grafo, definido na classe Graph.h
    
    Tree *t = new Tree(); //Objeto tree, definido na classe Graph.h
    
	Acarch *acarch = new Acarch();


    // Estrutura para representar o schedule da rede, considerando os slots vazios
    vector<struct schedNode> schedList; //escalonamento da c-tree com lacunas incluídas
    map<int, vector<struct schedNode> > aroundFlows; //fluxos around
    int aroundFlowID; //Id dos fluxos around
    vector<struct schedNode> clusterAllocation; //vector temporário para armazenar os pares de clusters alocados
    
    //Estruturas para Configuração, confirmação e autorização de comunicação Around
    map<pair<int,int>,bool> aroundAuthor;
    
    int maxConfirmTimer; //valor da quantidade máxima de temporizadores utilizados para esperar a confirmação dos nodes around
    map<int,int> confirmTimerCount; //irá contar a quantidade de temporizadores utilizados para esperar a confirmação dos nodes around
    queue<int> confirmTimers; //guarda a sequencia dos fluxos que estão temporizados e esperando a confirmação.
    
    //Defini a variável para poder receber o NED
    int maxChildren;
    
    //Buffer de descida
    queue< cPacket* > DownTXBuffer;
    
    
    //Buffer AROUND
    map<int,Basic802154AroundPacket*> aroundBuffer;
    
    simtime_t timeBeacon;
    simtime_t timeBeaconPAN;
    
    //variável para contar o numero de pacotes recebidos
    int contPKT;

    

  protected:

	/*--- Interface functions of the 802.15.4 MAC ---*/
	Basic802154AroundPacket *newConnectionRequest(int);
	Basic802154AroundPacket *newGtsRequest(int,int);
	Basic802154AroundPacket *getCurrentPacket() { return currentPacket; }
	void transmitPacket(Basic802154AroundPacket *pkt, int retries = 0, bool GTSonly = false, double delay = 0);
	int getCurrentMacState() { return macState; }
	int getAssociatedPAN() { return associatedPAN; }
    bool getisCH() { return (isCH || isPANCoordinator); }
    int getStartAddrBlock() { return startAddrBlock; }
    int getEndAddrBlock() { return endAddrBlock; }
    int getDepth() { return depth; }
    int getNchildren() { return nchildren; }
	int getTotalchildren() { return totalchildren; }
	bool getPrio() { return isPrio; }

	/*--- Virtual interface functions can be overwritten by a decision module ---*/
	virtual void startup();
	virtual void timerFiredCallback(int);
	virtual void finishSpecific();
	
	/*--- HUB-specific desicions ---*/
	virtual void dataReceived_hub(Basic802154AroundPacket *) {}
	virtual int gtsRequest_hub(Basic802154AroundPacket *) { return 0; }			//default: always reject
	virtual void prepareBeacon_hub(Basic802154AroundPacket *) {}

    
//_________________________________________________________________________________________________________________________________________
//#################################################				DANIEL				#######################################################
 
	virtual int distributed_hub(Basic802154AroundPacket *, double);
    virtual void calcRSSI_avg();			
	virtual void createTable();													
	virtual void readTable(string, int);
    virtual void createTableCH();		
	virtual void readTableCH(string);									
	
//#################################################				DANIEL				#######################################################
//_________________________________________________________________________________________________________________________________________ 

	/*--- Node-specific desicions ---*/
	virtual void connectedToPAN_node() {}
	virtual void disconnectedFromPAN_node() {}
	virtual void assignedGTS_node(int) {}
	virtual void receiveBeacon_node(Basic802154AroundPacket *);
	virtual void startedGTS_node() {}

	/*--- General desicions ---*/
	virtual bool acceptNewPacket(Basic802154AroundPacket *);
	virtual void transmissionOutcome(Basic802154AroundPacket *, bool, string);
    virtual int bufferPacket(cPacket *);
    
    /* --- ERICO ---*/
    Basic802154AroundPacket *newAssociationRequest(int);
    
    // Para manusear os pacotes de comandos vindo da Aplicação
    int handleControlCommand(cMessage * msg);
    
    //Ponteiro para o gerenciador de recursos
    //ResourceManager *MgrModule;
    
};

#endif	//BASIC802154RAOUND
