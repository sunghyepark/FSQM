/*----------------------------------------------------------------------------------------------*/
/* Description:     Core data structure for Quantum compiler                                    */
/*                                                                                              */
/* Author:          Sunghye Park, Postech                                                       */
/*                  Minhyuk Kweon, Postech                                                      */
/*                                                                                              */
/* Created:         02/10/2020 (for Quantum compiler)                                           */
/*----------------------------------------------------------------------------------------------*/

#ifndef __CIRCUIT__
#define __CIRCUIT__ 0
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>
#include <map>
#include <queue>
#include <list>
#include <set>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <cfloat>
#include <climits>
#include <algorithm>
#include <iterator>
#include <utility>

using namespace std;

namespace Qcircuit
{
    /////initial mapping
    enum class INITIALTYPE
    {
        IDENTITY_MAPPING,
        RANDOM_MAPPING,
        GRAPH_MATCHING_MIX_UPDATE,
    };

    /////coupling graph
    enum class ARCHITECTURE
    {
        IBM_Q_20,
    };

    enum class GATETYPE
    {
        U, RX, RZ, CNOT, X, Y, Z, H, S, SDG, T, TDG
    };

    ///Circuit info (node, edge) 
    struct Gate
    {
        int id; 
        int control, target;                //logical qubits //if single qubit control = -1
        GATETYPE type;
            
        //// use just in QASM // double theta, double phi, double lambda //
        char output_type[128];
    };
  
    struct Circuit
    {
        vector<Gate> nodeset;   /* gate set */
    };

    struct Node;
    struct Edge;

    struct Node
    {
        private:
        ////variables
        static int global_id;
        int id;
        int weight;
        public:
        set<int> edges;

        public:
        ////constructors
        Node();
        Node(int weight);

        ////getters
        int getglobalid();
        int getid();
        int getweight();
        void setweight(int weight);

        static void init_global_id();
        
    };

    struct Edge
    {
        private:
        ////variables
        static int global_id;
        int id;
        int weight;
        public:
        Node* source;
        Node* target;

        public:
        ////constructors
        Edge();
        Edge(Node* source, Node* target, int weight);

        ////getters
        int getglobalid();
        int getid();
        int getweight();
        int getsourceid();
        int gettargetid();
        void setweight(int weight);
        void setsource(Node* source);
        void settarget(Node* target);

        static void init_global_id();

    };

    struct Graph
    {
        ////graph node and edge
        map<int, Node> nodeset;
        map<int, Edge> edgeset;
        
        map<int, int> nodeid;       //table index <--> graph node id
        int node_size;
        int** dist;
        int center;

        public:
        ////constructors
        Graph();

        ////methods
            //graph.cpp
        void init_global_id();
        int  addnode(int weight = 1);
        bool deletenode(int id);
        bool addedge(int source, int target, int weight = 1);
        bool deleteedge(int id);

            //graphFunction.cpp
        void build_dist_table();
        void print_dist_table();
        void delete_dist_table();
        int bfs(int start, int end);
        int bfs_weight(int start, int end);
        int cal_graph_center();
        pair<int, bool> graph_characteristics(bool i);
    };

    class QMapper
    {
        public:
        /////////////////////////// variables
            //file names
            string fileName_input;
            string fileName_output;
            string fileName_reverse;

            //cost parameters
            double param_alpha;
            int param_beta;
            int param_gamma;

            //quantity of logical, physical qubits
            unsigned int nqubits;   //qubit quantity of quantum circuit (logical  qubits)
            unsigned int positions; //qubit quantity of couping graph   (physical qubits)
            
            //Quantum circuit
            Circuit Dgraph;
            Circuit Dgraph_reverse;
            Circuit Dgraph_cnot;
            
            //Coupling graph
            Graph coupling_graph;

            //Dlist
            vector<list<int> > Dlist;
            vector<list<int> > Dlist_all;

            //mapping
            map<int, int> layout_L;  //L[logical_qubit] = physical_qubit
            map<int, int> qubit_Q;   //Q[physical_qubit] = logical_qubit

            ////for post-processing
            map<int, int> new_layout_L;
            map<int, int> new_qubit_Q;
            
            //Final quantum circuit
            Circuit FinalCircuit; //final quantum circuit
            Circuit Best_FinalCircuit; //best final quantum circuit
            
            //from mapping.cpp
            int node_id;
            int add_cnot_num;
            int add_swap_num;
            int add_bridge_num;
            int least_cnot_num;     //least cnot_num
            int least_swap_num;     //least swap_num
            int least_bridge_num;   //least bridge_num

            //print.cpp
            void nodeset_out(ofstream &of, Circuit graph);
            void node_print(int N, Circuit graph);

