/*----------------------------------------------------------------------------------------------*/
/* Description:     Source file for QASM parsing                                                */
/*                                                                                              */
/* Author:          Alwin Zulehner, Robert Wille                                                */
/*                                                                                              */
/* Revised by:      Sunghye Park, Postech                                                       */
/*                                                                                              */
/* Created:         02/10/2020 (for Quantum compiler)                                           */
/*----------------------------------------------------------------------------------------------*/
#include "QASMparser.h"
#include <algorithm>
#include <cmath>

QASMparser::QASMparser(std::string filename) {
	in = new std::ifstream (filename, std::ifstream::in);
    if(!in->good()) {
        std::cerr << "ERROR opening file " << filename << std::endl;
        std::exit(1);
    }

	this->scanner = new QASMscanner(*this->in);
	this->fname = filename;
    nqubits = 0;
    ngates = 0;
}

QASMparser::~QASMparser() {
	delete scanner;
	delete in;

	for(auto it = compoundGates.begin(); it != compoundGates.end(); it++) {
		for(auto it2 = it->second.gates.begin(); it2 != it->second.gates.end(); it2++) {
			delete *it2;
		}
	}
}

void QASMparser::scan() {
	t = la;
	la = scanner->next();
	sym = la.kind;
}

void QASMparser::check(Token::Kind expected) {
	if (sym == expected) {
		scan();
	} else {
		std::cerr << "ERROR while parsing QASM file: expected '" << Token::KindNames[expected] << "' but found '" << Token::KindNames[sym] << "' in line " << la.line << ", column " << la.col << std::endl;
	}
}

std::pair<int, int> QASMparser::QASMargumentQreg() {
	check(Token::Kind::identifier);
	std::string s = t.str;
	if(qregs.find(s) == qregs.end()) {
		std::cerr << "Argument is not a qreg: " << s << std::endl;
	}

	if(sym == Token::Kind::lbrack) {
		scan();
		check(Token::Kind::nninteger);
		int offset = t.val;
		check(Token::Kind::rbrack);
		return std::make_pair(qregs[s].first+offset, 1);
	}
	return std::make_pair(qregs[s].first, qregs[s].second);
}

std::pair<std::string, int> QASMparser::QASMargumentCreg() {
	check(Token::Kind::identifier);
	std::string s = t.str;
	if(cregs.find(s) == cregs.end()) {
		std::cerr << "Argument is not a creg: " << s << std::endl;
	}

	int index = -1;
	if(sym == Token::Kind::lbrack) {
		scan();
		check(Token::Kind::nninteger);
		index = t.val;
		if(index < 0 || index >= cregs[s].first) {
			std::cerr << "Index of creg " << s << " is out of bounds: " << index << std::endl;
		}
		check(Token::Kind::rbrack);
	}

	return std::make_pair(s, index);
}


QASMparser::Expr* QASMparser::QASMexponentiation() {
	Expr* x;

	if(sym == Token::Kind::real) {
		scan();
		return new Expr(Expr::Kind::number, NULL, NULL, t.valReal, "");
		//return t.val_real;
	} else if(sym == Token::Kind::nninteger) {
		scan();
		return new Expr(Expr::Kind::number, NULL, NULL, t.val, "");
		//return t.val;
	} else if(sym == Token::Kind::pi) {
		scan();
		return new Expr(Expr::Kind::number, NULL, NULL, 3.141592653589793238463, "");
		//return 3.141592653589793238463;
	} else if(sym == Token::Kind::identifier) {
		scan();
		return new Expr(Expr::Kind::id, NULL, NULL, 0, t.str);
		//return it->second;
	} else if(sym == Token::Kind::lpar) {
		scan();
		x = QASMexp();
		check(Token::Kind::rpar);
		return x;
	} else if(unaryops.find(sym) != unaryops.end()) {
		Token::Kind op = sym;
		scan();
		check(Token::Kind::lpar);
		x = QASMexp();
		check(Token::Kind::rpar);
		if(x->kind == Expr::Kind::number) {
			if(op == Token::Kind::sin) {
				x->num = std::sin(x->num);
			} else if(op == Token::Kind::cos) {
				x->num = std::cos(x->num);
			} else if(op == Token::Kind::tan) {
				x->num = std::tan(x->num);
			} else if(op == Token::Kind::exp) {
				x->num = std::exp(x->num);
			} else if(op == Token::Kind::ln) {
				x->num = std::log(x->num);
			} else if(op == Token::Kind::sqrt) {
				x->num = std::sqrt(x->num);
			}
			return x;
		} else {
			if(op == Token::Kind::sin) {
				return new Expr(Expr::Kind::sin, x, NULL, 0, "");
			} else if(op == Token::Kind::cos) {
				return new Expr(Expr::Kind::cos, x, NULL, 0, "");
			} else if(op == Token::Kind::tan) {
				return new Expr(Expr::Kind::tan, x, NULL, 0, "");
			} else if(op == Token::Kind::exp) {
				return new Expr(Expr::Kind::exp, x, NULL, 0, "");
			} else if(op == Token::Kind::ln) {
				return new Expr(Expr::Kind::ln, x, NULL, 0, "");
			} else if(op == Token::Kind::sqrt) {
				return new Expr(Expr::Kind::sqrt, x, NULL, 0, "");
			}
		}
	} else {
		std::cerr << "Invalid Expression" << std::endl;
	}
	return NULL;
}

QASMparser::Expr* QASMparser::QASMfactor() {
	Expr* x;
	Expr* y;
	x = QASMexponentiation();
	while (sym == Token::Kind::power) {
		scan();
		y = QASMexponentiation();
		if(x->kind == Expr::Kind::number && y->kind == Expr::Kind::number) {
			x->num = pow(x->num, y->num);
			delete y;
		} else {
			x = new Expr(Expr::Kind::power, x, y, 0, "");
		}
	}

	return x;
}

