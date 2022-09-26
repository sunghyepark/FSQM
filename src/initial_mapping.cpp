//initial_mapping.cpp
#include "circuit.h"
#include <random>

using namespace std;
using namespace Qcircuit;

void Qcircuit::QMapper::initial_mapping(ARCHITECTURE archi, bool prepro, bool postpro, bool BRIDGE_MODE, INITIALTYPE initial_type)
{
    //initial mapping
    if(!prepro)
    {
        for(int i=0; i<positions; i++)
        {
            layout_L[i] = i; //logical -> physical
            qubit_Q[i] = i;  //physical -> logical
        }
        return;
    }

    //1st mapping preparation
    if(!postpro && !BRIDGE_MODE)
    {
        select_coupling_graph(archi);
        coupling_graph.build_dist_table();
    }

    //post-processing
    if(postpro) 
    {
        layout_L = new_layout_L;
        qubit_Q = new_qubit_Q;
        return;
    }
    
    //Initial mapping mode?
    int q_num;
    switch(initial_type)
    {
        case INITIALTYPE::IDENTITY_MAPPING:
            identical_mapping();
            break;
        case INITIALTYPE::RANDOM_MAPPING:
            random_mapping();
            break;
        case INITIALTYPE::GRAPH_MATCHING_MIX_UPDATE:
            graph_matching_processing(archi); 
            q_num = nqubits;
            for(int i=0; i<positions; i++)
            {
                if(qubit_Q[i] == -1)
                {
                    qubit_Q[i] = q_num;
                    layout_L[q_num] = i;
                    q_num++;
                }
            }
            break;
    }
    //print_qubit(qubit_Q); 
    //print_layout(layout_L);
}