        /////////////////////////// functions
            //parser.cpp
            void parsing(int argc, char** argv, bool postpro = false);
            void QASM_parse(string filename, bool isreversed = false);
            void make_reverseCircuit(string input, string output);      //for post-processing
            void reverse(istream& in, ostream& out);                    //for post-processing

            //couplinggraph.cpp
            void select_coupling_graph(ARCHITECTURE archi);
            void build_graph_IBM_Q_20();            

            //initial_mapping.cpp
            void initial_mapping(ARCHITECTURE archi, bool prepro, bool postpro, bool BRIDGE_MODE, INITIALTYPE initial_type);
            void graph_matching_processing(ARCHITECTURE archi); 
            void identical_mapping();
            void random_mapping();
            void pre_processing(ARCHITECTURE archi);
            
            void print_layout(const map<int, int> &layout);
            void print_qubit(const map<int, int> &qubit);
            void print_layout();
            void print_qubit();

            //mappingPreparation.cpp 
            int sort_degree_return(vector<int>& candi_loc, Graph& graph);
            void sort_degree(vector<int>& candi_loc, Graph& graph);
            
            void make_Dlist(Circuit& dgraph);
            void make_Dlist_all(Circuit& dgraph);
            void make_CNOT(bool i);
            Graph make_interactionGraph(bool i);
            Graph make_interactionNumberGraph(bool i);
            Graph make_interactionMixgraph(bool i, int n);
            
            void bfs_queue_gen(queue<int>& queue, Graph& graph, int start, bool order);
            
            void make_ref_loc(vector<int>& ref_loc, Graph& graph, int start, bool order);
            void make_candi_loc(int current_q, Graph& graph, vector<int>& candi_loc_1, vector<int>& candi_loc_2, int& degree);
            void make_candi_loc_dist(int current_qc, vector<int>& candi_loc, vector<int>& ref_loc, Graph& coupling_graph, Graph& interaction_graph, bool equal_order);

            //mapping.cpp
            void Circuit_Mapping(Circuit& dgraph, ARCHITECTURE archi, bool forward = false, bool prepro = false, bool postpro = false, bool BRIDGE_MODE=false);
            void update_front_n_act_list(list<int>& front_list, list<int>& act_list, vector<bool>& frozen);
            void find_singlequbit_list(int gateid, list<int>& singlequbit_list, Circuit& dgraph);
            void update_act_dist2_list(list<int>& act_dist2_list, list<int>& act_list, Circuit& dgraph);
            bool check_direct_act_list(list<int>& act_list, list<int>& singlequbit_list, vector<bool>& frozen, Circuit& dgraph);
            void generate_candi_list(list<int>& act_list, vector< pair< pair<int, int>, int> >& candi_list, Circuit& dgraph);

            //mappingFunction.cpp
            int cal_SWAP_effect(const int q1, const int q2, const int Q1, const int Q2);
            double cal_MCPE(const pair<int, int> p, Circuit& dgraph);
            void find_max_cost(pair<int, int>& SWAP, int& gateid, double& max_cost,
                    vector< pair< pair<int, int>, pair<int, double> > >& v, list<int>& act_list,
                    vector< pair< pair<int, int>, pair<int, double> > >& history);

            void layout_swap(const int b1, const int b2);
            void add_cnot(int c_qubit, int t_qubit, Circuit& graph);
            void add_swap(int b1, int b2, Circuit& graph);
            void add_bridge(int qs, int qt, Circuit& graph);

            //outputwriter.cpp
            void FinalCircuit_info(Circuit& graph, bool final_circuit = false);
            void write_output(Circuit& graph);
    };
}

//----------------------------- for debug ----------------------------//

using namespace Qcircuit;

inline ostream& operator << (ostream& os, Node n)
{
    os << "Node[" << setw(3) << n.getid() << "] | weight: " << setw(2) << n.getweight();
    if(!n.edges.empty())
    {
        os << " | edge ids: ";
        for(auto e : n.edges)
            os << setw(3) << e << " ";
    }
    return os;
}

inline ostream& operator << (ostream& os, Edge e)
{
    os << "Edge[" << setw(3) << e.getid() << "] | weight: " << setw(2) << e.getweight();
    os << " | " << setw(3) << e.getsourceid() << " <--> " << setw(3) << e.gettargetid();
    return os;
}

inline ostream& operator << (ostream& os, map<int, Node> nodeset)
{
    for(auto& kv : nodeset)
        os << kv.second << endl;
    return os;
}

inline ostream& operator << (ostream& os, map<int, Edge> edgeset)
{
    for(auto& kv : edgeset)
        os << kv.second << endl;
    return os;
}

