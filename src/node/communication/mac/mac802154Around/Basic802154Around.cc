//****************************************************************************
//*  Copyright (c) Faculty of Engineering, University of Porto, Portugal     *
//*  All rights reserved                                                     *
//*                                                                          *
//*  Module: Mac ARounD Approach for Castalia Simulator                      *
//*  File: Basic802154Around.cc                                              *
//*  Date: 2015-03-05                                                        *
//*  Version:  0.1                                                           *
//*  Author(s): Erico Leão <ericoleao@gmail.com>                             *
//****************************************************************************/

#include "Basic802154Around.h"
#include "VirtualMobilityManager.h"
//#include "AroundControlMessage_m.h"



// This module is virtual and can not be used directly
Define_Module(Basic802154Around);


// Função de Inicialização da Simulação
void Basic802154Around::startup(){
    
    //Inicializando uma nova simulação
    if(SELF_MAC_ADDRESS == 0)
        trace() << "NEW SIMULATION!";  // around - debug
    
    //Testando pegar a energia do nó, utilizando o método do gerenciador de recursos
    //trace() << "ENERGY " << resMgrModule->getSpentEnergy();
    
    //setando Timer de gasto de energia para jogar no trace o gasto de energia a cada tempo
    //setTimer(SPENT_ENERGY,(1-getClock()));
    
    // Around: Destruindo as estruturas statics antes de inicializar uma simulação
    if(SELF_MAC_ADDRESS == 0){
        interfMatrix.clear();
        RSSIinterfMatrix.clear();
        ctree.clear();
        graph.clear();
        repnodes.clear();
        CHrepnodes.clear();
        treerepnodes.clear();
        scheduleCH.clear();
        soCH.clear();
        offsetCH.clear();
        offsetFormation.clear();
        routingTable.clear();
        invroutingTable.clear();
        addressBlock.clear();
        checkSched.clear();
    }
    
    maxformationtime = par("maxformationtime");  //tempo máximo para o procedimento de formação de clusters
    //Definindo um temporizador de tempo máximo de formação. Após esse temporizador esgotar, todos os
    //timer ativos serão cancelados e a rede passará para fase de definição do domínio de colisão.
    setTimer(FORMATION_TIME, maxformationtime - getClock()); // fazendo assim, eu defino o tempo que eu quero para formação de clusters
  	
    //setTimer(CHANGE_MODE, (maxformationtime+100) - getClock());

    //Impressão dos EStados do MAC
    printStateTransitions = par("printStateTransitions");
	if (printStateTransitions) {
		stateDescr[1000] = "MAC_STATE_SETUP";
		stateDescr[1001] = "MAC_STATE_SLEEP";
		stateDescr[1002] = "MAC_STATE_CAP";
		stateDescr[1003] = "MAC_STATE_GTS";
        stateDescr[1004] = "MAC_STATE_FORMATION";
        stateDescr[1005] = "MAC_STATE_CAP_CH";
        stateDescr[1006] = "MAC_STATE_DISCOVERY";
	}
    
    //É coordenador PAN (Bool) - pega parâmetro do .NED - default (false)
	isPANCoordinator = par("isPANCoordinator");
	//É um nó FFD (bool) - pega parâmetro do .NED - default (false)
    isFFD = par("isFFD");
    
    // Around: provisoriamente, todo mundo é FFD
    isFFD = true;

	//pega prioridade definida no .NED - default (false)
	isPrio = par("isPrio");
	
	maxChildren = par("maxChildren");
    
    //Definindo o tamanho do buffer do MAC
    macBufferSize = par("macBufferSize");

	// Tamanho mínimo do CAP (int) - pega parâmetro do .NED - default (440)
	aMinCAPLength = par("aMinCAPLength");

	// Parâmetros que definem o Slot do 802.15.4 - em símbolos
	// aUnitBackoffPeriod (int) - pega parâmetro do .NED - default (20)
    aUnitBackoffPeriod = par("aUnitBackoffPeriod");
	// aBaseSlotduration (int) - pega parâmetro do .NED - default (60)
    aBaseSlotDuration = par("aBaseSlotDuration");
	// aNumSuperframeSlots (int) - pega parâmetro do .NED - default (16)
    aNumSuperframeSlots = par("aNumSuperframeSlots");
	// Tamanho base da duração de superframe (int) = aBaseSlotDuration * aNumSuperframeSlots
    aBaseSuperframeDuration = aBaseSlotDuration * aNumSuperframeSlots;

	// Parâmetros que definem o protocolo CSMA
    // Habilita o modo com beacon (bool) - pega parâmetro do .NED - default (true)
	enableSlottedCSMA = par("enableSlottedCSMA");
	// MacMinBE (int) - pega parâmetro do .NED - default (5) [OBS: default do padrão = 3] [Valores de 0 - maxMacBE]
    macMinBE = par("macMinBE");
	// MacMaxBE (int) - pega parâmetro do .NED - default (7) [Valores de 3 - 8]
    macMaxBE = par("macMaxBE");
	// macMacCSMABackoffs (int) - pega parâmetro do .NED - default (4) [Valores de 0 - 5]
    macMaxCSMABackoffs = par("macMaxCSMABackoffs");
	// macMacFrameRetries (int) - pega parâmetro do .NED - default (2) [OBS: default do padrão = 3] [Valores de 0 - 7]
    macMaxFrameRetries = par("macMaxFrameRetries");
	// (bool) - pega parâmetro do .NED - default (false)
    macBattLifeExt = par("macBattLifeExt");

	// Parâmetros da e dependentes da Camada Física
	// (double) - pega parâmetro do .NED - não tem default
    phyDataRate = par("phyDataRate");
	// (double) - pega parâmetro do .NED - default (0.2) in ms
    phyDelaySleep2Tx = (double)par("phyDelaySleep2Tx") / 1000.0;
	// (double) - pega parâmetro do .NED - default (0.02) in ms
    phyDelayRx2Tx = (double)par("phyDelayRx2Tx") / 1000.0;
    // (double) - pega parâmetro do .NED - default (0.02) in ms
    phyDelayRx2Sleep = (double)par("phyDelayRx2Sleep") / 1000.0;
	// (double) - pega parâmetro do .NED - default (0.128)
    phyDelayForValidCS = (double)par("phyDelayForValidCS") / 1000.0;
	// (int) - pega parâmetro do .NED - default (6)
    phyLayerOverhead = par("phyFrameOverhead");  //Nao usado no código MAC
	// (int) - pega parâmetro do .NED - não tem default
    phyBitsPerSymbol = par("phyBitsPerSymbol");
	symbolLen = 1 / (phyDataRate * 1000 / phyBitsPerSymbol);
	macAckWaitDuration = symbolLen * aUnitBackoffPeriod +
			phyDelayRx2Tx * 2 + TX_TIME(ACK_PKT_SIZE);

    //Around - Definindo o período interframes LIFS
    macLIFSPeriod = par("macLIFSPeriod");
	
    // Parâmetros de conexão do HUB
	// (double) - pega parâmetro do .NED - default (1)
    guardTime = (double)par("guardTime") / 1000.0;
    // Tentativas que irá causar a perda de sincronização - aMaxLostBeacons (int) - pega parâmetro do .NED - default (4)
	aMaxLostBeacons = par("aMaxLostBeacons");

	// inicialização do MAC Geral
	currentPacket = NULL;
	macState = MAC_STATE_SETUP;
	associatedPAN = -1;
	currentFrameStart = 0;
	GTSstart = 0;
	GTSend = 0;
	CAPend = 0;
	//backgroundSeqNum = 0; //número de sequencia de quadros backgrounds
    seqNum = 0;
    macBSN = 0;
    recMacBSN = 0;
    aroundConfSeqNum = 0;
    aroundSeqNum = 0;

	// Estatísticas para reportar
	lostBeacons = 0;
	sentBeacons = 0;
	recvBeacons = 0;
	packetoverflow = 0;
	desyncTime = 0;
	desyncTimeStart = 0;
	packetBreak.clear();
	declareOutput("pkt TX state breakdown");
    replacedPackets = 0; //pacotes de dados substituídos ao chegar da network
    
    // Around Approach
    maxFormationFrame = par("maxFormationFrame");
    formationFrameCount = 0;
    knowledgeFrameCount = 0; //daniel
    // Around: Variáveis para o cluster-tree
    isCH = false; // Se o nó é Cluster-head
    parentCH = 0; // Não tem CH pai ainda
    hops = par("hops");   //máxima distancia em hops de um nó para o seu CH
    ttl = par("ttl");
    depth = -1;
    nchildren = 0;      //quantidade de filhos de um nó CH
    totalchildren = 0;  // quantidade de filhos acumulados
    formationSeqNum = 0;
    formationTime = par("formationTime");
    nowFormationACK = par("nowFormationACK");
    ackWaitFormation = par("ackWaitFormation");
    fixedPacketRate = par("fixedPacketRate");
    basePacketRate = par("basePacketRate");
    startAddrBlock = 0; // zerando variável de bloco de endereçamento
    endAddrBlock = 0;   // zerando variável de bloco de endereçamento
    
    // Variáveis para Comunicação Indireta
    numPendingAddr = 0;      //iniciando a quantidade de mensagens pendentes
    pendingAddrList.clear(); //iniciando lista de endereços pendentes
    pendingAddrTxBuffer.clear();  //iniciando com a lista de pacotes pendentes vazia
    maxNumPendingAddr = par("maxNumPendingAddr"); //numero máximo de endereços pendentes. Por padrão, max é 7.
    hasPendingPkt = false;
    withMACBuffer = par("withMACBuffer");  //se será usado ou não bufferizado pacotes de background no MAC
    
    //ESQUEMA DE ALOCACAO DE SD, Teste de escalonamento
    superframeOrder = par("superframeOrder");     // Superframe Order, caso o esquema de escalonamento seja estático
    beaconOrder = par("beaconOrder");             //Beacon Order
    
    beaconInterval = aBaseSuperframeDuration * (1 << beaconOrder);
    superframeDuration = aBaseSuperframeDuration * (1 << superframeOrder);

    // Definição do Tamanho do CAP
    CAPlength = aNumSuperframeSlots;

    declareOutput("PERCA");
    
	//para calculo de energia
    contPKT = 0; //

    //probabilidades para o metodo de descobrimento
    prob_t = par("prob_t");
    prob_l = par("prob_l");
    prob_s = 1-prob_t-prob_l;
    nSlotsDiscovery = par("nSlotsDiscovery");

	packet_rate_prio = par("packet_rate_prio");
	packet_rate_noprio = par("packet_rate_noprio");   
	rssi_threshold = par("rssi_threshold");
	difference_neighbors = par("difference_neighbors");	
    
    nhTable.clear();
    chTable.clear();
    knn_neighbors.clear();
	
	// Inicialização do nó Coordenador
	if (isPANCoordinator) {
		// Teste para saber se é nó FFD
        if (!isFFD) {
			opp_error("Only full-function devices (isFFD=true) can be PAN coordinators");
		}

		//Combinações inválidas de Superframe e Beacon Order
		if (superframeOrder < 0 || beaconOrder < 0 || beaconOrder > 14
			|| superframeOrder > 14 || beaconOrder < superframeOrder) {
			opp_error("Invalid combination of superframeOrder and beaconOrder parameters. Must be 0 <= superframeOrder <= beaconOrder <= 14");
		}

		// Combinações inválidas para o BI e SD
		if (beaconInterval <= 0 || superframeDuration <= 0) {
			opp_error("Invalid parameter combination of aBaseSlotDuration and aNumSuperframeSlots");
		}


        // Se é um nó FFD, inicializa o nó Coordenador PAN
		associatedPAN = SELF_MAC_ADDRESS;       //associatedPAN é o endereço do nó
        
        // Around: inicialização da profundidade do PAN
        depth = 0;
		
        //chTable[0] = true;

        //Setando o seu endereço como root no grafo de colisão da classe graph.h
        g->setRoot(SELF_MAC_ADDRESS);
        
        t->setRoot(SELF_MAC_ADDRESS);
        
        aroundFlowID = 0; //iniciando o identificado dos fluxos around
        
        maxConfirmTimer = par("maxConfirmTimer"); //inicializando a quant máx de temporizadores para confirmação de	
    
        // Inicializa o TIMER CLUSTER_FORMATION
        //setTimer(CLUSTER_FORMATION, 25);	//frame start is NOW
        
	}
		
	 setTimer(NEIGHBORHOOD_KNOWLEDGE, (1-getClock()));

}



//----------------------------------------------------------------------------------------------------------------------------
//
//  TIMERS
//
//----------------------------------------------------------------------------------------------------------------------------



