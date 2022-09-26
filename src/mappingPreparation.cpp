//mapping preparation.cpp
#include "circuit.h"

using namespace std;
using namespace Qcircuit;

bool compare_cost(pair<int, int> a, pair<int, int> b)
{
    return a.first < b.first;
}
bool compare_graph_bfs(pair< pair<int, int>, int> a, pair< pair<int, int>, int> b)
{
    if(a.first.first == b.first.first)
        return a.first.second < b.first.second;
    else
        return a.first.first < b.first.first;
}
bool compare_graph_bfs_number(pair< pair<int, int>, int> a, pair< pair<int, int>, int> b)
{
    if(a.first.first == b.first.first)
        return a.first.second > b.first.second;
    else
        return a.first.first < b.first.first;
}
bool compare(const pair<int, int>& a, const pair<int, int>& b)
{
    if(a.first == b.first)
        return a.second < b.second;
    return a.first > b.first;
}

int Qcircuit::QMapper::sort_degree_return(vector<int>& candi_loc, Graph& graph)
{
    vector<pair<int, int>> degree_candi_loc;
    vector<int> temp_candi_loc;
    int degree;
    int min_dist = 1;
    int return_degree = 0;

    //initial candi loc
    for(auto v : candi_loc)
    {
        degree = 0;
        for(int i = 0; i < graph.node_size; i++)
        {
            if(i == v) continue;
            if(graph.dist[v][i] > min_dist) continue;
            if(qubit_Q.count(i) && qubit_Q[i] == -1)
                degree++;
        }
        degree_candi_loc.push_back(make_pair(degree, v));
        return_degree += degree;
    }

    sort(degree_candi_loc.begin(), degree_candi_loc.end(), compare);
    for(auto v : degree_candi_loc)
        temp_candi_loc.push_back(v.second);
    candi_loc = temp_candi_loc;

    return return_degree;
}

void Qcircuit::QMapper::sort_degree(vector<int>& candi_loc, Graph& graph)
{
    vector<pair<int, int>> degree_candi_loc;
    vector<int> temp_candi_loc;
    int degree;
    int min_dist = 1;
    
    //initial candi loc
    for(auto v : candi_loc)
    {
        degree = 0;
        for(int i = 0; i < graph.node_size; i++)
        {
            if(i == v) continue;
            if(graph.dist[v][i] > min_dist) continue;
            if(qubit_Q.count(i) && qubit_Q[i] == -1)
                degree++;
        }
        degree_candi_loc.push_back(make_pair(degree, v));
    }

    sort(degree_candi_loc.begin(), degree_candi_loc.end(), compare);
    for(auto v : degree_candi_loc)
        temp_candi_loc.push_back(v.second);
    candi_loc = temp_candi_loc;
}

void Qcircuit::QMapper::make_Dlist(Circuit& dgraph)
{
    Dlist.clear(); 
    for(int i = 0; i<nqubits; i++)
    {
        list<int> temp;
        Dlist.push_back(temp);
    }

    for(const auto& gate : dgraph.nodeset)
    {
        if(gate.type != GATETYPE::CNOT) continue;   //only cnot --> dlist
        const int   id      = gate.id;
        const int   target  = gate.target;
        const int   control = gate.control;
        Dlist[target].push_back(id);
        Dlist[control].push_back(id);
    }
}

void Qcircuit::QMapper::make_Dlist_all(Circuit& dgraph)
{
    Dlist_all.clear(); 
    for(int i = 0; i<nqubits; i++)
    {
        list<int> temp;
        Dlist_all.push_back(temp);
    }

    for(const auto& gate : dgraph.nodeset)
    {
        if(gate.type == GATETYPE::CNOT)   // cnot --> dlist
        {
            const int   id      = gate.id;
            const int   target  = gate.target;
            const int   control = gate.control;

            Dlist_all[target].push_back(id);
            Dlist_all[control].push_back(id);
        }
        else   // single-qubit gate --> dlist
        {
            const int   id      = gate.id;
            const int   target  = gate.target;
            Dlist_all[target].push_back(id);
        }
    }
}

void Qcircuit::QMapper::make_CNOT(bool i)
{
    Dgraph_cnot.nodeset.clear();

    if(i)
    {
        for(const auto& gate : Dgraph.nodeset)
        {
            if(gate.type != GATETYPE::CNOT) continue;
            const int target  = gate.target;
            const int control = gate.control;
            add_cnot(control, target, Dgraph_cnot);
            add_cnot_num--;
        }
    }
}

