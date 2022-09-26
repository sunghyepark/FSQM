#include "circuit.h"

#define PRINT_OUTPUT_info 0
#define FINALNODE_PRINT 0

using namespace std;
using namespace Qcircuit;

void Qcircuit::QMapper::FinalCircuit_info(Circuit& graph, bool final_circuit)
{
    //final circuit
    if(!final_circuit)
    {
#if PRINT_OUTPUT_info
        cout << "*** Final Circuit ***" << endl;
        cout << "# add_cnot_num: " << add_cnot_num << endl;
        cout << "# add_swap_num: " << add_swap_num << endl;
        cout << "# add_bridge_num: " << add_bridge_num << endl;
#endif    
    }
    else
    {
        cout  << "*** Best Final Circuit ***" << endl;
        cout << "# add_cnot_num: " << least_cnot_num << endl;
        cout << "# add_swap_num: " << least_swap_num << endl;
        cout << "# add_bridge_num: " << least_bridge_num << endl;
        cout << endl;
    }
#if FINALNODE_PRINT
    cout << graph.nodeset << endl;
    cout << " ==> Print Final Circuit Successfully Finished" << endl;
#endif    
}    

void Qcircuit::QMapper::write_output(Circuit& graph)
{
    ofstream of(fileName_output);
    of << "OPENQASM 2.0;" << endl;
    of << "include \"qelib1.inc\";" << endl;
    of << "qreg q[20];" << endl; 
    of << "creg c[20];" << endl; 
    for (auto g : graph.nodeset)
    {
        if(g.control==-1)
            of << g.type <<  g.output_type << " " << "q[" << g.target << "];" << endl;
        else
            of << g.type << " " << "q[" << g.control << "],q[" << g.target << "];" << endl; 
    }
    of.close();
#if PRINT_OUTPUT_info
    cout << " ==> Output File (.qasm) Is Successfully Generated" << endl;
#endif
}

