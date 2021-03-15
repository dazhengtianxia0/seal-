#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "semant.h"
#include "utilities.h"
#include <vector>
#include <map>

extern int semant_debug;
extern char *curr_filename;

static ostream& error_stream = cerr;
static int semant_errors = 0;
static Decl curr_decl = 0;
static bool has_return = false;

Symbol return_type;


typedef SymbolTable<Symbol, Symbol> ObjectEnvironment; // name, type
ObjectEnvironment objectEnv;

typedef std::map<Symbol, Symbol> CallTable;
CallTable callTable;

typedef std::map<Symbol, Symbol> GlobalVariables;
GlobalVariables globalVars;


typedef std::map<Symbol, Symbol> LocalVariables;
LocalVariables localVars;


typedef std::map<Symbol, bool> InstallTable;
InstallTable installTable;

typedef std::vector<Symbol> MethodClass;
typedef std::map<Symbol, MethodClass> MethodTable;
MethodTable methodTable;



///////////////////////////////////////////////
// helper func
///////////////////////////////////////////////


static ostream& semant_error() {
    semant_errors++;
    return error_stream;
}

static ostream& semant_error(tree_node *t) {
    error_stream << t->get_line_number() << ": ";
    return semant_error();
}

static ostream& internal_error(int lineno) {
    error_stream << "FATAL:" << lineno << ": ";
    return error_stream;
}

//////////////////////////////////////////////////////////////////////
//
// Symbols
//
// For convenience, a large number of symbols are predefined here.
// These symbols include the primitive type and method names, as well
// as fixed names used by the runtime system.
//
//////////////////////////////////////////////////////////////////////

static Symbol 
    Int,
    Float,
    String,
    Bool,
    Void,
    Main,
    print
    ;



int callDecl_l = 0;
int cur_l= 0;
int loop_l = 0;

SymbolTable<Symbol, Symbol> *fu = new SymbolTable<Symbol, Symbol>();
SymbolTable<Symbol, Symbol> *fm = new SymbolTable<Symbol, Symbol>();
SymbolTable<Symbol, Symbol> *va = new SymbolTable<Symbol, Symbol>();
SymbolTable<Symbol, Symbol> *lvar = new SymbolTable<Symbol, Symbol>();
SymbolTable<Symbol, Variables> *fup = new SymbolTable<Symbol, Variables>();

bool isValidCallName(Symbol type) {
    return type != (Symbol)print;
}

bool isValidTypeName(Symbol type) {
    return type != Void;
}

//
// Initializing the predefined symbols.
//

static void initialize_constants(void) {
    // 4 basic types and Void type
    Bool        = idtable.add_string("Bool");
    Int         = idtable.add_string("Int");
    String      = idtable.add_string("String");
    Float       = idtable.add_string("Float");
    Void        = idtable.add_string("Void");  
    // Main function
    Main        = idtable.add_string("main");

    // classical function to print things, so defined here for call.
    print        = idtable.add_string("printf");
}

/*
    TODO :
    you should fill the following function defines, so that semant() can realize a semantic 
    analysis in a recursive way. 
    Of course, you can add any other functions to help.
*/




//static bool sameType(Symbol name1, Symbol name2) {
//    return strcmp(name1->get_string(), name2->get_string()) == 0;
//}


static void install_calls(Decls decls) {
    fu->enterscope();
    fu->addid(print, new Symbol(Void));
    fup->enterscope();

    for (int i = decls->first(); decls->more(i); i = decls->next(i))
    {
        curr_decl = decls->nth(i);
        Symbol name = curr_decl->getName();
        Symbol type = curr_decl->getType();
        if (curr_decl->isCallDecl())
        {
            if (fu->lookup(name) == NULL)
            {
                fu->addid(name, new Symbol(curr_decl->getType()));
                fup->addid(name, new Variables(CallDecl(curr_decl)->getVariables()));
            }
            else if (name == print)
            {
                semant_error(curr_decl) << "Function printf cannot be redefination.\n";
                semant_error(curr_decl) << "Function printf cannot have a name as printf.\n";
            }
            else
                semant_error(curr_decl) << "Function " << name << " was previously defined.\n";
        }
        /*
        if (decls->nth(i)->isCallDecl()) {
            if (callTable[name] != NULL) {
                semant_error(decls->nth(i))<<"Function "<<name<<" was previously defined."<<endl;
            } else if (type != Int && type != Void && type != String && type != Float && type != Bool) {
                semant_error(decls->nth(i))<<"Function returnType error."<<endl;
            } else if (!isValidCallName(name)) {
                semant_error(decls->nth(i))<<"Function printf cannot have a name as printf"<<endl;
            }
            callTable[name] = type;
            installTable[name] = false;
            decls->nth(i)->check();
        }
    }*/
    }
    
        
}

