#include "acarch.h"

using namespace std;

Define_Module(Acarch);

//Constructor
Acarch::Acarch(){
    
}

//Destructor
Acarch::~Acarch(){
    
}

//Add relations in the nhTable
void Acarch::setNeighborhoodTable(int from_id, int to_id, float rssi){


	trace() << "ACARCH: ACARCH: ACARCH: Nodo " << to_id << " descoberto com RSSI = " << rssi;

	if(neighborhoodTable[from_id].find(to_id) == neighborhoodTable[from_id].end()){
        neighborhoodTable[from_id][to_id] = rssi;
        neighborhoodCont[to_id] = 1;
		
    }else{
		neighborhoodTable[from_id][to_id] += rssi;
		neighborhoodCont[to_id]++;  
	}

}


//Exclude the nodes with the rssi below than a thresholds (rssi_threshold & max_children - ini file)
float Acarch::make_neighborhood_threshold(int self_id, float rssi_threshold, int maxChildren) {

    map <int, double>::const_iterator iter;
    vector <double> ordenado;
   	float maxChildren_rssi_threshold;
	
    for(iter = neighborhoodTable[self_id].begin(); iter != neighborhoodTable[self_id].end(); iter++){
		
		//For more precision, it make de avg rssi for each node in the neighborhood table    
        neighborhoodTable[self_id][iter->first] = iter->second/neighborhoodCont[iter->first];
        
		//Delete from table that nodes that rssi is below of the threshold
        if(neighborhoodTable[self_id][iter->first] >= rssi_threshold){
            ordenado.push_back(neighborhoodTable[self_id][iter->first]);
        }
    }

    sort(ordenado.begin(),ordenado.end());
   
    if(ordenado.size()>maxChildren){
        maxChildren_rssi_threshold = ordenado[ordenado.size()-maxChildren];
	}else{
		maxChildren_rssi_threshold = rssi_threshold;
	}    	

	for(iter = neighborhoodTable[self_id].begin(); iter != neighborhoodTable[self_id].end(); iter++){
	    if(iter->second < maxChildren_rssi_threshold)
	        neighborhoodTable[self_id].erase(iter->first);
	}

	return maxChildren_rssi_threshold;
}