// Função com Timers
void Basic802154Around::timerFiredCallback(int index)
{
	switch (index) {

		// Procedimento de Formação de cluster
        case CLUSTER_FORMATION: {


            // Sendo um coordenador, cria e broadcast pacote de formação de cluster
            if (((isPANCoordinator) || (isCH))) {
                formationFrameCount++;
                if(formationFrameCount <= maxFormationFrame){
                    formationPacket = new Basic802154AroundPacket("Cluster Formation packet", MAC_LAYER_PACKET);
                    formationPacket->setMac802154AroundPacketType(MAC_802154_FORMATION_PACKET);     // Tipo:  2 bytes
                    formationPacket->setPANid(SELF_MAC_ADDRESS);                                    // PANid: 2 bytes
                    formationPacket->setDstID(BROADCAST_MAC_ADDRESS);                               // DstID: 2 bytes
                    formationPacket->setFormBSN(formationSeqNum);                                   // BO:    2 bytes
                    formationPacket->setDepth(depth); 
                   	formationPacket->setByteLength(FORMATION_PKT_SIZE);
                    
                    //Seta setMacState para MAC_STATE_FORMATION
                    setMacState(MAC_STATE_FORMATION);
                    
                    // Envia o pacote de formação do cluster para o rádio
                    trace() << "Transmitting [Cluster Formation packet] now!!";
                    toRadioLayer(formationPacket);
                    toRadioLayer(createRadioCommand(SET_STATE, TX));
                  
					setTimer(ATTEMPT_TX, TX_TIME(formationPacket->getByteLength()));

                    formationPacket = NULL;    // define para nulo o formation packet
                    formationSeqNum++;
                    //Atualiza o tempo do início do próximo superframe, iniciado logo abaixo
                    currentFrameStart = getClock() + phyDelayRx2Tx;
                    
                    setTimer(CLUSTER_FORMATION, formationTime);
                    
                } else {
                    //AROUND: Após enviar todos os quadros de formação, ir para seleção de CCH
                    formationSeqNum = 0;
                    selectCCH();
                }
            
            }
            break;
        }
        
        // Seleção de Candidatos a Cluster-Heads - CCHs
        case CCH_SELECT: {
            //trace() << "XX " << SELF_MAC_ADDRESS << " associatedPAN " << associatedPAN;  // Around - Debug
            
            if ((isPANCoordinator) || (isCH)) {
                selectCCH();
            }
            else {
                //trace() << "ACORDANDO!!";  // Around - Debug
                toRadioLayer(createRadioCommand(SET_STATE, RX));
            
                if(associatedPAN != -1){
                    //trace() << "Associado";  // Around - Debug
                }
                else{
                    //trace() << "NAO-Associado";  // Around - Debug
                }
            }
      
            break;
        }
            
        // Definição do Domínio de Colisão e estruturas para o algoritmo ARounD
        case FORMATION_TIME: {
            trace() << "ENTREI NO FORMATION TIME " << SELF_MAC_ADDRESS << " com AssociatedPAN: " << associatedPAN;  //around - debug
            
            //Cancela o Timer de formação de cluster
            if(getTimer(CLUSTER_FORMATION) != -1){
///                trace() << "Existe Timer Cluster_Formation de " << SELF_MAC_ADDRESS; //around - debug
                cancelTimer(CLUSTER_FORMATION);
                //Caso tenha sido selecionado como CCH, mas não teve "tempo" de associar nenhum filho
                //então deixa de ser CH
                if(nchildren == 0){
///                    trace() << "DESALOCANDO POR NAO ENTRAR NA FORMACAO DE CLUSTER " << SELF_MAC_ADDRESS; //around - debug
                    isCH = false;
                    //setTimer(SLEEP_START,0); //vai dormir. Cancelado, por enquanto
                }else{
///                    trace() << "NAO TERMINEI O PROCESSO DE FORMACAO, MAS ASSOCIEI FILHOS " << SELF_MAC_ADDRESS; //around - debug
                    ctree[SELF_MAC_ADDRESS] = associatedPAN;
                    scheduleCH.push_back(SELF_MAC_ADDRESS);
                    
                    Basic802154Around *node = check_and_cast<Basic802154Around*>(getParentModule()->getParentModule()->getParentModule()->getSubmodule("node",0)->getSubmodule("Communication")->getSubmodule("MAC"));
                    node->t->setRelation(SELF_MAC_ADDRESS, associatedPAN);
                }
            }
            
            if(getTimer(CCH_SELECT) != -1)
                cancelTimer(CCH_SELECT);
            
            //trace() << "OO formation " << getTimer(CLUSTER_FORMATION);  //around - debug
            //trace() << "OO select " << getTimer(CCH_SELECT);  //around - debug
            
            toRadioLayer(createRadioCommand(SET_STATE, RX));
            setTimer(COLLISION_DOMAIN, maxFormationFrame * formationTime + ((double)SELF_MAC_ADDRESS)/100);
            break;
        }
            
        // Definição do Domínio de Colisão e estruturas para o algoritmo ARounD
        case COLLISION_DOMAIN: {
///            trace() << "ENTREI NO COLLISION_DOMAIN " << SELF_MAC_ADDRESS; //around - debug
            toRadioLayer(createRadioCommand(SET_STATE, RX));
            collisionDomain();
            break;
        }
        
        // Escalonamento dos Beacons
        case BEACON_SCHEDULE: {
            trace() << "ENTREI NO BEACON_SCHEDULE " << SELF_MAC_ADDRESS;  //around - debug
            
            //Formando quadro de escalonamento de Beacon ao longo da árvore
            schedulePacket = new Basic802154AroundPacket("Beacon Schedule packet", MAC_LAYER_PACKET);
            schedulePacket->setMac802154AroundPacketType(MAC_802154_BEACONSCHEDULE_PACKET);     // Tipo:  2 bytes
            schedulePacket->setPANid(SELF_MAC_ADDRESS);                                         // PANid: 2 bytes
            schedulePacket->setDstID(BROADCAST_MAC_ADDRESS);                                    // DstID: 2 bytes
            schedulePacket->setByteLength(BEACONSCHEDULE_PKT_SIZE);
            
            //Seta setMacState para MAC_STATE_FORMATION
            setMacState(MAC_STATE_FORMATION);
            
            if(isPANCoordinator){
                baseBI = simTime();
            }
            
            // Envia o pacote de escalonamento de beacon para os CHs filhos
///            trace() << "Transmitting [Beacon Scheduling and Addressing packet] now!!";
            toRadioLayer(schedulePacket);
            toRadioLayer(createRadioCommand(SET_STATE, TX));
            
            setTimer(ATTEMPT_TX, TX_TIME(schedulePacket->getByteLength()));
            currentFrameStart = getClock() + phyDelayRx2Tx;
            
            // Confirmar ao pai que é CH
            // PAN Coordinator não precisa fazer isso
            if(!isPANCoordinator){
                chAckPacket = new Basic802154AroundPacket("isCH Response", MAC_LAYER_PACKET);
                chAckPacket->setMac802154AroundPacketType(MAC_802154_CH_RESPONSE_PACKET);
                chAckPacket->setDstID(associatedPAN);
                chAckPacket->setSrcID(SELF_MAC_ADDRESS);
                chAckPacket->setByteLength(CHACK_PKT_SIZE);
                toRadioLayer(chAckPacket);
                toRadioLayer(createRadioCommand(SET_STATE, TX));
            }
            
            //Ativa o timer para criação do endereçamento hierarquico a 1 * BI a frente (já considerando os clusters escalonados!
            //O coordenador PAN será o responsável por começar a solicitação dos filhos
            if(isPANCoordinator){
                setTimer(ADDRESSING_REQUEST,  (beaconInterval * symbolLen) );
                //Definindo o time de atribuição para UM BI + o último terço do BI
                //O primeiro terço é usado para request, o segundo para response e terceiro para assign
                setTimer(ADDRESSING_ASSIGNMENT,  (beaconInterval * symbolLen) + (2*(beaconInterval * symbolLen)/3) );
                
                //Ativa o timer para ajustar a carga de pacotes dentro do cluster 2 * BI a frente (já considerando os clusters escalonados!
                //O coordenador PAN será o responsável por começar esse ajuste
                setTimer(RATE_ADJUSTMENT,  2 * (beaconInterval * symbolLen));
            
            }
            
            
            //Ativa o timer para envio de beacons a 3 * BI na frente!!
            //Todos os nós irão setar seu envio de beacon, considerando o seu escalonamento
            //setTimer(FRAME_START,  (3 * beaconInterval * symbolLen) + ( (offsetCH[SELF_MAC_ADDRESS] * symbolLen) - (simTime() - baseBI)) );
            
            //Ao invés de chamar o timer para gerar beacon, eu agora chamo um timer para re-escalonar a rede (apenas o PAN chama para re-escalonar)
            // Só depois, eu irei chamar o FRAME_START considerando o novo escalonamento
            if(isPANCoordinator)
                setTimer(RE_SCHEDULING,  (3 * beaconInterval * symbolLen) + ( (offsetCH[SELF_MAC_ADDRESS] * symbolLen) - (simTime() - baseBI)) );
            
            
            break;
        }
        
            
            
        // Escalonamento hierarquico - Solicitação da quantidade de filhos
        case ADDRESSING_REQUEST: {
            trace() << "ENTREI NO ADDRESSING_REQUEST " << SELF_MAC_ADDRESS;  //around - debug
            
            if(isPANCoordinator){
                baseBI = simTime();
            }
            
            //Solicita a quantidade de filhos
            addressingRequestPacket = new Basic802154AroundPacket("Addressing Request packet", MAC_LAYER_PACKET);
            addressingRequestPacket->setMac802154AroundPacketType(MAC_802154_ADDRESSING_REQUEST_PACKET);     // Tipo:  2 bytes
            addressingRequestPacket->setPANid(SELF_MAC_ADDRESS);                                         // PANid: 2 bytes
            addressingRequestPacket->setDstID(BROADCAST_MAC_ADDRESS);                                    // DstID: 2 bytes
            addressingRequestPacket->setByteLength(ADDRESSING_REQUEST_PKT_SIZE);
            
            //Seta setMacState para MAC_STATE_FORMATION
            //setMacState(MAC_STATE_FORMATION);
            
            // Envia o pacote de solicitação de filhos para os CHs filhos
///            trace() << "Transmitting [Addressing Request packet] now!!";
            toRadioLayer(addressingRequestPacket);
            toRadioLayer(createRadioCommand(SET_STATE, TX));
            
            setTimer(ATTEMPT_TX, TX_TIME(addressingRequestPacket->getByteLength()));
            currentFrameStart = getClock() + phyDelayRx2Tx;
            
            break;
        }
            
        // Escalonamento hierarquico - Envio da quantidade de filhos
        case ADDRESSING_RESPONSE: {
            trace() << "ENTREI NO ADDRESSING_RESPONSE " << SELF_MAC_ADDRESS;  //around - debug
            
            //Responde com sua quantidade de filhos acumulado
            addressingResponsePacket = new Basic802154AroundPacket("Addressing Response packet", MAC_LAYER_PACKET);
            addressingResponsePacket->setMac802154AroundPacketType(MAC_802154_ADDRESSING_RESPONSE_PACKET);      // Tipo:  2 bytes
            addressingResponsePacket->setSrcID(SELF_MAC_ADDRESS);                                             // PANid: 2 bytes
            addressingResponsePacket->setDstID(associatedPAN);                                                // DstID: 2 bytes
            //Calcula a quantidade de filhos acumulado
            totalchildren = totalchildren + nchildren;
            trace() << "SEND " << SELF_MAC_ADDRESS << " ENVIO FILHOS ACUMULADOS = " << totalchildren << " PARA " << associatedPAN; // around - debug
            addressingResponsePacket->setChildrenCont(totalchildren);                                         // DstID: 2 bytes
            addressingResponsePacket->setByteLength(ADDRESSING_RESPONSE_PKT_SIZE);
            
            //Seta setMacState para MAC_STATE_FORMATION
            //setMacState(MAC_STATE_FORMATION);
            
            // Envia o pacote de solicitação de filhos para os CHs filhos
///            trace() << "Transmitting [Addressing Response packet] now!!";
            toRadioLayer(addressingResponsePacket);
            toRadioLayer(createRadioCommand(SET_STATE, TX));
            
            setTimer(ATTEMPT_TX, TX_TIME(addressingResponsePacket->getByteLength()));
            currentFrameStart = getClock() + phyDelayRx2Tx;
            
            break;
        }
            
        // Escalonamento hierarquico - Atribuição de endereços hierárquicos e blocos para os CHs filhos
        case ADDRESSING_ASSIGNMENT: {
            
            trace() << "ENTREI NO ADDRESSING_ASSIGNMENT " << SELF_MAC_ADDRESS;  //around - debug
            
            map <int,bool>::const_iterator addrit;
            //vector<int>::const_iterator addrit2;
            vector<int>::iterator addrit2;
            
            if(isPANCoordinator){
                //Definindo o seu bloco de endereços
                startAddrBlock = 1;
                endAddrBlock = totalchildren + associatedDevices.size();  // os filhos requisitados + seus filhos
                //Definindo o seu proprio endereco para hirarquico 0, apenas na estrutura global
                routingTable[SELF_MAC_ADDRESS] = 0; //Estrutura global
                invroutingTable[0] = SELF_MAC_ADDRESS; //Para ser utilizada na analise protocolar
                
                addressCount = 1; //contador de ajuda na atribuição
                
                // Atribuindo hierarquico e bloco para os CHs filhos
                for (addrit2 = associatedCH.begin(); addrit2 != associatedCH.end(); addrit2++) {
                    
                    childAddress[*addrit2] = addressCount; //Estrutura de endereços hierarquicos locais
                    routingTable[*addrit2] = addressCount; //Estrutura de endereços hierarquicos globais
                    invroutingTable[addressCount] = *addrit2; //Para ser utilizada na analise protocolar
                    
                    trace() << "Enviando ADDRESSING_ASSIGNMENT para " << *addrit2;

                    //vector auxiliar
                    auxBlockVector.clear();
                    auxBlockVector.push_back(addressCount);  //incluindo o endereco inicial
                    auxBlockVector.push_back(addressCount + chAddrCount[*addrit2]);  //incluindo o endereco final
                    
                    //Alimentando as estruturas local e global que guarda o CH e seu respectivo endereço inicil e final do seu bloco
                    chAddrBlock[*addrit2] = auxBlockVector;  // local
                    addressBlock[*addrit2] = auxBlockVector; // global
                    
                    //Pacote para atribuir bloco de endereços para os CHs filhos
                    addressingAssignmentPacket = new Basic802154AroundPacket("Addressing AssigNment packet", MAC_LAYER_PACKET);  //definindo o pacote
                    addressingAssignmentPacket->setMac802154AroundPacketType(MAC_802154_ADDRESSING_ASSIGNMENT_PACKET);  // Tipo:  2 bytes
                    addressingAssignmentPacket->setSrcID(SELF_MAC_ADDRESS);                                             // PANid: 2 bytes
                    addressingAssignmentPacket->setByteLength(ADDRESSING_ASSIGNMENT_PKT_SIZE);
                    //Defining the Start and End of the Address block for each CH
                    addressingAssignmentPacket->setDstID(*addrit2);                                          // DstID: 2 bytes
                    //O endereço inicial do bloco para o CH filho
                    addressingAssignmentPacket->setStartAddressBlock(addressCount);                         // StartAddressBlock: 2 bytes
                    //O endereço final do bloco para o CH filho, note que aqui é incluído um endereço para o próprio filho (addressCount)
                    addressingAssignmentPacket->setEndAddressBlock(addressCount + chAddrCount[*addrit2]);    // EndAddressBlock: 2 bytes
                    
                    toRadioLayer(addressingAssignmentPacket);
                    toRadioLayer(createRadioCommand(SET_STATE, TX));
                    
                    setTimer(ATTEMPT_TX, TX_TIME(addressingAssignmentPacket->getByteLength()));

                    addressingAssignmentPacket = new Basic802154AroundPacket("Addressing AssigNment packet", MAC_LAYER_PACKET);  //definindo o pacote
                    addressingAssignmentPacket->setMac802154AroundPacketType(MAC_802154_ADDRESSING_ASSIGNMENT_PACKET);  // Tipo:  2 bytes
                    addressingAssignmentPacket->setSrcID(SELF_MAC_ADDRESS);                                             // PANid: 2 bytes
                    addressingAssignmentPacket->setByteLength(ADDRESSING_ASSIGNMENT_PKT_SIZE);
                    //Defining the Start and End of the Address block for each CH
                    addressingAssignmentPacket->setDstID(*addrit2);                                          // DstID: 2 bytes
                    //O endereço inicial do bloco para o CH filho
                    addressingAssignmentPacket->setStartAddressBlock(addressCount);                         // StartAddressBlock: 2 bytes
                    //O endereço final do bloco para o CH filho, note que aqui é incluído um endereço para o próprio filho (addressCount)
                    addressingAssignmentPacket->setEndAddressBlock(addressCount + chAddrCount[*addrit2]);    // EndAddressBlock: 2 bytes

                    toRadioLayer(addressingAssignmentPacket);
                    toRadioLayer(createRadioCommand(SET_STATE, TX));
                    
                    setTimer(ATTEMPT_TX, TX_TIME(addressingAssignmentPacket->getByteLength()));

                    currentFrameStart = getClock() + phyDelayRx2Tx;
                    addressingAssignmentPacket = NULL;
                    
                    addressCount = addressCount + chAddrCount[*addrit2] + 1;  //Atualizando addressCount com o início do próximo bloco
             
                }
                
                // Atribuindo hierarquico para seus outros filhos
                for (addrit = associatedDevices.begin(); addrit != associatedDevices.end(); addrit++) {
                    bool flag = false;  //flag para saber se o nó filho é ou não CH filho. Se for, já foi endereçado
                    for(addrit2 = associatedCH.begin(); addrit2 != associatedCH.end(); addrit2++){
                        if(addrit->first == *addrit2)
                            flag = true;
                    }
                    if(!flag){
                        childAddress[addrit->first] = addressCount; //Estrutura de endereços hierarquicos locais
                        routingTable[addrit->first] = addressCount; //Estrutura de endereços hierarquicos locais
                        invroutingTable[addressCount] = addrit->first; //Para ser utilizado na analise protocolar
///                        trace() << "Atribui ao no " << addrit->first << " o end hierarquico: " << addressCount;  // around - debug
                        addressCount++;
                    }
                }
                
            } else {
                
                addressCount = startAddrBlock; //contador de ajuda na atribuição
                
                // Atribuindo hierarquico e bloco para os CHs filhos
                for (addrit2 = associatedCH.begin(); addrit2 != associatedCH.end(); addrit2++) {
                    
                    childAddress[*addrit2] = addressCount; //Estrutura de endereços hierarquicos locais
                    routingTable[*addrit2] = addressCount; //Estrutura de endereços hierarquicos globais
                    invroutingTable[addressCount] = *addrit2; //Para ser utilizado na analise protocolar
                    
                    //vector auxiliar
                    auxBlockVector.clear();
                    auxBlockVector.push_back(addressCount);  //incluindo o endereco inicial
                    auxBlockVector.push_back(addressCount + chAddrCount[*addrit2]);  //incluindo o endereco final
                    
                    //Alimentando as estruturas local e global que guarda o CH e seu respectivo endereço inicil e final do seu bloco
                    chAddrBlock[*addrit2] = auxBlockVector;
                    addressBlock[*addrit2] = auxBlockVector;
                    
                    //Pacote para atribuir bloco de endereços para os CHs filhos
                    addressingAssignmentPacket = new Basic802154AroundPacket("Addressing AssigNment packet", MAC_LAYER_PACKET);  //definindo o pacote
                    addressingAssignmentPacket->setMac802154AroundPacketType(MAC_802154_ADDRESSING_ASSIGNMENT_PACKET);  // Tipo:  2 bytes
                    addressingAssignmentPacket->setSrcID(SELF_MAC_ADDRESS);                                             // PANid: 2 bytes
                    addressingAssignmentPacket->setByteLength(ADDRESSING_ASSIGNMENT_PKT_SIZE);
                    //Defining the Start and End of the Address block for each CH
                    addressingAssignmentPacket->setDstID(*addrit2);                                          // DstID: 2 bytes
                    //O endereço inicial do bloco para o CH filho
                    addressingAssignmentPacket->setStartAddressBlock(addressCount);                         // StartAddressBlock: 2 bytes
                    //O endereço final do bloco para o CH filho, note que aqui é incluído um endereço para o próprio filho (addressCount)
                    addressingAssignmentPacket->setEndAddressBlock(addressCount + chAddrCount[*addrit2]);    // EndAddressBlock: 2 bytes
                    
                    toRadioLayer(addressingAssignmentPacket);
                    toRadioLayer(createRadioCommand(SET_STATE, TX));
                    
                    setTimer(ATTEMPT_TX, TX_TIME(addressingAssignmentPacket->getByteLength()));
                    currentFrameStart = getClock() + phyDelayRx2Tx;
                    addressingAssignmentPacket = NULL;
                    
                    addressCount = addressCount + chAddrCount[*addrit2] + 1;  //Atualizando addressCount com o início do próximo bloco
                    
                }
                
                // Atribuindo hierarquico para seus outros filhos
                for (addrit = associatedDevices.begin(); addrit != associatedDevices.end(); addrit++) {
                    bool flag = false;  //flag para saber se o nó filho é ou não CH filho. Se for, já foi endereçado
                    for(addrit2 = associatedCH.begin(); addrit2 != associatedCH.end(); addrit2++){
                        if(addrit->first == *addrit2)
                            flag = true;
                    }
                    if(!flag){
                        childAddress[addrit->first] = addressCount; //Estrutura de endereços hierarquicos locais
                        routingTable[addrit->first] = addressCount; //Estrutura de endereços hierarquicos locais
                        invroutingTable[addressCount] = addrit->first; //Para ser utilizado na analise protocolar
///                        trace() << "Atribui ao no " << addrit->first << " o end hierarquico: " << addressCount;  // around - debug
                        addressCount++;
                    }
                }
                
            }

            

            break;
        }
 
        
        // Início de ajuste de taxa de pacotes para cada cluster
        case RATE_ADJUSTMENT: {
///            trace() << "ENTREI NO RATE_ADJUSTMENT " << SELF_MAC_ADDRESS;  //around - debug
            
            if ((isPANCoordinator) || (isCH)) {
///                trace() << "RATE ADJUSTMENT agora!";  // Around - Debug
                
                rateAdjustPacket = new Basic802154AroundPacket("Rate Adjustment packet", MAC_LAYER_PACKET);
                rateAdjustPacket->setMac802154AroundPacketType(MAC_802154_RATE_ADJUSTMENT_PACKET);
                rateAdjustPacket->setDstID(BROADCAST_MAC_ADDRESS);
                rateAdjustPacket->setPANid(SELF_MAC_ADDRESS);
                
                //Calculo da taxa de pacotes para cada cluster
                // Se fixedPacketRate for verdadeiro (default), todos os nós tem a mesma taxa de dados (basePacketRate)
                // Se for falso, a taxa de dados é com base na quant de nós do cluster
				if(isPrio)
					basePacketRate = packet_rate_prio;
				else 
					basePacketRate = packet_rate_noprio;                
					
				if(fixedPacketRate)
                    rateAdjustPacket->setRate(basePacketRate);
                else
                    rateAdjustPacket->setRate(basePacketRate / (nchildren - associatedCH.size()) ); // sem considerar os CH's filhos, posto que eles não irão gerar dados de background
				
                    //rateAdjustPacket->setRate(basePacketRate / nchildren); // considerando os CH's filhos
                
                rateAdjustPacket->setByteLength(ADDRESSING_RATE_ADJUSTMENT_PKT_SIZE);
                
                if(isPANCoordinator){
                    baseBI = simTime();
                }
                
///                trace() << "Transmitting [Rate Adjustment Packet] now! " << SELF_MAC_ADDRESS;
                toRadioLayer(rateAdjustPacket);
                toRadioLayer(createRadioCommand(SET_STATE, TX));
                
                setTimer(ATTEMPT_TX, TX_TIME(rateAdjustPacket->getByteLength()));
                rateAdjustPacket = NULL;    // define para nulo o beaconpacket
                
            }
            break;
        }
        
        
            
        // Timer para re-escalonar a rede (escalonamento valendo e final)
        case RE_SCHEDULING: {
///            trace() << "RE-ESCALONANDO A REDE!"; //Around - debug
            reScheduling();
            break;
        }
            
        
        //Timer para os nós escalonra
        case BEGIN_BEACON: {
///            trace() << "ENTREI NO SCHEDULING FINAL DOS BEACONS PARA O  FRAME_START " << SELF_MAC_ADDRESS;  //around - debug
            
            //Formando quadro de escalonamento de Beacon ao longo da árvore
            schedulePacket = new Basic802154AroundPacket("Beacon Schedule packet", MAC_LAYER_PACKET);
            schedulePacket->setMac802154AroundPacketType(MAC_802154_BEGINBEACON_PACKET);     // Tipo:  2 bytes
            schedulePacket->setPANid(SELF_MAC_ADDRESS);                                         // PANid: 2 bytes
            schedulePacket->setDstID(BROADCAST_MAC_ADDRESS);                                    // DstID: 2 bytes
            schedulePacket->setByteLength(BEACONSCHEDULE_PKT_SIZE);
            
            //Seta setMacState para MAC_STATE_FORMATION
            setMacState(MAC_STATE_FORMATION);
            
            if(isPANCoordinator){
                baseBI = simTime();
            }
            
            // Envia o pacote de escalonamento de beacon para os CHs filhos
            ///            trace() << "Transmitting [Beacon Scheduling and Addressing packet] now!!";
            toRadioLayer(schedulePacket);
            toRadioLayer(createRadioCommand(SET_STATE, TX));
            
            setTimer(ATTEMPT_TX, TX_TIME(schedulePacket->getByteLength()));
            currentFrameStart = getClock() + phyDelayRx2Tx;
            
           
            //Definindo o SO para cada CH
            // Os SO's foram calculados na funcao rescheduling e guardados dentro do map soCH
            // Assim, cada CH abritui seu proprio SO armazenado no map soCH
            superframeOrder = soCH[SELF_MAC_ADDRESS];
            beaconOrder = globalBO;
            beaconInterval = aBaseSuperframeDuration * (1 << beaconOrder);
            
            trace() << "CH " << SELF_MAC_ADDRESS << " com SO = " << superframeOrder;
            
            trace() << "Buffer Size do node " << SELF_MAC_ADDRESS << " tem tamanho " << macBufferSize;
            
            
            // VERIFICO SE JÁ TEM TEMPORIZADOR FRAME-START ATIVO, POIS SE TIVER, EH PQ EU JÁ PASSEI POR AQUI E TO TENTANDO REENVIAR O SCHEDULING PARA OS CHS FILHOS QUE NAO FORAM ESCLONADOS
            if(getTimer(FRAME_START) != -1){
                trace() << "JA TENHO FRAME_START ATIVO! ENTAO NAO ATIVO MAIS!";
                break;
            }
            
            //Ativa o timer para envio de beacons a 2*BI na frente!!
            //Todos os nós irão setar seu envio de beacon, considerando o seu escalonamento
            
		//daniel - timer fica negativo, entao tive que aumentar o termo de multiplicação de 3 para 5
            trace() << "CH " << SELF_MAC_ADDRESS << " enviar beacon em " << 5*(beaconInterval * symbolLen) + ( (offsetCH[SELF_MAC_ADDRESS] * symbolLen) - (simTime() - baseBI)) ;
            setTimer(FRAME_START,  3*(beaconInterval * symbolLen) + ( (offsetCH[SELF_MAC_ADDRESS] * symbolLen) - (simTime() - baseBI)) ); // o início dos beacons é um BI + schedule de cada CH
            
            
            //TENTANDO RESOLVER O PROBLEMA DE ALGUNS CHs NAO RECEBEREM O QUADRO DE BEACON_SCHEDULE.
            //Eles vão para um temporizador Check e se verificarem que seus CHs filhos não receberam o quadro de scheduling, eles entram novamente no BEGIN_BEACON
            
            setTimer(CHECK_SCHEDULING, (beaconInterval * symbolLen) + (offsetFormation[SELF_MAC_ADDRESS] * symbolLen));
            
            
            break;
        
        }
            
        // Início de um novo superframe
		case FRAME_START: {
			trace() << "ENTREI NO FRAME_START " << SELF_MAC_ADDRESS;  //around - debug
            
            if ((isPANCoordinator) || (isCH)) {	// Sendo um coordenador, cria e broadcast pacote beacon
///                trace() << "BEACON agora!";  // Around - Debug
                //cria o pacote BEACON
                beaconPacket = new Basic802154AroundPacket("PAN beacon packet", MAC_LAYER_PACKET);
				// Definde Endereço destino: BROADCAST
                beaconPacket->setDstID(BROADCAST_MAC_ADDRESS);
				// Define o ID da PAN: seu próprio endereço MAC
                beaconPacket->setPANid(SELF_MAC_ADDRESS);
				// Seta o tipo do pacote MAC: MAC_802154_BEACON_PACKET (Definido no enum no arquivo msg)
                beaconPacket->setMac802154AroundPacketType(MAC_802154_BEACON_PACKET);
				// Define o parâmetro Beacon Order do pacote beacon
                beaconPacket->setBeaconOrder(beaconOrder);
				// Define o parâmetro Superframe Order do pacote beacon
                beaconPacket->setSuperframeOrder(superframeOrder);
				// Número de sequencia do quadro beacon, se passar de 255, zera
                if (++macBSN > 255)
                    macBSN = 0;
                // Define o parâmetro macBSN do pacote beacon
				beaconPacket->setBSN(macBSN);
				beaconPacket->setCAPlength(aNumSuperframeSlots);
				
				// Campos de GTS e Tamanho do CAP são setados na camada de decisão, em StaticGTS802154
				prepareBeacon_hub(beaconPacket);
                
                // Campos de Endereços Pendentes - Teremos Comunicação Indireta
                
                //ZERO o buffer DownTXBuffer, pois os nós terão que solicitar novamente
                while(!DownTXBuffer.empty()){
                    DownTXBuffer.pop(); //vou apagando a fila original
                }
                
                //trace() << "MEU numPendingAddr: " << numPendingAddr;
                
                if(numPendingAddr > 0){
                    //Definindo os campos do beacon com os dados para comunicação indireta
                    beaconPacket->setNumAddrPending(numPendingAddr);   // Definindo a quantidade de "endereços" pendentes
                    beaconPacket->setAddrListArraySize(numPendingAddr);  // Definindo o tamanho do vetor dos endereços pendentes
                    for (int i=0; i < numPendingAddr; i++) {
                        //trace() << "Endereco Pendente: " << pendingAddrList[i];  // Around - debug
                        beaconPacket->setAddrList(i,pendingAddrList[i]);  // setando no beacon
                    }
                    
                }
                // Definindo o tamanho do quadro beacon:
                //BASE_BEACON_PKT_SIZE é definido utilizando #define no arquivo .h
                beaconPacket->setByteLength(BASE_BEACON_PKT_SIZE + beaconPacket->getGTSlistArraySize() * GTS_SPEC_FIELD_SIZE + numPendingAddr * 2);
				// Tamanho do CAP a partir do quadro beacon
                CAPlength = beaconPacket->getCAPlength();
				// Definição da Variável fim do CAP
                CAPend = CAPlength * aBaseSlotDuration * (1 << superframeOrder) * symbolLen;
				// Incrementa o contador de beacons enviados
                sentBeacons++;
                
                //Testando - Calculando o tempo para usar na fórmula do Around
                timeBeaconPAN = getClock() + TX_TIME(beaconPacket->getByteLength());
                
                // TRACE: transmitindo o quadro beacon com número de sequencia
				//trace() << "Transmitting [PAN beacon packet] now, BSN = " << macBSN;
				//Seta setMacState para MAC_STATE_CAP
                setMacState(MAC_STATE_CAP_CH);
				// Envia o pacote de beacon para o rádio
                toRadioLayer(beaconPacket);
				//Comando para o rádio para mudar para o modo TX
                toRadioLayer(createRadioCommand(SET_STATE, TX));
                // Seta timer ATTEMPT_TX com tempo definido pelo #define no arquivo .h
				setTimer(ATTEMPT_TX, TX_TIME(beaconPacket->getByteLength()));
				beaconPacket = NULL;    // define para nulo o beaconpacket
                
                //Atualiza o tempo do início do próximo superframe, iniciado logo abaixo
				currentFrameStart = getClock() + phyDelayRx2Tx;
				// Seta o timer para envio de Superframe, num tempo de Intervalos de Beacons
                setTimer(FRAME_START, beaconInterval * symbolLen);
                
                // Configurando o Duty Cycle para os coordenadores
                // Seta o timer para dormir após o período ativo
                
                //Para escalonamento bottom-up, os CHs mais profundos comecam enviando seus beacons e depois vao dormir
                // Assim, eles não estarão acordados para receber o beacon de seus pais, gerando um grande problema
                // Dessa forma, eu impeço que os CH's mais profundos durmam durante seu primeiro beacon para o escalonamento bottom-up
                if(sentBeacons == 1){
                    trace() << "Not Sleeping in first time!";
                }
			
            } else {    // Se não é um coordenador PAN, fica esperando por beacon
				// Comando para o rádio para setar o estado RX para receber beacon
                //toRadioLayer(createRadioCommand(SET_STATE, RX));
                // Seta time BEACON_TIMEOUT com tempo guardTime
                //trace() << "PREPARANDO PRO CAP";  // Around - Debug
                
                //setTimer(BEACON_TIMEOUT, guardTime * 3);
			}
			break;
		}
            
        // Se preparar para o CAP - nós recebendo beacons
        // Eu separei o frame_receive do frame_start, pois nós CHs também atuam como nós filhos
        case FRAME_RECEIVE: {
///            trace() << "ENTREI NO FRAME_RECEIVE " << SELF_MAC_ADDRESS;  //around - debug
            
            // Comando para o rádio para setar o estado RX para receber beacon
            toRadioLayer(createRadioCommand(SET_STATE, RX));
            // Seta time BEACON_TIMEOUT com tempo guardTime
            trace() << "PREPARANDO PRO CAP";  // Around - Debug
            
            setTimer(BEACON_TIMEOUT, guardTime * 3);
            break;
        }

        case GTS_START: {
			if (macState == MAC_STATE_SLEEP) {
				toRadioLayer(createRadioCommand(SET_STATE, RX));
			}
			setMacState(MAC_STATE_GTS);
			
			// we delay transmission attempt by the time requred by radio to wake up
			// note that GTS_START timer was scheduled exactly phyDelaySleep2Tx seconds
			// earlier than the actual start time of GTS slot
			setTimer(ATTEMPT_TX, phyDelaySleep2Tx);

			// set a timer to go to sleep after this GTS slot ends
			setTimer(SLEEP_START, phyDelaySleep2Tx + GTSlength);

			// inform the decision layer that GTS has started
			startedGTS_node();
			break;
		}

		// timeout de beacon atingido - indica que beacon foi perdido por esse nó
		case BEACON_TIMEOUT: {
			//incrementa lostBeacon, para testar se perdeu sincronia total com o PAN ou se apenas perdeu um ou outro beacon
            //Padrão define que um nó está fora da rede se perder aMaxLostBeacons beacons
            trace() << "ENTREI NO BEACON TIMEOUT!";
            lostBeacons++;
			//Se perder >= aMaxLostBeacons, perdeu sincronia com PAN
            if (lostBeacons >= aMaxLostBeacons) {
				// Imprime a perca de sincronização
                trace() << "Lost synchronisation with PAN " << associatedPAN;
				// Seta MacState para MAC_STATE_SETUP
                setMacState(MAC_STATE_SETUP);
				associatedPAN = -1;             //associatedPAN volta para o inicial -1
				desyncTimeStart = getClock();   //pega o momento que se desincronizou
				disconnectedFromPAN_node();     //essa função serve para resetar os slots de GTS
				//se tem pacote atual...
                if (currentPacket)
                    //...Chama a função para imprimir a mensagem do argumento
                    clearCurrentPacket("No PAN");
			} else
                // o nó é associado, mas perdeu um beacon e será acordado para o próximo
                if (associatedPAN != -1) {
				    //imprime no trace a perca do beacon
                    trace() << "Missed beacon from PAN " << associatedPAN <<
				    ", will wake up to receive next beacon in " <<
				    beaconInterval * symbolLen - guardTime * 3 << " seconds";
                    //Seta MacState para SLEEP
                    setMacState(MAC_STATE_SLEEP);
                    // Manda comando pro rádio para dormir
                    toRadioLayer(createRadioCommand(SET_STATE, SLEEP));
                    //Seta temporizador FRAME_START
                    //setTimer(FRAME_START, beaconInterval * symbolLen - guardTime * 3); //original("ATTEMPT_TX timer");
                    setTimer(FRAME_RECEIVE, beaconInterval * symbolLen - guardTime * 3);
                }
			break;
		}
		
		// packet was not received
		case ACK_TIMEOUT: {
			collectPacketHistory("NoAck");
			attemptTransmission("ACK timeout");
			break;
		}

		// Transmissão anterior é resetada, tenta uma nova transmissão
		case ATTEMPT_TX: {
			// pega o tempo do Timer ACK_TIMEOUT
            if(getTimer(ACK_TIMEOUT) != -1){       
			   break;
			}
			// Função definida abaixo
            attemptTransmission("ATTEMPT_TX timer");
			break;
		}

		// timer to preform Clear Channel Assessment (CCA) 
		case PERFORM_CCA: {
			if (macState == MAC_STATE_GTS || macState == MAC_STATE_SLEEP) break;
			CCA_result CCAcode = radioModule->isChannelClear();
			if (CCAcode == CLEAR) {
				//Channel clear
				if (--CW > 0) {
					setTimer(PERFORM_CCA, aUnitBackoffPeriod * symbolLen);
				} else {
					transmitCurrentPacket();
				}
			} else if (CCAcode == BUSY) {
				//Channel busy
				CW = enableSlottedCSMA ? 2 : 1;
				if (++BE > macMaxBE)
					BE = macMaxBE;
				if (++NB > macMaxCSMABackoffs) {
					collectPacketHistory("CSfail");
					currentPacketRetries--;
					attemptTransmission("Current NB exeeded maxCSMAbackoffs");
				} else {
					performCSMACA();
				}
			} else if (CCAcode == CS_NOT_VALID_YET) {
				//Clear Channel Assesment (CCA) pin is not valid yet
				setTimer(PERFORM_CCA, phyDelayForValidCS);
			} else {	
				//Clear Channel Assesment (CCA) pin is not valid at all (radio is sleeping?)
				trace() << "ERROR: isChannelClear() called when radio is not ready";
				toRadioLayer(createRadioCommand(SET_STATE, RX));
			}
			break;
		}

		case SLEEP_START: {
			// SLEEP_START timer can sometimes be scheduled in the end of a frame
			// i.e. when BEACON_ORDER = FRAME_ORDER, overlapping with the interval 
			// when a node already tries to prepare for beacon reception. Thus 
			// check if BEACON_TIMEOUT timer is set before going to sleep
			if (getTimer(BEACON_TIMEOUT) != -1) break;
            
            //Around - debug
            trace() << "INDO DORMIR!! ";  // Around - Debug
            
			cancelTimer(PERFORM_CCA);
			setMacState(MAC_STATE_SLEEP);
			toRadioLayer(createRadioCommand(SET_STATE, SLEEP));
			break;
		}
		
		case BACK_TO_SETUP: {
			// Este Timer é escalonado para o fim do período CAP quando beacon é recebio, mas o nó não está (ainda) conectado
            // Assim, quando esse timer expirar e o nó não estiver conectado ainda, ele terá que ir de volta para o estágio de SETUP
			if (associatedPAN == -1)       //ou seja, não terminou a conexão à PAN em questão
                setMacState(MAC_STATE_SETUP);   //volta para o estado MAC_STATE_SETUP
            break;
        }
            
        case SPENT_ENERGY: {
            // Este Timer imprimi no trace o consumo de energia de cada nó a cada segundo.
            // Posso inserir no ned uma variável para definir o tempo periódico
            // Posso também inserir esses valores numa estrutura e depois inserir essa estrutura no collectOutput (para se pensar)
            trace() << std::setprecision(5) << "ENERGY_NODE " << SELF_MAC_ADDRESS << " NODE_ENERGY " << resMgrModule->getSpentEnergy() << " NODE_CLOCK " << getClock();
            setTimer(SPENT_ENERGY,1);
            break;
        }
            
         
            
        // UM TEMPORIZADOR PARA CH's CHECAREM SE SEUS FILHOS RECEBERAM O QUADRO DE BEACON
        case CHECK_SCHEDULING:{
            trace() << "CHECK SCHEDULING " << SELF_MAC_ADDRESS;
            
            vector<int>::iterator checkCH;
            for (checkCH = associatedCH.begin(); checkCH != associatedCH.end(); checkCH++) {
                if(checkSched[*checkCH] == false){
                    trace() << "CH FILHO " << *checkCH << " nao foi escalonado!!";
                    setTimer(BEGIN_BEACON,0);
                } else {
                    trace() << "CH FILHO " << *checkCH << " ESCALONADO!!";
                }
            }
            break;
        }
        
        case NEIGHBORHOOD_KNOWLEDGE: {
			
			knowledgeFrameCount++;
	
			setMacState(MAC_STATE_DISCOVERY);
			
			knowledgePacket = new Basic802154AroundPacket("Neighborhood Knowledge packet", MAC_LAYER_PACKET);
            knowledgePacket->setMac802154AroundPacketType(MAC_802154_KNOWLEDGE_PACKET);     // Tipo:  2 bytes
            knowledgePacket->setSrcID(SELF_MAC_ADDRESS);                                    // Srcid: 2 bytes
            knowledgePacket->setDstID(BROADCAST_MAC_ADDRESS);                               // DstID: 2 bytes
            knowledgePacket->setByteLength(KNOWLEDGE_PKT_SIZE);

            double rand_num = (rand() % 1000);
            rand_num = rand_num/1000;
            //trace() << "Numero rand = " << rand_num;
            
            if(rand_num <= prob_s){
                toRadioLayer(createRadioCommand(SET_STATE, SLEEP));
                //trace() << "Dormindo!";
            }else{
                if(rand_num > prob_s && rand_num <= (1-prob_t) ){
                    toRadioLayer(createRadioCommand(SET_STATE, RX));
                    //trace() << "Escutando!";
                }else{
                    toRadioLayer(createRadioCommand(SET_STATE, TX));
                    toRadioLayer(knowledgePacket);
                    setTimer(ATTEMPT_TX, TX_TIME(knowledgePacket->getByteLength()));
                    //trace() << "Transmitting [Cluster KNOWLEDGE packet] now!!";
                }
            }


			// Envia o pacote de formação do cluster para o rádio
            
            // toRadioLayer(createRadioCommand(SET_STATE, TX));
            // toRadioLayer(knowledgePacket);
            // setTimer(ATTEMPT_TX, TX_TIME(knowledgePacket->getByteLength()));

            if(knowledgeFrameCount < nSlotsDiscovery){
				setTimer(NEIGHBORHOOD_KNOWLEDGE, TX_TIME(knowledgePacket->getByteLength())+(0.25/1000));
            }else{
                calcRSSI_avg();
                toRadioLayer(createRadioCommand(SET_STATE, RX));
                setMacState(MAC_STATE_FORMATION);
                if(SELF_MAC_ADDRESS == 0)
                    setTimer(CLUSTER_FORMATION, 1);	
            }  
            //trace() << "Tamanho do pacote de descobrimento = " << knowledgePacket->getByteLength();
            //trace() << "Tempo de transmissão do pacote de descobrimento = " << TX_TIME(knowledgePacket->getByteLength());
        break;			

		} 

        case CHANGE_MODE: {

            toRadioLayer(createRadioCommand(SET_MODE, ""));

        }

	}

}