QASMparser::Expr* QASMparser::QASMterm() {
	Expr* x = QASMfactor();
	Expr* y;

	while(sym == Token::Kind::times || sym == Token::Kind::div) {
		Token::Kind op = sym;
		scan();
		y = QASMfactor();
		if(op == Token::Kind::times) {
			if(x->kind == Expr::Kind::number && y->kind == Expr::Kind::number) {
				x->num = x->num * y->num;
				delete y;
			} else {
				x = new Expr(Expr::Kind::times, x, y, 0, "");
			}
		} else {
			if(x->kind == Expr::Kind::number && y->kind == Expr::Kind::number) {
				x->num = x->num / y->num;
				delete y;
			} else {
				x = new Expr(Expr::Kind::div, x, y, 0, "");
			}
		}
	}
	return x;
}

QASMparser::Expr* QASMparser::QASMexp() {
	Expr* x;
	Expr* y;
	if(sym == Token::Kind::minus) {
		scan();
		x = QASMterm();
		if(x->kind == Expr::Kind::number) {
			x->num = -x->num;
		} else {
			x = new Expr(Expr::Kind::sign, x, NULL, 0, "");
		}
	} else {
		x = QASMterm();
	}

	while(sym == Token::Kind::plus || sym == Token::Kind::minus) {
		Token::Kind op = sym;
		scan();
		y = QASMterm();
		if(op == Token::Kind::plus) {
			if(x->kind == Expr::Kind::number && y->kind == Expr::Kind::number) {
				x->num = x->num + y->num;
				delete y;
			} else {
				x = new Expr(Expr::Kind::plus, x, y, 0, "");
			}
		} else {
			if(x->kind == Expr::Kind::number && y->kind == Expr::Kind::number) {
				x->num = x->num - y->num;
				delete y;
			} else {
				x = new Expr(Expr::Kind::minus, x, y, 0, "");
			}
		}
	}
	return x;
}

void QASMparser::QASMexpList(std::vector<Expr*>& expressions) {
	Expr* x = QASMexp();
	expressions.push_back(x);
	while(sym == Token::Kind::comma) {
		scan();
		expressions.push_back(QASMexp());
	}
}

void QASMparser::QASMargsList(std::vector<std::pair<int, int> >& arguments) {
	arguments.push_back(QASMargumentQreg());
	while(sym == Token::Kind::comma) {
		scan();
		arguments.push_back(QASMargumentQreg());
	}
}

////////////////////////////////////////////////
void QASMparser::addXgate(int target) {
    Qcircuit::Gate g;

    g.id = ngates;
    g.target = target;
    g.control = -1;
    
    logi_qubits[target] = true;

    g.type = GATETYPE::X;
    snprintf ( g.output_type, 127," ");     // for output_file

    gatelistsL.push_back(g);
    
    ngates++;
}
void QASMparser::addYgate(int target) {
    Qcircuit::Gate g;

    g.id = ngates;
    g.target = target;
    g.control = -1;
    
    logi_qubits[target] = true;

    g.type = GATETYPE::Y;
    snprintf ( g.output_type, 127," ");     // for output_file

    gatelistsL.push_back(g);
    
    ngates++;
}
void QASMparser::addZgate(int target) {
    Qcircuit::Gate g;

    g.id = ngates;
    g.target = target;
    g.control = -1;
    
    logi_qubits[target] = true;

    g.type = GATETYPE::Z;
    snprintf ( g.output_type, 127," ");     // for output_file

    gatelistsL.push_back(g);
    
    ngates++;
}
void QASMparser::addHgate(int target) {
    Qcircuit::Gate g;

    g.id = ngates;
    g.target = target;
    g.control = -1;
    
    logi_qubits[target] = true;

    g.type = GATETYPE::H;
    snprintf ( g.output_type, 127," ");     // for output_file

    gatelistsL.push_back(g);
    
    ngates++;
}
void QASMparser::addSgate(int target) {
    Qcircuit::Gate g;

    g.id = ngates;
    g.target = target;
    g.control = -1;
    
    logi_qubits[target] = true;

    g.type = GATETYPE::S;
    snprintf ( g.output_type, 127," ");     // for output_file

    gatelistsL.push_back(g);
    
    ngates++;
}
void QASMparser::addSDGgate(int target) {
    Qcircuit::Gate g;

    g.id = ngates;
    g.target = target;
    g.control = -1;
    
    logi_qubits[target] = true;

    g.type = GATETYPE::SDG;
    snprintf ( g.output_type, 127," ");     // for output_file

    gatelistsL.push_back(g);
    
    ngates++;
}
void QASMparser::addTgate(int target) {
    Qcircuit::Gate g;

    g.id = ngates;
    g.target = target;
    g.control = -1;
    
    logi_qubits[target] = true;

    g.type = GATETYPE::T;
    snprintf ( g.output_type, 127," ");     // for output_file

    gatelistsL.push_back(g);
    
    ngates++;
}
void QASMparser::addTDGgate(int target) {
    Qcircuit::Gate g;

    g.id = ngates;
    g.target = target;
    g.control = -1;
    
    logi_qubits[target] = true;

    g.type = GATETYPE::TDG;
    snprintf ( g.output_type, 127," ");     // for output_file

    gatelistsL.push_back(g);
    
    ngates++;
}
////////////////////////////////////////////////

void QASMparser::addRXgate(int target, double theta, double phi, double lambda) {
    // rx(theta) = U3(theta, -pi/2, pi/2)
    Qcircuit::Gate g;

    g.id = ngates;
    g.target = target;
    g.control = -1;
    
    logi_qubits[target] = true;

    g.type = GATETYPE::RX;
    snprintf ( g.output_type, 127, "(%f)", theta);

    gatelistsL.push_back(g);
    
    ngates++;
}

void QASMparser::addRZgate(int target, double theta, double phi, double lambda) {
    // rz(phi) = U1(0, 0, pi)
    Qcircuit::Gate g;
    
    g.id = ngates;
    g.target = target;
    g.control = -1;
    
    logi_qubits[target] = true;

    g.type = GATETYPE::RZ;
    snprintf ( g.output_type, 127, "(%f)", lambda);
    
    gatelistsL.push_back(g);
    
    ngates++;
}