static void install_globalVars(Decls decls) {
    va->enterscope();

    for (int i = decls->first(); decls->more(i); i = decls->next(i))
    {
        curr_decl = decls->nth(i);
        Symbol name = curr_decl->getName();
        Symbol type = curr_decl->getType();
        if (!curr_decl->isCallDecl())
        {
            if (type == Void)
                semant_error(curr_decl) << "var " << name << " cannot be of type Void. Void can just be used as return type.\n";
            else if (va->lookup(name) == NULL)
                va->addid(name, new Symbol(type));
            else
                semant_error(curr_decl) << "var " << name << " was previously defined.\n";
        }
        /* if (!decls->nth(i)->isCallDecl()) {
            if (globalVars[name] != NULL) {
                semant_error(decls->nth(i))<<"Global variable redefined."<<endl;
            } else if (type == Void) {
                semant_error(decls->nth(i))<<"var "<<name<<" cannot be of type Void. Void can just be used as return type."<<endl;
            } else if (name == print) {
                semant_error(decls->nth(i))<<"Variable printf cannot have a name as printf"<<endl;
            }
            globalVars[name] = type;
        }*/
    }
}

static void check_calls(Decls decls) {
    //objectEnv.enterscope();
   for (int i = decls->first(); decls->more(i); i = decls->next(i))
    {   
        curr_decl = decls->nth(i);
        if (curr_decl->isCallDecl())
        //localVars.clear();
            curr_decl->check();
    }
    //objectEnv.exitscope();
}



static void check_main() {
   if (fu->lookup(Main) == NULL)
   //callTable[Main] == NULL
        semant_error() << "Main function is not defined."<<endl;
    
}

void VariableDecl_class::check() {
   Symbol name = this->getName();
   Symbol type = this->getType();
    if (type == Void)
        semant_error(this) << "var " << name << " cannot be of type Void. Void can just be used as return type."<<endl;
    /*else {
        objectEnv.addid(name, new Symbol(type));
        localVars[name] = type;
    }*/
}

void CallDecl_class::check() {
    ++callDecl_l;
    fm->enterscope();
    Variables vars = this->getVariables();
    Symbol funcName = this->getName(); 
    Symbol returnType = this->getType();
    StmtBlock stmtblock = this->getBody();
    //objectEnv.enterscope();
    if (getType() != Int && getType() != Float && getType() != Bool && getType() != String && getType() != Void)
        semant_error(this) << "Incorrect return type: " << getType() << '\n';

    if (getName() == Main)
    {
        if (getType() != Void)
            semant_error(this) << "Main function should have return type Void.\n";
        if (getVariables()->len() != 0)
            semant_error(this) << "Main function should not have any parameters.\n";
    }
    //methodTable[funcName] = mclass;
       // installTable[name] = true;

    

    for (int i = paras->first(); paras->more(i); i = paras->next(i))
    {
        Symbol type = paras->nth(i)->getType();
        Symbol name = paras->nth(i)->getName();

        if (paras->nth(i)->getType() == Void)
        {
            semant_error(this) << "Function " << getName() << "'s parameter has an invalid type Void."<<endl;
            continue;
        }
        if (fm->probe(paras->nth(i)->getName()) == NULL)
            fm->addid(paras->nth(i)->getName(), new Symbol(paras->nth(i)->getType()));
        else
            semant_error(this) << "Function " << getName() << "'s parameter has a duplicate name " << paras->nth(i)->getName() << ".\n";
    }

    return_type = getType();
    has_return = false;
    
    getBody()->check(Int);

    if (!has_return)
        semant_error(this) << "Function " << getName() << " must have an overall return statement.\n";

    callDecl_l--;
    /* VariableDecls varDecls = stmtblock->getVariableDecls();
        for (int j=varDecls->first(); varDecls->more(j); j=varDecls->next(j)) {
            varDecls->nth(j)->check();
        }
        stmtblock->check(returnType);
        if (!stmtblock->isReturn()) {
            semant_error(this)<<"Function "<<name<<" must have an overall return statement."<<endl;
        }
        if (stmtblock->isBreak()) {
            Stmts stmts = stmtblock->getStmts();
            for (int i=stmts->first(); stmts->more(i); i=stmts->next(i)) {
                if (stmts->nth(i)->isBreak()) {
                    semant_error(stmts->nth(i))<<"break must be used in a loop sentence"<<endl;
                }
            }
        }
        if (stmtblock->isContinue()) {
            Stmts stmts = stmtblock->getStmts();
            for (int i=stmts->first(); stmts->more(i); i=stmts->next(i)) {
                if (stmts->nth(i)->isContinue()) {
                    semant_error(stmts->nth(i))<<"continue must be used in a loop sentence."<<endl;
                }
            }
*/
  
}
bool judge_type(Symbol t1, Symbol t2)
{
    return  (t1 == Float && t2 == Int) || (t1 == Int && t2 == Float) || (t1 == Float && t2 == Float);
}