Graph Qcircuit::QMapper::make_interactionGraph(bool i)
{
    bool** gen_edge = new bool*[nqubits]; //edge generated (for interaction graph)
    for(int i = 0; i < nqubits; i++)
        gen_edge[i] = new bool[nqubits];

    //gen_edge initialize
    for(int i = 0; i < nqubits; i++)
        for(int j = 0; j < nqubits; j++)
            if(i == j)
                gen_edge[i][j] = true;
            else
                gen_edge[i][j] = false;
    Graph g;
    for(int i = 0; i < nqubits; i++)
        g.addnode();
    for(const auto& gate : Dgraph_cnot.nodeset)
    {
        if(gate.type != GATETYPE::CNOT) continue;
        const int id = gate.id;
        const int target = gate.target;
        const int control = gate.control;
        if(gen_edge[target][control]) continue;
        g.addedge(target, control, id);
        gen_edge[target][control] = true;
        gen_edge[control][target] = true;
    }
    
    for(int i = 0; i < nqubits; i++)
        delete[] gen_edge[i];
    delete[] gen_edge;

    return g;
}

Graph Qcircuit::QMapper::make_interactionNumberGraph(bool i)
{
    bool** gen_edge = new bool*[nqubits]; //edge generated (for interaction graph)
    for(int i = 0; i < nqubits; i++)
        gen_edge[i] = new bool[nqubits];

    //gen_edge initialize
    for(int i = 0; i < nqubits; i++)
        for(int j = 0; j < nqubits; j++)
            if(i == j)
                gen_edge[i][j] = true;
            else
                gen_edge[i][j] = false;

    Graph g;
    for(int i = 0; i < nqubits; i++)
        g.addnode();
    for(const auto& gate : Dgraph_cnot.nodeset)
    {
        if(gate.type != GATETYPE::CNOT) continue;
        const int id = gate.id;
        const int target = gate.target;
        const int control = gate.control;
        if(gen_edge[target][control])
        {
            for(auto& e : g.edgeset)
            {
                if(e.second.gettargetid()==target && e.second.getsourceid()==control)
                {
                    int weight = e.second.getweight();
                    e.second.setweight(weight+1);
                }
                else if(e.second.gettargetid()==control && e.second.getsourceid()==target)
                {
                    int weight = e.second.getweight();
                    e.second.setweight(weight+1);
                }

                else continue;
            }
        }
        else
        g.addedge(target, control, 1);
        gen_edge[target][control] = true;
        gen_edge[control][target] = true;

    }
    
    for(int i = 0; i < nqubits; i++)
        delete[] gen_edge[i];
    delete[] gen_edge;

    return g;
}


Graph Qcircuit::QMapper::make_interactionMixgraph(bool i, int n)
{
    bool** gen_edge = new bool*[nqubits]; //edge generated (for interaction graph)
    for(int i = 0; i < nqubits; i++)
        gen_edge[i] = new bool[nqubits];

    //gen_edge initialize
    for(int i = 0; i < nqubits; i++)
        for(int j = 0; j < nqubits; j++)
            if(i == j)
                gen_edge[i][j] = true;
            else
                gen_edge[i][j] = false;

    Graph g;
    for(int i = 0; i < nqubits; i++)
        g.addnode();

    int dgraphSize = Dgraph_cnot.nodeset.size();
    int m = dgraphSize / n;
    int remainder = dgraphSize % n;
    
    vector<double> costWeightTable;
    double costParam = 0.9;
    double cost = 1.0;
    costWeightTable.clear();
    for(int i=0; i<n+1; i++)
    {
        costWeightTable.push_back(cost);
        cost *= costParam;
    }
    
    
    int for_i=0;
    int for_n=0;
    double weight_cost;
    for(const auto& gate : Dgraph_cnot.nodeset)
    {
        if(for_i==m)
        {
            for_i=0;
            for_n++;
        }
        weight_cost = costWeightTable[for_n];
        const int id = gate.id;
        const int target = gate.target;
        const int control = gate.control;
        if(gen_edge[target][control])
        {
            for(auto& e : g.edgeset)
            {
                if(e.second.gettargetid()==target && e.second.getsourceid()==control)
                {
                    int weight = e.second.getweight();
                    e.second.setweight(weight+weight_cost*10);
                }
                else if(e.second.gettargetid()==control && e.second.getsourceid()==target)
                {
                    int weight = e.second.getweight();
                    e.second.setweight(weight+weight_cost*10);
                }

                else continue;
            }
        }
        else
        g.addedge(target, control, weight_cost*10);
        gen_edge[target][control] = true;
        gen_edge[control][target] = true;

        for_i++;
    }

    for(int i = 0; i < nqubits; i++)
        delete[] gen_edge[i];
    delete[] gen_edge;

    return g;

}