void QASMparser::addUgate(int target, double theta, double phi, double lambda) {
    Qcircuit::Gate g;

    g.id = ngates;
    g.target = target;
    g.control = -1;
    
    logi_qubits[target] = true;

    g.type = GATETYPE::U;
    snprintf ( g.output_type, 127, "(%f, %f, %f)", theta, phi, lambda);

    
    gatelistsL.push_back(g);
    
    ngates++;
}

void QASMparser::addCXgate(int target, int control) {
    Qcircuit::Gate g;

    g.id = ngates;
    g.target = target;
    g.control = control;
    
    logi_qubits[target] = true;
    logi_qubits[control] = true;

    g.type = GATETYPE::CNOT;
    snprintf ( g.output_type, 127, "CX");

    gatelistsL.push_back(g);

    ngates++;
}

void QASMparser::QASMgate(bool execute) {
////////////////////////////////////////////////
    if(sym == Token::Kind::xgate) {
		scan();
		std::pair<int, int> target = QASMargumentQreg();
		check(Token::Kind::semicolon);
		if(execute) {
			for(int i = 0; i < target.second; i++) {
			    addXgate(target.first+i);
			}
		}
#if VERBOSE
		std::cout << "Applied gate: X" << std::endl;
#endif
    }else if(sym == Token::Kind::ygate) {
		scan();
		std::pair<int, int> target = QASMargumentQreg();
		check(Token::Kind::semicolon);
		if(execute) {
			for(int i = 0; i < target.second; i++) {
			    addYgate(target.first+i);
			}
		}
#if VERBOSE
		std::cout << "Applied gate: Y" << std::endl;
#endif
    }else if(sym == Token::Kind::zgate) {
		scan();
		std::pair<int, int> target = QASMargumentQreg();
		check(Token::Kind::semicolon);
		if(execute) {
			for(int i = 0; i < target.second; i++) {
			    addZgate(target.first+i);
			}
		}
#if VERBOSE
		std::cout << "Applied gate: Z" << std::endl;
#endif
    }else if(sym == Token::Kind::hgate) {
		scan();
		std::pair<int, int> target = QASMargumentQreg();
		check(Token::Kind::semicolon);
		if(execute) {
			for(int i = 0; i < target.second; i++) {
			    addHgate(target.first+i);
			}
		}
#if VERBOSE
		std::cout << "Applied gate: H" << std::endl;
#endif
    }else if(sym == Token::Kind::sgate) {
		scan();
		std::pair<int, int> target = QASMargumentQreg();
		check(Token::Kind::semicolon);
		if(execute) {
			for(int i = 0; i < target.second; i++) {
			    addSgate(target.first+i);
			}
		}
#if VERBOSE
		std::cout << "Applied gate: S" << std::endl;
#endif
    }else if(sym == Token::Kind::sdggate) {
		scan();
		std::pair<int, int> target = QASMargumentQreg();
		check(Token::Kind::semicolon);
		if(execute) {
			for(int i = 0; i < target.second; i++) {
			    addSDGgate(target.first+i);
			}
		}
#if VERBOSE
		std::cout << "Applied gate: SDG" << std::endl;
#endif
    }else if(sym == Token::Kind::tgate) {
		scan();
		std::pair<int, int> target = QASMargumentQreg();
		check(Token::Kind::semicolon);
		if(execute) {
			for(int i = 0; i < target.second; i++) {
			    addTgate(target.first+i);
			}
		}
#if VERBOSE
		std::cout << "Applied gate: T" << std::endl;
#endif
    }else if(sym == Token::Kind::tdggate) {
		scan();
		std::pair<int, int> target = QASMargumentQreg();
		check(Token::Kind::semicolon);
		if(execute) {
			for(int i = 0; i < target.second; i++) {
			    addTDGgate(target.first+i);
			}
		}
#if VERBOSE
		std::cout << "Applied gate: TDG" << std::endl;
#endif
////////////////////////////////////////////////
    //if(sym == Token::Kind::rxgate) {
    } else if(sym == Token::Kind::rxgate) {
		scan();
		check(Token::Kind::lpar);
		Expr* theta = QASMexp();
		check(Token::Kind::comma);
		Expr* phi = QASMexp();
		check(Token::Kind::comma);
		Expr* lambda = QASMexp();
		check(Token::Kind::rpar);
		std::pair<int, int> target = QASMargumentQreg();
		check(Token::Kind::semicolon);

		if(execute) {
			for(int i = 0; i < target.second; i++) {
			    addRXgate(target.first+i, theta->num, phi->num, lambda->num);
			}
		}
		delete theta;
		delete phi;
		delete lambda;

#if VERBOSE
		std::cout << "Applied gate: RX" << std::endl;
#endif
	} else if(sym == Token::Kind::rzgate) {
		scan();
		check(Token::Kind::lpar);
		Expr* theta = QASMexp();
		check(Token::Kind::comma);
		Expr* phi = QASMexp();
		check(Token::Kind::comma);
		Expr* lambda = QASMexp();
		check(Token::Kind::rpar);
		std::pair<int, int> target = QASMargumentQreg();
		check(Token::Kind::semicolon);

		if(execute) {
			for(int i = 0; i < target.second; i++) {
			    addRZgate(target.first+i, theta->num, phi->num, lambda->num);
			}
		}
		delete theta;
		delete phi;
		delete lambda;

#if VERBOSE
		std::cout << "Applied gate: RZ" << std::endl;
#endif
	} else if(sym == Token::Kind::ugate) {
		scan();
		check(Token::Kind::lpar);
		Expr* theta = QASMexp();
		check(Token::Kind::comma);
		Expr* phi = QASMexp();
		check(Token::Kind::comma);
		Expr* lambda = QASMexp();
		check(Token::Kind::rpar);
		std::pair<int, int> target = QASMargumentQreg();
		check(Token::Kind::semicolon);

		if(execute) {
			for(int i = 0; i < target.second; i++) {
			    addUgate(target.first+i, theta->num, phi->num, lambda->num);
			}
		}
		delete theta;
		delete phi;
		delete lambda;

#if VERBOSE
		std::cout << "Applied gate: U" << std::endl;
#endif
	

	} else if(sym == Token::Kind::cxgate) {
		scan();
		std::pair<int, int> control = QASMargumentQreg();
		check(Token::Kind::comma);
		std::pair<int, int> target = QASMargumentQreg();
		check(Token::Kind::semicolon);

		if(execute) {
			if(control.second == target.second) {
				for(int i = 0; i < target.second; i++) {
					addCXgate(target.first+i, control.first+i);
				}
			} else if(control.second == 1) {
				for(int i = 0; i < target.second; i++) {
                    addCXgate(target.first+i, control.first);
				}
			} else if(target.second == 1) {
				for(int i = 0; i < target.second; i++) {
                    addCXgate(target.first, control.first+i);
				}
			} else {
				std::cerr << "Register size does not match for CX gate!" << std::endl;
			}
#if VERBOSE
		std::cout << "Applied gate: CX" << std::endl;
#endif
		}
	} else if(sym == Token::Kind::identifier) {
		scan();
		auto gateIt = compoundGates.find(t.str);
		if(gateIt != compoundGates.end()) {
			std::string gate_name = t.str;

			std::vector<Expr*> parameters;
			std::vector<std::pair<int, int> > arguments;
			if(sym == Token::Kind::lpar) {
				scan();
				if(sym != Token::Kind::rpar) {
					QASMexpList(parameters);
				}
				check(Token::Kind::rpar);
			}
			QASMargsList(arguments);
			check(Token::Kind::semicolon);

			if(execute) {
				std::map<std::string, std::pair<int, int> > argsMap;
				std::map<std::string, Expr*> paramsMap;
				int size = 1;
				int i = 0;
				for(auto it = arguments.begin(); it != arguments.end(); it++) {
					argsMap[gateIt->second.argumentNames[i]] = *it;
					i++;
					if(it->second > 1 && size != 1 && it->second != size) {
						std::cerr << "Register sizes do not match!" << std::endl;
					}
					if(it->second > 1) {
						size = it->second;
					}
				}
				for(unsigned int i = 0; i < parameters.size(); i++) {
					paramsMap[gateIt->second.parameterNames[i]] = parameters[i];
				}

				for(auto it = gateIt->second.gates.begin(); it != gateIt->second.gates.end(); it++) {
////////////////////////////////////////////////
					if(Xgate* x = dynamic_cast<Xgate*>(*it)) {
						for(int i = 0; i < argsMap[x->target].second; i++) {
                            addXgate(argsMap[x->target].first+i);
						}

                    } else if(Ygate* y = dynamic_cast<Ygate*>(*it)) {
						for(int i = 0; i < argsMap[y->target].second; i++) {
                            addYgate(argsMap[y->target].first+i);
						}

                    } else if(Zgate* z = dynamic_cast<Zgate*>(*it)) {
						for(int i = 0; i < argsMap[z->target].second; i++) {
                            addZgate(argsMap[z->target].first+i);
						}

                    } else if(Hgate* h = dynamic_cast<Hgate*>(*it)) {
						for(int i = 0; i < argsMap[h->target].second; i++) {
                            addHgate(argsMap[h->target].first+i);
						}

                    } else if(Sgate* s = dynamic_cast<Sgate*>(*it)) {
						for(int i = 0; i < argsMap[s->target].second; i++) {
                            addSgate(argsMap[s->target].first+i);
						}

                    } else if(SDGgate* sdg = dynamic_cast<SDGgate*>(*it)) {
						for(int i = 0; i < argsMap[sdg->target].second; i++) {
                            addSDGgate(argsMap[sdg->target].first+i);
						}

                    } else if(Tgate* t = dynamic_cast<Tgate*>(*it)) {
						for(int i = 0; i < argsMap[t->target].second; i++) {
                            addTgate(argsMap[t->target].first+i);
						}

                    } else if(TDGgate* tdg = dynamic_cast<TDGgate*>(*it)) {
						for(int i = 0; i < argsMap[tdg->target].second; i++) {
                            addTDGgate(argsMap[tdg->target].first+i);
						}
////////////////////////////////////////////////
                    } else if(RXgate* rx = dynamic_cast<RXgate*>(*it)) {
					//if(RXgate* rx = dynamic_cast<RXgate*>(*it)) {
						Expr* theta = RewriteExpr(rx->theta, paramsMap);
						Expr* phi = RewriteExpr(rx->phi, paramsMap);
						Expr* lambda = RewriteExpr(rx->lambda, paramsMap);

						for(int i = 0; i < argsMap[rx->target].second; i++) {
                            addRXgate(argsMap[rx->target].first+i, theta->num, phi->num, lambda->num);
						}
						delete theta;
						delete phi;
						delete lambda;
					} else if(RZgate* rz = dynamic_cast<RZgate*>(*it)) {
						Expr* theta = RewriteExpr(rz->theta, paramsMap);
						Expr* phi = RewriteExpr(rz->phi, paramsMap);
						Expr* lambda = RewriteExpr(rz->lambda, paramsMap);

						for(int i = 0; i < argsMap[rz->target].second; i++) {
                            addRZgate(argsMap[rz->target].first+i, theta->num, phi->num, lambda->num);
						}
						delete theta;
						delete phi;
						delete lambda;
					} else if(Ugate* u = dynamic_cast<Ugate*>(*it)) {
						Expr* theta = RewriteExpr(u->theta, paramsMap);
						Expr* phi = RewriteExpr(u->phi, paramsMap);
						Expr* lambda = RewriteExpr(u->lambda, paramsMap);

						for(int i = 0; i < argsMap[u->target].second; i++) {
                            addUgate(argsMap[u->target].first+i, theta->num, phi->num, lambda->num);
						}
						delete theta;
						delete phi;
						delete lambda;
					} else if(CXgate* cx = dynamic_cast<CXgate*>(*it)) {
						if(argsMap[cx->control].second == argsMap[cx->target].second) {
							for(int i = 0; i < argsMap[cx->target].second; i++) {
                                addCXgate(argsMap[cx->target].first+i, argsMap[cx->control].first+i);
                            }
						} else if(argsMap[cx->control].second == 1) {
							for(int i = 0; i < argsMap[cx->target].second; i++) {
                                addCXgate(argsMap[cx->target].first+i, argsMap[cx->control].first);
							}
						} else if(argsMap[cx->target].second == 1) {
							for(int i = 0; i < argsMap[cx->target].second; i++) {
                                addCXgate(argsMap[cx->target].first, argsMap[cx->control].first+i);
							}
						} else {
							std::cerr << "Register size does not match for CX gate!" << std::endl;
						}
					}
				}

#if VERBOSE
		std::cout << "Applied gate: " << gate_name << std::endl;
#endif
			}
		} else {
			std::cerr << "Undefined gate: " << t.str << std::endl;
		}
	}
}


