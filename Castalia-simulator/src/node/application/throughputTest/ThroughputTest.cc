/****************************************************************************
 *  Copyright: National ICT Australia,  2007 - 2011                         *
 *  Developed at the ATP lab, Networked Systems research theme              *
 *  Author(s): Athanassios Boulis, Yuriy Tselishchev                        *
 *  This file is distributed under the terms in the attached LICENSE file.  *
 *  If you do not find this file, copies can be found by writing to:        *
 *                                                                          *
 *      NICTA, Locked Bag 9013, Alexandria, NSW 1435, Australia             *
 *      Attention:  License Inquiry.                                        *
 *                                                                          *
 ****************************************************************************/

#include "ThroughputTest.h"

Define_Module(ThroughputTest);

void ThroughputTest::startup()
{
	packet_rate = par("packet_rate");
	recipientAddress = par("nextRecipient").stringValue();
	recipientId = atoi(recipientAddress.c_str());
	startupDelay = par("startupDelay");
	delayLimit = par("delayLimit");
	packet_spacing = packet_rate > 0 ? 1 / float (packet_rate) : -1;
	dataSN = 0;
	
	numNodes = getParentModule()->getParentModule()->par("numNodes");
	packetsSent.clear();
	packetsReceived.clear();
	bytesReceived.clear();

	if (packet_spacing > 0 && recipientAddress.compare(SELF_NETWORK_ADDRESS) != 0)
		setTimer(SEND_PACKET, packet_spacing + startupDelay);
	else
		trace() << "Not sending packets";

	declareOutput("Packets received per node");

    	//Temperature initialisation
    kibamPeriod = par("kibamPeriod");
    ifstream file("temperature.ini");
    if (!file.is_open()){
        //trace() << "Error reading from parameters file aroundFlows.ini";
        //opp_error("Error reading from parameters file aroundFlows.ini");
        trace() << "Not Temperature File"; //around - debug
        opp_error("Not Temperature File");
    }
    
    temperature.clear(); //inicialização: limpando o vector de temperaturas
    string s;
    while(getline(file, s)) {
        
        // Removendo comentários, espaços, tabulações e quebras de linha
        size_t pos = s.find('#');
        if (pos != string::npos)
            s.replace(s.begin() + pos, s.end(), "");
    
        // find and remove unneeded spaces
        pos = s.find_last_not_of(" \t\n");
        if (pos != string::npos) {
            s.erase(pos + 1);
            pos = s.find_first_not_of(" \t\n");
            if (pos != string::npos)
                s.erase(0, pos);
        } else
            s.erase(s.begin(), s.end());
    
        if (s.length() == 0)
            continue;
        
        trace() << s; //debug
        int hora;
        float temp_inst;
        const char *ct = NULL;
        char *ctmp;
        
        cStringTokenizer t(s.c_str(), ",");
        ct = t.nextToken();
        // Ler codigo_estacao
        ct = t.nextToken();
        // Ler data
        ct = t.nextToken();
        // Ler hora
        hora = stoi(ct);
        ct = t.nextToken();
        // Ler temp_inst
        temp_inst = strtof(ct, &ctmp);
        // Ler temp_max
        ct = t.nextToken();
        // Ler temp_min
        ct = t.nextToken();
        // Ler umid_inst
        ct = t.nextToken();
        // Ler pto_orvalho_inst
        ct = t.nextToken();
        // Ler pto_orvalho_max
        ct = t.nextToken();
        // Ler pto_orvalho_min
        ct = t.nextToken();
        // Ler pressao
        ct = t.nextToken();
        // Ler pressao_max
        ct = t.nextToken();
        // Ler pressao_min
        ct = t.nextToken();
        // Ler vento_direcao
        ct = t.nextToken();
        // Ler vento_vel
        ct = t.nextToken();
        // Ler vento_rajada
        ct = t.nextToken();
        // Ler radiacao
        ct = t.nextToken();
        // Ler precipitacao
        ct = t.nextToken();
        
        // Debug: Impressão
        //trace() << "Hora " << hora << " com temperatura: " << temp_inst;
        
        // Guardando as temperaturas em uma estrutura!!
        temperature.push_back(temp_inst);
        
    }
    
    // Imprimindo vector de temperaturas??
    vector<float>::iterator itvec;
    int i = 0;
    for (itvec = temperature.begin(); itvec != temperature.end(); itvec++) {
        //trace() << "Hora " << i << ": " << *itvec << " graus.";
        i++;
    }
    
    //controlar a temperatura no timer
    
    tempcontrol = temperature.begin();
	setTimer(TEMP_CHANGE , 0);
}

