//
//  teste.cpp
//
//
//  Created by Erico Meneses Leão on 10/07/15.
//
//

#ifndef H_graph
#define H_graph

#include <iostream>
#include <fstream>
#include <limits>
//#include <iomanip>
#include <map>
#include <unordered_map>
#include <vector>
#include <queue>
#include <set>
#include <list>
#include <algorithm>

#include <omnetpp.h>
#include "ResourceManager.h"
#include "Radio.h"
#include "TimerService.h"
#include "CastaliaModule.h"


using namespace std;

class Graph: public CastaliaModule{
public:
    // Determina se o grafo é vazio (retorna true se for).
    bool isEmpty();
    
    // Determina se um determinado vertice pertence ao grafo.
    bool isVertex(int vertex);
    
    // Criar um grafo a partir de parâmetros passados.
    void setRelation(int vertex, int adjacentVertex);
    
    // Imprimir um grafo
    void printGraph();
    
    // Imprimir a Lista de Adjacência dos vertices
    void printAdjacencyList();
    
    // Travessia em Largura de um grafo, a partir do root
    void breadthTraversal();
    
    //Retornar o grafo
    map<int, vector<int> > getGraph();
    
    //Retornar o root
    int getRoot();
    
    //Retornar o root
    void setRoot(int n);
    
    // Procura o menor caminho entre dois vertices
    vector<int> shortPathFind(int source, int destination);
    
    // Retorna as restrições Around para um par de cluster
    vector<int> getConstraints(int c1, int c2);
    
    //Constructor
    Graph();
    
    //Destructor
    ~Graph();

protected:
    
    //Primeiro Vertice a ser inserido no grafo
    int first;
    
    // Lista ordenada de adjacência: Nó - lista de nós adjacentes
    map <int, vector<int> > graph;
    map <int, vector<int> >::const_iterator i; // iterator do grafo
    
    // lista dos pesos para as arestas do grafo
    map <pair<int,int>, double> weight;
    
private:

};


class Tree: public CastaliaModule{
public:
    // Determina se a arvore esta vazio (retorna true se for).
    //bool isEmpty();
    
    // Determina se um determinado vertice pertence ao grafo.
    //bool isVertex(int vertex);
    
    // Criar um grafo a partir de parâmetros passados.
    void setRelation(int vertex, int adjacentVertex);
    
    // Imprimir um grafo
    //void printGraph();
    
    // Imprimir a Lista de Adjacência dos vertices
    //void printAdjacencyList();
    
    // Travessia em Largura de um grafo, a partir do root
    //int treePath();
    
    //Retornar o grafo
    //map<int, vector<int> > getGraph();
    
    //Retornar o root
    int getRoot();
    
    //Retornar o root
    void setRoot(int n);
    
    // Procura o menor caminho entre dois vertices
    vector<int> treePath(int source, int destination);
    
    // Retorna as restrições Around para um par de cluster
    //vector<int> getConstraints(int c1, int c2);
    
    //Constructor
    Tree();
    
    //Destructor
    ~Tree();
    
protected:
    
    //Primeiro Vertice a ser inserido no grafo
    int root;
    
    // Lista ordenada de adjacência: Nó - lista de nós adjacentes
    map <int, vector<int> > tree;
    map <int, vector<int> >::const_iterator i; // iterator do grafo
    
    // lista dos pesos para as arestas do grafo
    map <pair<int,int>, double> weight;
    
private:
    
};



#endif