void QASMparser::QASMidList(std::vector<std::string>& identifiers) {
	check(Token::Kind::identifier);
	identifiers.push_back(t.str);
	while(sym == Token::Kind::comma) {
		scan();
		check(Token::Kind::identifier);
		identifiers.push_back(t.str);
	}
}

QASMparser::Expr* QASMparser::RewriteExpr(Expr* expr, std::map<std::string, Expr*>& exprMap) {
	if(expr == NULL) {
		return NULL;
	}
	Expr* op1 = RewriteExpr(expr->op1, exprMap);
	Expr* op2 = RewriteExpr(expr->op2, exprMap);

	if(expr->kind == Expr::Kind::number) {
		return new Expr(expr->kind, op1, op2, expr->num, expr->id);
	} else if(expr->kind == Expr::Kind::plus) {
		if(op1->kind == Expr::Kind::number && op2->kind == Expr::Kind::number) {
			op1->num = op1->num + op2->num;
			delete op2;
			return op1;
		}
	} else if(expr->kind == Expr::Kind::minus) {
		if(op1->kind == Expr::Kind::number && op2->kind == Expr::Kind::number) {
			op1->num = op1->num - op2->num;
			delete op2;
			return op1;
		}
	} else if(expr->kind == Expr::Kind::sign) {
		if(op1->kind == Expr::Kind::number) {
			op1->num = -op1->num;
			return op1;
		}
	} else if(expr->kind == Expr::Kind::times) {
		if(op1->kind == Expr::Kind::number && op2->kind == Expr::Kind::number) {
			op1->num = op1->num * op2->num;
			delete op2;
			return op1;
		}
	} else if(expr->kind == Expr::Kind::div) {
		if(op1->kind == Expr::Kind::number && op2->kind == Expr::Kind::number) {
			op1->num = op1->num / op2->num;
			delete op2;
			return op1;
		}
	} else if(expr->kind == Expr::Kind::power) {
		if(op1->kind == Expr::Kind::number && op2->kind == Expr::Kind::number) {
			op1->num = pow(op1->num,op2->num);
			delete op2;
			return op1;
		}
	} else if(expr->kind == Expr::Kind::sin) {
		if(op1->kind == Expr::Kind::number) {
			op1->num = sin(op1->num);
			return op1;
		}
	} else if(expr->kind == Expr::Kind::cos) {
		if(op1->kind == Expr::Kind::number) {
			op1->num = cos(op1->num);
			return op1;
		}
	} else if(expr->kind == Expr::Kind::tan) {
		if(op1->kind == Expr::Kind::number) {
			op1->num = tan(op1->num);
			return op1;
		}
	} else if(expr->kind == Expr::Kind::exp) {
		if(op1->kind == Expr::Kind::number) {
			op1->num = exp(op1->num);
			return op1;
		}
	} else if(expr->kind == Expr::Kind::ln) {
		if(op1->kind == Expr::Kind::number) {
			op1->num = log(op1->num);
			return op1;
		}
	} else if(expr->kind == Expr::Kind::sqrt) {
		if(op1->kind == Expr::Kind::number) {
			op1->num = sqrt(op1->num);
			return op1;
		}
	} else if(expr->kind == Expr::Kind::id) {
		return new Expr(*exprMap[expr->id]);
	}

	return new Expr(expr->kind, op1, op2, expr->num, expr->id);
}