bool judge_type_plus(Symbol t1, Symbol t2)
{
    return  judge_type(t1, t2) || (t1 == Int && t2 == Int);
}


void check_int(Expr_class *e1)
{
    e1->check(Int);
}

void check_int(Expr_class *e1, Expr_class *e2)
{
    e1->check(Int);
    e2->check(Int);
}

void StmtBlock_class::check(Symbol type) {
    //Stmts stmts = this->getStmts(); 
    //for (int j=stmts->first(); stmts->more(j); j=stmts->next(j)) {
    //    stmts->nth(j)->check(type);
   // }

    lvar->enterscope();
    cur_l++;
    
    for (int i = vars->first(); vars->more(i); i = vars->next(i))
    {
        vars->nth(i)->check();
        Symbol type = vars->nth(i)->getType();
        Symbol name = vars->nth(i)->getName();
        if (vars->nth(i)->getType() == Void)
            continue;
        if (lvar->probe(vars->nth(i)->getName()) == NULL)
            lvar->addid(vars->nth(i)->getName(), new Symbol(vars->nth(i)->getType()));
        else
            semant_error(vars->nth(i)) << "var " << vars->nth(i)->getName() << " was previously defined.\n";
    }

    for (int i = stmts->first(); stmts->more(i); i = stmts->next(i))
        stmts->nth(i)->check(Int);

    cur_l--;
    lvar->exitscope();
}

void IfStmt_class::check(Symbol type) {
    Expr condition = this->getCondition();
   
    
   getCondition()->check(Int);
   StmtBlock thenExpr = this->getThen();
   StmtBlock elseExpr = this->getElse();
   if (getCondition()->getType() != Bool)
        semant_error(this) << "Condition must be a Bool, got " << getCondition()->getType() << '\n';
   getThen()->check(Int);
   getElse()->check(Int);
   //thenExpr->check(type);
    //elseExpr->check(type);
}

void WhileStmt_class::check(Symbol type) {
    loop_l = cur_l + 1;
    getCondition()->check(Int);
    Expr condition = this->getCondition();
    StmtBlock body = this->getBody();

    Symbol conditionType = condition->checkType();
    if (getCondition()->getType() != Bool)
        semant_error(this) << "Condition must be a Bool, got " << getCondition()->getType() << '\n';
    getBody()->check(Int);
    loop_l = 0;
}

void ForStmt_class::check(Symbol type) {
    Expr init = this->getInit();
    Expr condition = this->getCondition();
    Expr loop = this->getLoop();
    StmtBlock body = this->getBody();
    loop_l = cur_l + 1;
    check_int(getInit(), getCondition());
    if (!getCondition()->is_empty_Expr())
        if (getCondition()->getType() != Bool)
            semant_error(this) << "Condition must be a Bool, got " << getCondition()->getType() << '\n';
    check_int(getLoop());
    getBody()->check(Int);
    loop_l = 0;
}

void ReturnStmt_class::check(Symbol type) {
    /* Expr expr = this->getValue();
    
    // check if return Type match
    Symbol returnType = expr->checkType();
    if (returnType != Void) {
        if (expr->is_empty_Expr() && type != Void) {
            semant_error(this)<<"Returns Void, but need "<<type<<endl;
        } else if (!expr->is_empty_Expr() && type != returnType) {
            semant_error(this)<<"Returns "<<returnType<<" but need "<<type<<endl;
        }
    }*/
   if (cur_l == callDecl_l)
        has_return = true;
   getValue()->check(Int);
   if (return_type != getValue()->getType())
        semant_error(this) << "Returns " << getValue()->getType() << " , but need " << return_type << '\n';
}

