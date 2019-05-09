//****************************************************************************
//*  Copyright (c) Faculty of Engineering, University of Porto, Portugal     *
//*  All rights reserved                                                     *
//*                                                                          *
//*  Module:   ARounD Approach for Castalia Simulator                        *
//*  File: AroundApproachApp.cc                                              *
//*  Date: 2015-03-05                                                        *
//*  Version:  0.1                                                           *
//*  Author(s): Erico Leão <ericoleao@gmail.com>                             *
//****************************************************************************/

#include "AroundApproachApp.h"
#include "Basic802154AroundControl_m.h"

Define_Module(AroundApproachApp);

void AroundApproachApp::startup()
{
    
    // ---- DEFININDO O DATARATE: com base na posicao no ambiente
//        VirtualMobilityManager *nodeMobilityModule = check_and_cast<VirtualMobilityManager*>(getParentModule()->getSubmodule("MobilityManager"));
//        double xCH = nodeMobilityModule->getLocation().x;
//        double yCH = nodeMobilityModule->getLocation().y;
//        //trace() << "MINHAS COORDENADAS: X = " << xCH << " e Y = " << yCH;  //debug
//
//        if(xCH > yCH){ //está no triangulo inferior do ambiente
//            packet_rate = 0.01;
//            packet_spacing = 1 / 0.01;
//            trace() << "TRIANGULO " << selfAddress.c_str() << " 0 - datarate " << packet_spacing;   //debug
//        } else { //está no triangulo superior do ambiente
//            packet_rate = 0.05;
//            packet_spacing = 1 / 0.05;
//            trace() << "TRIANGULO " << selfAddress.c_str() << " 1 - datarate " << packet_spacing;   //debug
//        }
    // ---------------------------------------------------------------------------------
    
    
    // ---- DEFININDO O DATARATE: através do arquivo de inicializacao-----
    packet_rate = par("packet_rate");  //Taxa de geração de pacotes (pkts/sec)
    packet_spacing = packet_rate > 0 ? 1 / float (packet_rate) : -1;  // Período entre o envio de pacotes (pkts/sec)
    // ---------------------------------------------------------------------------------
    
 
    
    // Endereço do dispositivo sink
	sinkAddress = par("sink").stringValue();
    sinkId = atoi(sinkAddress.c_str());
	// Atraso para início da geração de dados
    startupDelay = par("startupDelay");
    // Limite de atraso para a aplicação
    // Valor 0 é infinito. Descarta após ultrapassar esse limite.
	delayLimit = par("delayLimit");
    
    
    
    //Node irá gerar dados de background (default é true)
    dataGen = par("dataGen");
    
    
    // Números de sequências dos pacotes de aplicação
    dataSN = 0;     // Tráfego de base
    src2snkSN = 0;  // Tráfego source/sink
    aroundSN = 0; // Tráfego Around source/sink
    sentPacketCount = 0; //Contador para pacotes de background enviados
    sentPacketMAX = par("sentPacketMAX"); //Quantidade de pacotes de background máximo para os nós
    
	
    // Número de nós da rede - PAra estatísticas
	numNodes = getParentModule()->getParentModule()->par("numNodes");
	
    // limpa as estruturas
    packetsSent = 0;
	packetsReceived.clear();
    s2sPacketsSent = 0;
    s2sPacketsReceived.clear();
    aroundPacketsSent = 0;
    aroundPacketsReceived.clear();
    
    //contPKT = 0; //
    
    //Estatísticas - Castalia Results
    declareOutput("Generated Background Packets");
    declareOutput("BUFFER CHEIO");
    
}

void AroundApproachApp::fromNetworkLayer(ApplicationPacket * rcvPacket, const char *source, double rssi, double lqi){
/////	trace () << "ENTREI NO APP FROM NETWORK!"; //around - debug
    
    int sequenceNumber = rcvPacket->getSequenceNumber();
	int sourceId = atoi(source);
    
    AroundDataPacket *aroundPacket = check_and_cast<AroundDataPacket*>(rcvPacket);
    
/////    trace() << "PACOTE RECEBIDO DE " << aroundPacket->getSourceNode(); //around - debug
    
    switch (aroundPacket->getAroundDataPacketType()) {
        
        //Pacote de Background
        case AROUND_DATA_BACKGROUND:
/////            trace() << "RECEBI PACOTE DE TRAFEGO DE BASE DE " << aroundPacket->getSourceNode() << " SeqNum " << aroundPacket->getSeqNumber(); //around - debug
            trace()<< "BACKGROUND DATA: RECEIVED BY " << SELF_NETWORK_ADDRESS << " FROM NODE " <<source<< " - NumSeq " << aroundPacket->getSeqNumber() << " - Delay: " << simTime() - rcvPacket->getCreationTime() << " DATA: " << aroundPacket->getData();
            
			
            
            //Usando o tipo sim_time estava gerando problemas em simulações grandes. Acredito que pela capacidade de armazenar grandes valores
            //averageDelay[aroundPacket->getSourceNode()] += (simTime() - rcvPacket->getCreationTime());
            
            // Assim, troquei pelas linhas abaixo, fazendo cast do sim_time para double. Aparentemente funcionou
            double packetTime;
            packetTime = (simTime().dbl() - rcvPacket->getCreationTime().dbl());
           
        
            averageDelay[aroundPacket->getSourceNode()] += packetTime;
            packetsReceived[aroundPacket->getSourceNode()]++;
            
            break;
    
    }
}