void QASMparser::QASMopaqueGateDecl() {
	check(Token::Kind::opaque);
	check(Token::Kind::identifier);

	CompoundGate gate;
	std::string gateName = t.str;
	if(sym == Token::Kind::lpar) {
		scan();
		if(sym != Token::Kind::rpar) {
			QASMidList(gate.parameterNames);
		}
		check(Token::Kind::rpar);
	}
	QASMidList(gate.argumentNames);

	compoundGates[gateName] = gate;

	check(Token::Kind::semicolon);
	//Opaque gate has an empty body
}


void QASMparser::QASMgateDecl() {
	check(Token::Kind::gate);
	check(Token::Kind::identifier);

	CompoundGate gate;
	std::string gateName = t.str;
	if(sym == Token::Kind::lpar) {
		scan();
		if(sym != Token::Kind::rpar) {
			QASMidList(gate.parameterNames);
		}
		check(Token::Kind::rpar);
	}
	QASMidList(gate.argumentNames);
	check(Token::Kind::lbrace);


	while(sym != Token::Kind::rbrace) {
////////////////////////////////////////////////
		if(sym == Token::Kind::xgate) {
			scan();
			check(Token::Kind::identifier);
			gate.gates.push_back(new Xgate(t.str));
			check(Token::Kind::semicolon);
		
        } else if(sym == Token::Kind::ygate) {
			scan();
			check(Token::Kind::identifier);
			gate.gates.push_back(new Ygate(t.str));
			check(Token::Kind::semicolon);
		
        } else if(sym == Token::Kind::zgate) {
			scan();
			check(Token::Kind::identifier);
			gate.gates.push_back(new Zgate(t.str));
			check(Token::Kind::semicolon);
		
        } else if(sym == Token::Kind::hgate) {
			scan();
			check(Token::Kind::identifier);
			gate.gates.push_back(new Hgate(t.str));
			check(Token::Kind::semicolon);
		
        } else if(sym == Token::Kind::sgate) {
			scan();
			check(Token::Kind::identifier);
			gate.gates.push_back(new Sgate(t.str));
			check(Token::Kind::semicolon);
		
        } else if(sym == Token::Kind::sdggate) {
			scan();
			check(Token::Kind::identifier);
			gate.gates.push_back(new SDGgate(t.str));
			check(Token::Kind::semicolon);
		
        } else if(sym == Token::Kind::tgate) {
			scan();
			check(Token::Kind::identifier);
			gate.gates.push_back(new Tgate(t.str));
			check(Token::Kind::semicolon);
		
        } else if(sym == Token::Kind::tdggate) {
			scan();
			check(Token::Kind::identifier);
			gate.gates.push_back(new TDGgate(t.str));
			check(Token::Kind::semicolon);
////////////////////////////////////////////////
		//if(sym == Token::Kind::rxgate) {
        } else if(sym == Token::Kind::rxgate) {
			scan();
			check(Token::Kind::lpar);
			Expr* theta = QASMexp();
			check(Token::Kind::comma);
			Expr* phi = QASMexp();
			check(Token::Kind::comma);
			Expr* lambda = QASMexp();
			check(Token::Kind::rpar);
			check(Token::Kind::identifier);

			gate.gates.push_back(new RXgate(theta, phi, lambda, t.str));
			check(Token::Kind::semicolon);
		} else if(sym == Token::Kind::rzgate) {
			scan();
			check(Token::Kind::lpar);
			Expr* theta = QASMexp();
			check(Token::Kind::comma);
			Expr* phi = QASMexp();
			check(Token::Kind::comma);
			Expr* lambda = QASMexp();
			check(Token::Kind::rpar);
			check(Token::Kind::identifier);

			gate.gates.push_back(new RZgate(theta, phi, lambda, t.str));
			check(Token::Kind::semicolon);
		} else if(sym == Token::Kind::ugate) {
			scan();
			check(Token::Kind::lpar);
			Expr* theta = QASMexp();
			check(Token::Kind::comma);
			Expr* phi = QASMexp();
			check(Token::Kind::comma);
			Expr* lambda = QASMexp();
			check(Token::Kind::rpar);
			check(Token::Kind::identifier);

			gate.gates.push_back(new Ugate(theta, phi, lambda, t.str));
			check(Token::Kind::semicolon);
		} else if(sym == Token::Kind::cxgate) {
			scan();
			check(Token::Kind::identifier);
			std::string control = t.str;
			check(Token::Kind::comma);
			check(Token::Kind::identifier);
			gate.gates.push_back(new CXgate(control, t.str));
			check(Token::Kind::semicolon);

		} else if(sym == Token::Kind::identifier) {
			scan();
			std::string name = t.str;

			std::vector<Expr* > parameters;
			std::vector<std::string> arguments;
			if(sym == Token::Kind::lpar) {
				scan();
				if(sym != Token::Kind::rpar) {
					QASMexpList(parameters);
				}
				check(Token::Kind::rpar);
			}
			QASMidList(arguments);
			check(Token::Kind::semicolon);

			CompoundGate g = compoundGates[name];
			std::map<std::string, std::string> argsMap;
			for(unsigned int i = 0; i < arguments.size(); i++) {
				argsMap[g.argumentNames[i]] = arguments[i];
			}

			std::map<std::string, Expr*> paramsMap;
			for(unsigned int i = 0; i < parameters.size(); i++) {
				paramsMap[g.parameterNames[i]] = parameters[i];
			}

			for(auto it = g.gates.begin(); it != g.gates.end(); it++) {
////////////////////////////////////////////////
				if(Xgate* x = dynamic_cast<Xgate*>(*it)) {
					gate.gates.push_back(new Xgate(argsMap[x->target]));
				
                }else if(Ygate* y = dynamic_cast<Ygate*>(*it)) {
					gate.gates.push_back(new Ygate(argsMap[y->target]));
				
                }else if(Zgate* z = dynamic_cast<Zgate*>(*it)) {
					gate.gates.push_back(new Zgate(argsMap[z->target]));
				
                }else if(Hgate* h = dynamic_cast<Hgate*>(*it)) {
					gate.gates.push_back(new Hgate(argsMap[h->target]));
				
                }else if(Sgate* s = dynamic_cast<Sgate*>(*it)) {
					gate.gates.push_back(new Sgate(argsMap[s->target]));
				
                }else if(SDGgate* sdg = dynamic_cast<SDGgate*>(*it)) {
					gate.gates.push_back(new SDGgate(argsMap[sdg->target]));
				
                }else if(Tgate* t = dynamic_cast<Tgate*>(*it)) {
					gate.gates.push_back(new Tgate(argsMap[t->target]));
				
                }else if(TDGgate* tdg = dynamic_cast<TDGgate*>(*it)) {
					gate.gates.push_back(new TDGgate(argsMap[tdg->target]));
////////////////////////////////////////////////
				//if(RXgate* rx = dynamic_cast<RXgate*>(*it)) {
                } else if(RXgate* rx = dynamic_cast<RXgate*>(*it)) {
					gate.gates.push_back(new RXgate(RewriteExpr(rx->theta, paramsMap), RewriteExpr(rx->phi, paramsMap), RewriteExpr(rx->lambda, paramsMap), argsMap[rx->target]));
				} else if(RZgate* rz = dynamic_cast<RZgate*>(*it)) {
					gate.gates.push_back(new RZgate(RewriteExpr(rz->theta, paramsMap), RewriteExpr(rz->phi, paramsMap), RewriteExpr(rz->lambda, paramsMap), argsMap[rz->target]));
				} else if(Ugate* u = dynamic_cast<Ugate*>(*it)) {
					gate.gates.push_back(new Ugate(RewriteExpr(u->theta, paramsMap), RewriteExpr(u->phi, paramsMap), RewriteExpr(u->lambda, paramsMap), argsMap[u->target]));
				} else if(CXgate* cx = dynamic_cast<CXgate*>(*it)) {
					gate.gates.push_back(new CXgate(argsMap[cx->control], argsMap[cx->target]));
				} else {
					std::cerr << "Unexpected gate!" << std::endl;
				}
			}

			for(auto it = parameters.begin(); it != parameters.end(); it++) {
				delete *it;
			}
		} else if(sym == Token::Kind::barrier) {
			scan();
			std::vector<std::string> arguments;
			QASMidList(arguments);
			check(Token::Kind::semicolon);
			//Nothing to do here for the simulator
		} else {
			std::cerr << "Error in gate declaration!" << std::endl;
		}
	}

#if VERBOSE & 0
	std::cout << "Declared gate \"" << gateName << "\":" << std::endl;
	for(auto it = gate.gates.begin(); it != gate.gates.end(); it++) {
////////////////////////////////////////////////
		if(Xgate* x = dynamic_cast<Xgate*>(*it)) {
			std::cout << "  X " << x->target << ";" << std::endl;
        
        }else if(Ygate* y = dynamic_cast<Ygate*>(*it)) {
			std::cout << "  Y " << y->target << ";" << std::endl;
        
        }else if(Zgate* z = dynamic_cast<Zgate*>(*it)) {
			std::cout << "  Z " << z->target << ";" << std::endl;
        
        }else if(Hgate* h = dynamic_cast<Hgate*>(*it)) {
			std::cout << "  H " << h->target << ";" << std::endl;
        
        }else if(Sgate* s = dynamic_cast<Sgate*>(*it)) {
			std::cout << "  S " << s->target << ";" << std::endl;
        
        }else if(SDGgate* sdg = dynamic_cast<SDGgate*>(*it)) {
			std::cout << "  SDG " << sdg->target << ";" << std::endl;
        
        }else if(Tgate* t = dynamic_cast<Tgate*>(*it)) {
			std::cout << "  T " << t->target << ";" << std::endl;
        
        }else if(TDGgate* tdg = dynamic_cast<TDGgate*>(*it)) {
			std::cout << "  TDG " << tdg->target << ";" << std::endl;
////////////////////////////////////////////////
		//if(RXgate* rx = dynamic_cast<RXgate*>(*it)) {
        } else if(RXgate* rx = dynamic_cast<RXgate*>(*it)) {
			std::cout << "  RX(";
			printExpr(rx->theta);
			std::cout << ", ";
			printExpr(rx->phi);
			std::cout << ", ";
			printExpr(rx->lambda);
			std::cout << ") "<< rx->target << ";" << std::endl;
		} else if(RZgate* rz = dynamic_cast<RZgate*>(*it)) {
			std::cout << "  RZ(";
			printExpr(rz->theta);
			std::cout << ", ";
			printExpr(rz->phi);
			std::cout << ", ";
			printExpr(rz->lambda);
			std::cout << ") "<< rz->target << ";" << std::endl;
		} else if(Ugate* u = dynamic_cast<Ugate*>(*it)) {
			std::cout << "  U(";
			printExpr(u->theta);
			std::cout << ", ";
			printExpr(u->phi);
			std::cout << ", ";
			printExpr(u->lambda);
			std::cout << ") "<< u->target << ";" << std::endl;
		} else if(CXgate* cx = dynamic_cast<CXgate*>(*it)) {
			std::cout << "  CX " << cx->control << ", " << cx->target << ";" << std::endl;
		} else {
			std::cout << "other gate" << std::endl;
		}
	}
#endif

	compoundGates[gateName] = gate;

	check(Token::Kind::rbrace);
}

