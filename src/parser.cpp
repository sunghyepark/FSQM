//parser.cpp

#include "circuit.h"
#include "QASMtoken.hpp"
#include "QASMscanner.hpp"
#include "QASMparser.h"

#define PRINT_OPTION 1

using namespace std;
using namespace Qcircuit;

void Qcircuit::QMapper::parsing(int argc, char** argv, bool postpro)
{
    if(argc != 7)
    {
        cout << "ERROR: invalid arguments" << endl;
        exit(1);
    }

    fileName_input   = argv[1];
    fileName_output  = argv[2];
    fileName_reverse = argv[3];

    string alpha = argv[4];
    string beta  = argv[5];
    string gamma = argv[6];
    param_alpha = stod(alpha);
    param_beta  = stoi(beta);
    param_gamma = stoi(gamma);

#if PRINT_OPTION
    cout << "*** Option ***" << endl;
    cout << "Input file   : "  << fileName_input << endl;
    cout << "Output file  : "  << fileName_output << endl;
    cout << "Reverse file : "  << fileName_reverse << endl;
#endif

    QASM_parse(fileName_input);
    if(postpro)
    {
        //make_reverseCircuit(fileName_input, fileName_reverse);
        QASM_parse(fileName_reverse, true);
    }
}

void Qcircuit::QMapper::QASM_parse(string filename, bool isreversed)
{
    Circuit* circuit;
    if(!isreversed)
        circuit = &Dgraph;
    else
        circuit = &Dgraph_reverse;

    QASMparser* parser = new QASMparser(filename);
    parser->Parse();
    nqubits = parser->getNqubits();
    circuit->nodeset = parser->getGatelists();

    delete parser;
}

void Qcircuit::QMapper::make_reverseCircuit(string input, string output)
{
    ifstream in(input);
    ofstream out(output);
    out << "OPENQASM 2.0;" << endl;
    out << "include \"qelib1.inc\";" << endl;
    out << "qreg q[16];" << endl;
    out << "creg q[16];";
    
    reverse(in, out);
}

void Qcircuit::QMapper::reverse(istream& in, ostream& out)
{
    vector<string> str;
    while(!in.eof() && !in.fail())
    {
        str.push_back("");
        getline(in, str.back());
    }
    std::copy(str.rbegin(), str.rend()-4, ostream_iterator<string> (out, "\n"));
}