void AroundApproachApp::timerFiredCallback(int index)
{
	switch (index) {
		// Enviar pacote de trafego background
        case SEND_PACKET:{
/////			trace() << "Sending packet #" << dataSN; //around - debug
			//Appinfos()<<"S "<<SELF_NETWORK_ADDRESS<<" "<<dataSN;
            trace()<< "BACKGROUND DATA: SENT BY " << SELF_NETWORK_ADDRESS << " - NumSeq " << dataSN;
            //Envia um pacote recem criado para a camada de rede
            // createGenericPacket tem os seguintes parâmetros: (double data, int sequenceNum, int size)
            // Se o tamanho não é fornecido, então ele é definido para constantDataPayload da aplicação
            // to NetworkLayer tem os seguintes parâmetros: (*msg) ou (*msg, const char *dstAddr)
			//toNetworkLayer(createGenericDataPacket(0, dataSN), sinkAddress.c_str());
            //toNetworkLayer(createGenericDataPacket(0, dataSN), "-1");
            
            // Criando o próprio pacote de aplicação
            AroundDataPacket *dataPkt = new AroundDataPacket("App Base Traffic Packet", APPLICATION_PACKET);
            dataPkt->setAroundDataPacketType(AROUND_DATA_BACKGROUND);
            dataPkt->setSourceNode(atoi(selfAddress.c_str()));
            dataPkt->setDestinationNode(-1);
            dataPkt->setData(0);
            dataPkt->setSeqNumber(dataSN);
            dataPkt->setByteLength(AROUND_DATA_BACKGROUND_SIZE);
            
            toNetworkLayer(dataPkt, "-1");
            
            // Coletar estatísticas para pacotes gerados!!
        	collectOutput("Generated Background Packets");
			packetsSent++;
			
            dataSN++;
			setTimer(SEND_PACKET, packet_spacing);
            
            sentPacketCount++;
            //Definindo aqui o timer para parar o envio com base na quantidade máxima de pacotes definida
            if(sentPacketCount == sentPacketMAX)
                setTimer(STOP_PACKET,0);
            
			break;
		}
            
            
        // Enviar pacote de trafego background
        case STOP_PACKET:{
            //cancelando envio de dados background, caso exista
            if(getTimer(SEND_PACKET) != -1){
                trace() << "Cancelado trafego de background existente!"; // around - debug
                cancelTimer(SEND_PACKET);
          
            }
            break;
        }
        
	}
}


void AroundApproachApp::handleMacControlMessage(cMessage *msg){
    trace() << "ENTREI NO HANDLE MAC CONTROL: "; //around - debug
    
    AroundControlMessage *cmd = check_and_cast <AroundControlMessage*>(msg);
    
    switch(cmd->getAroundControlMessageKind()) {
        
        case SET_RATE: {
            trace() << "ENTREI NO HANDLE MAC SET_RATE para ativar a geração de pacotes com datarate " << cmd->getRate(); //around - debug
            
            // Se dataGen é true o node irá gerar dados (default é true). Caso contrário, o node não gerará dados de background.
            if(dataGen) {
                
                // Para implementação do journal de análise protocolar, packet_spacing é definido para cada node, ou seja
                // cada node tem seu próprio packet rate. Assim, as definições abaixo serão realizadas no início da aplicação
                packet_rate = cmd->getRate();
                packet_spacing = packet_rate > 0 ? 1 / float (packet_rate) : -1;
            
                //trace() << packet_spacing + genk_dblrand(0)*0.05;
            	//acho que não é necessário esse sinkaddress.compare, pois o 0 nunca irá semear, nunca irá entrar aqui.
                if (packet_spacing > 0 && sinkAddress.compare(SELF_NETWORK_ADDRESS) != 0){ 
        
                    //double timeGen = 10 + genk_dblrand(0)*packet_spacing; //o 10 vem de mais ou menos dois períodos de BI a frente, considerando BO=8 (3,93s)
                    //trace() << "TEMPO GERAR: " << timeGen;
                    //setTimer(SEND_PACKET, packet_spacing + genk_dblrand(0)*100);  //Gera pacotes com tempos iniciais aleatórios (padrão 0.05, acredito que escolhi baseado na quantidade de 500 nós)
                    //setTimer(SEND_PACKET, timeGen);
                    
                    //Para a implementação do journal da analise protocolar, a geração de pacotes só começa agora quando recebe o primeiro beacon
                    setTimer(SEND_PACKET, 0);
                
                    //Defini um Timer STOP Packet para parar o envio de dados background a fim de poder analisar a taxa de pacotes perdidos.
                    // Assim, eu paro o envio de background antes do fim da simulação, dando tempo suficiente para todos os pacotes enviados de poderem tentar transmissão.
                    // Com isso, nenhum pacote é "deixado de tentar transmitir" simplesmente porque a simulação se finalizou.
                    //Como o timer SEND_PACKET é realizado mais ou menos no tempo 43 segundos, vou dar 100 segundos de comunicação
                    // Para isso, vou definir um tempo de simulação para uns 50 segundos a mais, ou seja, 43 + 100 + 50 = da perto dos 200
//                    setTimer(STOP_PACKET,100);
                }
                else
                    trace() << "Not sending packets"; //around - debug

            } else {
                trace() << "Not background data!"; //around - debug
            }
            
                
            break;
        }
            
        case BUFFER_FULL: {
            trace() << "ENTREI NO HANDLE MAC BUFFER_FULL DO NODE: " << SELF_NETWORK_ADDRESS; //around - debug
            
            collectOutput("BUFFER CHEIO");
            
            break;
        }
            
        default:{
            trace() << "WARNING: unknown control message type received [" << cmd->getName() << "]"; //around - debug
        
        }
    
    
    }
     
}