void QASMparser::printExpr(Expr* expr) {
	if(expr->kind == Expr::Kind::number) {
		std::cout << expr->num;
	} else if(expr->kind == Expr::Kind::plus) {
		printExpr(expr->op1);
		std::cout << " + ";
		printExpr(expr->op2);
	} else if(expr->kind == Expr::Kind::minus) {
		printExpr(expr->op1);
		std::cout << " - ";
		printExpr(expr->op2);
	} else if(expr->kind == Expr::Kind::sign) {
		std::cout << "( - ";
		printExpr(expr->op1);
		std::cout << " )";
	} else if(expr->kind == Expr::Kind::times) {
		printExpr(expr->op1);
		std::cout << " * ";
		printExpr(expr->op2);
	} else if(expr->kind == Expr::Kind::div) {
		printExpr(expr->op1);
		std::cout << " / ";
		printExpr(expr->op2);
	} else if(expr->kind == Expr::Kind::power) {
		printExpr(expr->op1);
		std::cout << " ^ ";
		printExpr(expr->op2);
	} else if(expr->kind == Expr::Kind::sin) {
		std::cout << "sin(";
		printExpr(expr->op1);
		std::cout << ")";
	} else if(expr->kind == Expr::Kind::cos) {
		std::cout << "cos(";
		printExpr(expr->op1);
		std::cout << ")";
	} else if(expr->kind == Expr::Kind::tan) {
		std::cout << "tan(";
		printExpr(expr->op1);
		std::cout << ")";
	} else if(expr->kind == Expr::Kind::exp) {
		std::cout << "exp(";
		printExpr(expr->op1);
		std::cout << ")";
	} else if(expr->kind == Expr::Kind::ln) {
		std::cout << "ln(";
		printExpr(expr->op1);
		std::cout << ")";
	} else if(expr->kind == Expr::Kind::sqrt) {
		std::cout << "sqrt(";
		printExpr(expr->op1);
		std::cout << ")";
	} else if(expr->kind == Expr::Kind::id) {
		std::cout << expr->id;
	}
}