inline ostream& operator << (ostream& os, Graph graph)
{
    os << "||Graph||" << endl;
    os << "||Node Set||" << endl;
    os << graph.nodeset << endl;
    os << "||Edge Set||" << endl;
    os << graph.edgeset << endl;
    return os;
}

inline ostream& operator << (ostream& os, const GATETYPE& type)
{
    switch(type)
    {
        case GATETYPE::U    : return os << "u"  ;
        case GATETYPE::RX   : return os << "rx" ;
        case GATETYPE::RZ   : return os << "rz" ;
        case GATETYPE::CNOT : return os << "cx" ;
        case GATETYPE::X    : return os << "x"  ;
        case GATETYPE::Y    : return os << "y"  ;
        case GATETYPE::Z    : return os << "z"  ;
        case GATETYPE::H    : return os << "h"  ;
        case GATETYPE::S    : return os << "s"  ;
        case GATETYPE::SDG  : return os << "sdg";
        case GATETYPE::T    : return os << "t"  ;
        case GATETYPE::TDG  : return os << "tdg";
        default             : return os;
    }
}

inline ostream& operator << (ostream& os, const Gate& gate)
{
    os << "Gate";
    if(gate.control == -1) //U, RX, RZ gate
        os << "(" << setw(5) << gate.id << ")\t" << setw(2) << gate.type << " " << "q[" << gate.target << "]";
    else //CNOT gate
        os << "(" << setw(5) << gate.id << ")\t" << setw(2) << gate.type << " " << "q[" << gate.control << "], q[" << gate.target << "]";
    return os;   
}

inline ostream& operator << (ostream& os, const vector<Gate>& nodeset)
{
    for(auto gate : nodeset)
        os << gate << endl;
    return os;
}

inline ostream& operator << (ostream& os, const queue<int> q)
{
    queue<int> test = q;
    while(!test.empty())
    {
        os << test.front() << " ";
        test.pop();
    }
    return os;
}

inline ostream& operator << (ostream& os, const pair<int, int> p)
{
    return os << "pair: " << p.first << " " << p.second;
}

inline ostream& operator << (ostream& os, const vector< pair< pair<int, int>, int> > v)
{
    for(auto kv : v)
        os << "SWAP: " << kv.first.first << " " << kv.first.second << " act_gate_id: " << kv.second << endl;
    return os;
}

inline ostream& operator << (ostream& os, const vector< pair< pair<int, int>, pair<int, int>> > v)
{
    for(auto kv : v)
    {
        os << "SWAP: " << kv.first.first << " " << kv.first.second;
        os << " act_gate_id: " << kv.second.first << " MCPE cost: " << kv.second.second << endl;
    }
    return os;
}

inline ostream& operator << (ostream& os, const vector< pair< pair<int, int>, pair<int, double>> > v)
{
    for(auto kv : v)
    {
        os << "SWAP: " << kv.first.first << " " << kv.first.second;
        os << " act_gate_id: " << kv.second.first << " MCPE cost: " << kv.second.second << endl;
    }
    return os;
}

inline ostream& operator << (ostream& os, const vector<list<int>>& list)
{
    for(int n=0; n<list.size(); n++)
    {
        os << "n[" << setw(4) << n << "]: " << "size: " << list[n].size() << " ";
        for(auto ll : list[n])
        {
            os << "-> " << ll; 
        }
        os << endl;
    }
    return os;
}

inline ostream& operator << (ostream& os, const map<int, vector<int>>& edgemap)
{
    for(auto it : edgemap)
    {
        vector<int> ids = it.second;
        os << "Gate[" << setw(5) << it.first << "]: ";
        for(auto id : ids)
        {
            os << setw(5) << id << " ";
        }
        os << endl;
    }
    return os;
}

inline ostream& operator << (ostream& os, const vector<int>& list)
{
    for(auto v : list)
        os << v << " ";
    return os;
}

inline ostream& operator << (ostream& os, const vector<bool>& list)
{
    for(auto v : list)
        os << v << " ";
    return os;
}

inline ostream& operator << (ostream& os, const map<int, int>& m)
{
    for(auto kv : m)
        os << "map[" << setw(3) << kv.first << "] = " << kv.second << endl;
    return os;
}

inline ostream& operator << (ostream& os, const list<int>& llist)
{
    for(auto v : llist)
        os << v << " ";
    return os;
}

inline ostream& operator << (ostream& os, const vector<pair<int, int> >& llist)
{
    os << "{ ";
    for(auto v : llist)
        os << "(" << v.first << ", " << v.second << ")  ";
    os << "}" << endl;
    return os;
}

inline ostream& operator << (ostream& os, const vector<vector<int>>& list)
{
    for(auto v : list)
        os << v << endl;
    return os;
}

#endif
