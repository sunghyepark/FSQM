/*----------------------------------------------------------------------------------------------*/
/* Description:     Source file for QASM token                                                  */
/*                                                                                              */
/* Author:          Alwin Zulehner, Robert Wille                                                */
/*                                                                                              */
/* Revised by:      Sunghye Park, Postech                                                       */
/*                                                                                              */
/* Created:         02/10/2020 (for Quantum compiler)                                           */
/*----------------------------------------------------------------------------------------------*/
#include "QASMtoken.hpp"

std::map<Token::Kind, std::string> Token::KindNames = {
			{Kind::none,"none"},
			{Kind::include,"include"},
			{Kind::identifier,"<identifier>"},
			{Kind::number,"<number>"},
			{Kind::plus,"+"},
			{Kind::semicolon,";"},
			{Kind::eof,"EOF"},
			{Kind::lpar,"("},
			{Kind::rpar,")"},
			{Kind::lbrack,"["},
			{Kind::rbrack,"]"},
			{Kind::lbrace,"{"},
			{Kind::rbrace,"}"},
			{Kind::comma,","},
			{Kind::minus,"-"},
			{Kind::times,"*"},
			{Kind::nninteger,"<nninteger>"},
			{Kind::real,"<real>"},
			{Kind::qreg,"qreg"},
			{Kind::creg,"creg"},
			{Kind::ugate,"U"},
			/////////////////////
            {Kind::xgate,"X"}, 
            {Kind::xgate,"Y"}, 
            {Kind::xgate,"Z"}, 
            {Kind::xgate,"H"}, 
            {Kind::xgate,"S"}, 
            {Kind::xgate,"SDG"}, 
            {Kind::xgate,"T"}, 
            {Kind::xgate,"TDG"}, 
			/////////////////////
            {Kind::rxgate,"RX"}, /*RX gate*/
			{Kind::rzgate,"RZ"}, /*RZ gate*/
			{Kind::cxgate,"CX"},
			{Kind::gate,"gate"},
			{Kind::pi,"pi"},
			{Kind::measure,"measure"},
			{Kind::openqasm,"openqasm"},
			{Kind::probabilities,"probabilities"},
			{Kind::opaque,"opaque"},
			{Kind::sin,"sin"},
			{Kind::cos,"cos"},
			{Kind::tan,"tan"},
			{Kind::exp,"exp"},
			{Kind::ln,"ln"},
			{Kind::sqrt,"sqrt"},
			{Kind::div,"/"},
			{Kind::power,"^"},
			{Kind::string,"string"},
			{Kind::gt,">"},
			{Kind::barrier,"barrier"},
			{Kind::_if,"if"},
			{Kind::eq,"=="},
			{Kind::reset,"reset"}
	};