void Qcircuit::QMapper::graph_matching_processing(ARCHITECTURE archi) 
{
    //initiallize layout -> -1
    for(int i=0; i<nqubits; i++)
        layout_L[i] = -1; //logical -> physical
    for(int i=0; i<positions; i++)
        qubit_Q[i] = -1;  //physical -> logical

    //CNOT set
    make_CNOT(true);
    
    //Make interaction graph
    int CNOTNodesetSize = Dgraph_cnot.nodeset.size();
    int NodesetSize     = Dgraph.nodeset.size();
    int n = CNOTNodesetSize/param_beta;
    if (nqubits<=10 || nqubits==20) 
        n = 1;
    if ( (nqubits==15 && NodesetSize<=5000) || nqubits==11)
        n = param_beta;

    Graph interactionGraph;
    interactionGraph = make_interactionMixgraph(true, n);
    interactionGraph.build_dist_table();

    Graph interaction_graph_order     = make_interactionGraph(true);
    Graph interaction_graph_number    = make_interactionNumberGraph(true);
    interaction_graph_order.build_dist_table();
    interaction_graph_number.build_dist_table();
    
    pair<int, bool> graph_charact = interaction_graph_number.graph_characteristics(false);
    int idx_num   = graph_charact.first;
    int equal_num = graph_charact.second;
    
    //    *** GRAPH Matching Processing Start  ***
    int qc = interaction_graph_number.cal_graph_center();
    int Qc = coupling_graph.cal_graph_center();
    
    queue<int> igraph_qc_queue, igraph_bfs_queue;
    int graph_dist, graph_node;
    bfs_queue_gen(igraph_qc_queue, interactionGraph, qc, false);
    bfs_queue_gen(igraph_bfs_queue, interactionGraph, qc, false);

    vector<int> candi_loc1, candi_loc2;
    int location_degree;
    bool first_loop = true;

    while(!igraph_qc_queue.empty())
    {
        int current_qc = igraph_qc_queue.front();
        bool dist_1_mapping = false;

        // (1) qc mapping ----------------------------------------------------------
        if(first_loop)
        {
            qubit_Q[Qc] = qc;
            layout_L[qc] = Qc;
            dist_1_mapping = true;
        }
        else
        {
            if(layout_L[current_qc]!=-1 && equal_num!=true)
                dist_1_mapping = true;
            else if(layout_L[current_qc]!=-1 && equal_num== true)
                dist_1_mapping = false;
                //new center --> mapping
            else
            {
                vector<int> ref_loc;
                make_ref_loc(ref_loc, interaction_graph_number, current_qc, false);
                make_candi_loc_dist(current_qc, candi_loc1, ref_loc, coupling_graph, interaction_graph_number, false);
                
                sort_degree(candi_loc1, coupling_graph);
                Qc = candi_loc1[0];
                qubit_Q[Qc] = current_qc;
                layout_L[current_qc] = Qc;

                dist_1_mapping = true;
            }

            // generate new bfs_queue
            while(!igraph_bfs_queue.empty())
                igraph_bfs_queue.pop();
            bfs_queue_gen(igraph_bfs_queue, interaction_graph_number, current_qc, equal_num);
        }

        // (2) dist_1 set ---------------------------------------------------------
        if(dist_1_mapping)
        {
            //Location to center --> temp_set1, temp_set2
            vector<int> temp_set1, temp_set2;
            vector<pair<int, int>> dist_temp_set;
            bool first_bfs = true; 
            
            //candi_location update (disjoint set) --> candi_loc1, candi_loc2
            candi_loc1.clear();
            candi_loc2.clear();
            make_candi_loc(layout_L[current_qc], coupling_graph, candi_loc1, candi_loc2, location_degree);
            
            while(!igraph_bfs_queue.empty())
            {
                int current_bfs = igraph_bfs_queue.front();
                int dist_from_qc = interaction_graph_number.dist[current_qc][igraph_bfs_queue.front()];
                    //qc==bfs
                if(current_qc == current_bfs)
                    igraph_bfs_queue.pop();
                    //has physical qubit
                else if(layout_L[current_bfs]!=-1)
                    igraph_bfs_queue.pop();
                    //not dist_1
                else if(dist_from_qc != 1)
                {
                    dist_temp_set.push_back(make_pair(dist_from_qc, current_bfs));
                    igraph_bfs_queue.pop();
                }
                    //only dist_1_set
                else
                {
                    if(!first_bfs)
                    {
                        if(interaction_graph_number.dist[temp_set1[0]][current_bfs] == 1)
                            temp_set1.push_back(current_bfs);
                        else
                            temp_set2.push_back(current_bfs);
                        igraph_bfs_queue.pop();
                    }
                    else
                    {
                        temp_set1.push_back(igraph_bfs_queue.front());
                        igraph_bfs_queue.pop();

                        first_bfs = false;
                    }
                }
            }

            //graph shape
            graph_dist = interaction_graph_order.dist[current_qc][igraph_bfs_queue.back()];
            graph_node = temp_set1.size() + temp_set2.size();
            
            //dist_1_set is empty
            if(temp_set1.size()==0 && temp_set2.size()==0 && graph_node >graph_dist)
            {
                for(auto dist_temp : dist_temp_set)
                {
                    if(dist_temp.first == dist_temp_set[0].first)
                        temp_set1.push_back(dist_temp.second);
                    else if(dist_temp.first == dist_temp_set[0].first+1)
                        temp_set2.push_back(dist_temp.second);
                    else
                        continue;
                }
            }

            //mapping temp_set-->candi_loc
            if(temp_set1.size() < temp_set2.size() )
                swap(temp_set1, temp_set2);
            int Q_location;

            //for candi_loc1    //////////////////////
            if(candi_loc1.size() >= temp_set1.size())
            {
                int Q_index = 0;
                for(auto q : temp_set1)
                {
                    int Q_location = candi_loc1[Q_index];
                    qubit_Q[Q_location] = q;
                    layout_L[q] = Q_location;
                    candi_loc1.erase(candi_loc1.begin());
                }
                temp_set1.clear();
            }
            else
            {
                int q_index = 0;
                for(auto Q : candi_loc1)
                {
                    int qq = temp_set1[q_index];
                    qubit_Q[Q] = qq;
                    layout_L[qq] = Q;
                    temp_set1.erase(temp_set1.begin());
                }
                candi_loc1.clear();
            }
            
            //for candi_loc2    //////////////////////
            if(candi_loc2.size() >= temp_set2.size())
            {
                int Q_index=0;
                for(auto q : temp_set2)
                {
                    int Q_location = candi_loc2[Q_index];
                    qubit_Q[Q_location] = q;
                    layout_L[q] = Q_location;
                    candi_loc2.erase(candi_loc2.begin());
                }
                temp_set2.clear();
            }
            else
            {
                int q_index = 0;
                for(auto Q : candi_loc2)
                {
                    int qq = temp_set2[q_index];
                    qubit_Q[Q] = qq;
                    layout_L[qq] = Q;
                    temp_set2.erase(temp_set2.begin());
                }
                candi_loc2.clear();
            }

            if(temp_set1.size() + temp_set2.size() >= 0 && Dgraph.nodeset.size()>=3500)
            {
                //candi & temp_set update!
                temp_set1.insert(temp_set1.end(), temp_set2.begin(), temp_set2.end());
                candi_loc1.insert(candi_loc1.end(), candi_loc2.begin(), candi_loc2.end());

                // Remained node
                if(candi_loc1.size() >= temp_set1.size())
                {
                    int Q_index = 0;
                    for(auto q : temp_set1)
                    {
                        int Q_location = candi_loc1[Q_index];
                        qubit_Q[Q_location] = q;
                        layout_L[q] = Q_location;
                        
                        candi_loc1.erase( remove(candi_loc1.begin(), candi_loc1.end(), Q_location) );
                    }
                    temp_set1.clear();

                }
                else
                {
                    int q_index = 0;
                    for(auto Q : candi_loc1)
                    {
                        int qq = temp_set1[q_index];
                        qubit_Q[Q] = qq;
                        layout_L[qq] = Q;
                        temp_set1.erase(temp_set1.begin());
                    }
                    candi_loc1.clear();
                }
            }
            queue<int> temp_bfs_queue2;

            for(auto v : temp_set1)
                temp_bfs_queue2.push(v);
            while(!igraph_bfs_queue.empty())
            {
                temp_bfs_queue2.push(igraph_bfs_queue.front());
                igraph_bfs_queue.pop();
            }
            igraph_bfs_queue = temp_bfs_queue2;
        }
        // (3) ready to next loop ------------------------------------------------
        first_loop = false;
        igraph_qc_queue.pop();
        dist_1_mapping = false;
    }
}