//----------------------------------------------------------------------------------------------------------------------------
//
//  FROM RADIO LAYER
//
//----------------------------------------------------------------------------------------------------------------------------



// Função para manusear um quadro MAC recebido da camada mais abaixo (física ou rádio)
void Basic802154Around::fromRadioLayer(cPacket * pkt, double rssi, double lqi)
{
    //criar um pacote rcvPacket, que irá receber pkt
    Basic802154AroundPacket *rcvPacket = dynamic_cast<Basic802154AroundPacket*>(pkt);
    if (!rcvPacket) {
        return;
    }
    
    //Testa se o DstID é diferente do dele ou BROADCAST_MAC_ADDRESS. Se o endereço destino não é o dele ou não for de Broadcast, a função retorna
    if (rcvPacket->getDstID() != SELF_MAC_ADDRESS &&
        rcvPacket->getDstID() != BROADCAST_MAC_ADDRESS) {
        return;
    }
    
    //Entra no switch, considerando o tipo do Pacote
    switch (rcvPacket->getMac802154AroundPacketType()) {
            
        //Recebeu um pacote de Formação
        case MAC_802154_FORMATION_PACKET: {
            //Coordenadores PAN, cluster-heads e nós já associados ignoram quadros de formação de clusters
            if ((isPANCoordinator) || (isCH) || (associatedPAN != -1))
                break;   

            trace() << "FORMATION_PACKET recebido de " << rcvPacket->getPANid(); 

            map <int,double> my_rel;
            my_rel = nhTable[SELF_MAC_ADDRESS];

            if(my_rel.find(rcvPacket->getPANid()) == my_rel.end()){
                 trace() << "O nodo "<< rcvPacket->getPANid() << " nao esta na minha tabela.";
                 break;
            }				

			if(relationTable.size() == 0){
				createTable();
			}

            double offset = TX_TIME(rcvPacket->getByteLength());    // Função do OMNeT, que filtra o tamanho total do pacote (incluindo GTS)
            currentFrameStart = getClock() - offset;                //O inicio do quadro de formação está no passado
            
///            trace() << "offset " << offset << " currentframestart " << currentFrameStart << " time agora " << getClock();  // Around - Debug
            
			   

            // Setando as variáveis recebidas do pacote
            depth = rcvPacket->getDepth() + 1;                      //pega profundidade no cluster-tree
            formationSeqNum = rcvPacket->getFormBSN();
///         trace() << "Depth " << depth << " e formationSeqNum " << formationSeqNum;  // Around - Debug
			
			               

            // Seta o estado do MAC para MAC_STATE_FORMATION
            setMacState(MAC_STATE_FORMATION);     

	        // Função para transmitir um quadro de associação
	        Basic802154AroundPacket *associate = new Basic802154AroundPacket("PAN Associate Request", MAC_LAYER_PACKET);
	        associate->setDstID(rcvPacket->getPANid());
	        associate->setPANid(rcvPacket->getPANid());
	        associate->setMac802154AroundPacketType(MAC_802154_ASSOCIATE_PACKET);
	        associate->setSrcID(SELF_MAC_ADDRESS);
	        associate->setByteLength(ASSOCIATE_PKT_SIZE);
			associate->setPriority(isPrio);  
			const char *str = relationTable.c_str();
			associate->setTableNH(str);
	        
	        transmitPacket(associate);
            
            //Around: Após enviar quadro de associação, ir para Seleçao de CH
            //Tempo: é o tempo de formação vezes a quantidade de envio de pacotes de formação, menos o offset (quando o pacote foi enviado pelo CH) e o guardTime (começar antes do root).
            //Considero também o envio de vários quadros de formação = formationSeqNum * formationTime
            
			setTimer(CCH_SELECT, maxFormationFrame * formationTime - offset - guardTime - formationSeqNum * formationTime);
			
                     
            break;
        }
        
            
        // Recebeu uma confirmação de associação e autorização para construir novo cluster
        case MAC_802154_CCH_PACKET: {
            //Autorização para novo cluster é enviado diretamente para um nó específico
            if(SELF_MAC_ADDRESS != rcvPacket->getDstID()){
///                trace() << "Nao eh meu mas recebi. No " << SELF_MAC_ADDRESS;  // Around - Debug
                break;
            }
            
			trace() << "Confirmacao recebida";

            //Valores de RSSI e LQI
///            trace() << "RSSI = " << rssi << "; e LQI = " << lqi;  // around - debug
            
            //Define sua associação ao Coordinator/Cluster-Head que enviou a confirmação
            associatedPAN = rcvPacket->getPANid();
            formationSeqNum = 0;
///            trace() << "No " << SELF_MAC_ADDRESS << " Associado ao PAN: " << associatedPAN << " com CH: " << rcvPacket->getNewCH();  // Around - Debug
            if (rcvPacket->getNewCH()) {
                isCH = true;  //Definindo que é CH. Porém, isso só deve acontecer na hora de buscar novos filhos. Se não conseguir, ele não deve ser cluster-head
				
				readTableCH(rcvPacket->getTableCH());
                position = rcvPacket->getPosition();
				
                
                //trace() << "timer : " << sqrt(pow(rcvPacket->getStartTime(),2));
				
                
				setTimer(SLEEP_START,0);
                //trace() << "Sou CH e estou indo dormir! Irei acordar " << (getClock() + sqrt(pow(rcvPacket->getStartTime(),2)));
				setTimer(CLUSTER_FORMATION, sqrt(pow(rcvPacket->getStartTime(),2)));  // com um delay com sobra de 0.01
            }else{
                if(rcvPacket->getPANid() != -1){
                    setTimer(SLEEP_START,0);
                    //trace() << "Sou nodo comum e estou indo dormir!";
                } 
            }
            
            //Se o getPANid for -1, é porque trata-se de uma desalocação. Assim, foi utilizado CSMA.
            // Então é necessário enviar um quadro de ACK para avisar o CH que deixou de ser CH e está tentando desalocar seus nós
            if(rcvPacket->getPANid() == -1){
                trace() << "MEU CH " << rcvPacket->getSrcID() << " FOI DESALOCADO. MEU NOVO ASSOCIATEDPAN: " << associatedPAN;
                Basic802154AroundPacket *ackPacket = new Basic802154AroundPacket("PAN associate response", MAC_LAYER_PACKET);
                ackPacket->setMac802154AroundPacketType(MAC_802154_ACK_PACKET);
                ackPacket->setSrcID(SELF_MAC_ADDRESS);
                ackPacket->setDstID(rcvPacket->getSrcID());
                ackPacket->setByteLength(ACK_PKT_SIZE);
                
                toRadioLayer(ackPacket);
                toRadioLayer(createRadioCommand(SET_STATE, TX));
                setTimer(ATTEMPT_TX, TX_TIME(ACK_PKT_SIZE));
            }
            
        
            break;
        }
            
            
            
        // Tive que criar esse case, para o caso de nós CH re-confirmando nós que são seus fihos
        // O procedimento de re-confirmação ocorre no collisionDomain
        case MAC_802154_AGAINCCH_PACKET: {
            //Autorização para novo cluster é enviado diretamente para um nó específico
            if(SELF_MAC_ADDRESS != rcvPacket->getDstID()){
                ///                trace() << "Nao eh meu mas recebi. No " << SELF_MAC_ADDRESS;  // Around - Debug
                break;
            }
            
            //Define minha associação ao meu pai que me avisou que eu sou seu filho
            associatedPAN = rcvPacket->getPANid();
            
            //Tenho que re-enviar o pacote para participar do CollisionDomain
            // Cria um pacote
            collisionDomainPacket = new Basic802154AroundPacket("Again Collision Domain Formation", MAC_LAYER_PACKET);
            
            collisionDomainPacket->setMac802154AroundPacketType(MAC_802154_COLLISIONDOMAIN_PACKET);
            collisionDomainPacket->setSrcID(SELF_MAC_ADDRESS);
            collisionDomainPacket->setDstID(BROADCAST_MAC_ADDRESS);
            collisionDomainPacket->setPANid(associatedPAN);
            if((isPANCoordinator) || (isCH)){
                collisionDomainPacket->setIsCH(true);
                if(nchildren < maxChildren)
                    collisionDomainPacket->setAssociationAllow(true);
                else
                    collisionDomainPacket->setAssociationAllow(false);
            }
            else{
                collisionDomainPacket->setIsCH(false);
                collisionDomainPacket->setAssociationAllow(false);
            }
            collisionDomainPacket->setByteLength(CCH_PKT_SIZE);
            toRadioLayer(collisionDomainPacket);
            toRadioLayer(createRadioCommand(SET_STATE, TX));
            setTimer(ATTEMPT_TX, TX_TIME(CCH_PKT_SIZE));
            
            break;
        }
        
        
        //
        case MAC_802154_COLLISIONDOMAIN_PACKET: {
            
            map <int,double> my_rel;
            my_rel = nhTable[SELF_MAC_ADDRESS];

            

			// Nós que estão conectados ao cluster-tree não participam disso
			if (associatedPAN == -1){
                int t = 1;
				if(rcvPacket->getIsCH() == true){
	                if(rcvPacket->getAssociationAllow() == true){
                            trace() << "SOU ORFAO. CH ACEITANDO NOS: " << rcvPacket->getSrcID();

                            //Vou pedir associação para esse nó
                            Basic802154AroundPacket *associate = new Basic802154AroundPacket("PAN Associate Request", MAC_LAYER_PACKET);
                            associate->setDstID(rcvPacket->getSrcID());
                            associate->setPANid(rcvPacket->getSrcID());
                            associate->setMac802154AroundPacketType(MAC_802154_ASSOCIATE_PACKET);
                            associate->setSrcID(SELF_MAC_ADDRESS);
                            associate->setByteLength(COMMAND_PKT_SIZE);                    
                            transmitPacket(associate);
                            
                            //tenho que verificar se o node que solicitou sua inclusão ainda tem TIMER para CollisionDomain ativo.
                            // Se tiver, ele ainda entrará no collisionDomain e não será necessário chamá-lo novamente
                            // Caso contrário, alinho no buffer um novo pacote para collisionDomain
                                if(getTimer(COLLISION_DOMAIN) != -1){
        ///                            trace() << "TIMER ROLANDO!"; //around - debun
                                } else {
        ///                            trace() << "JA SE FOI!"; //around - debug
                                    //Assim, chamo novamente o timer CollisionDomain para agora!!!
                                    collisionDomainPacket = new Basic802154AroundPacket("Again Collision Domain Formation", MAC_LAYER_PACKET);
                                    collisionDomainPacket->setMac802154AroundPacketType(MAC_802154_COLLISIONDOMAIN_PACKET);
                                    collisionDomainPacket->setSrcID(SELF_MAC_ADDRESS);
                                    collisionDomainPacket->setDstID(BROADCAST_MAC_ADDRESS);
                                    collisionDomainPacket->setPANid(rcvPacket->getSrcID());
                                    collisionDomainPacket->setIsCH(false);
                                    collisionDomainPacket->setAssociationAllow(false);
                                    collisionDomainPacket->setByteLength(CCH_PKT_SIZE);
                                    acceptNewPacket(collisionDomainPacket);
                                    
                                }
                        
                	} else {
                    //trace() << "SOU ORFAO. CH NAO ACEITANDO NOS: " << rcvPacket->getSrcID();
                	}
                	t = 0;
				
                }
                if(t)
                    trace() << "SOU ORFAO. NAO RECEBI DE NENHUM CH!!";
								
				break;
            }
            
            //Valores de RSSI e LQI
///            trace() << "RSSI = " << rssi << "; e LQI = " << lqi;  // around - debug
            
///            trace() << "RECEBI DE " << rcvPacket->getSrcID();  // Around - Debub
            
            map<int,bool>::const_iterator colMit;
            // Procedimento para retirar inconsistencias na lista de filhos dos Cluster-Heads!!
            if((isPANCoordinator) || (isCH)){
                //Modificação do Procedimento original - Nesse caso, se o PAN do nó que mandou o pacote for -1 e ele consta como meu filho
                //Então eu o aviso que ele será meu filho a partir de agora
                if(associatedDevices.find(rcvPacket->getSrcID()) != associatedDevices.end()){  //o .end() é retornado, caso o elemento não pertenca ao map
                    //trace() << "??? ACHEI " << rcvPacket->getSrcID();  //around - debug
                    if(rcvPacket->getPANid() != SELF_MAC_ADDRESS){
                        //trace() << "??? NAO ME PERTENCE " << rcvPacket->getSrcID();  // Around - Debub
                        
                        //Se o PAN do nó recebido é -1, eu vou avisá-lo que será meu filho agora
                        if(rcvPacket->getPANid() == -1){
///                            trace() << "Confirmando que sou o pai para o node: " << rcvPacket->getSrcID(); //Around - debug
                            formationAckPacket = new Basic802154AroundPacket("Again PAN Associate Response", MAC_LAYER_PACKET);
                            formationAckPacket->setMac802154AroundPacketType(MAC_802154_AGAINCCH_PACKET);
                            formationAckPacket->setPANid(SELF_MAC_ADDRESS);
                            formationAckPacket->setDstID(rcvPacket->getSrcID());
                            formationAckPacket->setNewCH(false);
                            formationAckPacket->setByteLength(CCH_PKT_SIZE);
                            toRadioLayer(formationAckPacket);
                            toRadioLayer(createRadioCommand(SET_STATE, TX));
                            
                        } else { //Ele já tem outro PAN e tenho que tirá-lo mesmo da minha lista de filhos
                            associatedDevices.erase(associatedDevices.find(rcvPacket->getSrcID()));
                            nchildren--;
                        }
                    }
                }
                
            }   else {
                //Outra possível inconsistência é o associatedPAN de um determinado nó nem ser Cluster-Head.
                //Isso pode acontecer caso algum candidato a cluster-head falhar por pouco nós. Aí, a falha acontece
                //quando ele envia pacote para desalocar os seus nós associados. Se algum deixar de receber, irá continuar
                //achando que ele será seu cluster-head, sendo que nem cluster-head ele é mais.
                // Assim, faço o else do if anterior (ou seja, não é CH nem PAN), depois verifico se o PAN do nó recebido é igual ao meu endereço.
                // Se for, errado!!!
//                if(rcvPacket->getPANid() == SELF_MAC_ADDRESS){
//                    trace() << "ERRADO!! Eu nao sou CH e o seguinte no me considera CH: " << rcvPacket->getSrcID();
//                    //AVisar para ele que eu não sou seu CH
//                    formationAckPacket = new Basic802154AroundPacket("PAN Associate Response", MAC_LAYER_PACKET);
//                    formationAckPacket->setMac802154AroundPacketType(MAC_802154_CCH_PACKET);
//                    formationAckPacket->setPANid(-1);
//                    formationAckPacket->setDstID(rcvPacket->getSrcID());
//                    formationAckPacket->setNewCH(false);
//                    formationAckPacket->setByteLength(CCH_PKT_SIZE);
//                    toRadioLayer(formationAckPacket);
//                    toRadioLayer(createRadioCommand(SET_STATE, TX));
//                    
//                }
            }
            
            
            // Como agora todo mundo envia um pacote de collisionDomain, mesmo o que estão com associatedPAN = -1
            //tenho que impedir que esses nós entrem nas estruturas do Around.
            if(rcvPacket->getPANid() == -1){
///                trace() << "PACOTE DE NAO ASSOCIADO! NAO PARTICIPA DAS ESTRUTURAS"; //Around - debug
                break;
            }
            
///            trace() << "PACOTE DE ASSOCIADO! VAI PARA AS ESTRUTURAS"; //Around - debug
            
            
            // Criando a matriz de interferência
            //mat[rcvPacket->getPANid()][SELF_MAC_ADDRESS] = 1;
            
            //Criando a matriz de interferência no formato map para todos os nós
            // Essa estrutura mapeia o alcance de transmissão dos nós
            //interfMatrix[rcvPacket->getSrcID()].push_back(SELF_MAC_ADDRESS);
            interfMatrix[SELF_MAC_ADDRESS].push_back(rcvPacket->getSrcID());
            RSSIinterfMatrix[SELF_MAC_ADDRESS].push_back(rssi);
            
            
            //CURIOSIDADE: posso usar o mapa[valor].push_back(VALOR) , com value sendo vector
            // Criando o grafo de overlapping, para cada CH, a lista de CHs em overlapping
            //vector<int>::const_iterator it;
            vector<int>::iterator it;
            
            bool tem = false;
            if((isPANCoordinator) || (isCH)){
                if(SELF_MAC_ADDRESS != rcvPacket->getPANid()){
                    Basic802154Around *AroundModule = check_and_cast<Basic802154Around*>(getParentModule()->getParentModule()->getParentModule()->getSubmodule("node",0)->getSubmodule("Communication")->getSubmodule("MAC"));
                    
                    // verifica se o pacote recebido foi enviado de CH ou não
                    if(rcvPacket->getIsCH()){
                        // Insere CH na lista dos overlapping
                        if(find(graph[SELF_MAC_ADDRESS].begin(), graph[SELF_MAC_ADDRESS].end(), rcvPacket->getSrcID()) == graph[SELF_MAC_ADDRESS].end()){
///                            trace() << "1- INSERIDO " << rcvPacket->getSrcID() << " em " << SELF_MAC_ADDRESS;
                            graph[SELF_MAC_ADDRESS].push_back(rcvPacket->getSrcID());
                            
                            //Alimentando o grafo do nó root
                            AroundModule->g->setRelation(SELF_MAC_ADDRESS,rcvPacket->getSrcID());
                        }
                        // e o vice-versa também
                        if(find(graph[rcvPacket->getSrcID()].begin(), graph[rcvPacket->getSrcID()].end(), SELF_MAC_ADDRESS) == graph[rcvPacket->getSrcID()].end()){
///                            trace() << "2- INSERIDO " << SELF_MAC_ADDRESS << " em " << rcvPacket->getSrcID();
                            graph[rcvPacket->getSrcID()].push_back(SELF_MAC_ADDRESS);
                            
                            //Alimentando o grafo do nó root
                            AroundModule->g->setRelation(rcvPacket->getSrcID(), SELF_MAC_ADDRESS);
                        }
                        
                        //Alimentando o grafo do nó root
                        //Basic802154Around *AroundModule = check_and_cast<Basic802154Around*>(getParentModule()->getParentModule()->getParentModule()->getSubmodule("node",0)->getSubmodule("Communication")->getSubmodule("MAC"));
                        //AroundModule->g->setRelation(SELF_MAC_ADDRESS,rcvPacket->getSrcID());
                        
                    } else { // nós que não são CH
///                        trace() << "No nao CH";
                        
                        // Insere o pai do nó não CH na lista dos overlapping
                        if(find(graph[SELF_MAC_ADDRESS].begin(), graph[SELF_MAC_ADDRESS].end(), rcvPacket->getPANid()) == graph[SELF_MAC_ADDRESS].end()){
///                            trace() << "3- INSERIDO " << rcvPacket->getPANid() << " em " << SELF_MAC_ADDRESS;
                            graph[SELF_MAC_ADDRESS].push_back(rcvPacket->getPANid());
                            
                            //Alimentando o grafo do nó root
                            AroundModule->g->setRelation(SELF_MAC_ADDRESS,rcvPacket->getPANid());
                        }
                        // e o vice-versa também
                        if(find(graph[rcvPacket->getPANid()].begin(), graph[rcvPacket->getPANid()].end(), SELF_MAC_ADDRESS) == graph[rcvPacket->getPANid()].end()){
///                            trace() << "4- INSERIDO " << SELF_MAC_ADDRESS << " em " << rcvPacket->getPANid();
                            graph[rcvPacket->getPANid()].push_back(SELF_MAC_ADDRESS);
                            
                            AroundModule->g->setRelation(rcvPacket->getPANid(),SELF_MAC_ADDRESS);
                        }
                        
                        
                    }
                }
            }
            
                    
            
            break;
        }
            
        
        //Recebeu um pacote de BEACON
        case MAC_802154_BEACONSCHEDULE_PACKET: {
            //Só processa o pacote se for CH
            if(!isCH)
                break;
            //Só processa pacote se for CH filho do emissor
            if (associatedPAN != rcvPacket->getPANid())
                break;
            
            //Valores de RSSI e LQI
///            trace() << "RSSI = " << rssi << "; e LQI = " << lqi; // around - debug
            
///            trace() << "RECEBI BEACON SCHEDULE " << SELF_MAC_ADDRESS;  //around - debug
            
            double offset = TX_TIME(rcvPacket->getByteLength());    // Função do OMNeT, que filtra o tamanho total do pacote (incluindo GTS)
            currentFrameStart = getClock() - offset;
            
            //setTimer(BEACON_SCHEDULE, (offsetCH[SELF_MAC_ADDRESS] - offsetCH[associatedPAN]) * symbolLen - offset);
            setTimer(BEACON_SCHEDULE, (offsetCH[SELF_MAC_ADDRESS] * symbolLen) - (simTime() - baseBI));
            
            break;
        }
            
        
        // Recebeu uma confirmação de um CCH que é sim Cluster-Head
        case MAC_802154_CH_RESPONSE_PACKET: {
            //Somente o pai do CCH que enviou é o interessado
            if (SELF_MAC_ADDRESS != rcvPacket->getDstID())
                break;
            
            //Valores de RSSI e LQI
///            trace() << "RSSI = " << rssi << "; e LQI = " << lqi; // around - debug
            
            associatedCH.push_back(rcvPacket->getSrcID());
            
            break;
        }
            
            
        //Recebeu um pacote de Endereçamento - Requisição de filhos
        case MAC_802154_ADDRESSING_REQUEST_PACKET: {
            //trace() << "RECEBI REQUEST " << SELF_MAC_ADDRESS;  // around - debug
            
            //Só processa o pacote se for CH
            if(!isCH)
                break;
            //Só processa pacote se for CH filho do emissor
            if (associatedPAN != rcvPacket->getPANid())
                break;
            
            //Valores de RSSI e LQI
///            trace() << "RSSI = " << rssi << "; e LQI = " << lqi; // around - debug
            
            double offset = TX_TIME(rcvPacket->getByteLength());    // Função do OMNeT, que filtra o tamanho total do pacote (incluindo GTS)
            currentFrameStart = getClock() - offset;
            
            // Note que para o endereçamento, eu uso o escalonamento para ativar o timer ADDRESSING_REQUEST dos nós CHs
            // Porém, eu utilizo o tempo entre eles dividido por 2, a fim de sobrar bastante tempo para processar o endereçamento no fim
            //setTimer(ADDRESSING_REQUEST, (offsetCH[SELF_MAC_ADDRESS] * symbolLen)/2 - (simTime() - baseBI) );
            
            //Divindo o período de BI por 3 para fazer o Request, para Responder e para atribuir
            setTimer(ADDRESSING_REQUEST, (offsetCH[SELF_MAC_ADDRESS] * symbolLen)/3 - (simTime() - baseBI) );
            
            
            //No caso de Dividir o BI por 3, é necessário definir o Response para 2/3 do período
            setTimer(ADDRESSING_RESPONSE,  ((2*(beaconInterval * symbolLen)/3) - (simTime() - baseBI)) - ( ( (offsetCH[SELF_MAC_ADDRESS] * symbolLen)/3) + ((superframeDuration * symbolLen)/3) ) );
            
            
            
            break;
        }
            
        //Recebeu um pacote de Endereçamento - Resposta
        case MAC_802154_ADDRESSING_RESPONSE_PACKET: {
            //trace() << "RECEBI RESPONSE " << SELF_MAC_ADDRESS;  // around - debug
            
            //Só processa o pacote se for CH
            if((!isCH)&&(!isPANCoordinator))
                break;
            //Só processa pacote se for CH filho do emissor
            if (SELF_MAC_ADDRESS != rcvPacket->getDstID())
                break;
            
            //Valores de RSSI e LQI
///            trace() << "RSSI = " << rssi << "; e LQI = " << lqi; // around - debug
            
///            trace() << "RECEIVE " << SELF_MAC_ADDRESS << " Recebi de " << rcvPacket->getSrcID() << " com filhos acumulados " << rcvPacket->getChildrenCont();
            totalchildren = totalchildren + rcvPacket->getChildrenCont();
            chAddrCount[rcvPacket->getSrcID()] = rcvPacket->getChildrenCont();
            
            break;
        }
            
        
        //Recebeu um pacote de Endereçamento - Assignment
        case MAC_802154_ADDRESSING_ASSIGNMENT_PACKET: {
            trace() << "RECEBI ASSIGNMENT " << SELF_MAC_ADDRESS << " do nodo " << rcvPacket->getSrcID();  // around - debug

            if(!isCH)
                break;
            //Só processa pacote se for CH filho do emissor
            if (SELF_MAC_ADDRESS != rcvPacket->getDstID())
                break;
            
            //Valores de RSSI e LQI
///            trace() << "RSSI = " << rssi << "; e LQI = " << lqi; // around - debug
            
            //Definindo o seu proprio endereco para hirarquico 0, apenas na estrutura global
            routingTable[SELF_MAC_ADDRESS] = rcvPacket->getStartAddressBlock(); //Se inclui na tabela global de endereços hierarquicos
            invroutingTable[rcvPacket->getStartAddressBlock()] = SELF_MAC_ADDRESS; //Para ser utilizado na analise protocolar
            //Definindo o seu bloco de endereços
            startAddrBlock = rcvPacket->getStartAddressBlock() + 1; //primeiro end é o do próprio CH
            endAddrBlock = rcvPacket->getEndAddressBlock();
            
            trace() << "EU " << SELF_MAC_ADDRESS << " - End. Hierar: " << rcvPacket->getStartAddressBlock() << " e Bloco de endereços: " << rcvPacket->getStartAddressBlock() + 1 << " - " << rcvPacket->getEndAddressBlock();
   
            //Seta o Timer para enviar o bloco de endereços para seus filhos (CHs e não CHs)
            setTimer(ADDRESSING_ASSIGNMENT, (offsetCH[SELF_MAC_ADDRESS] * symbolLen)/3 - (simTime() - (baseBI + (2*(beaconInterval * symbolLen)/3) )) );
            
            
            break;
        }
            
            
        //Ajuste da taxa de cada cluster
        case MAC_802154_RATE_ADJUSTMENT_PACKET: {
            //Coordenadores PAN ignoram beacons de outras PANs
            if (isPANCoordinator)
                break;
            // Nós associados a outras PANs ignoram também
            if (associatedPAN != -1 && associatedPAN != rcvPacket->getPANid())
                break;
            
            //Valores de RSSI e LQI
///            trace() << "RSSI = " << rssi << "; e LQI = " << lqi; // around - debug
            
///            trace() << "RECEBI PACOTE DE AJUSTE DE TAXA DE DADOS: " << SELF_MAC_ADDRESS;
            
            //Todos os nodes geram dados, inclusive cluster-heads
            AroundControlMessage *cmd = new AroundControlMessage("Rate Adjustment packet", MAC_CONTROL_MESSAGE);
            cmd->setAroundControlMessageKind(SET_RATE);
            cmd->setRate(rcvPacket->getRate());
            
            //Testando - Colocar o estado para setup, para evitar comunicação!!
            setMacState(MAC_STATE_SETUP);
            
            

            //Se for CH, irá ativar o timer RATE_ADJUSTMENT
            if(isCH){
                setTimer(RATE_ADJUSTMENT, (offsetCH[SELF_MAC_ADDRESS] * symbolLen) - (simTime() - baseBI) );
            }
            
            
            break;
        }
            
        //SCHEDULING FINAL
        case MAC_802154_BEGINBEACON_PACKET: {
            //Só processa o pacote se for CH
            if(!isCH)
                break;
            //Só processa pacote se for CH filho do emissor
            if (associatedPAN != rcvPacket->getPANid())
                break;
            
            //Valores de RSSI e LQI
            ///            trace() << "RSSI = " << rssi << "; e LQI = " << lqi; // around - debug
            
            trace() << "RECEBI BEACON SCHEDULE " << SELF_MAC_ADDRESS;  //around - debug
            
            //Marco na estrutura checkSched que recebi o quadro do escalonamento
            checkSched[SELF_MAC_ADDRESS] = true;
            
            double offset = TX_TIME(rcvPacket->getByteLength());    // Função do OMNeT, que filtra o tamanho total do pacote (incluindo GTS)
            currentFrameStart = getClock() - offset;
            
            //Para o escalonamento bottom-up, o timer abaixo gera um problema, pois os CHs mais profundos terao seus offsets perto de 0
            // Assim, o temporizador tende a ser negativo, o que gera um erro de execução.
            // Dessa forma, para escalonamento bottom-up, eu altero o temporizador para ser apenas o offset. Isso deve ser feito para que os CHs
            // de mesma profundidade nao entrem no BEGIN_BEACON e enviem os pacotes ao mesmo tempo.
            
            // Bottom-up Scheduling
            //Modificando o timer para bottom-up funcionar
            setTimer(BEGIN_BEACON, (offsetFormation[SELF_MAC_ADDRESS] * symbolLen));
            
            break;
        }
            
            
        case MAC_802154_BEACON_PACKET: {
            //Coordenadores PAN ignoram beacons de outras PANs
            if (isPANCoordinator)
                break;
            // Nós associados a outras PANs ignoram também
            if (associatedPAN != -1 && associatedPAN != rcvPacket->getPANid())
                break;
            //Nodes nao associados ignoram beacons!!!
            if (associatedPAN == -1)
                break;
            
            
            if(recvBeacons == 0){
                //Startando a geração de pacotes
             

				//trace() << "Ajustando set rate " << basePacketRate;
                
				AroundControlMessage *cmd = new AroundControlMessage("Rate Adjustment packet", MAC_CONTROL_MESSAGE);
                cmd->setAroundControlMessageKind(SET_RATE);
        			
				if(isPrio)
					cmd->setRate(packet_rate_prio);
				else
					cmd->setRate(packet_rate_noprio);
                //Enviando a taxa de geração de pacotes para a aplicação
                toNetworkLayer(cmd);
            }
            
            
            
            //Valores de RSSI e LQI
///            trace() << "RSSI = " << rssi << "; e LQI = " << lqi; // around - debug
            
            // Cancela mensagem de timeout de beacon (se presente)
            cancelTimer(BEACON_TIMEOUT);
            recvBeacons++;
///            trace() << "no " <<SELF_MAC_ADDRESS<< " recebeu beacon!";  // Around - Debug
            //Assim, este nó é conectado a essa PAN (ou irá tentar conectar), atualiza parâmetros do frame
            double offset = TX_TIME(rcvPacket->getByteLength());    // Função do OMNeT, que filtra o tamanho total do pacote (incluindo GTS)
            timeBeacon = getClock() - offset;
            //trace() << "OFFSET DO BEACON: " << offset;
            
            currentFrameStart = getClock() - offset;                //O inicio do superframe está no passado
            lostBeacons = 0;
         
//            superframeOrder = rcvPacket->getSuperframeOrder();                //pega SuperFrame Order (SO)
//            beaconOrder = rcvPacket->getBeaconOrder();              //pega Beacon Order     (BO)
            
            beaconInterval = aBaseSuperframeDuration * (1 << rcvPacket->getBeaconOrder());  //Cálculo do BI
            recMacBSN = rcvPacket->getBSN();           // número de sequencia do quadro beacon (não usado)
            CAPlength = rcvPacket->getCAPlength();  //Pega tamanho do CAP
            CAPend = CAPlength * aBaseSlotDuration * (1 << rcvPacket->getSuperframeOrder()) * symbolLen;  //Calcula o final do CAP
            GTSstart = 0;       //Campos do GTS: início do CFP
            GTSend = 0;         //Campos do GTS: fim do CFP
            GTSlength = 0;      //Campos do gTS: Tamanho do CFP
            
            //analisar se existe GTS para setar os campos de GTS: GTSstart, GTSend e GTSlength
            for (int i = 0; i < (int)rcvPacket->getGTSlistArraySize(); i++) {
                if (rcvPacket->getGTSlist(i).owner == SELF_MAC_ADDRESS) {
                    GTSstart = (rcvPacket->getGTSlist(i).start - 1) *
                    aBaseSlotDuration * (1 << rcvPacket->getSuperframeOrder()) * symbolLen;
                    GTSend = GTSstart + rcvPacket->getGTSlist(i).length *
                    aBaseSlotDuration * (1 << rcvPacket->getSuperframeOrder()) * symbolLen;
                    GTSlength = GTSend - GTSstart;
                    trace() << "GTS slot from " << getClock() + GTSstart << " to " << getClock() + GTSend << " length " << GTSlength;
                }
            }
            
      
            
            // Seta o estado do MAC para MAC_STATE_CAP
            setMacState(MAC_STATE_CAP);
            trace() << "Seta status de CAP com final em: " << CAPend;
            //Como um nó filho pode ser um cluster-head, o currentPacket dele pode ser um dado pendente que ele tentou transmitir no seu período ativo
            //e não conseguiu. Assim, tenho que apagar esse currentPacket, pois ele pode acabar sendo enviado agora no CAP (o que seria um grande prejuízo).
            //Vejo isso pelo endereço do destino, se for para baixo, era porque o currentPacket era um dado pendente.
            
            //Verificando então o endereço do destino do currentPacket
            if(isCH && currentPacket != NULL){
                int tempHierAddress = routingTable[currentPacket->getDstID()]; //pegando o endereço hierarquico do destino do currentpacket
/////                trace() << "Current packet nao eh nulo.";
/////                trace() << "Destino dele: " << currentPacket->getDstID() << " e end hierarquico: " << tempHierAddress;
/////                trace() << "MEU BLOCO DE ENDERECO: " << startAddrBlock << " e " << endAddrBlock;
                if (tempHierAddress >= startAddrBlock && tempHierAddress <= endAddrBlock) { //o currentpacket era para baixo
/////                    trace() << "Identifiquei como pacote para baixo! Vou apagar ele!";
                    currentPacket = NULL;
//                    if(currentPacket == NULL)
//                        trace() << "CURRENTPACKET FOI CANCELADO!!";
                }
                
            }
            
            
            
            
            //COMECEI A MUDANCA AQUI!!!!!!!
            pendingCount = 0; // Zerando o contador de pendências
            if(rcvPacket->getNumAddrPending() > 0){
                // Se tiver endereços pendentes, vai analisar se o seu está na lista
                for (int i = 0; i < rcvPacket->getNumAddrPending(); i++) {
                    //trace() << "Endereco Pendente: " << rcvPacket->getAddrList(i); // around - debug
                    if(rcvPacket->getAddrList(i) == SELF_MAC_ADDRESS){
///                        trace() << "Meu endereco existe como pendente de mensagens!"; // around - debug
                        pendingCount++;
                    }
                }
                
                //Se tiver pendencias, então solicito um-por-um os dados
                if(pendingCount > 0){
                    // Mecanismo para solicitar ao PAN dados pendentes
                    Basic802154AroundPacket *dataRequestPacket = new Basic802154AroundPacket("Data Request Packet", MAC_LAYER_PACKET);
                    dataRequestPacket->setDstID(associatedPAN);
                    dataRequestPacket->setPANid(associatedPAN);
                    dataRequestPacket->setMac802154AroundPacketType(MAC_802154_DATA_REQUEST_PACKET);
                    dataRequestPacket->setSrcID(SELF_MAC_ADDRESS);
                    dataRequestPacket->setByteLength(COMMAND_PKT_SIZE);
                    
                    //AQUI ENTRA UM QUESTÃO, QUEM É PRIORIDADE: Enviar dados ao coordenador (caso seja CH, poderá ter vários na fila) ou solicitar dados pendentes ???
                    
                    // Se o pacote pendente não for prioridade, a solução seria enviar a solicitação de dados pendentes para a fila e ele tenha que esperar a vez dele
                    //acceptNewPacket(dataRequestPacket); //como estava antes
                    
                    //Se o pacote pendnete for prioridade, então tenho que buscar uma alternativa de enviar a solicitação dos dados, mesmo tendo pacote no currentPacket ou buffer
                    if(currentPacket == NULL){ //Não tem pacote, então posso transmitir a vontade a requisição dos dados pendentes
                        transmitPacket(dataRequestPacket);
                    
                    } else { //CurrentPacket não é vazio. Provavelmente ele será um quadro de dados. Além disso, tem que verificar o buffer
                        //Verificando se tem dado no buffer
                        if(TXBuffer.size() == 0){ //buffer está vazio, por prudência poderia jogar a requisição do dados no buffer
                            acceptNewPacket(dataRequestPacket);
                            
                        } else { //Aqui é o problema. Se o buffer não ta vazio, os dados pendentes vão para onde: início do buffer ou fim do buffer??
                            //indo para fim do buffer
                            //acceptNewPacket(dataRequestPacket);
                            
                            //indo para início do buffer
                            queue< cPacket* > BufferAux;
                            
                            BufferAux.push(dataRequestPacket);
                            
                            while(!TXBuffer.empty()){
                                Basic802154AroundPacket *p = check_and_cast<Basic802154AroundPacket*>(TXBuffer.front()); //pego elemento da frente da fila TXBuffer
                                BufferAux.push(p);
                                TXBuffer.pop(); //vou apagando a fila original
                            }
                            TXBuffer.swap(BufferAux);
                        
                        }
                    
                    }
                }
            }
            
            
            //Caso o currentPacket é vazio, mas TXBuffer tem pacotes, é preciso jogar o pacote da ponta em currentPacket,
            // senão o nó não terá nada para transmitir no attemptTransmission
            if(currentPacket == NULL && TXBuffer.size() != 0){
/////                trace() << "PROBLEMA AQUI!!"; //around - debug
                Basic802154AroundPacket *packet = check_and_cast<Basic802154AroundPacket*>(TXBuffer.front());
                TXBuffer.pop();
                
                //Pego o pacote do buffer e já mando para transmissão
                transmitPacket(packet);
            }

            
            
            //se o nó em questão já está associado a PANid do pacote, ou seja, o nó já está Associado
            if (associatedPAN == rcvPacket->getPANid()) {
                // Se início de seu GTS é diferente do fim do CAP
                if (GTSstart != CAPend)
                    // Então, ele vai dormir após CAP, a menos que os slots GTS comecem logo após
                    setTimer(SLEEP_START, CAPend - offset);  //original
                    //testando dormir antes, considerando o atraso na hora de receber o beacon, ACABA FUNCIONANDO
                    //setTimer(SLEEP_START, CAPend - offset - 0.003);
                // Se inicio de GTS for maior que zero, ou seja, tem slots GTS
                if (GTSstart > 0) {
                    // seta o timer GTS_START phyDelaySleep2Tx segundos antes desde que o radio pode estar dormindo
                    setTimer(GTS_START, GTSstart - phyDelaySleep2Tx - offset);  //original
                    //testando dormir antes, considerando o atraso na hora de receber o beacon
                    //setTimer(GTS_START, GTSstart - phyDelaySleep2Tx - offset - 0.001);
                }
            } else { //nesse caso, o nó recebeu o pacote de beacon e não está associado, ou seja, precisa se associar!!
                // Seta o timer BACK_TO_SETUP
//                setTimer(BACK_TO_SETUP, CAPend - offset);
            }
            
            // Função para reagir ao beacon: se o nó não está conectado a uma PAN, ele transmite um pacote de requisição de associação
//            receiveBeacon_node(rcvPacket);
            //
            attemptTransmission("CAP started");
            // settimer que eu criei, mas ele deve acordar depois de um BI e não de um SD
            setTimer(FRAME_RECEIVE, aBaseSuperframeDuration * (1 << rcvPacket->getBeaconOrder()) * symbolLen - guardTime - offset);
            //setTimer(FRAME_START, aBaseSuperframeDuration * (1 << beaconOrder) * symbolLen - guardTime - offset);   // setTimer original
            
            break;
        }
            
        
        // Recebe uma requisição de associação
        case MAC_802154_DATA_REQUEST_PACKET:{
            
            // Apenas coordenadores podem aceitar pedido de requisição de dados
            if ((!isPANCoordinator) && (!isCH))
                break;
            
            // Se o PANid não bate - não faz nada!!
            if (rcvPacket->getPANid() != SELF_MAC_ADDRESS)
                break;
            
            //Valores de RSSI e LQI
///            trace() << "RSSI = " << rssi << "; e LQI = " << lqi; // around - debug
            
///            trace() << "RECEBI Pedido de requisicao de dados do no: " << rcvPacket->getSrcID();

            
            // Cria um pacote de ACK para reply ao nó pedinte da requisição de dados, para evitar que ele tente retransmitir a solicitação de dados
            Basic802154AroundPacket *dataReqAckPacket = new Basic802154AroundPacket("Data Request response", MAC_LAYER_PACKET);
            dataReqAckPacket->setPANid(SELF_MAC_ADDRESS);  //ok CH confirma recebimento do Data request
            dataReqAckPacket->setMac802154AroundPacketType(MAC_802154_ACK_PACKET);
            dataReqAckPacket->setDstID(rcvPacket->getSrcID());
            dataReqAckPacket->setByteLength(ACK_PKT_SIZE);
            
            toRadioLayer(dataReqAckPacket);
            toRadioLayer(createRadioCommand(SET_STATE, TX));
            setTimer(ATTEMPT_TX, TX_TIME(ACK_PKT_SIZE));
            
            
            //Procura a mensagem pendente do nó solicitante
            //vector<struct pendingAddrStruct>::const_iterator pendingfind;
            vector<struct pendingAddrStruct>::iterator pendingfind;
            
            for (pendingfind = pendingAddrTxBuffer.begin(); pendingfind != pendingAddrTxBuffer.end(); pendingfind++) {
                if(pendingfind->node == rcvPacket->getSrcID()){
                    //trace() << "INFO PENDING " << pendingfind->node;  // around - debug
                    //trace() << "INFO PENDING " << pendingfind->pendingAddrPkt->getSrcID();  // around - debug
/////                    trace() << "INFO PENDING " << pendingfind->pendingAddrPkt->getDestination();  // around - debug
                    
                    //Criando o pacote de dados para enviar ao nó solicitante
                    Basic802154AroundPacket *macPacket = new Basic802154AroundPacket("802.15.4 MAC data packet FWD", MAC_LAYER_PACKET);
                    macPacket = pendingfind->pendingAddrPkt->dup();
                    
                    
                    if(currentPacket == NULL){
                        // Enviando o pacote de dados acesso CSMA
                        hasPendingPkt = true; //essa variável faz com que ele possa acessar o CSMA-CA
                        transmitPacket(macPacket);
                        
                    } else {
                        //Para tornar essa comunicação possível, sem criar problema ao buffer do MAC de subida na árvore, eu crio um buffer de descida (DownTXBuffer)
                        
                        //Se currentPacket não é vazio, isso implica que esse CH tem algum pacote para enviar. Assim tenho que analisar as possibilidades
                        // Como esse nó está atuando como CH, ele só consegue se comunicar para baixo. Usando CSMA, o único pacote para baixo é um pacote de dados pendentes
                        // Então, o ponto crucial é verificar se o endereço do Destino é para baixo ou para cima.
                        //      - Se for para cima, tenho que armazenar esse no buffer TXbuffer e colocar o dado pendente no currentPacket
                        //      - Se for para baixo, tenho me manter o currentPacket e guardar o dado pendente no buffer para baixo (DownTXBuffer)
                        
                        //Verificando então o endereço do destino do currentPacket
                        int tempHierAddress = routingTable[currentPacket->getDstID()]; //pegando o endereço hierarquico do destino do currentpacket
                        if (tempHierAddress < startAddrBlock || tempHierAddress > endAddrBlock) { //o currentpacket é para cima
/////                            trace() << "Endereco destino " << tempHierAddress << " nao esta no meu cluster, nem abaixo!";  // around - debug
                            //tenho que guardar o currentPacket no TXBuffer
                            queue< cPacket* > BufferAux;
                            BufferAux.push(currentPacket);
                            
                            while(!TXBuffer.empty()){
                                Basic802154AroundPacket *p = check_and_cast<Basic802154AroundPacket*>(TXBuffer.front()); //pego elemento da frente da fila TXBuffer
                                BufferAux.push(p);
                                TXBuffer.pop(); //vou apagando a fila original
                            }
                            TXBuffer.swap(BufferAux);
                            
                            //Apagando o currentPacket
                            currentPacket = NULL;
                            
                            //Mandando para a transmissão um pacote de dados, que vai substituir o currentPacket
                            hasPendingPkt = true;
                            transmitPacket(macPacket);
                            
                        } else {  // Eh para baixo
                            //Nesse caso o currentPacket é para baixo, então deixa do jeito que está. Assim, jogo o macPacket para a fila do buffer de descida
                            DownTXBuffer.push(macPacket);
                        }
                        
                        
                    }
                    
                    //Testando o break para enviar só um pacote por vez
                    break;
                    
                }
                
            }
            break;
        }
            
            
        // Recebe uma requisição de associação
        case MAC_802154_ASSOCIATE_PACKET:{
            
            // Apenas coordenadores PAN podem aceitar requisições de associação
            // Se comunicação MULTIHOP for permitido, torna-se necessário modificar isso
            // Em particular, qualquer nó FFD pode torna-se coordenador e aceitar requisições de associcação
            
            map <int, double> my_rel;
            my_rel = nhTable[SELF_MAC_ADDRESS];

			trace() << "ASSOCIATE_PACKET recebido de " << rcvPacket->getSrcID();
			
            if(nhTable.find(rcvPacket->getSrcID()) == nhTable.end())
                readTable(rcvPacket->getTableNH(), rcvPacket->getSrcID());
	
            if ((!isPANCoordinator) && (!isCH))
                break;
            
            // Se o PANid não bate - não faz nada!!
            if (rcvPacket->getPANid() != SELF_MAC_ADDRESS)
                break;


            if(my_rel.find(rcvPacket->getSrcID()) != my_rel.end()){
             
				//trace() << "Root " << SELF_MAC_ADDRESS << " recebeu associate packet!";  // Around - Debug
                //escreve no trace que aceita requisicao de associação do nó pedinte!!
                //trace() << "Accepting association request from " << rcvPacket->getSrcID();
                
                //conta a quantidade de filhos, evitando duplos pedidos de associacao
                if(associatedDevices[rcvPacket->getSrcID()] != true){
///                    trace() << "EU " << SELF_MAC_ADDRESS << " RECEBI a primeira vez e contei como filho o no " << rcvPacket->getSrcID();  //around - debug
                    nchildren++;  //conta a quantidade de filhos
                    
                }
                
                // Atualiza associatedDevices (map <int,bool> com os elementos true associados ao PAN)
                associatedDevices[rcvPacket->getSrcID()] = true;
                associatedDeviceRSSI[rcvPacket->getSrcID()] = rssi;
                
				trace() << "NODO ACEITO ...... " << rcvPacket->getSrcID();
                trace() << "NODE_prob "<< rcvPacket->getSrcID() << " PAN_prob " << SELF_MAC_ADDRESS << " RSSI_prob " << rssi << " DEPTH_prob " << (depth+1);
                
                //Around - Se envia está em FORMACAO com envio imediato de ACK ou aceitando nós durante CAP, enviam o ACK de imediato
                //Se estiver em formação sem envio imediato de ACK, não faz o if, ou seja, não envia pacote de ACK
                if ((macState != MAC_STATE_FORMATION) || (nowFormationACK)) {
                    // Cria um pacote de ACK para reply ao nó pedinte da associação
                    Basic802154AroundPacket *ackPacket = new Basic802154AroundPacket("PAN associate response", MAC_LAYER_PACKET);
                    // Seta o id do PAN como o dele do nó
                    ackPacket->setPANid(SELF_MAC_ADDRESS);
                    // Seta o tipo do pacote para MAC_802154_ACK_PACKET, que naturalmente, após ser recebido do radio (fromRadioLayer),
                    // será tratado no case MAC_802154_ACK_PACKET
                    ackPacket->setMac802154AroundPacketType(MAC_802154_ACK_PACKET);
                    // Seta o ID de destino, que será o nó que solicitou a associação
                    ackPacket->setDstID(rcvPacket->getSrcID());
                    // Seta o tamanho do pacote de ACK, que será ACK_PKT_SIZE, definido no #define do .h
                    ackPacket->setByteLength(ACK_PKT_SIZE);
                        
                    // Manda o pacote para o rádio
                    toRadioLayer(ackPacket);
                    // Comando do rádio para ir ao estado TX
                    toRadioLayer(createRadioCommand(SET_STATE, TX));
                    // Seta o time ATTEMPT_TX com o tempo relativo ao tamanho do pacote
                    setTimer(ATTEMPT_TX, TX_TIME(ACK_PKT_SIZE));
                }
                
                
                
            }
            // Não existe uma implementação de um(ns) motivo(s) de não aceitação de associação de um nó
/////            else {
/////                trace() << "Denied association request from " << rcvPacket->getSrcID();
                // Além de não existir uma implmementação para envio de um pacote para negar a requisição
                // além de enviar o nó novamente para um estado de SETUP. Isso, acredito, que possa ser facilmente feito
/////            }
            
            
            break;
        }
            
            // Recebe uma requisição de GTS
        case MAC_802154_GTS_REQUEST_PACKET:{
            
            // Apenas coordenadores PAN podem aceitar requisições de GTS
            if (!isPANCoordinator)
                break;
            
            // Testa se PANid corresponde ao do coordenador em questão, se não for, não faz nada
            if (rcvPacket->getPANid() != SELF_MAC_ADDRESS)
                break;
            
            //Valores de RSSI e LQI
///            trace() << "RSSI = " << rssi << "; e LQI = " << lqi; // around - debug
            
            //informa no trace que recebeu pedido de GTS do nó específico
///            trace() << "Received GTS request from " << rcvPacket->getSrcID();
            
            // Reply com um ACK
            //Cria um pacote de ACK
            Basic802154AroundPacket *ackPacket = new Basic802154AroundPacket("PAN GTS response", MAC_LAYER_PACKET);
            // Seta PANid como o dele mesmo
            ackPacket->setPANid(SELF_MAC_ADDRESS);
            // SEta type do pacote como ACK PACKET
            ackPacket->setMac802154AroundPacketType(MAC_802154_ACK_PACKET);
            // Seta Id destino como o de origem da requisição do GTS
            ackPacket->setDstID(rcvPacket->getSrcID());
            // Seta o tamanho do pacote como ACK_PKT_SIZE, definido com #define no arquivo .h
            ackPacket->setByteLength(ACK_PKT_SIZE);
            // Função gtsRequest_hub toma a decisão da quantidade de GTS par alocar
            ackPacket->setGTSlength(gtsRequest_hub(rcvPacket));
            
            // Envia o pacote ao rádio
            toRadioLayer(ackPacket);
            // Envia comando do rádio para entrar em modo TX
            toRadioLayer(createRadioCommand(SET_STATE, TX));
            // Seta o time ATTEMPT_TX para o tempo do tamanho do pacote ACK
            setTimer(ATTEMPT_TX, TX_TIME(ACK_PKT_SIZE));
            
            break;
        }
            
            // Recebe um quadro ACK, que será manuseado pela função handleAclPacket
        case MAC_802154_ACK_PACKET:{
            // Testa se ele é o endereço destino do pacote ACK. Se não for, descarta o pacote
            if (rcvPacket->getDstID() != SELF_MAC_ADDRESS)
                break;

			//trace() << "ENTROUUU NO HANDLE!!! " << SELF_MAC_ADDRESS;
            // A função handleAckPacket irá manusear o pacote de ACK
            handleAckPacket(rcvPacket);
            break;
        }

		case MAC_802154_KNOWLEDGE_PACKET:{
	
            //trace() << "Nodo " << rcvPacket->getSrcID() << " descoberto com RSSI = " << rssi;
            if(macState == MAC_STATE_DISCOVERY){

				acarch->setNeighborhoodTable(SELF_MAC_ADDRESS, rcvPacket->getSrcID(), rssi);
					

                if(nhTable[SELF_MAC_ADDRESS].find(rcvPacket->getSrcID()) == nhTable[SELF_MAC_ADDRESS].end()){
                    nhTable[SELF_MAC_ADDRESS][rcvPacket->getSrcID()] = rssi;
                    nhCont[rcvPacket->getSrcID()] = 1;
		
                }else{
                    nhTable[SELF_MAC_ADDRESS][rcvPacket->getSrcID()] = nhTable[SELF_MAC_ADDRESS][rcvPacket->getSrcID()] + rssi;
                    nhCont[rcvPacket->getSrcID()]++;
                }

            }

			break;
		}
            
        // Recebe um quadro de dados
        // Importante notar que, para a implementação em questão, dados sempre são enviados para o sink, que é o coordenador PAN
        case MAC_802154_DATA_PACKET:{
            
            //Valores de RSSI e LQI
            //trace() << "RSSI = " << rssi << "; e LQI = " << lqi; // around - debug
            trace() << "BACK DATA PKT vindo de " << rcvPacket->getSrcID() << " com ORIGEM: " << rcvPacket->getSource() << " com SeqNum: " << rcvPacket->getSequenceNumber() << ", local clock " << getClock();
            
            //CHEGOU AO PAN (SINK)
            if( (rcvPacket->getDstID() == SELF_MAC_ADDRESS) && (rcvPacket->getSink() == SELF_MAC_ADDRESS) ){
                trace() << "RECEBI PACOTE # " << rcvPacket->getSeqNum() << " DE BACKGROUND DO NO " << rcvPacket->getSrcID() << " START EM " << rcvPacket->getStartTime();  // around - debug
                // Testa se o nó não é duplicado
                if (isNotDuplicatePacket(rcvPacket)) {
                    // A função dataReceived_hub ... (esta função, até então, não ta fazendo nada). É uma função virtual no .h
                    dataReceived_hub(rcvPacket);
                    // Manda o pacote para a camada de routing, desencapsulando-a
                    toNetworkLayer(decapsulatePacket(rcvPacket));
                    //toNetworkLayer(decapsulatePacket(pkt));
                    
                    //Se o pacote veio do pai, é comunicação indireta, assim decremento pendingCount
                    if (rcvPacket->getSrcID() == associatedPAN)
                        pendingCount--;
                    
                } else {
                    // Senão, informa no trace que o pacote é duplicado
                    trace() << "Packet [" << rcvPacket->getName() << "] from node "
                    << rcvPacket->getSrcID() << " is a duplicate. ORIGEM: " << rcvPacket->getSource();
                }
                
                
                
            
                // Se o quadro foi enviado para endereço de broadcast, nada mais precisa ser feito
                if (rcvPacket->getDstID() == BROADCAST_MAC_ADDRESS)
                    break;
            
                // Caso contrário, gera e envia um ACK
                // Cria um pacote de ACK
                Basic802154AroundPacket *ackPacket = new Basic802154AroundPacket("Ack packet", MAC_LAYER_PACKET);
                // Seta PANid para o próprio endereço
                ackPacket->setSrcID(SELF_MAC_ADDRESS);
                // Seta o tipo de pacote como ACK
                ackPacket->setMac802154AroundPacketType(MAC_802154_ACK_PACKET);
                // Seta o destino como sendo a origem do pacote
                ackPacket->setDstID(rcvPacket->getSrcID());
                // Seta o número de sequencia do pacote
                ackPacket->setSeqNum(rcvPacket->getSeqNum());
                // Seta o tamanho do pacote ACK, onde ACK_PKT_SIZE é definido usando #define no arquivo .h
                ackPacket->setByteLength(ACK_PKT_SIZE);
            
                // Envia o pacote para o rádio
                toRadioLayer(ackPacket);
                // Comando para rádio para entrar em modo de transmissão
                toRadioLayer(createRadioCommand(SET_STATE, TX));
                // Seta o temporizador para ATTEMPT_TX com tempo referente ao tamanho do ACK
                setTimer(ATTEMPT_TX, TX_TIME(ACK_PKT_SIZE));
            
            } else {  //Nao chegou ainda ao seu destino final (sink)
                trace() << "PACOTE NAO CHEGOU AO SEU SINK AINDA!";  // around - debug
                
                if (isNotDuplicatePacket(rcvPacket)) {
                    
                    //Se o pacote veio do pai, é comunicação indireta, assim decremento pendingCount
                    if (rcvPacket->getSrcID() == associatedPAN)
                        pendingCount--;
                    
                    //Re-configurando os campos do pacote para reencaminhamento
                    Basic802154AroundPacket *macPacket = new Basic802154AroundPacket("802.15.4 MAC data packet FWD", MAC_LAYER_PACKET);
                    //macPacket->setSrcID(SELF_MAC_ADDRESS);
                    macPacket = rcvPacket->dup();  // recebo uma cópia do pacote original para ser reencaminhado
                    macPacket->setSrcID(SELF_MAC_ADDRESS);
                    macPacket->setMac802154AroundPacketType(MAC_802154_DATA_PACKET);
                    macPacket->setSink(rcvPacket->getSink());
                
                    int tempHierAddress = routingTable[rcvPacket->getSink()];
                    trace() << "Meu bloco de enderecos " << startAddrBlock << " " << endAddrBlock << ", com " << tempHierAddress;  //around - debug
                    if (tempHierAddress < startAddrBlock || tempHierAddress > endAddrBlock) {
                        trace() << "Endereco destino " << tempHierAddress << " nao esta no meu cluster, nem abaixo!";  // around - debug
                        macPacket->setDstID(associatedPAN);
                    
                        macPacket->setSeqNum(seqNum++);//rcvPacket->getSeqNum());
                        if (!acceptNewPacket(macPacket))
                            packetoverflow++;
                
                    } else {
                        trace() << "Endereco destino " << tempHierAddress << " esta no meu cluster ou abaixo!";  // around - debug
                        map <int, int>::const_iterator it;
                        int flag = 0;
                        for (it = childAddress.begin(); it != childAddress.end(); it++) {
                            if(it->first == rcvPacket->getSink()){
                                flag = 1;
                            }
                        }
                        if (flag) {
/////                            trace() << "O destino eh um no filho";
                            macPacket->setDstID(rcvPacket->getSink());
                        } else {
/////                            trace() << "O destino esta em um CH filho";
                            map <int, vector<int> >::const_iterator it2;
                            for(it2 = chAddrBlock.begin(); it2 != chAddrBlock.end(); it2++){
/////                                trace() << "BLOCO DO NODE " << it2->first << " eh " << it2->second[0] << " - " << it2->second[1];
                                if( (tempHierAddress >= it2->second[0]) && (tempHierAddress <= it2->second[1]) ){
/////                                    trace() << "O no destino esta no cluster do node " << it2->first;
                                    macPacket->setDstID(it2->first);  // Define que o próximo destino já o filho em questão
                                }
                            }
                        }
                    
                        // configurando as variáveis para uma comunicação indireta
                        if(numPendingAddr < maxNumPendingAddr-1){
                            numPendingAddr++; //Incrementando a quantidade de endereços pendentes
                            pendingAddrList.push_back(macPacket->getDstID()); // Acrescentando endereço na lista de endereços pendentes
                            //pendingAddrTxBuffer[macPacket->getDstID()] = macPacket;  //Acrescentando pacote no buffer de pacotes pendentes do CH
                            struct pendingAddrStruct tmpStruct;
                            tmpStruct.node = macPacket->getDstID(); //guardando o destino
                            tmpStruct.pendingAddrPkt = macPacket;
                            pendingAddrTxBuffer.push_back(tmpStruct);
                    
                            //trace() << "TESTE ESTRUTURA: NODE " << pendingAddrTxBuffer[0].node;  // around - debug
                            //trace() << "PACOTE SRC " << pendingAddrTxBuffer[0].pendingAddrPkt->getSrcID();  // around - debug
                            //trace() << "PACOTE SNK " << pendingAddrTxBuffer[0].pendingAddrPkt->getSink();  // around - debug
                        }
/////                        else {
/////                            trace() << "Lista de enderecos pendentes chegou a seu limite máximo!";
/////                        }
                    
                    }
                
                }
/////                else { //trata-se de um pacote duplicado!!
/////                    trace() << "DUPLICADO! Packet [" << rcvPacket->getName() << "] from node " << rcvPacket->getSrcID() << " is a duplicate. ORIGEM: " << rcvPacket->getSource();
                   
/////                }
                
                // Se o quadro foi enviado para endereço de broadcast, nada mais precisa ser feito
                if (rcvPacket->getDstID() == BROADCAST_MAC_ADDRESS)
                    break;
                
                // Recebeu o pacote e confirma o nó que enviou
                // Cria um pacote de ACK
                Basic802154AroundPacket *ackPacket = new Basic802154AroundPacket("Ack packet", MAC_LAYER_PACKET);
                // Seta PANid para o próprio endereço
                ackPacket->setSrcID(SELF_MAC_ADDRESS);
                // Seta o tipo de pacote como ACK
                ackPacket->setMac802154AroundPacketType(MAC_802154_ACK_PACKET);
                // Seta o destino como sendo a origem do pacote
                ackPacket->setDstID(rcvPacket->getSrcID());
                // Seta o número de sequencia do pacote
                ackPacket->setSeqNum(rcvPacket->getSeqNum());
                // Seta o tamanho do pacote ACK, onde ACK_PKT_SIZE é definido usando #define no arquivo .h
                ackPacket->setByteLength(ACK_PKT_SIZE);
                
                // Envia o pacote para o rádio
                toRadioLayer(ackPacket);
                // Comando para rádio para entrar em modo de transmissão
                toRadioLayer(createRadioCommand(SET_STATE, TX));
                // Seta o temporizador para ATTEMPT_TX com tempo referente ao tamanho do ACK
                setTimer(ATTEMPT_TX, TX_TIME(ACK_PKT_SIZE));
                
                
            }
            
            //Se o pacote veio de seu coordenador, eh pq foi comunicaçao indireta. Assim, avalio se ainda tenho dados pendentes para solicitá-los
            if (rcvPacket->getSrcID() == associatedPAN) {
/////                trace() << "FOI COMUNICACAO INDIRETA";
                
                //E SE TIVER MAIS PACOTES PENDENTES
                if(pendingCount > 0){
/////                    trace() << "Ainda tenho mais dados pendentes!! Vou solicitar!!";
                    // Mecanismo para solicitar ao PAN dados pendentes
                    Basic802154AroundPacket *dataRequestPacket = new Basic802154AroundPacket("Data Request Packet", MAC_LAYER_PACKET);
                    dataRequestPacket->setDstID(associatedPAN);
                    dataRequestPacket->setPANid(associatedPAN);
                    dataRequestPacket->setMac802154AroundPacketType(MAC_802154_DATA_REQUEST_PACKET);
                    dataRequestPacket->setSrcID(SELF_MAC_ADDRESS);
                    dataRequestPacket->setByteLength(COMMAND_PKT_SIZE);
                    
                    if(currentPacket == NULL && TXBuffer.size() == 0){ //Não tem pacote, então posso transmitir a vontade a requisição dos dados pendentes
                        transmitPacket(dataRequestPacket);
                        
                    } else { //CurrentPacket não é vazio. Provavelmente ele será um quadro de dados. Além disso, tem que verificar o buffer
                        //Verificando se tem dado no buffer
                        if(TXBuffer.size() == 0){ //buffer está vazio, por prudência poderia jogar a requisição do dados no buffer
                            acceptNewPacket(currentPacket);
                            currentPacket = NULL;
                            transmitPacket(dataRequestPacket);
                        
                        } else {
                            queue< cPacket* > BufferAux;
                            BufferAux.push(currentPacket);
                        
                            while(!TXBuffer.empty()){
                                Basic802154AroundPacket *p = check_and_cast<Basic802154AroundPacket*>(TXBuffer.front()); //pego elemento da frente da fila TXBuffer
                                BufferAux.push(p);
                                TXBuffer.pop(); //vou apagando a fila original
                            }
                            TXBuffer.swap(BufferAux);
                            currentPacket = NULL;
                            transmitPacket(dataRequestPacket);
                        
                        }
                        
                    }
                }
            }
            
            
            
            break;
        }      

  
        // Caso nenhum, avisa que o quadro não é reconhecido!!
        default:{
            trace() << "WARNING: unknown packet type received [" << rcvPacket->getName() << "]";
        }
    }
}



