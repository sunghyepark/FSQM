//mapping.cpp
#include "circuit.h"

#define Dlist_all_mode 0 //for single-qubit gate

using namespace std;
using namespace Qcircuit;

void Qcircuit::QMapper::Circuit_Mapping(Circuit& dgraph, ARCHITECTURE archi, bool forward, bool prepro, bool postpro, bool BRIDGE_MODE)
{
    // (0) Make Dlist //////////////////////////////////
    make_Dlist(dgraph);
    make_Dlist_all(dgraph);

    // (1) Circuit mapping ////////////////////////////

    //initialize --------------------------
    list<int> fron_list;
    list<int> act_list;
    list<int> act_dist2_list;
    vector<bool> frozen(nqubits, 0);
    list<int> singlequbit_list;

    int loop_end = 0;
    
    //initialize for post processing --------------------------
    FinalCircuit.nodeset.clear(); 
    node_id = dgraph.nodeset.size(); //for add SWAP
    add_cnot_num = 0;
    add_swap_num = 0;
    add_bridge_num = 0;

    vector< pair< pair<int, int>, pair<int, double> > > MCPE_flag; // SWAP candidate, act_gate_id, cost
    int history_size = 2;

    do{

        bool complete_act_list = true;
        do{
            // (1-1) Update front and act list
            update_front_n_act_list(fron_list, act_list, frozen);
            // (1-2) Check direct act list
            complete_act_list = check_direct_act_list(act_list, singlequbit_list, frozen, dgraph);
            // (1-3) Sort act list
            act_list.sort();
        }while(complete_act_list);

        //(2) Make swap & bridge candi
            // 1. SWAP 
        vector< pair<pair<int, int>, int> > candi_list; // SWAP candidate, act_gate_id
        generate_candi_list(act_list, candi_list, dgraph);
            // 2. BRIDGE
        if(BRIDGE_MODE)
            update_act_dist2_list(act_dist2_list, act_list, dgraph);

        //(3) Calculate MCPE_cost
        vector< pair< pair<int, int>, pair<int, double> > > MCPE_test; // SWAP candidate, act_gate_id, cost
        for(auto kv : candi_list)
        {
            pair<int, int> SWAP_pair = kv.first;
            double cost = cal_MCPE(SWAP_pair, dgraph);
            MCPE_test.push_back(make_pair(SWAP_pair, make_pair(kv.second, cost)));
        }
       
        //(3) Solve connectivity constraint
        if(!act_list.empty())
        {
            pair<int, int> SWAP;
            int gateid;
            double max_cost;
            find_max_cost(SWAP, gateid, max_cost, MCPE_test, act_list, MCPE_flag);
            
            std::list<int>::iterator it;
            if(BRIDGE_MODE)
                it = std::find(act_dist2_list.begin(), act_dist2_list.end(), gateid);
            if(BRIDGE_MODE == 1 && it != act_dist2_list.end() && max_cost <= param_gamma*10) 
            {
                // push bridge                
                int control = dgraph.nodeset[gateid].control;
                int target  = dgraph.nodeset[gateid].target;
                Dlist[control].pop_front();
                Dlist[target ].pop_front();
                frozen[control] = false;
                frozen[target ] = false;
                
                add_bridge(layout_L[control], layout_L[target], FinalCircuit);
                
                #if Dlist_all_mode
                Dlist_all[control].pop_front();
                Dlist_all[target ].pop_front();
                #endif
            
                act_list.erase( remove(act_list.begin(), act_list.end(), gateid) );
            }
            else
            {
                if(MCPE_flag.size() > history_size)
                    MCPE_flag.erase(MCPE_flag.begin());
                MCPE_flag.push_back(MCPE_test[0]);
                
                //cout << "we swap " << SWAP << endl;
                int f_SWAPqubit = qubit_Q[SWAP.first];
                int s_SWAPqubit = qubit_Q[SWAP.second];
                add_swap(f_SWAPqubit,    s_SWAPqubit, FinalCircuit);
                layout_swap(f_SWAPqubit, s_SWAPqubit);
            
            }
        }
        loop_end = 0; 
        for(int q=0; q<nqubits; q++)
            if(Dlist[q].empty()) loop_end++;

#if Dlist_all_mode
        if(loop_end==nqubits)
        {
            for(int q=0; q<nqubits; q++)
                for(auto& id : Dlist_all[q])
                    singlequbit_list.push_back(id);
            singlequbit_list.sort();
            for(auto& id : singlequbit_list)
                FinalCircuit.nodeset.push_back(dgraph.nodeset[id]);
        }
#endif
    }while( true && (!fron_list.empty()|| loop_end!=nqubits) );


    // (2) Mapping end ////////////////////////////
    if(BRIDGE_MODE && !postpro)
    {
        if(least_cnot_num < add_cnot_num)
        {
            layout_L = new_layout_L;
            qubit_Q  = new_qubit_Q;
        }
    }
    
    //mapping update
    new_layout_L = layout_L;
    new_qubit_Q  = qubit_Q;

    //for postprocessing
    if(forward)
    {
        if(least_cnot_num > add_cnot_num)
        {
            least_cnot_num = add_cnot_num;    
            least_swap_num = add_swap_num;
            least_bridge_num = add_bridge_num;
            Best_FinalCircuit.nodeset = FinalCircuit.nodeset;
        }
    }
}

