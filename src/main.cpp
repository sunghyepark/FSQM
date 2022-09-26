/*----------------------------------------------------------------------------------------------*/
/* Description:     Main source code for Quantum compiler                                       */
/*                                                                                              */
/* Author:          Sunghye Park, Postech                                                       */
/*                  Minhyuk Kweon, Postech                                                      */
/*                                                                                              */
/* Created:         02/10/2020 (for Quantum compiler)                                           */
/*----------------------------------------------------------------------------------------------*/

#include "circuit.h"
#include "QASMtoken.hpp"
#include "QASMscanner.hpp"
#include "QASMparser.h"
#include "mymeasure.h"

#define ARCHI ARCHITECTURE::IBM_Q_20
#define INITIAL INITIALTYPE::GRAPH_MATCHING_MIX_UPDATE  

#define PRE_PROCESSING 1
#define POST_PROCESSING 1
#define PROCESS_NUM 7
#define BRIDGE_MODE 1

using namespace std;
using namespace Qcircuit;

Qcircuit::QMapper mapper;
static CMeasure measure;

int main(int argc, char** argv)
{
    cout << endl;
    cout << "===========================================================================" << endl;
    cout << "          FSQM (A fast and scalable qubit mapping) by Sunghye Park         " << endl;
    cout << "                                                      Minhyuk Kweon        " << endl;
    cout << "===========================================================================" << endl;
    cout << endl;
    measure.start_clock();
    
    //parsing
    cout << "\n\t================== 1. Circuit parsing ========================" << endl;
    mapper.parsing(argc, argv, POST_PROCESSING);
    measure.stop_clock("QASM parsing");
    
    //mapping
    cout << "\n\t============= 2. Circuit mapping (1st mapping) ===============" << endl;
    bool forward = true;

        // No Bridge
    mapper.initial_mapping(ARCHI, PRE_PROCESSING, false, false, INITIAL);
    mapper.Circuit_Mapping(mapper.Dgraph, ARCHI, forward, PRE_PROCESSING, false, false);
    
    mapper.least_cnot_num = mapper.add_cnot_num;
    mapper.least_swap_num = mapper.add_swap_num;
    mapper.least_bridge_num = mapper.add_bridge_num;

    mapper.Best_FinalCircuit.nodeset = mapper.FinalCircuit.nodeset;
    mapper.FinalCircuit_info(mapper.FinalCircuit, false);

        // Bridge
    mapper.initial_mapping(ARCHI, PRE_PROCESSING, false, true, INITIAL);
    mapper.Circuit_Mapping(mapper.Dgraph, ARCHI, forward, PRE_PROCESSING, false, true);
    mapper.FinalCircuit_info(mapper.FinalCircuit, false);

    measure.stop_clock("Initial & Main Mapping");

#if POST_PROCESSING
    cout << "\n\t============= 3. Post Processing (Nth mapping) ===============" << endl;
    for(int i=0; i<PROCESS_NUM; i++)
    {
        string str1 = to_string(i+1)+"_Reverse";
        string str2 = to_string(i+1)+"_Forward";

            /*for reverse circuit*/ 
        forward = false;
        mapper.initial_mapping(ARCHI, PRE_PROCESSING, true, BRIDGE_MODE, INITIAL);
        mapper.Circuit_Mapping(mapper.Dgraph_reverse, ARCHI, forward, PRE_PROCESSING, true, BRIDGE_MODE);
        //measure.stop_clock(str1);

            /*for final mapping*/ 
        forward = true;
        mapper.initial_mapping(ARCHI, PRE_PROCESSING, true, BRIDGE_MODE, INITIAL);
        mapper.Circuit_Mapping(mapper.Dgraph, ARCHI, forward, PRE_PROCESSING, true, BRIDGE_MODE);
        //measure.stop_clock(str2);

        mapper.FinalCircuit_info(mapper.FinalCircuit, false);
    }
    measure.stop_clock("Post-processing");
#endif
    
    //best circuit information
    mapper.FinalCircuit_info(mapper.Best_FinalCircuit, true);
    mapper.write_output(mapper.Best_FinalCircuit);

    measure.stop_clock("END Program");
    cout << endl <<  "*** Time & Memory Measure ***" << endl;
    measure.print_clock();
    measure.printMemoryUsage();

    return 0;
}