//----------------------------------------------------------------------------------------------------------------------------
//
//  FROM NETWORK LAYER
//
//----------------------------------------------------------------------------------------------------------------------------



// Pacote recebido da camada superior (Rede)
void Basic802154Around::fromNetworkLayer(cPacket * pkt, int dstMacAddress)
{
	//Criando um novo pacote "macPacket" (pacote de dados), que irá encapsular o pacote da camada de rede
    Basic802154AroundPacket *macPacket = new Basic802154AroundPacket("802.15.4 MAC data packet", MAC_LAYER_PACKET);
	//encapsula um pacote de routing pkt em um pacote MAC macPacket
    //deve ser feito antes de setar qualquer campo do pacote MAC, pois ele "envia" o campo de destino para BROADCAST_MAC_ADDRESS
    encapsulatePacket(macPacket, pkt);
    
    macPacket->setSrcID(SELF_MAC_ADDRESS);	//if connected to PAN, would have a short MAC address assigned,
											//but we are not using short addresses in this model
    
    
    //Seta os campos do pacote MAC
 //   macPacket->setDstID(dstMacAddress); //com base na App padrão, dstMacAddress está sempre para 0
    //modificando para setar o destino para o associatedPAN
    //if(dstMacAddress)
    if(dstMacAddress == -1){
        trace() << "Pacote de background gerado por " << macPacket->getSrcID();  // around - debug
        
        macPacket->setDstID(associatedPAN); //o destino sempre será o seu pai (caminho imediato da árvore)
        //Agora, quem será o sink: o seu pai ou o root da rede, ou outro nó qualquer??
        // Aqui, define-se isso! Apenas uma opção deve ser definida
        //macPacket->setSink(associatedPAN);  //Considerando que o sink é o seu próprio pai
        macPacket->setSink(0);              //Considerando que o sink é o root da rede

    }
//    else {
///////        trace() << "Outro tipo de Pacote de App!";  // around - debug
//        if(dstMacAddress != SELF_MAC_ADDRESS){  //se o pacote for para o seu próprio END., algo está errado!!
//
//            int tempHierAddress = routingTable[dstMacAddress];
//
//            if(isCH or isPANCoordinator){  //source é CH e verifica se o destino está abaixo dele ou acima
///////                trace() << "MEU BLOCO DE ENDERECO: " << startAddrBlock << " - " << endAddrBlock;
//
//                // O destino não está nem no cluster e nem abaixo dele
//                if (tempHierAddress < startAddrBlock || tempHierAddress > endAddrBlock) {
///////                    trace() << "Endereco destino " << tempHierAddress << " nao esta no meu cluster, nem abaixo!";  // around - debug
//                    macPacket->setDstID(associatedPAN);
//                    macPacket->setSink(dstMacAddress);
//                } else { // O destino está no cluster ou abaixo dele
//                    macPacket->setSink(dstMacAddress);  // Define o destino final
///////                    trace() << "Endereco destino " << tempHierAddress << " esta no meu cluster ou abaixo!";  // around - debug
//                    map <int, int>::const_iterator it;
//                    int flag = 0;
//                    for (it = childAddress.begin(); it != childAddress.end(); it++) {
//                        if(it->first == dstMacAddress){  // olha se o endereco destino está na lista de filhos
//                            flag = 1;
//                        }
//                    }
//                    if (flag) {
///////                        trace() << "O destino eh um no filho"; // around - debug
//                        macPacket->setDstID(dstMacAddress);  // Define que o próximo destino já o filho em questão
//
//                    } else {
///////                        trace() << "O destino esta em um CH filho"; // // around - debug
//                        map <int, vector<int> >::const_iterator it2;
//                        for(it2 = chAddrBlock.begin(); it2 != chAddrBlock.end(); it2++){
///////                            trace() << "BLOCO DO NODE " << it2->first << " eh " << it2->second[0] << " - " << it2->second[1];
//                            if( (tempHierAddress >= it2->second[0]) && (tempHierAddress <= it2->second[1]) ){
///////                                trace() << "O no destino esta no cluster do node " << it2->first;
//                                macPacket->setDstID(it2->first);  // Define que o próximo destino já o filho em questão
//                            }
//                        }
//                    }
//                    macPacket->setMac802154AroundPacketType(MAC_802154_DATA_PACKET);  // Define o tipo do pacote
//                    macPacket->setSeqNum(seqNum++);
//
//                    // configurando as variáveis para uma comunicação indireta - respeitando o limite máximo do buffer
//                    if(numPendingAddr < maxNumPendingAddr-1){
//                        numPendingAddr++; //Incrementando a quantidade de endereços pendentes
//                        pendingAddrList.push_back(macPacket->getDstID()); // Acrescentando endereço na lista de endereços pendentes
//                        //pendingAddrTxBuffer[macPacket->getDstID()] = macPacket;  //Acrescentando pacote no buffer de pacotes pendentes do CH
//                        struct pendingAddrStruct tmpStruct;
//                        tmpStruct.node = macPacket->getDstID(); //guardando o destino
//                        tmpStruct.pendingAddrPkt = macPacket;
//                        pendingAddrTxBuffer.push_back(tmpStruct);
//
//                        //trace() << "TESTE ESTRUTURA: NODE " << pendingAddrTxBuffer[0].node;  // around - debug
//                        //trace() << "PACOTE SRC " << pendingAddrTxBuffer[0].pendingAddrPkt->getSrcID();  // around - debug
//                        //trace() << "PACOTE SNK " << pendingAddrTxBuffer[0].pendingAddrPkt->getSink();  // around - debug
//                    }
///////                    else {
///////                        trace() << "Lista de enderecos pendentes chegou a seu limite máximo!";
///////                    }
//
//                    return;
//                }
//
//            } else { //source não é CH e apenas deve mandar pacote para seu CH
//                macPacket->setDstID(associatedPAN);
//                macPacket->setSink(dstMacAddress);
//            }
//
//        } else {
//            trace() << "Warning! Source and Sink Nodes are the same!";  // around - debug
//            return;
//        }
//    }
	macPacket->setMac802154AroundPacketType(MAC_802154_DATA_PACKET);
	macPacket->setSeqNum(seqNum++);
	if (seqNum > 255)
        seqNum = 0;
    
    if(withMACBuffer){
        trace() << "USANDO BUFFER DO MAC!";
        if (!acceptNewPacket(macPacket))
            packetoverflow++;
    } else {
        
        //Terceira implementação: Agora todos os nós, incluindo CH, geram dados de background
        //Assim, eles terão que verificar seus buffers para ver se tem pacote de dados SEUS, pois ele também podem
        // agregar pacotes de dados dos seus nós filhos para enviar
/////        trace() << "SEM USAR BUFFER DO MAC!";
        if(currentPacket != NULL){
/////            trace() << "CurrentPacket is NOT NULL!";
            
            //Verifico se é um CH ou se é um nó comum
            if(isCH){ //É um CH, tem que avaliar
                //verifico se o current packet é de dados e se é MEU, se for, substituo
                if(currentPacket->getMac802154AroundPacketType() == MAC_802154_DATA_PACKET && currentPacket->getSource() == SELF_MAC_ADDRESS){
/////                    trace() << "SUBSTITUIDO! CurrentPacket eh pacote de dados de CH!";
                    currentPacket = macPacket; //acrescentei isso para evitar problemas
                    replacedPackets++; //contandor para pacotes substituídos vindo da App
               
                } else { //senao, verifico se tem pacote de dados MEU no buffer, se tiver substituo, senao, insiro no buffer do MAC
                    if ((int)TXBuffer.size() != 0){ //tem pacote no buffer??
                        //FIla auxiliar. Eu jogo TXBuffer dentro de BufferAux e depois retorno para TXBuffer
                        queue< cPacket* > BufferAux;
                        
                        //Procurar no TXBuffer por pacote de dados MEU
                        //Como filas não permite acesso nem substituições, para fazer isso eu vou acessando a frente da fila de TXBuffer
                        //e vou jogando os elementos na fila auxiliar BufferAux.
                        //Naturalmente, se achar um pacote de dados do CH, ao inves de jogar o que estava no TXBuffer, eu jogo o novo pacote de dados
                        //exatamente na posicao.
                        bool flag = false; //flag é definido para true caso encontre dados do CH na fila.
                        while(!TXBuffer.empty()){
                            Basic802154AroundPacket *p = check_and_cast<Basic802154AroundPacket*>(TXBuffer.front()); //pego elemento da frente da fila TXBuffer
                            //trace() << "TIPO DO PACOTE NO BUFFER: " << p->getMac802154AroundPacketType() << " COM ORIGEM: " << p->getSource(); //around - debug
                            
                            //Testo se o pacote atual da fila é de dados e se é do CH. Se for substituo
                            if(p->getMac802154AroundPacketType() == MAC_802154_DATA_PACKET && p->getSource() == SELF_MAC_ADDRESS){
/////                                trace() << "SUBSTITUIDO! Pacote de dados de CH em fila!"; //around - debug
                                replacedPackets++; //contandor para pacotes substituídos vindo da App
                                BufferAux.push(macPacket);
                                flag = true; //se for para true, eh pq encontrei pacote e substitui por novo
                            } else { //se nao eh dados do CH, incluo o pacote no buffer auxiliar e continuo
                                BufferAux.push(p);
                            }
                            
                            TXBuffer.pop(); //vou apagando a fila original
                        }
                        
                        //trace() << "Tamanho de TXBuffer: " << (int)TXBuffer.size() << " e de TXBuffer2: " << (int)BufferAux.size(); //around - debug
                        TXBuffer.swap(BufferAux);
                        //trace() << "Apos SWAP!!";
                        //trace() << "Tamanho de TXBuffer: " << (int)TXBuffer.size() << " e de TXBuffer2: " << (int)BufferAux.size(); //around - debug
                        
                        //Se flag é false, eh pq nao encontrei dados do CH no buffer, assim incluo o pacote no final da fila
                        if(!flag){
/////                            trace() << "Nao tem pacote do CH no buffer! Pacote de dados do CH vai pro fim da fila!"; //around - debug
                            if (!acceptNewPacket(macPacket))
                                packetoverflow++;
                        }
      
                    } else { //nao tem pacote no buffer
/////                        trace() << "Nao tem pacote no buffer! Pacote de dados vai pro buffer!";
                        acceptNewPacket(macPacket);
                    }
                    
                    
                }
                
            } else { //É um nó comum
                //verifico se o current packet é de dados, se for, substituo
                if(currentPacket->getMac802154AroundPacketType() == MAC_802154_DATA_PACKET){
/////                    trace() << "SUBSTITUIDO! CurrentPacket eh pacote de dados - NO COMUM!";
/////                    trace() << "Origem do currentPacket: " << currentPacket->getSource();
                    replacedPackets++; //contandor para pacotes substituídos vindo da App
                    //transmitPacket(macPacket); //estava gerando problemas, resultando na perda do primeiro pacote do buffer
                    currentPacket = macPacket; //acrescentei isso para evitar problemas
                    
                } else { //senao, verifico se tem pacote de dados no buffer, se tiver substituo, senao, insiro no buffer do MAC
                    if ((int)TXBuffer.size() != 0){ //tem pacote no buffer
                        //Procuro se os pacotes do buffer é algum pacote de dados, se for substituo!!
                        Basic802154AroundPacket *p = check_and_cast<Basic802154AroundPacket*>(TXBuffer.front());
                        if(p->getMac802154AroundPacketType() == MAC_802154_DATA_PACKET){
/////                            trace() << "SUBSTITUIDO! Pacote na frente do buffer eh de dados - NO COMUM! ";
                            replacedPackets++; //contandor para pacotes substituídos vindo da App
                            TXBuffer.pop();
                            TXBuffer.push(macPacket);
                        }
                    } else { //nao tem pacote no buffer
/////                        trace() << "Nao tem pacote no buffer! Pacote de dados vai pro buffer!";
                        acceptNewPacket(macPacket);
                    }
                }
            }
            
        } else { //como currentPacket é NULL, envio o pacote de dados direto para transmissão
/////            trace() << "CurrentPacket eh NULL! Assim, mando para a transmissao o pacote de dados!";
            transmitPacket(macPacket);
        }
        
        
        
        
    }
}