void ContinueStmt_class::check(Symbol type) {
   if (cur_l < loop_l || loop_l == 0)
        semant_error(this) << "continue must be used in a loop sentence.\n";
}

void BreakStmt_class::check(Symbol type) {
   if (cur_l < loop_l || loop_l == 0)
        semant_error(this) << "break must be used in a loop sentence.\n";
}


/*Symbol Call_class::checkType(){
    Symbol name = this->getName();
    Actuals actuals = this->getActuals();
    unsigned int j = 0;
    
    if (name == print) {
        if (actuals->len() == 0) {
            semant_error(this)<<"printf() must has at last one parameter of type String."<<endl;
            this->setType(Void);
            return type;
        }
        Symbol sym = actuals->nth(actuals->first())->checkType();
        if (sym != String) {
            semant_error(this)<<"printf()'s first parameter must be of type String."<<endl;
            this->setType(Void);
            return type;
        }
        this->setType(Void);
        return type;
    }

    if (actuals->len() > 0){
        if (actuals->len() != int(methodTable[name].size())) {
            semant_error(this)<<"Wrong number of paras"<<endl;
        }
        for (int i=actuals->first(); actuals->more(i) && j<methodTable[name].size(); i=actuals->next(i)) {
            Expr expr = actuals->nth(i)->copy_Expr();
            Symbol sym = expr->checkType();
            // check function call's paras fit funcdecl's paras
            if (sym != methodTable[name][j]) {
                semant_error(this)<<"Function "<<name<<", type "<<sym<<" does not conform to declared type "<<methodTable[name][j]<<endl;
            }
            j ++;
            actuals->nth(i)->checkType();
        }
    }
    
    if (callTable[name] == NULL) {
        semant_error(this)<<"Object "<<name<<" has not been defined"<<endl;
        this->setType(Void);
        return type;
    } 
    this->setType(callTable[name]);
    return type;
}
*/
Symbol Call_class::checkType(){
  if (getName() == print)
    {
        if (actuals->len() < 1)
        {
            semant_error(this) << "printf() must has at last one parameter of type String."<<endl;
            setType(Void);
            return type;
        }
        actuals->nth(0)->check(Int);
        if (actuals->nth(0)->getType() != String)
            semant_error(this) << "printf()'s first parameter must be of type String."<<endl;
        else
        {
            for (int i = actuals->first(); actuals->more(i); i = actuals->next(i))
                actuals->nth(i)->check(Int);
        }
        setType(Void);
        return type;
    }

    if (fu->lookup(getName()) == NULL)
    {
        semant_error(this) << "Function " << getName() << " has not been defined.\n";
        setType(Void);
        return type;
    }
    else
    {
        if (fup->lookup(getName()) != NULL)
        {
            Variables v = *fup->lookup(getName());
            if (getActuals()->len() != v->len())
                semant_error(this) << "Function " << getName() << " called with wrong number of arguments.\n";
            else
                for (int i = actuals->first(); actuals->more(i); i = actuals->next(i))
                {
                    actuals->nth(i)->check(Int);
                    if (actuals->nth(i)->getType() != v->nth(i)->getType())
                    {
                        semant_error(this) << "Function " << getName() << ", the " << i + 1
                                           << " parameter should be " << v->nth(i)->getType()
                                           << " but provided a " << actuals->nth(i)->getType()
                                           << ".\n";
                        break;
                    }
                }
        }
        setType(*fu->lookup(getName()));
        return type;
    }
    return type;
}

Symbol Actual_class::checkType(){
   
    check_int(expr);
    setType(expr->getType());
    return type;
}


/*Symbol Assign_class::checkType(){
    if (objectEnv.lookup(lvalue) == NULL && globalVars[lvalue] == NULL) {
        semant_error(this)<<"Undefined value"<<endl;
    } 
    Symbol ls = localVars[lvalue];
    Symbol rs = value->checkType();
    if (ls != rs) {
        semant_error(this)<<"assign value mismatch"<<endl;
    }    
    this->setType(rs);
    return type;
}
*/
Symbol Assign_class::checkType(){
    check_int(value);
    Symbol up;
    if (lvar->lookup(lvalue) != NULL)
    {
        up = *lvar->lookup(lvalue);
        if (up != value->getType())
            semant_error(this) << "Right value must have type " << up << " , got " << value->getType() << ".\n";
    }
    else if (va->lookup(lvalue) != NULL)
    {
        up = *va->lookup(lvalue);
        if (up != value->getType())
            semant_error(this) << "Right value must have type " << up << " , got " << value->getType() << ".\n";
    }
    else if (fm->probe(lvalue) != NULL)
    {
        up = *fm->probe(lvalue);
        if (up != value->getType())
            semant_error(this) << "Right value must have type " << up << " , got " << value->getType() << ".\n";
    }
    
    else
    {
        up = Void;
        semant_error(this) << "Left value " << lvalue << " has not been defined.\n";
    }
    setType(up);
    return type;
}



