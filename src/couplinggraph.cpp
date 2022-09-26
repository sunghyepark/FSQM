//couplinggraph.cpp
#include "circuit.h"
using namespace std;
using namespace Qcircuit;

void Qcircuit::QMapper::select_coupling_graph(ARCHITECTURE archi)
{
    switch(archi)
    {
        case ARCHITECTURE::IBM_Q_20:
            build_graph_IBM_Q_20();
            break;
        default:
            cout << "No architecture specified!" << endl;
            exit(1);
    }
}

//build a graph representing the coupling map of IBM Q 20
void Qcircuit::QMapper::build_graph_IBM_Q_20()
{   
    positions = 20;
    for(int i=0; i<positions; i++)
        coupling_graph.addnode();
    coupling_graph.addedge(0, 1);
    coupling_graph.addedge(0, 5);
    coupling_graph.addedge(1, 2);
    coupling_graph.addedge(1, 6);
    coupling_graph.addedge(1, 7);
    coupling_graph.addedge(2, 3);
    coupling_graph.addedge(2, 6);
    coupling_graph.addedge(2, 7);
    coupling_graph.addedge(3, 4);
    coupling_graph.addedge(3, 8);
    coupling_graph.addedge(3, 9);
    coupling_graph.addedge(4, 8);
    coupling_graph.addedge(4, 9);
    coupling_graph.addedge(5, 6);
    coupling_graph.addedge(5, 10);
    coupling_graph.addedge(5, 11);
    coupling_graph.addedge(6, 7);
    coupling_graph.addedge(6, 10);
    coupling_graph.addedge(6, 11);
    coupling_graph.addedge(7, 8);
    coupling_graph.addedge(7, 12);
    coupling_graph.addedge(7, 13);
    coupling_graph.addedge(8, 9);
    coupling_graph.addedge(8, 12);
    coupling_graph.addedge(8, 13);
    coupling_graph.addedge(9, 14);
    coupling_graph.addedge(10, 11);
    coupling_graph.addedge(10, 15);
    coupling_graph.addedge(11, 12);
    coupling_graph.addedge(11, 16);
    coupling_graph.addedge(11, 17);
    coupling_graph.addedge(12, 13);
    coupling_graph.addedge(12, 16);
    coupling_graph.addedge(12, 17);
    coupling_graph.addedge(13, 14);
    coupling_graph.addedge(13, 18);
    coupling_graph.addedge(13, 19);
    coupling_graph.addedge(14, 18);
    coupling_graph.addedge(14, 19);
    coupling_graph.addedge(15, 16);
    coupling_graph.addedge(16, 17);
    coupling_graph.addedge(17, 18);
    coupling_graph.addedge(18, 19);
    //cout <<  " # This graph is: IBM Q 20" << endl << endl;
}