void QASMparser::QASMqop(bool execute) {
	if(sym == Token::Kind::ugate || 
//////////////////////
        sym == Token::Kind::xgate ||
        sym == Token::Kind::ygate ||
        sym == Token::Kind::zgate ||
        sym == Token::Kind::hgate ||
        sym == Token::Kind::sgate ||
        sym == Token::Kind::sdggate ||
        sym == Token::Kind::tgate ||
        sym == Token::Kind::tdggate ||
//////////////////////
            sym == Token::Kind::rxgate || sym == Token::Kind::rzgate || sym == Token::Kind::cxgate || sym == Token::Kind::identifier) {
		QASMgate(execute);
	} else if(sym == Token::Kind::measure) {
		scan();
		std::pair<int, int> qreg = QASMargumentQreg();

		check(Token::Kind::minus);
		check(Token::Kind::gt);
		std::pair<std::string, int> creg = QASMargumentCreg();
		check(Token::Kind::semicolon);

		std::cout << "Warning: Measurements currently not supported" << std::endl;
		if(execute) {
			int creg_size = (creg.second == -1) ? cregs[creg.first].first : 1;

			if(qreg.second == creg_size) {
				if(creg_size == 1) {
					//cregs[creg.first].second[creg.second] = MeasureOne(nqubits-1-(qreg.first));
				} else {
					for(int i = 0; i < creg_size; i++) {
						//cregs[creg.first].second[i] = MeasureOne(nqubits-1-(qreg.first+i));
					}
				}
			} else {
				std::cerr << "Mismatch of qreg and creg size in measurement" << std::endl;
			}
		}
	} else if(sym == Token::Kind::reset) {
		scan();
		std::pair<int, int> qreg = QASMargumentQreg();

        std::cout << "Warning: Reset currently not supported" << std::endl;

        check(Token::Kind::semicolon);

		if(execute) {
			for(int i = 0; i < qreg.second; i++) {
				//ResetQubit(nqubits-1-(qreg.first+i));
			}
		}

	}
}

