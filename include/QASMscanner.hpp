/*----------------------------------------------------------------------------------------------*/
/* Description:     Header file for QASM scan                                                   */
/*                                                                                              */
/* Author:          Alwin Zulehner, Robert Wille                                                */
/*                                                                                              */
/* Revised by:      Sunghye Park, Postech                                                       */
/*                                                                                              */
/* Created:         02/10/2020 (for Quantum compiler)                                           */
/*----------------------------------------------------------------------------------------------*/
#ifndef QASMSCANNER_HPP_
#define QASMSCANNER_HPP_

#include <iostream>
#include <fstream>     
#include <istream>
#include <map>
#include <wctype.h>
#include <ctype.h>
#include "QASMtoken.hpp"
#include <sstream>
#include <stack>


class QASMscanner {


public:
    QASMscanner(std::istream& in_stream);
    Token next();
    void addFileInput(std::string fname);

private:
  	std::istream& in;
  	std::stack<std::istream*> streams;
  	char ch;
  	std::map<std::string, Token::Kind> keywords;
  	int line;
    int col;
    void nextCh();
    void readName(Token& t);
    void readNumber(Token& t);
    void readString(Token& t);
    void skipComment();

    class LineInfo {
    public:
    	char ch;
    	int line, col;

    	LineInfo(char ch, int line, int col) {
    		this->ch = ch;
    		this->line = line;
    		this->col = col;
    	}
    };

  	std::stack<LineInfo> lines;

};

#endif /* QASMSCANNER_HPP_ */