// This method processes a received carrier sense interupt. Used only for demo purposes
// in some simulations. Feel free to comment out the trace command.
void AroundApproachApp::handleRadioControlMessage(RadioControlMessage *radioMsg)
{
	switch (radioMsg->getRadioControlMessageKind()) {
		case CARRIER_SENSE_INTERRUPT:
			trace() << "CS Interrupt received! current RSSI value is: " << radioModule->readRSSI();
                        break;
	}
}

void AroundApproachApp::finishSpecific() {
    cTopology *topo;	// temp variable to access packets received by other nodes
    topo = new cTopology("topo");
    topo->extractByNedTypeName(cStringTokenizer("node.Node").asVector());
    
    declareOutput("Received Background Packets");
    declareOutput("Packets Reception Rate");
    declareOutput("Packets Loss Rate");
    declareOutput("Average end-to-end Delay");
    declareOutput("BUFFER CHEIO");
    
    //iterator para os maps
    map<int,int>::const_iterator iter;
    

    // ------- pacotes de background recebidos -----------//
    // Devo escolher um dos procedimentos abaixo:
    // 1) Considerando que o nó root é o sink de todos os nós
    // 2) Mesmo que anterior, mas calculando as taxas baseado em cada nó e não no root
    // 3) considerando que cada CH é o sink dos seus nós
    
    
    // 2) Considerando que o nó root é o sink de todos os nós
    //    Root gera a estatística, porém fiz de um jeito para considerar caso não receba pacotes de algum nó que gerou pacotes
    if(sinkAddress.compare(SELF_NETWORK_ADDRESS) == 0){
//        declareOutput("Received Background Packets by the Sink Node");
        //Taxa de recepção de pacotes
//        declareOutput("Packets Reception Rate");
        //Taxa de perda de pacotes
//        declareOutput("Packets Loss Rate");
        //Delay acumulado dos pacotes recebidos
        //declareOutput("Average end-to-end Delay");
       
        //Rodando todos os nós
        for(int i = 0; i < numNodes; i++){

            //Pegando o módulo de cada um dos nós
            AroundApproachApp *appModule = dynamic_cast<AroundApproachApp*>
            (topo->getNode(i)->getModule()->getSubmodule("Application"));
            
            int pktsSent = appModule->getPacketsSent();
            
/////            trace() << "NODE " << i << " GEROU QUANT PACOTES: " << pktsSent;
            
            if(pktsSent > 0){ //Procurando nós que geraram pacotes de dados
/////                trace() << "ENTREI PARA ESTATISTICA: " << i;
                collectOutput("Received Background Packets", i, "Sink Node", packetsReceived[i]);
                
                //Coletando as taxas de recebimento e de perca
                float rate = (float) packetsReceived[i] / pktsSent;
                collectOutput("Packets Reception Rate", i, "Total", rate);
                collectOutput("Packets Loss Rate", i, "Total", 1-rate);
                
/////                trace() << "PARA NODE " << i << " GANHO E PERCA: " << rate << " - " << 1-rate;
                
                //Coletando o end-to-end delay médio
                if(packetsReceived[i] > 0){
                    //double avDelay = SIMTIME_DBL(averageDelay[i]);
                    //collectOutput("Average end-to-end Delay", i, "Total", SIMTIME_DBL(averageDelay[i])/packetsReceived[i]);
                    collectOutput("Average end-to-end Delay", i, "Total", averageDelay[i]/packetsReceived[i]);
                } else {
                    collectOutput("Average end-to-end Delay", i, "Total", 0);
                }
            }
        }
    }
    
    delete(topo);
    
}

