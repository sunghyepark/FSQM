//graphFunction.cpp
#include "circuit.h"

using namespace std;
using namespace Qcircuit;

void Qcircuit::Graph::build_dist_table()
{
    int idx = 0;
    for(auto& kv : nodeset)
        nodeid[idx++] = kv.first;
    node_size = nodeset.size();

    dist = new int*[node_size];
    for(int i = 0; i < node_size; i++)
        dist[i] = new int[node_size];

    for(int i = 0; i < node_size; i++)
        for(int j = 0; j < node_size; j++)
            if(i < j)
                dist[i][j] = bfs(i, j);
            else if(i == j)
                dist[i][j] = 0;
            else
                dist[i][j] = dist[j][i];
}

void Qcircuit::Graph::print_dist_table()
{
    cout << endl << " # Distance table " << endl;
    cout << "i↓  j→";
    for(int i=0; i<node_size; i++)
        cout << setw(4) << i;
    cout << endl;
    for(int i=0; i<node_size; i++)
    {
        cout << setw(6) << i;
        for(int j=0; j<node_size; j++)
            cout << setw(4) << dist[i][j];
        cout << endl;
    }
    cout << endl;            
}

void Qcircuit::Graph::delete_dist_table()
{
    for(int i=0; i<node_size; i++)
        delete[] dist[i];
    delete[] dist;
}

int Qcircuit::Graph::bfs(int start, int end)
{
    int return_val = -1;
    
    // table index
    vector<bool> isvisited(node_size, false);
    vector<int>  dist(node_size, 0);
    queue<int> queue;
    queue.push(start);
    isvisited[start] = true;

    while(!queue.empty())
    {
        int table_idx = queue.front();
        queue.pop();
        
        if(table_idx == end && isvisited[table_idx])
        {
            return_val = dist[table_idx];
            break;
        }

        //find adjacent node of table_idx node
        int g_id = nodeid[table_idx];
        Node& n = nodeset[g_id];
        for(auto& i : n.edges)
        {
            Edge& e = edgeset[i];
            int s = e.source->getid();
            int t = e.target->getid();
            int id = (s == g_id) ? t : s;         
            
            //adjacent node id (table_idx)
            int t_id = nodeid.find(id)->first;
            
            if(isvisited[t_id]) continue;

            isvisited[t_id] = true;
            dist[t_id] = dist[table_idx] + 1;
            queue.push(t_id);
        }
    }
    return return_val;
}

int Qcircuit::Graph::bfs_weight(int start, int end)
{
    int return_val = -1;
    
    vector<bool> isvisited(node_size, false);
    vector<int>  dist(node_size, 0);
    vector<int>  weight(node_size, 0);
    queue<int> queue;
    queue.push(start);
    isvisited[start] = true;

    while(!queue.empty())
    {
        int table_idx = queue.front();
        queue.pop();
        
        if(table_idx == end && isvisited[table_idx])
        {
            return_val = dist[table_idx];
            return_val = weight[table_idx];
            break;
        }

        //find adjacent node of table_idx node
        int g_id = nodeid[table_idx];
        Node& n = nodeset[g_id];
        for(auto& i : n.edges)
        {
            Edge& e = edgeset[i];
            int s = e.source->getid();
            int t = e.target->getid();
            int w = e.getweight();
            int id = (s == g_id) ? t : s;         
            
            //adjacent node id (table_idx)
            int t_id = nodeid.find(id)->first;
            
            if(isvisited[t_id])
            {
                if(dist[t_id] < dist[table_idx] + 1) continue;
                if(weight[t_id] < weight[table_idx] + w) continue;
                dist[t_id] = dist[table_idx] + 1;
                weight[t_id] = weight[table_idx] + w;
                continue;
            }

            isvisited[t_id] = true;
            dist[t_id] = dist[table_idx] + 1;
            weight[t_id] = weight[table_idx] + w;
            queue.push(t_id);
        }
    }
    return return_val;
}

int Qcircuit::Graph::cal_graph_center()
{
    int min_value = INT_MAX;
    int min_index = -1;
    for(int i = 0; i < node_size; i++)
    {
        int sum = 0;
        for(int j = 0; j < node_size; j++)
            sum += dist[i][j];
        if(min_value > sum)
        {   
            min_value = sum;
            min_index = i;
        }
    }
    return nodeid[min_index];
}

bool vector_equal(const vector<int>& v)
{
    const auto& first = v[0];
    for (const auto& c : v)
        if (first != c)
            return false;
    return true;
}

pair<int, bool> Qcircuit::Graph::graph_characteristics(bool i)
{
    int nqubits = nodeset.size();
    int idx_max, idx_min;

    vector<int> edge_weight(nqubits, 0);
    vector<int> edge_degree(nqubits, 0);
    for(auto edge : edgeset)
    {
        const int weight    = edge.second.getweight();
        const int sourceid  = edge.second.getsourceid();
        const int targetid  = edge.second.gettargetid();

        edge_weight[sourceid] += weight;
        edge_weight[targetid] += weight;
        
        edge_degree[sourceid] += 1;
        edge_degree[targetid] += 1;
    }
    bool bool_vector_equal = vector_equal(edge_weight);
    idx_max = max_element(edge_weight.begin(), edge_weight.end()) - edge_weight.begin();
    idx_min = min_element(edge_weight.begin(), edge_weight.end()) - edge_weight.begin();
    
    int idx = i==true ? idx_min : idx_max;
    return make_pair(idx, bool_vector_equal);
}