Symbol Add_class::checkType(){

    /*Expr lvalue = e1;
    Expr rvalue = e2;
    Symbol ls = lvalue->checkType();
    Symbol rs = rvalue->checkType();

    if (ls != rs && !(ls == Int && rs == Float) && !(ls == Float && rs == Int)) {
        semant_error(this)<<"lvalue and rvalue should have same type."<<endl;
    }
    if ((ls == Float && rs == Int)||(ls == Int && rs == Float)) {
        this->setType(Float);
        return type;
    }

    this->setType(ls);
    return type;*/

    check_int(e1, e2);

    if (judge_type(e1->getType(), e2->getType()))
        setType(Float);
    else if (e1->getType() == Int && e2->getType() == Int)
        setType(Int);
    else
    {
        semant_error(this) << "Cannot add a " << e1->getType() << " and a " << e2->getType() << ".\n";
        setType(Void);
    }
    return type;
}

Symbol Minus_class::checkType(){
    check_int(e1, e2);
    if (judge_type(e1->getType(), e2->getType()))
        setType(Float);
    else if (e1->getType() == Int && e2->getType() == Int)
        setType(Int);
    else
    {
        semant_error(this) << "Cannot minus a " << e1->getType() << " and a " << e2->getType() << ".\n";
        setType(Void);
    }
    return type;
}

Symbol Multi_class::checkType(){
    check_int(e1, e2);
    if (judge_type(e1->getType(), e2->getType()))
        setType(Float);
    else if (e1->getType() == Int && e2->getType() == Int)
        setType(Int);
    else
    {
        semant_error(this) << "Cannot multi a " << e1->getType() << " and a " << e2->getType() << ".\n";
        setType(Void);
    }
    return type;
}

Symbol Divide_class::checkType(){
    check_int(e1, e2);
    if (judge_type(e1->getType(), e2->getType()))
        setType(Float);
    else if (e1->getType() == Int && e2->getType() == Int)
        setType(Int);
    else
    {
        semant_error(this) << "Cannot div a " << e1->getType() << " and a " << e2->getType() << ".\n";
        setType(Void);
    }
    return type;
}

Symbol Mod_class::checkType(){
    check_int(e1, e2);
    if (e1->getType() == Int && e2->getType() == Int)
        setType(Int);
    else
    {
        semant_error(this) << "Cannot mod a " << e1->getType() << " and a " << e2->getType() << ".\n";
        setType(Void);
    }
    return type;
}

Symbol Neg_class::checkType(){
    check_int(e1);
    if (e1->getType() == Int || e1->getType() == Float)
        setType(e1->getType());
    else
    {
        semant_error(this) << "A " << e1->getType() << " doesn't have a negative.\n";
        setType(Void);
    }
    return type;
}

Symbol Lt_class::checkType(){
    check_int(e1, e2);
    if (judge_type_plus(e1->getType(), e2->getType()))
        setType(Bool);
    else
    {
        semant_error(this) << "Cannot compare a " << e1->getType() << " and a " << e2->getType() << ".\n";
        setType(Void);
    }
    return type;
}

Symbol Le_class::checkType(){
    check_int(e1, e2);
    if (judge_type_plus(e1->getType(), e2->getType()))
        setType(Bool);
    else
    {
        semant_error(this) << "Cannot compare a " << e1->getType() << " and a " << e2->getType() << ".\n";
        setType(Void);
    }
    return type;
}

Symbol Equ_class::checkType(){
    check_int(e1, e2);
    if ( (judge_type_plus(e1->getType(), e2->getType())) ||
         (e1->getType() == Bool && e2->getType() == Bool))
        setType(Bool);
    else
    {
        semant_error(this) << "Cannot compare a " << e1->getType() << " and a " << e2->getType() << ".\n";
        setType(Void);
    }
    return type;
}