//----------------------------------------------------------------------------------------------------------------------------
//
//  FINISH
//
//----------------------------------------------------------------------------------------------------------------------------



// Coleta dos dados para Estatística no CastaliaResults
// declareOutput define o nome da estatística
// collectOutput coleta os dados e joga no declareOutput específico
void Basic802154Around::finishSpecific()
{
    //Around - debug: Imprime se é CH
    if((isCH)||(isPANCoordinator)){
        trace() << "SOU CH " << SELF_MAC_ADDRESS << " COM PROFUNDIDADE " << depth;
    }
    
    
    //Around - debug: Imprimindo no arquivo MAC cada CH com a lista de seus filhos
//    map <int,bool>::const_iterator i1;
//    if((isCH)||(isPANCoordinator)){
//        for (i1 = associatedDevices.begin(); i1 != associatedDevices.end(); i1++) {
//            Macinfos() << SELF_MAC_ADDRESS << " " << i1->first;
//        }
//    }
    
  
    //Around - debug: Imprimindo a estrutura scheduleCH - vetor dos CHs na ordem para serem escalonados
    vector<int>::iterator it1;
    
    //Variável para definir qual é a profunidade máxima de cada simulação
    int depthMAX = 0;
    
    if(isPANCoordinator){
        trace() << "Estrutura SCHEDULE CH:"; // Around - Debug
        for (it1 = scheduleCH.begin(); it1 != scheduleCH.end(); it1++) {
            trace() << "ZZ " << *it1;
        }
    }
    
    //Around - debug: Imprimindo a estrutura ctree - relação pai-filho do cluster-tree
    map <int,int>::const_iterator i2;
    if(isPANCoordinator){
        //Variável para definir q quantidade de coordenadores de cada simulação
        int contCHs = 0;
        trace() << "Estrutura CLUSTER-TREE:"; // Around - Debug
        for (i2 = ctree.begin(); i2 != ctree.end(); i2++) {
            trace() << "YY1 " << i2->first << " " << i2->second;
            contCHs++;
        }
        trace() << "Number of Coordinator Nodes: " << contCHs;
        //Incluindo no CastaliaResults a quantidade de clusters
        declareOutput("Number of Clusters");
        collectOutput("Number of Clusters", "", contCHs);
        
        
    }
    
    //Contando o numero de nodes orfãos na rede
    if(isPANCoordinator){
        int contOrphan = 0;
        float depthAvg = 0;
        int total = getParentModule()->getParentModule()->getParentModule()->par("numNodes");
        

        trace() << "PPPNode " << SELF_MAC_ADDRESS << " PPPdepth " << depth << " n PPPfilhos " << nchildren;
        for (int i = 1; i <= total-1; i++){ // total de nodes, excluindo o proprio PAN
            Basic802154Around *Node = check_and_cast<Basic802154Around*>(getParentModule()->getParentModule()->getParentModule()->getSubmodule("node",i)->getSubmodule("Communication")->getSubmodule("MAC"));
            //trace() << "PPP Node " << i << " associated pan = " << Node->getAssociatedPAN();
            trace() << "PPPNode " << i << " PPPdepth " << Node->getDepth() << " n PPPfilhos " << Node->getNchildren();
            depthAvg += Node->getDepth();
            if(Node->getAssociatedPAN() == -1){ //Node não associado. Conta como órfão!!
                trace() << "ORPHAN: " << i;
                contOrphan++;
            }        
        }
               
        depthAvg = depthAvg/(total-1); //media de profundiade excluindo coordenador PAN

        trace() << "Profundidade média: "<<(depthAvg);

        declareOutput("Depth Average");
        collectOutput("Depth Average", "", depthAvg);

        //Incluindo no CastaliaResults a quantidade de nodos orfaos
        declareOutput("Number of Orphan Nodes");
        collectOutput("Number of Orphan Nodes", "", contOrphan);
    
        declareOutput("Sum of SD");
        collectOutput("Sum of SD", "", schedList[schedList.size()-1].off);


    }

    if(nchildren > 0){         
        declareOutput("Num of Associated Devices");
        collectOutput("Num of Associated Devices", "", nchildren);
    }

    declareOutput("My nhTable size");
    collectOutput("My nhTable size", "", nhCont.size());    
        
        
        
    
    
    //Around - debug: Imprimindo a lista de CHs de cada Coordenador
    //vector<int>::const_iterator it2;
//    vector<int>::iterator it2;
//    if(isPANCoordinator || isCH){
////        trace() << "Lista de CHs do Coordenador: " << SELF_MAC_ADDRESS; // Around - Debug
//        for (it2 = associatedCH.begin(); it2 != associatedCH.end(); it2++) {
////            trace() << "YY2 " << SELF_MAC_ADDRESS << " " << *it2;
//        }
//    }
//    
//    //Around - debug: Imprimindo a estrutura graph - para cada CH, a lista de CHs em overlapping
//    map <int, vector<int> >::const_iterator i3;
//    //vector<int>::const_iterator it;
//    vector<int>::iterator it;
//    if(isPANCoordinator) {
////        trace() << "Estrutura GRAFO:"; // Around - Debug
//        for (i3 = graph.begin(); i3 != graph.end(); i3++){
//            trace() << "CH " << i3->first;
//            for (it = i3->second.begin(); it != i3->second.end(); it++) {
////                trace() << *it << " em Overlapping";
//            }
//        }
//    }
    
  
    
    //Around - debug: Imprimindo o mapeamento de endereço MAC para endereço hierarquico
//    map <int,int>::const_iterator i6;
//    if(isPANCoordinator){
//        for (i6 = routingTable.begin(); i6 != routingTable.end(); i6++) {
//            trace() << "End MAC: "<< i6->first << " com End hierarquico: " << i6->second;
//        }
//    }
    
    
    // Around: para ser usado no matlab
    trace() << "XX " << SELF_MAC_ADDRESS << " associatedPAN " << associatedPAN;  // Around - Debug
    
    // Around: Saber quantos filhos cada CH tem
    if((isCH)||(isPANCoordinator)){
        trace() << "No: " << SELF_MAC_ADDRESS << " associado a " << associatedPAN << " com " << nchildren << " filhos!";
        trace() << "DEPTH " << depth << " CH Node: " << SELF_MAC_ADDRESS << " - Hierachical Address: " << routingTable[SELF_MAC_ADDRESS] << " - Addressing Block: [" << startAddrBlock - 1 << ", " << endAddrBlock << "]";
        
/////    }else{
/////        trace() << "No: " << SELF_MAC_ADDRESS << " associado a " << associatedPAN << " NAO";
    }
    
    
    
    if (currentPacket)
		cancelAndDelete(currentPacket);
    if (desyncTimeStart >= 0)
		desyncTime += getClock() - desyncTimeStart;
    
    map <string,int>::const_iterator iter;

	declareOutput("Packet breakdown");
    //trace() << "OVERFLOW = " << packetoverflow;
	if (packetoverflow > 0)
	for (iter = packetBreak.begin(); iter != packetBreak.end(); ++iter) {
		if (iter->first.compare("Success") == 0) {
			collectOutput("Packet breakdown", "Success, first try", iter->second);
		} else if (iter->first.compare("Broadcast") == 0) {
			collectOutput("Packet breakdown", "Broadcast", iter->second);
		} else if (iter->first.find("Success") != string::npos) {
			collectOutput("Packet breakdown", "Success, not first try", iter->second);
		} else if (iter->first.find("NoAck") != string::npos) {
			collectOutput("Packet breakdown", "Failed, no ack", iter->second);
		} else if (iter->first.find("CSfail") != string::npos) {
			collectOutput("Packet breakdown", "Failed, busy channel", iter->second);
		} else if (iter->first.find("NoPAN") != string::npos) {
			collectOutput("Packet breakdown", "Failed, no PAN", iter->second);
        } else if (iter->first.find("Replaced") != string::npos) {                          //incluído para pacotes substituidos
            collectOutput("Packet breakdown", "Failed, Packet was replaced", iter->second); //incluído para pacotes substituidos
		} else {
			trace() << "Unknown packet breakdown category: " <<
				iter->first << " with " << iter->second << " packets";
		}
	}
    
    //Estatísticas para pacotes de dados substituídos ao chegar da camada de network, por existir outro na fila
    declareOutput("Replaced Data Packets");
    if (replacedPackets > 0)
        collectOutput("Replaced Data Packets", "Replaced", replacedPackets);
    
    
    
    
    
    if(isPANCoordinator){
        trace() << "END SIMULATION!";
        interfMatrix.clear();
        RSSIinterfMatrix.clear();
        ctree.clear();
        graph.clear();
        repnodes.clear();
        CHrepnodes.clear();
        treerepnodes.clear();
        scheduleCH.clear();
        soCH.clear();
        offsetCH.clear();
        offsetFormation.clear();
        routingTable.clear();
        invroutingTable.clear();
        addressBlock.clear();
        checkSched.clear();
    }
    
    
}


