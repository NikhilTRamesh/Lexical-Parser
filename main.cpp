#include "parse.h"
#include "lex.h"
#include "lex.cpp"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;
LexItem lex;

int main(int argc, char* argv[]){
  istream* src;
  ifstream infile;
  int cnt = 1;

  if (argc > 1) {
      infile.open(argv[1]);
      if (infile.is_open() == false) {
          cerr << "CANNOT OPEN THE FILE " << argv[1] << endl;
          exit(1);
      }
  }
  else if (argc > 2) {
      cerr << "TOO MANY FILE NAMES" << endl;
      exit(1);
  }
  else{
    std::cout << "NO FILENAME ENTERED" << '\n';
    exit(1);
  }
  src = &infile;

  if(Prog(*src, cnt)){
    std::cout << "Parse Successful" << '\n';
  }
  else{
    std::cout << "\nParse Unsuccessful" << '\n';
    std::cout << "There were: " << error_count << "Syntax Error \n";
  }

  return 0;
}

bool Prog(istream& in, int& line){
  lex = Parser::GetNextToken(in, line);
  if(lex == BEGIN){
    StmtList(in, line);
    lex = Parser::GetNextToken(in, line);
    if(lex == END){
      return true;
    }
    else{
      return false;
    }
  }
  return false;
}

bool Var(istream& in, int& line){
  LexItem lexVar = Parser::GetNextToken(in, line);
  if(lexVar == IDENT){
    defVar[lexVar.GetLexeme()] = true;
    return true;
  }
  else{
    Parser::PushBackToken(lexVar);
    return false;
  }
}


bool PrintStmt(istream& in, int& line){
  LexItem lexPrint = Parser::GetNextToken(in, line);
  if(lexPrint == PRINT){
    if(ExprList(in, line)){
      return true;
    }
    else{
      ParseError(line, "Error with Expression statement");
      return false;
    }
  }
  else{
    Parser::PushBackToken(lexPrint);
    return false;
  }
}


bool Stmt(istream& in, int& line){
  if(PrintStmt(in, line)){
    return true;
  }
  else if(IfStmt(in, line)){
    return true;
  }
  else if(AssignStmt(in, line)){
    return true;
  }
  else{
    LexItem dump = Parser::GetNextToken(in, line);
    if(dump == END){
      Parser::PushBackToken(dump);
      return false;
    }
    else{
      ParseError(line, "Illegal Statement");
      return false;
    }
  }
}
bool StmtList(istream& in, int& line){
  if(Stmt(in, line)){
    LexItem lexT = Parser::GetNextToken(in, line);
    if(lexT == SCOMA){
      if(StmtList(in, line)){
        return true;
      }
      else{
        return false;
      }
    }
    else{
      ParseError(line, "Missing Semicolon");
      return false;
    }
  }
  else{
    LexItem lexT = Parser::GetNextToken(in, line);
    if(lexT == END || lexT == ERR){
      Parser::PushBackToken(lexT);
      return true;
    }
  }
  return false;
}

bool AssignStmt(istream& in, int& line){
  if(Var(in, line)){
    LexItem lexAssign = Parser::GetNextToken(in, line);
    if(lexAssign == EQ){
      if(Expr(in, line)){
        return true;
      }
      else{
        ParseError(line, "Missing Expression in Assignment");
        return false;
      }
    }
    else{
      Parser::PushBackToken(lexAssign);
      ParseError(line, "Missing EQ");
      return false;
    }
  }
  return false;
}

bool Expr(istream& in, int& line){
  if(Term(in,line)){
    LexItem lexExpr = Parser::GetNextToken(in, line);
    if(lexExpr == PLUS || lexExpr == MINUS){
      if(Expr(in, line)){
        return true;
      }
      else{
        return false;
      }
    }
    else{
      Parser::PushBackToken(lexExpr);
      return true;
    }
  }
  else{
    ParseError(line, "Illegal Expr");
    return false;
  }
}

bool Term(istream& in, int& line){
  if(Factor(in, line)){
    LexItem lexTerm = Parser::GetNextToken(in, line);
    if(lexTerm == MULT || lexTerm == DIV){
      if(Term(in, line)){
        return true;
      }
      else{
        return false;
      }
    }
    if(lexTerm == ERR){
      ParseError(line, "Not a Valid Term: " + lexTerm.GetLexeme());
      Parser::PushBackToken(lexTerm);
      return false;
    }
    else{
      Parser::PushBackToken(lexTerm);
      return true;
    }
  }
  return false;
}

bool ExprList(istream& in, int& line){
  if(Expr(in, line)){
    LexItem exprs = Parser::GetNextToken(in, line);
    if(exprs == COMA){
      if(ExprList(in, line)){
        return true;
      }
      else{
        return false;
      }
    }
    else{
      Parser::PushBackToken(exprs);
      return true;
    }
  }
  else{
    ParseError(line, "Unrecognized Input");
    return false;
  }
}


bool Factor(istream& in, int& line){
  LexItem fctr = Parser::GetNextToken(in, line);
  if(fctr == ICONST || fctr == RCONST || fctr == SCONST){
    return true;
  }
  else if(fctr == IDENT){
    if(defVar.count(fctr.GetLexeme())){
      return true;
    }
    else{
      ParseError(line, "Undefined Variable: " + fctr.GetLexeme());
      return false;
    }
  }
  else{
    Parser::PushBackToken(fctr);
    ParseError(line, "Unrecognized Input: " + fctr.GetLexeme());
    return false;
  }
}



bool IfStmt(istream& in, int& line){
  LexItem lexCheck = Parser::GetNextToken(in, line);
  if(lexCheck == IF){
    lexCheck = Parser::GetNextToken(in, line);
    if(lexCheck == LPAREN){
      if(Expr(in, line)){
        lexCheck = Parser::GetNextToken(in, line);
        if(lexCheck == RPAREN){
          lexCheck = Parser::GetNextToken(in, line);
          if(lexCheck == THEN){
            if(Stmt(in, line)){
              return true;
            }
            else{
              ParseError(line, "Missing Statement in If");
              return false;
            }
          }
          else{
            ParseError(line, "Missing THEN");
            return false;
          }
        }
        else{
          ParseError(line, "Missing RPAREN");
          return false;
        }
      }
      else{
        ParseError(line, "Missing Expr");
        return false;
      }
    }
    else{
      ParseError(line, "Missing LPAREN");
      return false;
    }
  }
  else{
    Parser::PushBackToken(lexCheck);
    return false;
  }
}
