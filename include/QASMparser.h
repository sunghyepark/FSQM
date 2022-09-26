/*----------------------------------------------------------------------------------------------*/
/* Description:     Header file for QASM parsing                                                */
/*                                                                                              */
/* Author:          Alwin Zulehner, Robert Wille                                                */
/*                                                                                              */
/* Revised by:      Sunghye Park, Postech                                                       */
/*                                                                                              */
/* Created:         02/10/2020 (for Quantum compiler)                                           */
/*----------------------------------------------------------------------------------------------*/
#ifndef QASM_SIMULATOR_H_
#define QASM_SIMULATOR_H_

#include "circuit.h"
#include "QASMscanner.hpp"
#include "QASMtoken.hpp"
#include <vector>
#include <set>


class QASMparser {
public:
	QASMparser(std::string fname);
	virtual ~QASMparser();

	void Parse();

    Qcircuit::Gate gate;
    map<int, bool> logi_qubits;
    
    /*get Gatelist*/ 
    std::vector<Qcircuit::Gate> getGatelists() {
        return gatelistsL;
    }

    int getNqubits() {
        return logi_qubits.size();
    }

    int getNgates() {
        return ngates;
    }

private:
	class Expr {
	public:
		enum class Kind {number, plus, minus, sign, times, sin, cos, tan, exp, ln, sqrt, div, power, id};
		double num;
		Kind kind;
		Expr* op1 = NULL;
		Expr* op2 = NULL;
		std::string id;

		Expr(Kind kind,  Expr* op1, Expr* op2, double num, std::string id) {
			this->kind = kind;
			this->op1 = op1;
			this->op2 = op2;
			this->num = num;
			this->id = id;
		}

		Expr(const Expr& expr) {
			this->kind = expr.kind;
			this->num = expr.num;
			this->id = expr.id;
			if(expr.op1 != NULL) {
				this->op1 = new Expr(*expr.op1);
			}
			if(expr.op2 != NULL) {
				this->op2 = new Expr(*expr.op2);
			}
		}

		~Expr() {
			if(op1 != NULL) {
				delete op1;
			}
			if(op2 != NULL) {
				delete op2;
			}
		}
	};

	class BasisGate {
	public:
		virtual ~BasisGate() = default;
	};

	class Ugate : public BasisGate {
	public:
		Expr* theta;
		Expr* phi;
		Expr* lambda;
		std::string target;

		Ugate(Expr* theta, Expr* phi, Expr* lambda, std::string target) {
			this->theta = theta;
			this->phi = phi;
			this->lambda = lambda;
			this->target = target;
		}

		~Ugate() {
			if(theta != NULL) {
				delete theta;
			}
			if(phi != NULL) {
				delete phi;
			}
			if(lambda != NULL) {
				delete lambda;

			}
		}
	};
///////////////////////////////////////////////////
    class Xgate : public BasisGate {
	public:
		std::string target;
		Xgate(std::string target) {
			this->target = target;
		}
	};
    class Ygate : public BasisGate {
	public:
		std::string target;
		Ygate(std::string target) {
			this->target = target;
		}
	};
    class Zgate : public BasisGate {
	public:
		std::string target;
		Zgate(std::string target) {
			this->target = target;
		}
	};
    class Hgate : public BasisGate {
	public:
		std::string target;
		Hgate(std::string target) {
			this->target = target;
		}
	};
    class Sgate : public BasisGate {
	public:
		std::string target;
		Sgate(std::string target) {
			this->target = target;
		}
	};
    class SDGgate : public BasisGate {
	public:
		std::string target;
		SDGgate(std::string target) {
			this->target = target;
		}
	};
    class Tgate : public BasisGate {
	public:
		std::string target;
		Tgate(std::string target) {
			this->target = target;
		}
	};
    class TDGgate : public BasisGate {
	public:
		std::string target;
		TDGgate(std::string target) {
			this->target = target;
		}
	};
///////////////////////////////////////////////////