// Função de ajuda para mudar o estado do MAC interno e imprimir uma declaração de debug se necessário
void Basic802154Around::setMacState(int newState)
{
	//Se o estado antigo macState for o memso do novo estado, ele apenas retorna e nada imprime
    if (macState == newState){
		//Se quiser imprimir qu enão houve mudança de estado
/////        trace() << "Same Current MAC state: " << stateDescr[macState]; //add ERICO
        return;
    }
	//imprime no trace que mudou de um estado macState (antigo) para um novo estado (newState)
/////    if (printStateTransitions)
/////		trace() << "MAC state changed from " << stateDescr[macState] << " to " << stateDescr[newState];
	macState = newState;
}

Basic802154AroundPacket *Basic802154Around::newConnectionRequest(int PANid) {
	Basic802154AroundPacket *result = new Basic802154AroundPacket("PAN associate request", MAC_LAYER_PACKET);
	result->setDstID(PANid);
	result->setPANid(PANid);
	result->setMac802154AroundPacketType(MAC_802154_ASSOCIATE_PACKET);
	result->setSrcID(SELF_MAC_ADDRESS);
	result->setByteLength(COMMAND_PKT_SIZE);
	return result;
}

Basic802154AroundPacket *Basic802154Around::newGtsRequest(int PANid, int slots) {
	Basic802154AroundPacket *result = new Basic802154AroundPacket("GTS request", MAC_LAYER_PACKET);
	result->setPANid(PANid);
	result->setDstID(PANid);
	result->setMac802154AroundPacketType(MAC_802154_GTS_REQUEST_PACKET);
	result->setSrcID(SELF_MAC_ADDRESS);
	result->setGTSlength(slots);
	result->setByteLength(COMMAND_PKT_SIZE);
	return result;
}



//----------------------------------------------------------------------------------------------------------------------------
//
//  HANDLE ACK PACKETS
//
//----------------------------------------------------------------------------------------------------------------------------



void Basic802154Around::handleAckPacket(Basic802154AroundPacket * rcvPacket)
{
	if (currentPacket == NULL) {
/////		trace() << "WARNING received ACK packet while currentPacket == NULL";
		return;
	}
		
	cancelTimer(ACK_TIMEOUT);

	switch (currentPacket->getMac802154AroundPacketType()) {

		//received an ack while waiting for a response to association request
		case MAC_802154_ASSOCIATE_PACKET: {
            associatedPAN = rcvPacket->getPANid();
			if (desyncTimeStart >= 0) {
				desyncTime += getClock() - desyncTimeStart;
				desyncTimeStart = -1;
			}

			//trace() << "Associated with PAN:" << associatedPAN << " with depth_node " << depth;
			
            //Antigo - Se associa e já vai para o CAP
            //setMacState(MAC_STATE_CAP);
			
            
            clearCurrentPacket("Success",true);
            
            //Around - Depois de se associar, dorme e espera instruções
            setTimer(SLEEP_START,0); //Cancelado por enquanto
            
            break;
		}

		//received an ack while waiting for a response to data packet
		case MAC_802154_DATA_PACKET: {
			if (currentPacket->getSeqNum() == rcvPacket->getSeqNum()) {
/////				trace() << "Data packet successfully transmitted to " << rcvPacket->getSrcID()
/////						<< ", local clock " << getClock() << " ORIGEM: " << currentPacket->getSource();
				clearCurrentPacket("Success",true);
                
                if(isCH || isPANCoordinator){
                    // Se tiver sido uma comunicação indireta (o source do ACK está abaixo do meu cluster)
                    // devo atualizar as variaveis do mecanismo de endereços pendentes
                    int tempHierAddress = routingTable[rcvPacket->getSrcID()];  //pegando o endereço hierarquico do nó que enviou o ack
                    if( (tempHierAddress >= startAddrBlock) && (tempHierAddress <= endAddrBlock) ){  // nó está abaixo do meu cluester
/////                        trace() << "Destino " << rcvPacket->getSrcID() << " ABAIXO do meu cluster -> ATUALIZAR Pending Address List";
/////                        trace() << "inicio do meu bloco: " << startAddrBlock;
/////                        trace() << "fim do meu bloco: " << endAddrBlock;
                        // Antes numPendingAddr-- estava aqui, mas estava gerando problemas quando o pacote era duplicado,
                        // como entrava duas vezes aqui, decrementava duas vezes e esse valor podia se tornar negativo.
                        //Assim, coloquei ele na condição. Assim, só decrementa se tiver o pacote na lista.
                        //numPendingAddr--; //decrementei a lista
                    
                        //vector <int>::const_iterator t;
                        vector <int>::iterator t;
                        for (t = pendingAddrList.begin(); t != pendingAddrList.end(); t++) {
                            if(rcvPacket->getSrcID() == *t){
/////                                trace() << "ACHEI O " << *t;  // around - debug
                                pendingAddrList.erase(t); //apago o nó da lista de endereços pendentes
                                break; // se acha o endereço que estava pendente, encerra o loop e o iterator fica na posicao do endereço
                            }
                        }
                    
                        //vector<struct pendingAddrStruct>::const_iterator t2;
                        vector<struct pendingAddrStruct>::iterator t2;
                        for (t2 = pendingAddrTxBuffer.begin(); t2 != pendingAddrTxBuffer.end(); t2++) {
                            if(rcvPacket->getSrcID() == t2->node){
/////                                trace() << "ACHEI O " << t2->node;  // around - debug
                                pendingAddrTxBuffer.erase(t2); //apago o nó da lista de endereços pendentes
                                numPendingAddr--; //decrementei a lista
                                break; // se acha o endereço que estava pendente, encerra o loop e o iterator fica na posicao do endereço
                            }
                        }
                    
                        // Around - debug: confirmando o tamanho das variaveis de endereços pendentes
/////                        trace() << "VALOR DO NUMERO DE ENDERECOS: " << numPendingAddr;  // around - debug
/////                        trace() << "TAMANHO DA LISTA: " << pendingAddrList.size();  // around - debug
/////                        trace() << "TAMANHO DO BUFFER: " << pendingAddrTxBuffer.size();  // around - debug
                   
                }
/////                else {
/////                    trace() << "Destino " << rcvPacket->getSrcID() << " ACIMA do meu cluster!";
/////                    }
                }
                
                
			} else {
				collectPacketHistory("NoAck");
				attemptTransmission("Wrong SeqNum in Ack");
			}
			break;
		}
            
        
        case MAC_802154_CCH_PACKET:{
            clearCurrentPacket("Success",true);
            
            break;
        }
         
            
		case MAC_802154_GTS_REQUEST_PACKET:{
			assignedGTS_node(rcvPacket->getGTSlength());
			clearCurrentPacket("Success",true);
			break;
		}
            
        //received an ack while waiting for a data request response
        case MAC_802154_DATA_REQUEST_PACKET: {
            //if (currentPacket->getSeqNum() == rcvPacket->getSeqNum()) {
/////                trace() << "Recebi Data Request ACK Packet do Cluster-head " << rcvPacket->getPANid();  // around - debug
                clearCurrentPacket("Success",true);
                toRadioLayer(createRadioCommand(SET_STATE, RX));
            //}
            break;
        }

		default:{
/////			trace() << "WARNING: received unexpected ACK to packet [" << currentPacket->getName() << "]";
			break;
		}
	}
}