void QASMparser::Parse() {

	scan();
	check(Token::Kind::openqasm);
	check(Token::Kind::real);
	check(Token::Kind::semicolon);

	do {
		if(sym == Token::Kind::qreg) {

			scan();
			check(Token::Kind::identifier);
			std::string s = t.str;
			check(Token::Kind::lbrack);
			check(Token::Kind::nninteger);
			int n = t.val;
			check(Token::Kind::rbrack);
			check(Token::Kind::semicolon);
			//check whether it already exists

			qregs[s] = std::make_pair(nqubits, n);

			//int* last_layer_new = new int[nqubits + n];
			//for(unsigned int i = 0; i < nqubits; i++) {
			//    last_layer_new[i] = last_layer[i];
			//}
			//for(unsigned i = nqubits; i < nqubits + n; i++) {
			//    last_layer_new[i] = -1;
			//}
			//delete[] last_layer;
			//last_layer = last_layer_new;
			nqubits = nqubits + n;

        } else if(sym == Token::Kind::creg) {
			scan();
			check(Token::Kind::identifier);
			std::string s = t.str;
			check(Token::Kind::lbrack);
			check(Token::Kind::nninteger);
			int n = t.val;
			check(Token::Kind::rbrack);
			check(Token::Kind::semicolon);
			int* reg = new int[n];

			//Initialize cregs with 0
			for(int i=0; i<n; i++) {
				reg[i] = 0;
			}
			cregs[s] = std::make_pair(n, reg);

		} else if(sym == Token::Kind::ugate || 
///////////////////////////////////////////////////////////
                sym == Token::Kind::xgate ||
                sym == Token::Kind::ygate ||
                sym == Token::Kind::zgate ||
                sym == Token::Kind::hgate ||
                sym == Token::Kind::sgate ||
                sym == Token::Kind::sdggate ||
                sym == Token::Kind::tgate ||
                sym == Token::Kind::tdggate ||
///////////////////////////////////////////////////////////
                sym == Token::Kind::rxgate || sym == Token::Kind::rzgate || sym == Token::Kind::cxgate || sym == Token::Kind::identifier || sym == Token::Kind::measure || sym == Token::Kind::reset) {
			QASMqop();
		} else if(sym == Token::Kind::gate) {
			QASMgateDecl();
		} else if(sym == Token::Kind::include) {
			scan();
			check(Token::Kind::string);
			std::string fname = t.str;
			scanner->addFileInput(fname);
			check(Token::Kind::semicolon);
		} else if(sym == Token::Kind::barrier) {
			scan();
			std::vector<std::pair<int, int> > args;
			QASMargsList(args);
			check(Token::Kind::semicolon);
			//Nothing to do here for simulator
		} else if(sym == Token::Kind::opaque) {
			QASMopaqueGateDecl();
		} else if(sym == Token::Kind::_if) {
			scan();
			check(Token::Kind::lpar);
			check(Token::Kind::identifier);
			std::string creg = t.str;
			check(Token::Kind::eq);
			check(Token::Kind::nninteger);
			int n = t.val;
			check(Token::Kind::rpar);

			auto it = cregs.find(creg);
			if(it == cregs.end()) {
				std::cerr << "Error in if statement: " << creg << " is not a creg!" << std::endl;
			} else {
				int creg_num = 0;
				for(int i = it->second.first-1; i >= 0; i--) {
					creg_num = (creg_num << 1) | (it->second.second[i] & 1);
				}
				QASMqop(creg_num == n);
			}

		} else {
            std::cerr << "ERROR: unexpected statement: started with " << Token::KindNames[sym] << "!" << std::endl;
            exit(1);
		}
	} while (sym != Token::Kind::eof);
}