void Qcircuit::QMapper::identical_mapping()
{
    for(int i=0; i<positions; i++)
    {
        qubit_Q[i] = i;
        layout_L[i] = i;
    }
}

void Qcircuit::QMapper::random_mapping()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(0,19);
    
    int rand_num;
    int i=0;
    
    //initiallize layout -> -1
    for(int i=0; i<nqubits; i++)
        layout_L[i] = -1; //logical -> physical
    for(int i=0; i<positions; i++)
        qubit_Q[i] = -1;  //physical -> logical
    while(i<20)
    {
        rand_num = dis(gen);
        if(qubit_Q[rand_num]!=-1) continue;
        else
        {
            qubit_Q[rand_num] = i;
            layout_L[i] = rand_num;
            cout << "qubit: " << i << ", layout: " << rand_num << endl;
            i++;
        }
    }
}

void Qcircuit::QMapper::print_layout(const map<int, int> &layout)
{
    cout.setf(ios::left);
    for(auto& it : layout)
        cout << " q" << setw(2) << it.first << "\t->    Q" << setw(2) << it.second << endl;
    cout << endl;
}

void Qcircuit::QMapper::print_qubit(const map<int, int> &qubit)
{
    cout.setf(ios::left);
    for(auto& it : qubit)
        cout << " Q" << setw(2) << it.first << "\t<-    q" << setw(2) << it.second << endl; 
    cout << endl;
}

void Qcircuit::QMapper::print_layout()
{
    const map<int, int> &layout = layout_L;
    cout.setf(ios::left);
    for(auto& it : layout)
        cout << " q" << setw(2) << it.first << "\t->    Q" << setw(2) << it.second << endl;
    cout << endl;
}

void Qcircuit::QMapper::print_qubit()
{
    const map<int, int> &qubit = qubit_Q;
    cout.setf(ios::left);
    for(auto& it : qubit)
        cout << " Q" << setw(2) << it.first << "\t<-    q" << setw(2) << it.second << endl; 
    cout << endl;
}