//----------------------------------------------------------------------------------------------------------------------------
//
//  HANDLE MAC COMMAND
//
//----------------------------------------------------------------------------------------------------------------------------


int Basic802154Around::handleControlCommand(cMessage * msg){
    trace() << "RECEBI COMANDO DA CAMADA SUPERIOR!";
    
    AroundControlMessage *cmd = check_and_cast <AroundControlMessage*>(msg);
    
    switch(cmd->getAroundControlMessageKind()) {
            
        case SET_AROUND: {
            break;
        }
            
        
        case CANCEL_AROUND: {
            break;
        }
            
        
        default:{
            trace() << "WARNING: unknown control message type received [" << cmd->getName() << "]";
        }
    }

    return 0;
}



//----------------------------------------------------------------------------------------------------------------------------
//
//  CSMA-CA ALGORITHM - SEVERAL FUNCTIONS
//
//----------------------------------------------------------------------------------------------------------------------------



/* Finishes the transmission attempt(s) for current packet
 * Records packet history and performs transmission outcome callback
 */
void Basic802154Around::clearCurrentPacket(const char * s, bool success) {
	if (currentPacket == NULL) return;
	if (s) collectPacketHistory(s);
	if (success) currentPacketSuccess = true;
	if (currentPacket->getMac802154AroundPacketType() == MAC_802154_DATA_PACKET) {
		if (currentPacket->getDstID() != BROADCAST_MAC_ADDRESS)
			packetBreak[currentPacketHistory]++;
		else
			packetBreak["Broadcast"]++;
    
	}
    
    //TESTANDO ESTATISTICA PARA PACOTES PERDIDOS
    if(currentPacket->getMac802154AroundPacketType() == MAC_802154_DATA_PACKET){
        if(currentPacketHistory.find("Success") == string::npos){
//            collectOutput("PERCA",currentPacket->getSource());
        }
  
    }
        
	
	// transmissionOutcome callback below might request another transmission by
	// calling transmitPacket(). Therefore, we save and clear the currentPacket 
	// variable and delete it only _after_ the callback.
	Basic802154AroundPacket *tmpPacket = currentPacket;
	
    //Código abaixo é o Original
    currentPacket = NULL;
    transmissionOutcome(tmpPacket, currentPacketSuccess, currentPacketHistory);
    cancelAndDelete(tmpPacket);
    
}

// This function provides a new packet to be transmitted by the MAC
// It may not be transmitted immediately, but MAC will keep it untill 
// one of the following happens:
// 1) All transmission attempts were exausted (if specified)
// 2) Delay limit was exceeded (if specified)
// 3) Packet can not be delivered, e.g. if PAN connection is lost
// 4) transmitPacket called again, replacing the old packet
void Basic802154Around::transmitPacket(Basic802154AroundPacket *pkt, int retries, bool state, double limit) {
	clearCurrentPacket("Replaced"); //acaba apagando o currentPacket. Assim coloquei o status "Replaced"
    //trace() << "transmitPacket([" << pkt->getName() << "]," << retries << "," << state << "," << limit << ")";
	currentPacket = pkt;
	currentPacketGtsOnly = state;
	currentPacketHistory = "";
	currentPacketLimit = limit;
	currentPacketSuccess = false;
	if (currentPacket->getDstID() == BROADCAST_MAC_ADDRESS) {
		currentPacketRetries = 1;
		currentPacketResponse = 0;
	} else {
		currentPacketRetries = retries == 0 ? macMaxFrameRetries : retries;
		currentPacketResponse = macAckWaitDuration;
        
        //Around - Se nowACK for true, quadros de associação esperam ACK imediato por um período ACK_TIMEOUT um pouco maior
        if (macState == MAC_STATE_FORMATION) {
            //Tendo ACK de imediato
            if (nowFormationACK) {
                currentPacketResponse = ackWaitFormation;
            } else { // Sem ACK de imediato
                currentPacketResponse = 0;
            }
            
        }
        
	}
    if (getTimer(ATTEMPT_TX) < 0 && getTimer(ACK_TIMEOUT) < 0){
        attemptTransmission("transmitPacket() called");
    }
}


// Esta função irá iniciar uma tentativa de transmissão (ou retransmissão)
void Basic802154Around::attemptTransmission(const char * descr)
{
	// Cancela o timer ATTEMPT_TX
    cancelTimer(ATTEMPT_TX);
	
    // Se o Estado MAC tiver como SLEEP ou SETUP, a função retorna
    if (macState == MAC_STATE_SLEEP || macState == MAC_STATE_SETUP)
        return;
	// Gravado no trace a transmissão e sua descrição, passado para quem chamou a função
   //trace() << "Attempt transmission, description: " << descr;

	// Se um pacote já enfileirado para transmissão - checa retries e atraso disponíveis
	if (currentPacket && (currentPacketRetries == 0 || (currentPacketLimit > 0 && (simTime() - currentPacket->getCreationTime()) > currentPacketLimit))) {
		clearCurrentPacket();
		return;
	}
	
	if (currentPacket) {
		//ERICO
        if (macState == MAC_STATE_GTS) {
/////			trace() << "Transmitting [" << currentPacket->getName() << "] in GTS";
			transmitCurrentPacket();
		} else
            if (macState == MAC_STATE_CAP && currentPacketGtsOnly == false) {
/////                trace() << "Transmitting [" << currentPacket->getName() << "] in CAP, starting CSMA_CA";
                NB = 0;
                CW = enableSlottedCSMA ? 2 : 1;
                BE = macBattLifeExt ? (macMinBE < 2 ? macMinBE : 2) : macMinBE;

                // simtime_t txTime = TX_TIME(currentPacket->getByteLength()) + currentPacketResponse;

                // trace() << "CurrentFrameStart: " << currentFrameStart;
                // trace() << "CurrentFrameEnd: " << currentFrameStart+CAPend;
                // trace() << "Buffer Size: " << TXBuffer.size();
                // trace() << "TX time: " << (TXBuffer.size()+1)*txTime;
                // simtime_t aux_CSMA = (double)rand()/ (RAND_MAX)*(currentFrameStart+CAPend-getClock()-((TXBuffer.size()+1)*txTime));

                // trace() << "Tempo de espalhamento: " << aux_CSMA;
                
                // if(aux_CSMA>=0)
                //     setTimer(CAP_FILLING, aux_CSMA);
                // else
                //     setTimer(CAP_FILLING, 0);

                performCSMACA();


            } else
                if(macState == MAC_STATE_FORMATION){
                    //trace() << "Transmitting [" << currentPacket->getName() << "] in CAP, starting CSMA_CA";
                    NB = 0;
                    CW = enableSlottedCSMA ? 2 : 1;
                    BE = macBattLifeExt ? (macMinBE < 2 ? macMinBE : 2) : macMinBE;
                    performCSMACA();
                } else
                    if(macState == MAC_STATE_CAP_CH){
                        int tempHierAddress = routingTable[currentPacket->getDstID()]; //pegando o endereço hierarquico do destino do currentpacket
                        if (tempHierAddress >= startAddrBlock && tempHierAddress <= endAddrBlock) { //o currentpacket era para baixo
                            NB = 0;
                            CW = enableSlottedCSMA ? 2 : 1;
                            BE = macBattLifeExt ? (macMinBE < 2 ? macMinBE : 2) : macMinBE;
                            hasPendingPkt = false;
                            performCSMACA();
                        }
/////                        else {
/////                            trace() << "Not Transmitting [" << currentPacket->getName() << "] now! I am acting as Cluster-Head!";
/////                        }
                    
  
            
                } /////else
                    /////trace() << "Skipping transmission attempt in CAP due to GTSonly flag";
            
	}else {
		//trace() << "Nothing to transmit";
	}
}

// continue CSMA-CA algorithm
void Basic802154Around::performCSMACA()
{
	//generate a random delay, multiply it by backoff period length
	int rnd = genk_intrand(1, (1 << BE) - 1) + 1;
	simtime_t CCAtime = rnd * (aUnitBackoffPeriod * symbolLen);

	//if using slotted CSMA - need to locate backoff period boundary
	if (enableSlottedCSMA) {
		simtime_t backoffBoundary = (ceil((getClock() - currentFrameStart) / (aUnitBackoffPeriod * symbolLen)) -
		     (getClock() - currentFrameStart) / (aUnitBackoffPeriod * symbolLen)) * (aUnitBackoffPeriod * symbolLen);
		CCAtime += backoffBoundary;
	}

/////	trace() << "CSMA/CA random backoff value: " << rnd << ", in " << CCAtime << " seconds";

	//set a timer to perform carrier sense after calculated time
	setTimer(PERFORM_CCA, CCAtime);
}

/* Transmit a packet by sending it to the radio */
void Basic802154Around::transmitCurrentPacket()
{
	if (currentPacket == NULL) {
/////		trace() << "WARNING: transmitCurrentPacket() called while currentPacket == NULL";
		return;
	}
	
	//check if transmission is allowed given current time and tx time
	simtime_t txTime = TX_TIME(currentPacket->getByteLength()) + currentPacketResponse;
	simtime_t txEndTime = getClock() + txTime;
	int allowTx = 1;
	
	if (macState == MAC_STATE_CAP) {	//currently in CAP
    //if (macState == MAC_STATE_CAP || macState == MAC_STATE_CAP_CH_SEND) {	//currently in CAP (member nodes and Cluster-head)
		if (currentFrameStart + CAPend < txEndTime && CAPend != GTSstart)
			//transmission will not fit in CAP 
			allowTx = 0;
	} else if (macState == MAC_STATE_GTS) {	//currently in GTS
		if (currentFrameStart + GTSend < txEndTime)
			//transmission will not fit in GTS
			allowTx = 0;
	}
	
	if (allowTx) {
		if (currentPacket->getMac802154AroundPacketType() == MAC_802154_DATA_PACKET) {
			if (macState == MAC_STATE_CAP) collectOutput("pkt TX state breakdown", "Contention");
            else collectOutput("pkt TX state breakdown", "Contention-free");
                
		}
		//decrement retry counter, set transmission end timer and modify mac and radio states.
		currentPacketRetries--;
		//trace() << "Transmitting [" << currentPacket->getName() << "] now, remaining attempts " << currentPacketRetries;
		setTimer(currentPacketResponse > 0 ? ACK_TIMEOUT : ATTEMPT_TX, txTime);
        
        toRadioLayer(currentPacket->dup());
		toRadioLayer(createRadioCommand(SET_STATE, TX));
        
	} else {
		//transmission not allowed
		trace() << "txTime " << txTime << " CAP:" << (currentFrameStart + CAPend - getClock()) << 
				" GTS:" << (currentFrameStart + GTSend - getClock());
		trace() << "Transmission of [" << currentPacket->getName() << "] stopped, not enough time";
	}
}

// String s represents an outcome of most recent transmission attempt for 
// current packet, it is saved (appended) to the final packet history
void Basic802154Around::collectPacketHistory(const char *s)
{
	if (!currentPacket) {
		return;
	}
	if (currentPacketHistory.size()) {
		currentPacketHistory.append(",");
		currentPacketHistory.append(s);
	} else {
		currentPacketHistory = s;
	}
}



//----------------------------------------------------------------------------------------------------------------------------
//
//  DECISION LAYER FUNCTIONS
//
//----------------------------------------------------------------------------------------------------------------------------



// A function to accept new packet from the Network layer
// ACTION: 	check if packet can be transmitted immediately
//			otherwise accept only if there is room in the buffer
bool Basic802154Around::acceptNewPacket(Basic802154AroundPacket *newPacket)
{
    trace() << "Entrei no accept " << SELF_MAC_ADDRESS;
	if (getAssociatedPAN() != -1 && getCurrentPacket() == NULL) {
		transmitPacket(newPacket);
		return true;
	}
	return bufferPacket(newPacket);
}

// Função para reagir a recepção de um beacon
// AÇÃO: Se não associado a um PAN, cria e transmite uma requisição de conexão
void Basic802154Around::receiveBeacon_node(Basic802154AroundPacket *beacon)
{
	// Se associatedPAN do nó é -1, é porque ele não está associado...
    if (getAssociatedPAN() == -1)
		// Transmite um pacote de requisção de associação, endereçado ao ID do PAN
        transmitPacket(newConnectionRequest(beacon->getPANid()));
}

// A function to react to packet transmission callback
// ACTION: Simply transmit next packet from the buffer if associated to PAN
void Basic802154Around::transmissionOutcome(Basic802154AroundPacket *pkt, bool success, string history)
{
	//Código original abaixo
//    if (getAssociatedPAN() != -1 && TXBuffer.size()) {
//		Basic802154AroundPacket *packet = check_and_cast<Basic802154AroundPacket*>(TXBuffer.front());
//		TXBuffer.pop();
//		transmitPacket(packet);
//	}
    
    //Novo código, considerando o buffer agora de saída
    if(macState == MAC_STATE_CAP_CH){ //eh pq está atuando como CH, então comunicou para baixo
        //a próxima tentativa de envio será para baixo
        if (getAssociatedPAN() != -1 && DownTXBuffer.size()) {
            Basic802154AroundPacket *packet = check_and_cast<Basic802154AroundPacket*>(DownTXBuffer.front());
            DownTXBuffer.pop();
            hasPendingPkt = true;
            transmitPacket(packet);
        }
        
    } else {
        //Não ta atuando como CH, então pega o dado do seu buffer para cima
        if (getAssociatedPAN() != -1 && TXBuffer.size()) {
            Basic802154AroundPacket *packet = check_and_cast<Basic802154AroundPacket*>(TXBuffer.front());
            TXBuffer.pop();
            transmitPacket(packet);
        }
    }
}

//Redefinindo a função de bufferização a fim de alterar o pacote de controle do MAC
int Basic802154Around::bufferPacket(cPacket * rcvFrame)
{
    if ((int)TXBuffer.size() >= macBufferSize) {
        cancelAndDelete(rcvFrame);
        // send a control message to the upper layer
        AroundControlMessage *fullBuffMsg = new AroundControlMessage("MAC buffer full", MAC_CONTROL_MESSAGE);
        fullBuffMsg->setAroundControlMessageKind(BUFFER_FULL);
        trace() << "BUFFER FULL!";
        send(fullBuffMsg, "toNetworkModule");
        return 0;
    } else {
        TXBuffer.push(rcvFrame);
        trace() << "PCKT ADD IN BUFFER";
        
        return 1;
    }
}



//----------------------------------------------------------------------------------------------------------------------------
//
//  ALGORITHM FUNCTIONS
//
//----------------------------------------------------------------------------------------------------------------------------



Basic802154AroundPacket *Basic802154Around::newAssociationRequest(int PANid) {
    Basic802154AroundPacket *result = new Basic802154AroundPacket("PAN associate request", MAC_LAYER_PACKET);
    result->setDstID(PANid);
    result->setPANid(PANid);
    result->setMac802154AroundPacketType(MAC_802154_ASSOCIATE_PACKET);
    result->setSrcID(SELF_MAC_ADDRESS);
    result->setByteLength(COMMAND_PKT_SIZE);
    return result;
}




