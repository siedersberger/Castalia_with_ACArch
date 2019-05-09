#ifndef H_ACARCH
#define H_ACARCH

#include <iostream>
#include <fstream>
#include <limits>
#include <map>
#include <unordered_map>
#include <vector>
#include <queue>
#include <set>
#include <list>
#include <algorithm>

#include "VirtualMac.h"
#include "CastaliaModule.h"

using namespace std;

class Acarch: public CastaliaModule{
public:

//Delete the relations that rssi is below than 'rssi_trheshold'
float make_neighborhood_threshold(int self_id, float rssi_threshold, int maxChildren);

//Add relations in the nhTable
void setNeighborhoodTable(int from_id, int to_id, float rssi);

//Constructor
Acarch();

//Destructor
~Acarch();

private:


protected:

map<int, map<int, double>>  neighborhoodTable;	    //tabela com as informações da vizinhança												
map<int, int>               neighborhoodCont;       //contador de descobrimento


};
#endif
