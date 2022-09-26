//graph.cpp
#include "circuit.h"

using namespace std;
using namespace Qcircuit;

int Qcircuit::Node::global_id = 0;
int Qcircuit::Edge::global_id = 0;

    ////////////Node

Qcircuit::Node::Node() { }
Qcircuit::Node::Node(int weight) : weight(weight) { id = global_id++; }

int Qcircuit::Node::getid() { return id; }
int Qcircuit::Node::getglobalid() { return global_id; }
int Qcircuit::Node::getweight() { return weight; }
void Qcircuit::Node::setweight(int weight) { this->weight = weight; }

//static
void Qcircuit::Node::init_global_id() { global_id = 0; }

    ////////////Edge

Qcircuit::Edge::Edge() { }
Qcircuit::Edge::Edge(Node* source, Node* target, int weight) 
{
    id = global_id++;
    this->weight = weight;

    this->source = source;
    this->target = target;

    source->edges.insert(id);
    target->edges.insert(id);
}

int Qcircuit::Edge::getid() { return id; }
int Qcircuit::Edge::getglobalid() { return global_id; }
int Qcircuit::Edge::getweight() { return weight; }
int Qcircuit::Edge::getsourceid() { return source->getid(); }
int Qcircuit::Edge::gettargetid() { return target->getid(); }
void Qcircuit::Edge::setweight(int weight) { this->weight = weight; }
void Qcircuit::Edge::setsource(Node* source) { this->source = source; }
void Qcircuit::Edge::settarget(Node* target) { this->target = target; }

//static
void Qcircuit::Edge::init_global_id() { global_id = 0; }

    ////////////Graph

Qcircuit::Graph::Graph()
{
    init_global_id();
}

void Qcircuit::Graph::init_global_id()
{
    Node n;
    Edge e;
    n.init_global_id();
    e.init_global_id();
}

int Qcircuit::Graph::addnode(int weight)
{
    Node n(weight);
    int id = n.getid();
    nodeset[id] = n;
    return id; 
}

bool Qcircuit::Graph::deletenode(int id)
{
    if(!nodeset.count(id)) return false;
    nodeset.erase(id);
    return true;    
}

bool Qcircuit::Graph::addedge(int source, int target, int weight)
{
    if(nodeset.find(source) == nodeset.end()) return false;
    if(nodeset.find(target) == nodeset.end()) return false;
    Node *s, *t;
    s = &(nodeset.find(source)->second);
    t = &(nodeset.find(target)->second);
    Edge e(s, t, weight);
    int id = e.getid();
    edgeset[id] = e;
    return true;
}

bool Qcircuit::Graph::deleteedge(int id)
{
    if(!edgeset.count(id)) return false;
    Edge& e = edgeset[id];
    e.source->edges.erase(id);
    e.target->edges.erase(id);
    edgeset.erase(id);
    return true;
}