void Qcircuit::QMapper::bfs_queue_gen(queue<int>& queue, Graph& graph, int start, bool order)
{
    vector< pair< pair<int, int>, int> >  bfs_sort_vector; // <dist, weight> , nodeid(table idx)
    for(int i=0; i<nqubits; i++)
    {
        if(layout_L[i]!=-1 && i!=start)
            continue;
        bfs_sort_vector.push_back(make_pair(make_pair(graph.dist[start][i], graph.bfs_weight(start, i)), i));
    }

    if(order) //order: small -> large
        sort(bfs_sort_vector.begin(), bfs_sort_vector.end(), compare_graph_bfs);
    else //number: large -> small
        sort(bfs_sort_vector.begin(), bfs_sort_vector.end(), compare_graph_bfs_number);
   
    queue.push(start);  //for graph matching 

    for(int i=0; i<bfs_sort_vector.size(); i++)
        queue.push(bfs_sort_vector[i].second);
    
    if(queue.front() == start) queue.pop();
}

void Qcircuit::QMapper::make_ref_loc(vector<int>& ref_loc, Graph& graph, int start, bool order)
{
    vector<pair<int, int>> temp; //edge weight, id
    //convert table idx to graph node id
    int g_id = graph.nodeid[start];
    Node& node = graph.nodeset[g_id];
    for(auto e_id : node.edges)
    {
        Edge& edge = graph.edgeset[e_id];
        int id = (edge.source->getid() == start) ? edge.target->getid() : edge.source->getid();
        if(layout_L[id] != -1) temp.push_back(make_pair(edge.getweight(), layout_L[id]));
    }
    sort(temp.begin(), temp.end(), compare_cost);
    if(!order)
        std::reverse(temp.begin(), temp.end());

    for(auto v : temp)
        ref_loc.push_back(v.second);
}

void Qcircuit::QMapper::make_candi_loc(int current_q, Graph& graph, vector<int>& candi_loc_1, vector<int>& candi_loc_2, int& degree)
{
    int start = current_q;
    set<int> initial_candi_loc;
    int min_dist = 1;

    do
    {
        //auto sorted by ascending order
        for(int i = 0; i < graph.node_size; i++)
        {
            if(i == start) continue;
            if(graph.dist[start][i] > min_dist) continue;
            if(qubit_Q.count(i) && qubit_Q[i] == -1)
                initial_candi_loc.insert(i);
        }
        min_dist++;
    } while(initial_candi_loc.empty());
    
    set<int>::iterator it;
    it = initial_candi_loc.begin();
    candi_loc_1.push_back(*it);
    initial_candi_loc.erase(it);
    
    // ** candi_loc ** // 
    for(auto val : initial_candi_loc)
    {
        if(graph.dist[*it][val]==1)
            candi_loc_1.push_back(val);
        else
            candi_loc_2.push_back(val);
    }

    int degree_1 = sort_degree_return(candi_loc_1, coupling_graph);
    int degree_2 = sort_degree_return(candi_loc_2, coupling_graph);
    
    if(degree_1 < degree_2)
        swap(candi_loc_1, candi_loc_2);

    degree = candi_loc_1.size() + candi_loc_2.size();
}


void Qcircuit::QMapper::make_candi_loc_dist(int current_qc, vector<int>& candi_loc, vector<int>& ref_loc, Graph& coupling_graph, Graph& interaction_graph, bool equal_order)
{
    vector<pair<int, int>> distance_candi_loc;
    vector<int> initial_candi_loc;

    //initial candi_loc
    for(int i = 0; i < coupling_graph.node_size; i++)
    {
        if(qubit_Q.count(i) && qubit_Q[i] == -1)
            initial_candi_loc.push_back(i);
    }

    //sub_num --> candi_loc
    if(equal_order)
    {
        for(auto r : ref_loc)
        {
            for(auto v : initial_candi_loc)
            {
                int dist = coupling_graph.dist[v][r];
                distance_candi_loc.push_back(make_pair(dist, v));
            }
            sort(distance_candi_loc.begin(), distance_candi_loc.end(), compare_cost);
            initial_candi_loc.clear();
            for(auto v : distance_candi_loc)
            {
                if(v.first == distance_candi_loc[0].first)
                    initial_candi_loc.push_back(v.second);
            }
            distance_candi_loc.clear();
        }
    }
    //dist_sum --> candi_loc
    else
    {
        for(auto v : initial_candi_loc)
        {
            int dist_sum = 0;
            int dist;
            for(auto r : ref_loc)
            {
                dist = coupling_graph.dist[v][r];
                dist_sum += dist;
            }
            distance_candi_loc.push_back(make_pair(dist_sum, v));
        }
        sort(distance_candi_loc.begin(), distance_candi_loc.end(), compare_cost);
        initial_candi_loc.clear();
        for(auto v : distance_candi_loc)
        {
            if(v.first == distance_candi_loc[0].first)
                initial_candi_loc.push_back(v.second);
        }
        distance_candi_loc.clear();
    }
    candi_loc = initial_candi_loc;
}