    /*RXgate*/ 
    class RXgate : public BasisGate {
	public:
		Expr* theta;
		Expr* phi;
		Expr* lambda;
		std::string target;

		RXgate(Expr* theta, Expr* phi, Expr* lambda, std::string target) {
			this->theta = theta;
			this->phi = phi;
			this->lambda = lambda;
			this->target = target;
		}

		~RXgate() {
			if(theta != NULL) {
				delete theta;
			}
			if(phi != NULL) {
				delete phi;
			}
			if(lambda != NULL) {
				delete lambda;

			}
		}
	};
	
    /*RZgate*/ 
    class RZgate : public BasisGate {
	public:
		Expr* theta;
		Expr* phi;
		Expr* lambda;
		std::string target;

		RZgate(Expr* theta, Expr* phi, Expr* lambda, std::string target) {
			this->theta = theta;
			this->phi = phi;
			this->lambda = lambda;
			this->target = target;
		}

		~RZgate() {
			if(theta != NULL) {
				delete theta;
			}
			if(phi != NULL) {
				delete phi;
			}
			if(lambda != NULL) {
				delete lambda;

			}
		}
	};

	class CXgate : public BasisGate {
	public:
		std::string control;
		std::string target;

		CXgate(std::string control, std::string target) {
			this->control = control;
			this->target = target;
		}
	};

	class CompoundGate {
	public:
		std::vector<std::string> parameterNames;
		std::vector<std::string> argumentNames;
		std::vector<BasisGate*> gates;
		bool opaque;
	};

	class Snapshot {
	public:
		~Snapshot() {
			if(probabilities != NULL) {
				delete[] probabilities;
			}
			if(statevector != NULL) {
				delete[] statevector;
			}
		}

		unsigned long long len;
		double* probabilities;
		std::string* statevector;
		std::map<std::string, double> probabilities_ket;
	};

	void scan();
	void check(Token::Kind expected);

	Token la,t;
	Token::Kind sym = Token::Kind::none;

	std::string fname;
  	std::istream* in;
	QASMscanner* scanner;
	std::map<std::string, std::pair<int ,int> > qregs;
	std::map<std::string, std::pair<int, int*> > cregs;
	std::pair<int, int> QASMargumentQreg();
	std::pair<std::string, int> QASMargumentCreg();
	Expr* QASMexponentiation();
	Expr* QASMfactor();
	Expr* QASMterm();
	Expr* QASMexp();
	void QASMgateDecl();
	void QASMopaqueGateDecl();
	void QASMgate(bool execute = true);
	void QASMqop(bool execute = true);
	void QASMexpList(std::vector<Expr*>& expressions);
	void QASMidList(std::vector<std::string>& identifiers);
	void QASMargsList(std::vector<std::pair<int, int> >& arguments);
	std::set<Token::Kind> unaryops {Token::Kind::sin,Token::Kind::cos,Token::Kind::tan,Token::Kind::exp,Token::Kind::ln,Token::Kind::sqrt};

	std::map<std::string, CompoundGate> compoundGates;
	Expr* RewriteExpr(Expr* expr, std::map<std::string, Expr*>& exprMap);
	void printExpr(Expr* expr);

    /*get gatelist*/
	std::vector<Qcircuit::Gate> gatelistsL;

	unsigned int nqubits;
    unsigned int ngates;

    void addUgate(int target, double theta, double phi, double lambda);
///////////////////////////////////////////////////
    void addXgate(int target);
    void addYgate(int target);
    void addZgate(int target);
    void addHgate(int target);
    void addSgate(int target);
    void addSDGgate(int target);
    void addTgate(int target);
    void addTDGgate(int target);
///////////////////////////////////////////////////
    /*RX & RZ gate*/
    void addRXgate(int target, double theta, double phi, double lambda);
    void addRZgate(int target, double theta, double phi, double lambda);
    
    void addCXgate(int target, int control);
};

#endif /* QASM_SIMULATOR_H_ */