void Qcircuit::QMapper::update_front_n_act_list(list<int>& front_list, list<int>& act_list, vector<bool>& frozen)
{
    for(int q = 0; q < nqubits; q++)
    {
        list<int>& Dlist_line = Dlist[q];
        if(!frozen[q])
        {
            if(Dlist_line.empty()) continue;
            int gateid = Dlist_line.front();
            
            if( find(front_list.begin(), front_list.end(), gateid) != front_list.end() )
            {
                front_list.erase( remove(front_list.begin(), front_list.end(), gateid));
                act_list.push_back(gateid);
            }
            else
                front_list.push_back(gateid);
            frozen[q] = true;
        }
    }
}

void Qcircuit::QMapper::find_singlequbit_list(int gateid, list<int>& singlequbit_list, Circuit& dgraph)
{
    singlequbit_list.clear();
    int control = dgraph.nodeset[gateid].control;
    int target  = dgraph.nodeset[gateid].target;

    int erase_control = 0;
    int erase_target = 0;
    
    //for control //
    for(auto& id : Dlist_all[control])
    {
        if(id == gateid) {
            erase_control++;
            break;
        }
        else {
            singlequbit_list.push_back(id);
            erase_control++;
        }
    }
    for(int i=0; i<erase_control; i++) Dlist_all[control].pop_front();

    //for target //
    for(auto& id : Dlist_all[target])
    {
        if(id == gateid) {
            erase_target++;
            break;
        }
        else {
            singlequbit_list.push_back(id);
            erase_target++;
        }
    }
    for(int i=0; i<erase_target; i++) Dlist_all[target].pop_front();
    
    singlequbit_list.sort();
}

void Qcircuit::QMapper::update_act_dist2_list(list<int>& act_dist2_list, list<int>& act_list, Circuit& dgraph)
{
    act_dist2_list.clear();
    for(auto& gateid : act_list)
    {
        int control = dgraph.nodeset[gateid].control;
        int target  = dgraph.nodeset[gateid].target;
        int Q_control = layout_L[control];
        int Q_target  = layout_L[target];
        if(coupling_graph.dist[Q_control][Q_target] == 2)
            act_dist2_list.push_back(gateid);
    }
}

bool Qcircuit::QMapper::check_direct_act_list(list<int>& act_list, list<int>& singlequbit_list, vector<bool>& frozen, Circuit& dgraph)
{
    bool complete_act_list = false;
    vector<int> act_list_erase;
    for(auto& gateid : act_list)
    {
        int control = dgraph.nodeset[gateid].control;
        int target  = dgraph.nodeset[gateid].target;
        int Q_control = layout_L[control];
        int Q_target  = layout_L[target];
        //CNOT
        if(coupling_graph.dist[Q_control][Q_target] == 1)
        {
            act_list_erase.push_back(gateid);
            Dlist[control].pop_front();
            Dlist[target ].pop_front();
            frozen[control] = false;
            frozen[target ] = false;

            #if Dlist_all_mode
            find_singlequbit_list(gateid, singlequbit_list, dgraph);
            for(auto& id : singlequbit_list)
                FinalCircuit.nodeset.push_back(dgraph.nodeset[id]);
            #endif
            
            FinalCircuit.nodeset.push_back(dgraph.nodeset[gateid]);
            complete_act_list = true;
        }
    }
    for(auto gateid : act_list_erase)
        act_list.erase( remove(act_list.begin(), act_list.end(), gateid) );

    return complete_act_list;
}

void Qcircuit::QMapper::generate_candi_list(list<int>& act_list, vector< pair<pair<int, int>, int> >& candi_list, Circuit& dgraph)
{
    for(auto& gateid : act_list)
    {
        int control = dgraph.nodeset[gateid].control;
        int target  = dgraph.nodeset[gateid].target;
        int Q_control = layout_L[control];
        int Q_target  = layout_L[target];
        for(int i = 0; i < coupling_graph.node_size; i++)
        {
            if(coupling_graph.dist[Q_control][i] == 1)
            {
                if(cal_SWAP_effect(control, target, i, Q_control) <= 0) continue;
                //cout << "SWAP(" << setw(2) << Q_control << "," << setw(2) << i << ")'s effect: " << cal_SWAP_effect(control, target, i, Q_control) << endl;
                (i < Q_control) ? candi_list.push_back(make_pair(make_pair(i, Q_control), gateid)) : 
                                  candi_list.push_back(make_pair(make_pair(Q_control, i), gateid));
            }
            if(coupling_graph.dist[Q_target][i] == 1)
            {
                if(cal_SWAP_effect(control, target, i, Q_target ) <= 0) continue;
                //cout << "SWAP(" << setw(2) << Q_target << "," << setw(2) << i << ")'s effect: " << cal_SWAP_effect(control, target, i, Q_target) << endl;
                (i < Q_target)  ? candi_list.push_back(make_pair(make_pair(i, Q_target),  gateid)) : 
                                  candi_list.push_back(make_pair(make_pair(Q_target, i),  gateid));
            }
        }
    }
}
