/***********************************************************************************************
 *  Copyright (c) Federal University of Para, brazil - 2012                                    *
 *  Developed at the Research Group on Computer Network and Multimedia Communication (GERCOM)  *
 *  All rights reserved                                                                        *
 *                                                                                             *
 *  Permission to use, copy, modify, and distribute this protocol and its documentation for    *
 *  any purpose, without fee, and without written agreement is hereby granted, provided that   *
 *  the above copyright notice, and the author appear in all copies of this protocol.          *
 *                                                                                             *
 *  Author(s): C. Perkins                                                                      *
 *  Reference Paper: RFC 3561                                                                  *
 *  Implementarion: Thiago Fernandes <thiago.oliveira@itec.ufpa.br> <thiagofdso.ufpa@gmail.com>* 
 **********************************************************************************************/

#include "graph.h"

using namespace std;

Define_Module(Graph);

//Constructor
Graph::Graph(){
    //graph = new map <int, vector<int> >();
}

//Destructor
Graph::~Graph(){
    graph.clear();
    weight.clear();
}

//Função para verificar se grafo está vazio
bool Graph::isEmpty() {
    return (graph.size() == 0);
}

bool Graph::isVertex(int vertex){
    for(i = graph.begin(); i != graph.end(); i++)
        if(vertex == i->first)
            return true;
    return false;
}

// Inserção de Aresta, considerando o Mapa ordenado (map)
void Graph::setRelation(int vertex, int adjacentVertex){
    // Guardando o primeiro nó do grafo - NÃO ESTÁ FAZENDO CORRETO, pois nem sempre o primeiro a inserir é o root
    //if(isEmpty())
    //    first = vertex;
    trace() << "Class Graph: Inserido " << adjacentVertex << " em " << vertex;
    graph[vertex].push_back(adjacentVertex);
    //graph[adjacentVertex].push_back(vertex);
    //cout << "NO ORIGEM: " << first << endl;
}


// Faz uma travessia no grafo "breadth First" em um grafo ordenado
void Graph::breadthTraversal(){
    trace() << ">> Breadh First Traversal <<";
    
    queue<int> q; //Fila para inserir os vertices visitados
    int u; //guardar vertice temporário
    
    // iterators
    //map <int, vector<int> >::const_iterator i;
    map<int,bool>::const_iterator v;
    vector <int>::const_iterator j;
    
    map<int,bool> visited;  //vetor visitado
    
    //iniciando o "vetor" de visitados
    for(i = graph.begin(); i != graph.end(); i++)
        visited[i->first] = false;
    
    for(v = visited.begin(); v != visited.end(); v++){
        //cout << v->first << " = false" << endl;
    }
    
    q.push(first);
    visited[first] = true;
    trace() << "| " << first << " | <- Origem";
    
    while(q.size() != 0){
        u = q.front();
        q.pop();
        for (j = graph[u].begin(); j != graph[u].end(); j++){
            if (!visited[*j]){
                q.push(*j);
                visited[*j] = true;
                trace() << "| " << *j << " |";
            }
        }
    }
    
}


// Função para retornar o menor caminho entre uma origem e um destino especificado
vector<int> Graph::shortPathFind(int source, int destination){
    trace() << ">> Menor Caminho entre " << source << " e " << destination << " <<";
    
    //const adjacency_list_t &adjacency_list,   vector de estrutura, contendo o vizinho e peso
    //std::vector<weight_t> &min_distance,      std::vector<double> min_distance;
    //std::vector<vertex_t> &previous)          std::vector<int> previous;
    
    const double max_weight = numeric_limits<double>::infinity(); //preciso usar include limits
    
    // iterators
    map <int, vector<int> >::const_iterator i;
    map<int,bool>::const_iterator v;
    vector <int>::const_iterator j;
    
    map<int,double> min_distance; min_distance.clear();    //min_distance.resize(n, max_weight);
    // Preenchendo o mapa de distancia minima
    for(i = graph.begin(); i != graph.end(); i++){
        min_distance[i->first] = max_weight;
    }
    
    min_distance[source] = 0;  //setando para 0 a distância da origem
    
    int n = graph.size();
    map<int,int> previous;
    previous.clear();
    // Preenchendo o mapa de previous
    for(i = graph.begin(); i != graph.end(); i++){
        previous[i->first] = -1;
    }
    
    //    map<int,int>::const_iterator aa;
    //    for(aa = previous.begin(); aa != previous.end(); aa++){
    //        cout << aa->first << " " << aa->second << endl;
    //    }
    
    //cout << "ERICO " << previous.size() << endl;
    
    set<pair<double, int> > vertex_queue;
    vertex_queue.insert(make_pair(min_distance[source], source));
    
    while (!vertex_queue.empty()){
        double dist = vertex_queue.begin()->first;
        int u = vertex_queue.begin()->second;
        
        //cout << "-> " << dist << " " << u << endl;
        
        vertex_queue.erase(vertex_queue.begin());
        
        // Visit each edge exiting u
        //const std::vector<neighbor> &neighbors = adjacency_list[u];
        //for (std::vector<neighbor>::const_iterator neighbor_iter = neighbors.begin(); neighbor_iter != neighbors.end(); neighbor_iter++){
        for (j = graph[u].begin(); j != graph[u].end(); j++){
            int v = (*j);
            //int v = neighbor_iter->target;
            
            // Inicialmente estou considerando os pesos iguais, exceto para o root, no qual eu colo um custo maior (tentar evitar o root).
            // Futuramente eu posso usar essa variável para criar outras métricas entre os clusters
            double weight;
//            if(v == first)
//                weight = 1.1;
//            else
//                weight = 1;
            weight = 1;
            
            //double weight = neighbor_iter->weight;
            double distance_through_u = dist + weight;
            if (distance_through_u < min_distance[v]) {
                vertex_queue.erase(make_pair(min_distance[v], v));
                
                min_distance[v] = distance_through_u;
                previous[v] = u;
                vertex_queue.insert(make_pair(min_distance[v], v));
                
            }
            
        }
    }
    
    vector<int> path;
    //list<int> path;
    
    for (; destination != -1; destination = previous[destination])
        //cout << destination << " ";
        //path.push_front(destination);  // Para list
        path.insert(path.begin(),destination);  // Para vector
    
    
    //    for(j = path.begin(); j != path.end(); j++)
    //        cout << *j << " ";
    
    return path;
    
    
    
}