void ThroughputTest::fromNetworkLayer(ApplicationPacket * rcvPacket,
		const char *source, double rssi, double lqi)
{
	int sequenceNumber = rcvPacket->getSequenceNumber();
	int sourceId = atoi(source);

	// This node is the final recipient for the packet
	if (recipientAddress.compare(SELF_NETWORK_ADDRESS) == 0) {
		if (delayLimit == 0 || (simTime() - rcvPacket->getCreationTime()) <= delayLimit) { 
			trace() << "Received packet #" << sequenceNumber << " from node " << source;
			collectOutput("Packets received per node", sourceId);
			packetsReceived[sourceId]++;
			bytesReceived[sourceId] += rcvPacket->getByteLength();
		} else {
			trace() << "Packet #" << sequenceNumber << " from node " << source <<
				" exceeded delay limit of " << delayLimit << "s";
		}
	// Packet has to be forwarded to the next hop recipient
	} else {
		ApplicationPacket* fwdPacket = rcvPacket->dup();
		// Reset the size of the packet, otherwise the app overhead will keep adding on
		fwdPacket->setByteLength(0);
		toNetworkLayer(fwdPacket, recipientAddress.c_str());
	}
}

void ThroughputTest::timerFiredCallback(int index)
{
	switch (index) {
		case SEND_PACKET:{
			trace() << "Sending packet #" << dataSN;
			toNetworkLayer(createGenericDataPacket(0, dataSN), recipientAddress.c_str());
			packetsSent[recipientId]++;
			dataSN++;
			setTimer(SEND_PACKET, packet_spacing);
			break;
		}

	        case TEMP_CHANGE: {
            		trace() << "Temperatura atual sera: " << *tempcontrol;
			//passando a temperatura para resource manager
			resMgrModule = check_and_cast <ResourceManager*>(getParentModule()->getSubmodule("ResourceManager"));
			//resMgrModule->setT(*tempcontrol); 
			
            		tempcontrol++;
            
            		if(tempcontrol == temperature.end())
               			tempcontrol = temperature.begin();
            
            		//Programando proximo timer
            		setTimer(TEMP_CHANGE, kibamPeriod); //atualiza a temperatura de hora/hora
           
            		break;
        	}
	}
}

// This method processes a received carrier sense interupt. Used only for demo purposes
// in some simulations. Feel free to comment out the trace command.
void ThroughputTest::handleRadioControlMessage(RadioControlMessage *radioMsg)
{
	switch (radioMsg->getRadioControlMessageKind()) {
		case CARRIER_SENSE_INTERRUPT:
			trace() << "CS Interrupt received! current RSSI value is: " << radioModule->readRSSI();
                        break;
	}
}

void ThroughputTest::finishSpecific() {
	declareOutput("Packets reception rate");
	declareOutput("Packets loss rate");

	cTopology *topo;	// temp variable to access packets received by other nodes
	topo = new cTopology("topo");
	topo->extractByNedTypeName(cStringTokenizer("node.Node").asVector());

	long bytesDelivered = 0;
	for (int i = 0; i < numNodes; i++) {
		ThroughputTest *appModule = dynamic_cast<ThroughputTest*>
			(topo->getNode(i)->getModule()->getSubmodule("Application"));
		if (appModule) {
			int packetsSent = appModule->getPacketsSent(self);
			if (packetsSent > 0) { // this node sent us some packets
				float rate = (float)packetsReceived[i]/packetsSent;
				collectOutput("Packets reception rate", i, "total", rate);
				collectOutput("Packets loss rate", i, "total", 1-rate);
			}

			bytesDelivered += appModule->getBytesReceived(self);
		}
	}
	delete(topo);

	if (packet_rate > 0 && bytesDelivered > 0) {
		double energy = (resMgrModule->getSpentEnergy() * 1000000000)/(bytesDelivered * 8);	//in nanojoules/bit
		declareOutput("Energy nJ/bit");
		collectOutput("Energy nJ/bit","",energy);
	}
}