// TADEU: Função de Seleção de CCH, escolhe os próximos CCHs de acordo com suas características
void Basic802154Around::selectCCH(){
    
    map <int,bool>::const_iterator iter;
	map <int,int>::const_iterator kiter;
    double tch = formationTime * maxFormationFrame; //tempo que leva para formar um cluster
    double tcch = tch + 0.12; //tempo de formação de cluster + uma sobra para troca de mensagens
    double confirm_offset; // tempo de envio dos acks para os nodos
    bool hasCCH = false;  //flag para definir se tem ou não CCH
    baseDepth = par("baseDepth"); //profundidade no qual garanto formação de cluster escalonado no tempo. A partir disso, a formação ocorre em paralelo
    //maxCCH = par("maxCCH"); //quantidade máxima de CCHs
    int i = 0, j = 0; //variavel de controle da quantidade máxima de CCHs
    map <int,int> jm; //variavel de controle para definição dos tempos de formação dos CCHs
    vector <int> cchs;
	vector <int> candidatos; //todos os nodos dentro do range; opcoes de escolha de CH 
	map <int, double> nh_rels, aux;
    vector <int>::iterator viter;
    int cont = 0, cont_threshold = 1, flag = 1, cont_diff, maior_diff_threshold, maior_diff, candidato, contzero_threshold = 0, ref_cont_threshold = 0; //contador de CCHs
	int sum_diff = 0, sum_diff_threshold = 0,  avg_diff = 0, avg_diff_threshold = 0, contzero_diff = 0, contzero_rssi; 
    double higher_RSSI, sum_rssi = 0, avg_rssi = 0;
    map <int,vector <int>>::const_iterator miter;
    
    vector <int>::const_iterator c1,c2;       
    map <int,double>  m1; 
	map <int,double>::const_iterator im1; 

    //transfere os membros do cluster para um vetor 'all_neighbors'
    if(SELF_MAC_ADDRESS != 0){
        for (iter = chTable.begin(); iter != chTable.end(); iter++)
            knn_neighbors[iter->first] = 0;
    }

    //chTable.clear();

	//transfere os ja associados para um vetor 'all_neighbors'
	for (iter = associatedDevices.begin(); iter != associatedDevices.end(); iter++){
        chTable[iter->first] = false;
		candidatos.push_back(iter->first);
        knn_neighbors[iter->first] = 0;
	}

	while(flag > 0){

        maior_diff_threshold = 0;
        maior_diff = 0;
        sum_diff = 0;
        sum_diff_threshold = 0;
        sum_rssi = 0;
        higher_RSSI = (-100);
        flag = 0;
        contzero_threshold = 0;
        contzero_diff = 0;
        contzero_rssi = 0;

        //percorre todos candidatos associados
        for (c1 = candidatos.begin(); c1 != candidatos.end(); c1++) {

            m1 = nhTable[*c1];
            cont_diff = 0;
            cont_threshold = 0;

            //contabiliza o numero de vizinhos, para cada candidato, que nao estao associados a rede (visao local)
            for(im1 = m1.begin(); im1 != m1.end(); im1++){
                //caso o nodo nao esteja associado cont_diff eh incrementado
                if(knn_neighbors.find(im1->first) == knn_neighbors.end()){
                    cont_diff++; //total de vizinhos nao associados
                    if(im1->second >= rssi_threshold)
                        cont_threshold++;  //vizinhos nao associados dentro do limiar 
                }
            }

            trace() << "O nodo " << *c1 << " possui " << cont_diff << " vizinhos e "<< cont_threshold << " no limite com rssi = " << m1[SELF_MAC_ADDRESS] ;
            
            

            if(cont_diff == 0) contzero_diff++; //excluo da lista de candidatos aqueles que nao possuem vizinhos potenciais
            else sum_diff += cont_diff;

            if(m1[SELF_MAC_ADDRESS]<(rssi_threshold+5)) sum_rssi += m1[SELF_MAC_ADDRESS];
            else contzero_rssi++;
            
            if(cont_threshold == 0) contzero_threshold++; //contagem dos numero de candidatos sem vizinhos dentro do limiar
            else  sum_diff_threshold += cont_threshold; 
           
        }


        trace() << "N de candidatos: " << candidatos.size() << "; Cont zerados: " <<contzero_diff<< ", " << contzero_threshold << " e " << contzero_rssi;
        
        //obtem as medias de vizinhos total e dentro do limiar ainda nao associados por candidato
        if(candidatos.size() > 0 && (candidatos.size()-contzero_diff) > 0){
            avg_diff = sum_diff/((candidatos.size()-contzero_diff));
            avg_rssi = sum_rssi/((candidatos.size()-contzero_rssi));
        }else{
            avg_diff = 0;
            avg_rssi = 0;
        }

        if(candidatos.size() > 0 && (candidatos.size()-contzero_threshold) > 0)
            avg_diff_threshold = sum_diff_threshold/((candidatos.size()-contzero_threshold));
        else
            avg_diff_threshold = 0;

        trace() << "Media de nao associados: " << avg_diff << " RSSI medio: " << avg_rssi <<" Media dentro do limiar: " << avg_diff_threshold;

        if(avg_diff > 0){
            //percorre todos candidados
            for (c1 = candidatos.begin(); c1 != candidatos.end(); c1++) {

                m1 = nhTable[*c1];
                cont_diff = 0;
                cont_threshold = 0;

                //contabiliza seus vizinhos
                for(im1 = m1.begin(); im1 != m1.end(); im1++){
                    if(knn_neighbors.find(im1->first) == knn_neighbors.end()){
                        cont_diff++; //total de vizinhos nao associados
                        if(im1->second >= rssi_threshold)
                            cont_threshold++;  //vizinhos nao associados dentro do limiar 
                    }
                }


                
                 //dentro das condicoes, determina o vizinho com maior quantidade de cobertura;
                if(cont_diff >= higher_RSSI){ 
                    if(cont_diff==higher_RSSI && cont_threshold > maior_diff_threshold){
                        flag = 1;      
                        maior_diff_threshold = cont_threshold;
                        higher_RSSI = cont_diff;
                        c2 = c1;
                        candidato = *c1;
                    }else{
                        flag = 1;      
                        higher_RSSI = cont_diff;
                        c2 = c1;
                        candidato = *c1;
                    }
                }   
            
                //trace() << "higher = " << higher_RSSI << ", maior_diff_threshold = " << maior_diff_threshold;

                //dentro das condicoes, determina o vizinho com maior RSSI;
                // if(cont_diff >= avg_diff && cont_threshold >= avg_diff_threshold && avg_diff_threshold > 0){
                //     flag = 1;
                //     if(m1[SELF_MAC_ADDRESS] > higher_RSSI){       
                //         higher_RSSI = m1[SELF_MAC_ADDRESS];
                //         c2 = c1;
                //         candidato = *c1;
                //     }     
                // }
            }
        }

        if(flag > 0){
            
            m1 = nhTable[candidato];
            cchs.push_back(candidato);
            jm[candidato] = cont; //atribuicao para alterar a ordem de formacao
            chTable[candidato] = true;
            cont++;
            trace() << "O candidato " << candidato << " foi SELECIONADO para CH com rssi = " << m1[SELF_MAC_ADDRESS];

            //adiciono todos os vizinhos do candidato selecionado ao vetor de comparacao
            for(im1 = m1.begin(); im1 != m1.end(); im1++){
                    if(im1->second >= rssi_threshold && chTable.find(im1->first) == chTable.end()){
                        knn_neighbors[im1->first] = 0;
                        chTable[im1->first] = false;
                    }
            }
            candidatos.erase(c2);
        }

    }

	if(cont>0 || associatedDevices.size() > 0){ 
		hasCCH = true;
        createTableCH();
	}		
     


    //Se Nó tiver menos de 1 filhos, não deixo ser CH
   if(associatedDevices.size() < 1){
        trace() << "DESALOCANDO POR POUCO NODES: " << SELF_MAC_ADDRESS << ". QUANT FILHOS: " << associatedDevices.size(); //around - debug
        isCH = false;
        hasCCH = false;
    }

    
    //Criando a estrutura de dados ctree - relação pai-filho da cluster-tree
    //Criando a estrutura de sequencia de cluster
    // Para não incluir o PAN, basta colocar uma condição && (!PANCoordinator)
    if(hasCCH){
        ctree[SELF_MAC_ADDRESS] = associatedPAN;
        scheduleCH.push_back(SELF_MAC_ADDRESS);
        
        Basic802154Around *node = check_and_cast<Basic802154Around*>(getParentModule()->getParentModule()->getParentModule()->getSubmodule("node",0)->getSubmodule("Communication")->getSubmodule("MAC"));
        node->t->setRelation(SELF_MAC_ADDRESS, associatedPAN);
        
    } else {
        //Não tendo CCH, deixa de ser CH
///        trace() << "DESALOCANDO POR NAO TER CCH: " << SELF_MAC_ADDRESS; //around - debug
        isCH = false;
        //setTimer(SLEEP_START,0); //vai dormir. Cancelado, por enquanto
    }
    
    
    // Around - debug - impressao dos CCHs escolhidos
    for(viter = cchs.begin(); viter != cchs.end(); viter++){
        trace() << "CCH Escolhido: " << *viter;
    }
    
    //Enviar os ACKs para formação
    for (iter = associatedDevices.begin() ; iter != associatedDevices.end(); iter++) {
        if (iter->second == true) {
            
            //trace() << "Lista Devices a enviar ACK: " << iter->first;  // Around - Debug
            // Cria um pacote de ACK para reply aos nós pelo qual recebeu pedido de associação
            formationAckPacket = new Basic802154AroundPacket("PAN Associate Response", MAC_LAYER_PACKET);
            // Seta o id do PAN como o dele do nó
            formationAckPacket->setPANid(SELF_MAC_ADDRESS);
            // Seta o tipo do pacote para MAC_802154_ACK_PACKET, que naturalmente, após ser recebido do radio (fromRadioLayer),
            // será tratado no case MAC_802154_ACK_PACKET
            formationAckPacket->setMac802154AroundPacketType(MAC_802154_CCH_PACKET);
            // Seta o ID de destino, que será o nó que solicitou a associação
            formationAckPacket->setDstID(iter->first);
            
            
            //Around - Procura se o dispositivo filho em questão foi escolhido como CCH
            bool isCCH = false;
            for(viter = cchs.begin(); viter != cchs.end(); viter++){
                if(iter->first == *viter)
                    isCCH = true;
            }
            
            if(jm.find(iter->first) != jm.end()) j = jm[iter->first];

			trace() << "Profundidade: " << depth << " Position: " << position << " j: " << j;

            if(isCCH){
				const char *str = TableCH.c_str();
				formationAckPacket->setTableCH(str);
                formationAckPacket->setNewCH(true);
                formationAckPacket->setByteLength(CCH_PKT_SIZE+30);
                if(isPANCoordinator){
                    formationAckPacket->setStartTime(tcch + j*tcch);
                    formationAckPacket->setPosition(j);
                }
                else
					if(depth < baseDepth){
                        formationAckPacket->setStartTime( (pow(3,depth) + 2*position - 1)*tcch + j*tcch + tcch);
                        formationAckPacket->setPosition(3*position + j);
                    }
                    else
                        if (depth == baseDepth){
                            formationAckPacket->setStartTime( (pow(3,depth) - 1 - position)*tcch + j*tcch + tcch );
                            formationAckPacket->setPosition(j);
                        }
                        else{
                            formationAckPacket->setStartTime((2-position)*tcch + j*tcch + tcch);
                            formationAckPacket->setPosition(j);
                        }
			}
            else{
                formationAckPacket->setNewCH(false);
                formationAckPacket->setByteLength(CCH_PKT_SIZE);
            }
            
            // Seta o tamanho do pacote de Formation ACK, que será ACK_CCH_SIZE, definido no #define do .h
            
            
            //comentei aqui porque quero que a confirmacao seja enviada para todos os nodos, para poderem dormir
            // if(hasCCH){
            //     if(nowFormationACK){
            //         if (formationAckPacket->getNewCH() == true) {
            //             toRadioLayer(formationAckPacket);
            //             toRadioLayer(createRadioCommand(SET_STATE, TX));
            //         }
                    
            //     } else {
            //         toRadioLayer(formationAckPacket);
            //         // Comando do rádio para ir ao estado TX
            //         toRadioLayer(createRadioCommand(SET_STATE, TX));
                    
            //     }

            if(hasCCH){
                
                toRadioLayer(formationAckPacket);
                toRadioLayer(createRadioCommand(SET_STATE, TX));


                //Seta o time ATTEMPT_TX com o tempo relativo ao tamanho do pacote    
                if(isCCH)
                    confirm_offset = TX_TIME(CCH_PKT_SIZE+60);
                else    
                    confirm_offset = TX_TIME(CCH_PKT_SIZE);
                    
                setTimer(ATTEMPT_TX, confirm_offset);
                trace() << "Pacote de ACK enviado para o nodo "<< iter->first;
            }
            else{
                formationAckPacket->setPANid(-1);
                formationAckPacket->setSrcID(SELF_MAC_ADDRESS);
                formationAckPacket->setNewCH(false);
                formationAckPacket->setByteLength(CCH_PKT_SIZE+2);
                acceptNewPacket(formationAckPacket);
            }
            
            
        }  //fim do if
    }  //fim do for
    
    //manda de novo mensagem de confirmacao para os CHS
    for (iter = associatedDevices.begin() ; iter != associatedDevices.end(); iter++) {
        if (iter->second == true) {
            
            //trace() << "Lista Devices a enviar ACK: " << iter->first;  // Around - Debug
            // Cria um pacote de ACK para reply aos nós pelo qual recebeu pedido de associação
            formationAckPacket = new Basic802154AroundPacket("PAN Associate Response", MAC_LAYER_PACKET);
            // Seta o id do PAN como o dele do nó
            formationAckPacket->setPANid(SELF_MAC_ADDRESS);
            // Seta o tipo do pacote para MAC_802154_ACK_PACKET, que naturalmente, após ser recebido do radio (fromRadioLayer),
            // será tratado no case MAC_802154_ACK_PACKET
            formationAckPacket->setMac802154AroundPacketType(MAC_802154_CCH_PACKET);
            // Seta o ID de destino, que será o nó que solicitou a associação
            formationAckPacket->setDstID(iter->first);

            bool isCCH = false;
            for(viter = cchs.begin(); viter != cchs.end(); viter++){
                if(iter->first == *viter){
                    isCCH = true;
                    const char *str = TableCH.c_str();
				    formationAckPacket->setTableCH(str);
                    formationAckPacket->setNewCH(true);
                    formationAckPacket->setByteLength(CCH_PKT_SIZE+30);
                }
            }

                 if(nowFormationACK){
                     if (formationAckPacket->getNewCH() == true) {
                         toRadioLayer(formationAckPacket);
                         toRadioLayer(createRadioCommand(SET_STATE, TX));
                         confirm_offset = TX_TIME(CCH_PKT_SIZE+60);
                         setTimer(ATTEMPT_TX, confirm_offset);
                         trace() << "REPETE: Pacote de ACK enviado para o CH "<< iter->first;
                     }
                 }        
            
                    
                

        }
    }

    setTimer(SLEEP_START, confirm_offset);
    
}


// Função para confirmar os nós filhos dos clusters
// Estruturas criadas:
// - função confirmCluster e seu cabecalho no .h
// - Timer CONFIRM_CLUSTER e incluido no enum Mac802154Timers no .h
void Basic802154Around::collisionDomain(){
    // Cria um pacote
    collisionDomainPacket = new Basic802154AroundPacket("Collision Domain Formation", MAC_LAYER_PACKET);
    
    collisionDomainPacket->setMac802154AroundPacketType(MAC_802154_COLLISIONDOMAIN_PACKET);
    collisionDomainPacket->setSrcID(SELF_MAC_ADDRESS);
    collisionDomainPacket->setDstID(BROADCAST_MAC_ADDRESS);
    collisionDomainPacket->setPANid(associatedPAN);
    if((isPANCoordinator) || (isCH)){
        collisionDomainPacket->setIsCH(true);
        if(nchildren < maxChildren)
            collisionDomainPacket->setAssociationAllow(true);
        else
            collisionDomainPacket->setAssociationAllow(false);
    }
    else{
        collisionDomainPacket->setIsCH(false);
        collisionDomainPacket->setAssociationAllow(false);
    }
    collisionDomainPacket->setByteLength(CCH_PKT_SIZE);
    toRadioLayer(collisionDomainPacket);
    toRadioLayer(createRadioCommand(SET_STATE, TX));
    setTimer(ATTEMPT_TX, TX_TIME(CCH_PKT_SIZE));
    
    //Around - Ir para o escalonamento de beacons (Apenas o PAN)
    if(isPANCoordinator)
        beaconSchedule();
    
}



// Função de escalonamento dos clusters (escalonamento de beacons)
// Criar uma estrutura de offsets, onde cada CH verifica seu offset de início
void Basic802154Around::beaconSchedule(){
    map <int,double>::const_iterator miter;
    vector <int>::iterator viter;
    int BI, SD;
    int sumSD = 0;
    int i = 0; //base para o calculo do offset de cada CH
    
    trace() << i;
    
    //Preciso, nesse ponto, saber o SO e BO para cada CH, para poder efetuar a alocação e escalonamento corretamente
    //Nesse nosso caso, consideraremos que todos os BOs e SOs são iguais, ou seja, todos tem o mesmo BI e SD
    //Assim o escalonamento é sequencial, onde o próximo CH começa SD tempo depois do anterior.
    // Caso seja utilizado outro algoritmo de escalonamento, deve ser implementado aqui!!!
    //TADEU, a implementação do escalonamento, por questões práticas, será realizada na proxima função: reScheduling()
    
    BI = aBaseSuperframeDuration * (1 << beaconOrder);
    SD = aBaseSuperframeDuration * (1 << superframeOrder);
    sumSD = scheduleCH.size() * SD;
    
    if(sumSD > BI){
        SD = BI/scheduleCH.size();
    }
    
    for (viter = scheduleCH.begin(); viter != scheduleCH.end(); viter++) {
        offsetCH[*viter] = SD * i;
        
        //adicionei a estrutura abaixo para auxiliar o escalonamento bottom-up, que é feito de forma top-down
        offsetFormation[*viter] = SD * i;
        
        i++;
    }
    
    
    // Indo pro Timer para escalonamento de beacons (6s)
    if(isPANCoordinator)
        setTimer(BEACON_SCHEDULE ,6);
    
    
}




// Função para re-escalonamento da rede (escalonamento final considerando a quantidade de nós de cada CH)
// Criar uma estrutura de offsets, onde cada CH verifica seu offset de início
void Basic802154Around::reScheduling(){
    map <int,double>::const_iterator miter;
    //vector <int>::const_iterator viter;
    vector <int>::iterator viter;

	double y,x, DC;

    
    //A estrutura scheduleCH define o escalonamento, que por padrão na formação do clusters, eles sao inseridos na estrutura
    // em uma forma top-down. Para usar um escalonamento bottom-up, torna-se necessário inverter a ordem dessa estrutura
    trace() << "Bottom-up Cluster Scheduling is being used!";
            
    //INVERTENDO O VETOR DE ESCALONAMENTO
    trace() << "scheduleCH original!!";
    for (viter = scheduleCH.begin(); viter != scheduleCH.end(); viter++) {
        trace() << "CH " << *viter;
    }
            
    //invertendo
    reverse(scheduleCH.begin(), scheduleCH.end());
    trace() << "scheduleCH invertido!!";
    for (viter = scheduleCH.begin(); viter != scheduleCH.end(); viter++) {
        trace() << "CH " << *viter;
    }
    
    //TADEU, nesse ponto será feito a alocação do Beacon Interval e Superframe duration para os clusters.
    // Na minha implementação inicial, eu defini uma série de esquemas de alocação, com base em vários critérios.
    // Para o seu caso, o tamanho dos SDs será com base na car

     // O esquema de alocação será definido a partir de parâmetros passados através do arquivo de ini
    // Cada cluster terá seu duty cycle baseado em sua característica específica.
    
    //Definindo BO na variavel global
    globalBO = beaconOrder;
        
	int BI, SD;
    int sumSD = 0;
    int i = 0; //base para o calculo do offset de cada CH
    int totalNodes = getParentModule()->getParentModule()->getParentModule()->par("numNodes");

    BI = aBaseSuperframeDuration * (1 << beaconOrder); //960*2^BO

    // ----------------------------------- Definindo o SO para cada CH - daniel -----------------------------------
	//--------------------------------------------------------------------------------------------------------------

    for (viter = scheduleCH.begin(); viter != scheduleCH.end(); viter++) {
        //defino um duty diferente para o PAN, baseado no numero de filhos
        if(SELF_MAC_ADDRESS == *viter){ //definindo duty cycle do PAN
        
			x = floor((1/packet_rate_prio)/(BI*phyBitsPerSymbol/250000));
			y = (1/x)*totalNodes+log2(totalNodes);
			DC = ceil(log2(ceil(y)));
            soCH[*viter] = DC;

        } else { //definindo duty cycle dos outros CHs
            
            // pegando ponteiro da aplicação para saber o datarate
            AroundApproachApp *node = check_and_cast<AroundApproachApp*>(getParentModule()->getParentModule()->getParentModule()->getSubmodule("node",*viter)->getSubmodule("Application"));
			
			Basic802154Around *nodeMAC = check_and_cast<Basic802154Around*>(getParentModule()->getParentModule()->getParentModule()->getSubmodule("node",*viter)->getSubmodule("Communication")->getSubmodule("MAC"));
			            
			x = floor((1/node->getdataRate())/(BI*phyBitsPerSymbol/250000));
			y = (1/x)*nodeMAC->getTotalchildren()+log2(nodeMAC->getTotalchildren());
			DC = ceil(log2(ceil(y+1.5)));
			soCH[*viter] = DC;

			//trace() << "DC calculado para o nodo " << *viter << " eh: " << DC << " numero de filhos, x, y: " << nodeMAC->getTotalchildren() << " " << x << " " << y;
        }

        trace() << "DC calculado para o nodo " << *viter << " eh: " << DC << " numero de filhos, x, y: "  << x << " " << y;

    }

	//-------------------------------------------------------------------------------------------------------------
	// ----------------------------------- Definindo o SO para cada CH - daniel -----------------------------------

    // De posse de todos os SO's de cada CH, agora eu defino a estrutura dos offsets, contendo o offset de cada CH
    // e a estrutura schedList que será usada pelo ARounD para escalonar seus fluxos
       
    for (viter = scheduleCH.begin(); viter != scheduleCH.end(); viter++) {
        SD = aBaseSuperframeDuration * (1 << soCH[*viter]);
        offsetCH[*viter] = sumSD;
        // Alimentando a Lista encadeada com o escalonamento que será utilizado para alocar um par Around
        struct schedNode tempNode;
        tempNode.ch = *viter;
        tempNode.off = sumSD;
        tempNode.time = SD;
        trace() << "RR: RESCHED: CH " << *viter << " tem offset " << sumSD << " e SD " << soCH[*viter] << " em symbols = " << SD << " em seg = " << SD * symbolLen; //around - debug
        sumSD = sumSD + SD; //acrescentando o SD acumulável do cluster-head
        schedList.push_back(tempNode);
        
        
    }
    
    // Impressão e teste simples de escalonamento!!!
    
    trace() << "AA " << scheduleCH.size() << " " << sumSD;  //Around - Debug
    trace() << "AA SOMATORIO de SDs " << sumSD;  //Around - Debug
    trace() << "AA BI " << BI;  //Around - Debug

    if(sumSD <= BI){
        trace() << "Protocol Constraint Verificado! Somatorio de SD <= BI";
    } else {
        trace() << "Protocol Constraint nao foi obedecido! Somatorio de SD extrapolou o valor de BI";
        opp_error("Protocol Constraint nao foi obedecido! Somatorio de SD extrapolou o valor de BI");
    }
   
    
    // Acrescentando espaço vazio
    if(sumSD < BI){
        struct schedNode tempNode;
        tempNode.ch = -1;
        tempNode.off = sumSD;
        tempNode.time = BI - sumSD;
        schedList.push_back(tempNode);
    }

    // Around - Debug
    for (miter = offsetCH.begin(); miter != offsetCH.end(); miter++) {
        trace() << "AA " << miter->first << " " << miter->second; // around - debug
    }

    //Around - debug: Imprimindo estrutura de escalonamento do Around
    for(int t = 0; t < schedList.size(); t++){
        trace() << "Around Schedule: " << schedList[t].ch << " " << schedList[t].off << " " << schedList[t].time; //around - debug
    }

    if(isPANCoordinator)
        setTimer(BEGIN_BEACON, 0);
    
}


bool Basic802154Around::DuplicatedPacket(cPacket * pkt){
    //extract source address and sequence number from the packet
    Basic802154AroundPacket *macPkt = check_and_cast <Basic802154AroundPacket*>(pkt);
    int src = macPkt->getSource();
    int dst = macPkt->getDestination();
    unsigned int sn = macPkt->getSeqNum();
    pair<int,int> temp = make_pair(src,dst);
    
    if(AroundPktHistory.find(make_pair(src,dst)) == AroundPktHistory.end() ){
        trace() << "Nao encontrei esse par src-dst: " << src << "-" << dst;
        AroundPktHistory[make_pair(src,dst)] = sn;
        return false;
    } else {
        if(sn <= AroundPktHistory[make_pair(src,dst)]){
            return true;
        } else {
            AroundPktHistory[make_pair(src,dst)] = sn;
            return false;
        }
    }
    
}

int Basic802154Around::distributed_hub(Basic802154AroundPacket * pkt, double rssi) {

	Basic802154AroundPacket *pacote = check_and_cast <Basic802154AroundPacket*>(pkt);

	map <int, double> my_rel, node_rel;
    map <int, double>::const_iterator iter;
    map <int, bool>::const_iterator biter;
	int retorno = 1;
    

    my_rel = nhTable[SELF_MAC_ADDRESS];
    node_rel = nhTable[pacote->getSrcID()];

    //if(nChildren > maxChildren) return 0;

    //trace() << "Pedido de associacao recebido de " << pacote->getSrcID() << " com rssi = " << rssi;

    
    if(nchildren >= maxChildren) return 0;


    //caso exista algum CH do meu cluster melhor posicionado que eu, nao aceito associacao
    for(biter = chTable.begin(); biter != chTable.end(); biter++){

        if(biter->second == 1 && biter->first != SELF_MAC_ADDRESS && node_rel.find(biter->first) != node_rel.end() && node_rel.find(SELF_MAC_ADDRESS) != node_rel.end()){
            if(node_rel[biter->first] > node_rel[SELF_MAC_ADDRESS]){
                trace() << "O CH " << biter->first << " esta melhor localizado, com RSSI = " << node_rel[biter->first] << " > " << rssi;
                return 0;
            } 
        }
    }   
    
	return retorno;
}

void Basic802154Around::createTableCH(){
	
    map <int, bool>::const_iterator iter;

	for(iter = chTable.begin(); iter != chTable.end(); iter++){
	
		TableCH += to_string(iter->first) + '/';		

        if(iter->second == true)
            TableCH += '1';
        else
            TableCH += '0';
		
        TableCH += ',';

	}

	trace() << " Tabela de CH para ser enviada: " << TableCH;
}

void Basic802154Around::readTableCH(string rTable){

    int node, vf;
	char *split;		
	char * rt = new char[rTable.size() + 1];
	std::copy(rTable.begin(), rTable.end(), rt);
	rt[rTable.size()] = '\0'; // don't forget the terminating 0

	split = strtok(rt, "/,");

	while(split != NULL){
						
        node = atoi(split);
		//trace() << "SPLIT " << split;
		
        split = strtok(NULL, "/,");
        
        vf = atoi(split);
        //trace() << "SPLIT " << split;

        chTable[node] = vf;
        
        trace() << "Lendo chTable " << node << " ch: " << vf;

        split = strtok(NULL, "/,");
	}	

}

//insere as relacoes em uma string
void Basic802154Around::createTable(){
	
	map <int, map<int, double>>::const_iterator iter;
    map <int, double>::const_iterator miter;

	for(iter=nhTable.begin(); iter != nhTable.end(); iter++){
	
		relationTable += to_string(iter->first);		
	
		for(miter = iter->second.begin() ; miter != iter->second.end(); miter++){ 		
	
			//trace() << " O nodo " << iter->first << " esta relacionado com " << miter->firt;			
		
			relationTable += ',' + to_string(miter->first);
			relationTable += '/' + to_string(miter->second);

		}

		relationTable += ';';

	}

	trace() << " TESTE: " << relationTable;
}
	

void Basic802154Around::readTable(string rTable, int nodeID){

	char *split1, *split2;		
	int relationID, i = 0;
    double rssi_value;
	char * rt = new char[rTable.size() + 1];
	std::copy(rTable.begin(), rTable.end(), rt);
	rt[rTable.size()] = '\0'; // don't forget the terminating 0

	split1 = strtok(rt, ",");
    split1 = strtok(NULL, ",/;");

	while(split1 != NULL){

        if(i%2 == 0) relationID = atoi(split1);
        //trace() << "Nodo " << nodeID << " esta relacionado com " << relationID;

        if(i%2 == 1){
            rssi_value = atof(split1);
            nhTable[nodeID][relationID] = rssi_value; 
            ///trace() << "Nodo " << nodeID << " esta relacionado com " << relationID << " com RSSI de " << rssi_value;
        }
        				       
        split1 = strtok(NULL, ",/;");
        i++;

	}

}
	
void Basic802154Around::calcRSSI_avg() {

    map <int, double> deletados;
    map <int, double>::const_iterator iter;
    vector <double> ordenado;
    vector <double>::const_iterator viter;
    double maior = -99;
    int cont = 0;
	float maxChildren_rssi_threshold;
   
    
    for(iter = nhTable[SELF_MAC_ADDRESS].begin(); iter != nhTable[SELF_MAC_ADDRESS].end(); iter++){

        //trace() << "Meu vizinho " << iter->first << " foi escutado " << nhCont[iter->first] <<" vezes e tem soma " << nhTable[SELF_MAC_ADDRESS][iter->first];
        nhTable[SELF_MAC_ADDRESS][iter->first] = iter->second/nhCont[iter->first];
        
        if(nhTable[SELF_MAC_ADDRESS][iter->first] < rssi_threshold){
            deletados[iter->first] = iter->second;
        }else{
            ordenado.push_back(nhTable[SELF_MAC_ADDRESS][iter->first]);
        }
    }

    for(iter = deletados.begin(); iter != deletados.end(); iter++)
       nhTable[SELF_MAC_ADDRESS].erase(iter->first);

    sort(ordenado.begin(),ordenado.end());

   
    if(ordenado.size()>maxChildren)
        maxChildren_rssi_threshold = ordenado[ordenado.size()-maxChildren];
    else
        maxChildren_rssi_threshold = rssi_threshold;
    
    trace() << "AROUND: rssi_threshold: " << maxChildren_rssi_threshold << " ordenado.size " << ordenado.size();

    for(iter = nhTable[SELF_MAC_ADDRESS].begin(); iter != nhTable[SELF_MAC_ADDRESS].end(); iter++){
        if(iter->second < maxChildren_rssi_threshold)
            deletados[iter->first] = iter->second;
    }

    for(iter = deletados.begin(); iter != deletados.end(); iter++)
        nhTable[SELF_MAC_ADDRESS].erase(iter->first);

	
	// trace() << "ACARCH: maxChidren_rssi_threshold: " << acarch->make_neighborhood_threshold(SELF_MAC_ADDRESS, rssi_threshold, 															maxChildren);  	
	

}