vector<int> Graph::getConstraints(int c1, int c2){
    vector<int> constraints;
    constraints.push_back(c1);
    constraints.push_back(c2);
    
    constraints.insert(constraints.end(), graph[c1].begin(), graph[c1].end());
    constraints.insert(constraints.end(), graph[c2].begin(), graph[c2].end());
    
    //ordenando o vetor de restrições
    sort( constraints.begin(), constraints.end() );
    //tirando duplicados
    constraints.erase( unique( constraints.begin(), constraints.end() ), constraints.end() );
    
    return constraints;
}


// Função para impressão de um grafo ordenado
void Graph::printGraph(){
    breadthTraversal(); //Basta atravessar o grafo inteiro por largura
}

// Função para impressão da Lista de Adjacência dos vertices
void Graph::printAdjacencyList(){
    map <int, vector<int> >::const_iterator i;
    vector <int>::const_iterator j;
    
    for (i = graph.begin(); i != graph.end() ; i++){
        trace() << "Vertice: " << i->first << " -> ";
        for (j = i->second.begin(); j != i->second.end() ; j++){
            trace() << "Adjacency Node: " << *j;
        }
    }
}


map<int, vector<int> > Graph::getGraph(){
    return graph;
}

int Graph::getRoot(){
    return first;
}

void Graph::setRoot(int n){
    first = n;
}






// TREE CLASS

//Constructor
Tree::Tree(){
    tree.clear();
    //weight.clear();
}

//Destructor
Tree::~Tree(){
    tree.clear();
    //weight.clear();
}


// Insere um relacionamento pai-filho.
void Tree::setRelation(int vertex, int adjacentVertex){
    //trace() << "Class Graph: Inserido " << adjacentVertex << " em " << vertex;
    tree[vertex].push_back(adjacentVertex);
    tree[adjacentVertex].push_back(vertex);
}

// Travessia em Largura de um grafo, a partir do root
vector<int> Tree::treePath(int source, int destination){
    const double max_weight = numeric_limits<double>::infinity(); //preciso usar include limits
    
    // iterators
    map <int, vector<int> >::const_iterator i;
    map<int,bool>::const_iterator v;
    vector <int>::const_iterator j;
    
    map<int,double> min_distance; min_distance.clear();    //min_distance.resize(n, max_weight);
    // Preenchendo o mapa de distancia minima
    for(i = tree.begin(); i != tree.end(); i++){
        min_distance[i->first] = max_weight;
    }
    
    min_distance[source] = 0;  //setando para 0 a distância da origem
    
    int n = tree.size();
    map<int,int> previous;
    previous.clear();
    // Preenchendo o mapa de previous
    for(i = tree.begin(); i != tree.end(); i++){
        previous[i->first] = -1;
    }
    
    set<pair<double, int> > vertex_queue;
    vertex_queue.insert(make_pair(min_distance[source], source));
    
    while (!vertex_queue.empty()){
        double dist = vertex_queue.begin()->first;
        int u = vertex_queue.begin()->second;
        
        //cout << "-> " << dist << " " << u << endl;
        
        vertex_queue.erase(vertex_queue.begin());
        
        // Visit each edge exiting u
        //const std::vector<neighbor> &neighbors = adjacency_list[u];
        //for (std::vector<neighbor>::const_iterator neighbor_iter = neighbors.begin(); neighbor_iter != neighbors.end(); neighbor_iter++){
        for (j = tree[u].begin(); j != tree[u].end(); j++){
            int v = (*j);
            //int v = neighbor_iter->target;
            
            // Inicialmente estou considerando os pesos iguais, exceto para o root, no qual eu colo um custo maior (tentar evitar o root).
            // Futuramente eu posso usar essa variável para criar outras métricas entre os clusters
            double weight;
            weight = 1;
            
            //double weight = neighbor_iter->weight;
            double distance_through_u = dist + weight;
            if (distance_through_u < min_distance[v]) {
                vertex_queue.erase(make_pair(min_distance[v], v));
                
                min_distance[v] = distance_through_u;
                previous[v] = u;
                vertex_queue.insert(make_pair(min_distance[v], v));
                
            }
            
        }
    }
    
    vector<int> path;
    //list<int> path;
    
    for (; destination != -1; destination = previous[destination])
        //cout << destination << " ";
        //path.push_front(destination);  // Para list
        path.insert(path.begin(),destination);  // Para vector
    
    
    //    for(j = path.begin(); j != path.end(); j++)
    //        cout << *j << " ";
    
    return path;
}


//Retornar o root
int Tree::getRoot(){
    return root;
}

//Retornar o root
void Tree::setRoot(int n){
    root = n;
}


