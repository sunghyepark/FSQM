
#include "circuit.h"

using namespace std;
using namespace Qcircuit;

// print node & edge 
void Qcircuit::QMapper::nodeset_out(ofstream &of, Qcircuit::Circuit graph)
{
    of << endl;
    for (auto& g : graph.nodeset)
    {
        if(g.control==-1)
            of << g.type <<  g.output_type << " " << "q[" << g.target << "]" << endl;
        else
            of << g.type << " " << "q[" << g.control << "],q[" << g.target << "]" << endl; 
    }
    cout << " Nodeset out is done" << endl << endl;
}

void Qcircuit::QMapper::node_print(int N, Qcircuit::Circuit graph)
{
        if(graph.nodeset[N].control==-1)
            cout << graph.nodeset[N].type << " (typenum: " << ") " << graph.nodeset[N].output_type << " " << "q[" << graph.nodeset[N].target << "]" << endl;
        else
            cout << graph.nodeset[N].type << " (typenum: " << ") " << " " << "q[" << graph.nodeset[N].control << "],q[" << graph.nodeset[N].target << "]" << endl; 
}