Symbol Neq_class::checkType(){
    check_int(e1, e2);
    if ( (judge_type_plus(e1->getType(), e2->getType())) ||
         (e1->getType() == Bool && e2->getType() == Bool))
        setType(Bool);
    else
    {
        semant_error(this) << "Cannot compare a " << e1->getType() << " and a " << e2->getType() << ".\n";
        setType(Void);
    }
    return type;
}

Symbol Ge_class::checkType(){
    check_int(e1, e2);
    if (judge_type_plus(e1->getType(), e2->getType()))
        setType(Bool);
    else
    {
        semant_error(this) << "Cannot compare a " << e1->getType() << " and a " << e2->getType() << ".\n";
        setType(Void);
    }
    return type;
}

Symbol Gt_class::checkType(){
    check_int(e1, e2);
    if (judge_type_plus(e1->getType(), e2->getType()))
        setType(Bool);
    else
    {
        semant_error(this) << "Cannot compare a " << e1->getType() << " and a " << e2->getType() << ".\n";
        setType(Void);
    }
    return type;
}

Symbol And_class::checkType(){
    check_int(e1, e2);
    if (e1->getType() == Bool && e2->getType() == Bool)
        setType(Bool);
    else
    {
        semant_error(this) << "Cannot use && between " << e1->getType() << " and " << e2->getType() << ".\n";
        setType(Void);
    }
    return type;
}

Symbol Or_class::checkType(){
   check_int(e1, e2);
    if (e1->getType() == Bool && e2->getType() == Bool)
        setType(Bool);
    else
    {
        semant_error(this) << "Cannot use || between " << e1->getType() << " and " << e2->getType() << ".\n";
        setType(Void);
    }
    return type;
}

Symbol Xor_class::checkType(){
    check_int(e1, e2);
    if ((e1->getType() == Bool && e2->getType() == Bool) ||
        (e1->getType() == Int && e2->getType() == Int))
        setType(e1->getType());
    else
    {
        semant_error(this) << "Cannot use ^ between " << e1->getType() << " and " << e2->getType() << ".\n";
        setType(Void);
    }
    return type;
}

Symbol Not_class::checkType(){
    check_int(e1);
    if (e1->getType() == Bool)
        setType(Bool);
    else
    {
        semant_error(this) << "Cannot use ! upon " << e1->getType() << ".\n";
        setType(Void);
    }
    return type;
}

Symbol Bitand_class::checkType(){
    check_int(e1, e2);
    if (e1->getType() == Int && e2->getType() == Int)
        setType(Int);
    else
    {
        semant_error(this) << "Cannot use & between " << e1->getType() << " and " << e2->getType() << ".\n";
        setType(Void);
    }
    return type;
}

Symbol Bitor_class::checkType(){
    check_int(e1, e2);
    if (e1->getType() == Int && e2->getType() == Int)
        setType(Int);
    else
    {
        semant_error(this) << "Cannot use | between " << e1->getType() << " and " << e2->getType() << ".\n";
        setType(Void);
    }
    return type;
}

Symbol Bitnot_class::checkType(){
    check_int(e1);
    if (e1->getType() == Int)
        setType(Int);
    else
    {
        semant_error(this) << "Cannot use unary op ~ upon " << e1->getType() << ".\n";
        setType(Void);
    }
    return type;
}

Symbol Const_int_class::checkType(){
    setType(Int);
    return type;
}

Symbol Const_string_class::checkType(){
    setType(String);
    return type;
}

Symbol Const_float_class::checkType(){
    setType(Float);
    return type;
}

Symbol Const_bool_class::checkType(){
    setType(Bool);
    return type;
}

Symbol Object_class::checkType(){
    /*if (objectEnv.lookup(var) == NULL) {
        semant_error(this)<<"object "<<var<<" has not been defined."<<endl;
        this->setType(Void);
        return type;
    }*/
    if (lvar->lookup(var) != NULL)
        setType(*lvar->lookup(var));
    else if (va->lookup(var) != NULL)
        setType(*va->lookup(var));
    else if (fm->probe(var) != NULL)
        setType(*fm->probe(var));
    
    else
    {
        semant_error(this) << "object " << var << " has not been defined.\n";
        setType(Void);
    }
    // Symbol ty = localVars[var];
    return type;
}

Symbol No_expr_class::checkType(){
    setType(Void);
    return getType();
}

void Program_class::semant() {
    initialize_constants();
    install_calls(decls);
    check_main();
    install_globalVars(decls);
    check_calls(decls);
    
    if (semant_errors > 0) {
        cerr << "Compilation halted due to static semantic errors." << endl;
        exit(1);
    }
}



