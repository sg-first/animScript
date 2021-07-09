#pragma once
#include "toolNode.h"
#include "scope.h"
#include <optional>

typedef pair<string, short> token;

#define REDUCE(SYMBOL, CODE) \
    {this->reduce(); \
    this->i++; \
    this->allToken.push_back(token(SYMBOL,CODE)); \
    return true;}

class program
{
private:
    vector<Variable*> paraList;
    ProNode* body;
    Scope* s;

public:
    program(vector<BasicNode*> paraList, ProNode* body, Scope* s) : body(body), s(s)
    {
        auto retType = body->getRetType();
        if (retType.size() != 1 || *(retType.begin()) != Num)
            throw string("Program return value must be Num");
        else
        {
            for (auto i : paraList)
                this->paraList.push_back((Variable*)i);
        }
    }

    BasicNode* call(vector<BasicNode*> arguments)
    {
        for (short i = 0;i < paraList.size();i++)
            paraList[i]->setVal(arguments[i]);
        return this->body->eval();
    }

    ~program()
    {
        delete body;
        //delete s;
    }
};

class buffer
{
private:
    static bool isNum(char c)
    {
        return c >= '0' && c <= '9';
    }

    static bool isNum(string s)
    {
        for (auto i : s)
        {
            if (!buffer::isNum(i))
                return false;
        }
        return true;
    }

    static bool isLetter(char c)
    {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z' || c=='_');
    }

    static bool isLetter(string s)
    {
        if (buffer::isLetter(s[0]))
        {
            for (size_t i = 1;i < s.size();i++)
            {
                if (!buffer::isLetter(s[i]) && !buffer::isNum(s[i]))
                    return false;
            }
            return true;
        }
        else
            return false;
    }

    bool isKeyword(vector<token>& allToken)
    {
        /*
        if (this->buffer == "if")
        {
            allToken.push_back(token("if", 2));
            this->clear();
            return true;
        }
        
        if (this->buffer == "then")
        {
            allToken.push_back(token("then", 3));
            this->clear();
            return true;
        }
        */
        if (this->buffer == "return")
        {
            allToken.push_back(token("return", 1));
            this->clear();
            return true;
        }
        else if (this->buffer == "true")
        {
            allToken.push_back(token("true", 30));
            this->clear();
            return true;
        }
        else if (this->buffer == "false")
        {
            allToken.push_back(token("false", 30));
            this->clear();
            return true;
        }
        else if (this->buffer == "Num")
        {
            allToken.push_back(token("Num", 29));
            this->clear();
            return true;
        }
        else if (this->buffer == "Bool")
        {
            allToken.push_back(token("Bool", 29));
            this->clear();
            return true;
        }
        //支持新类型需要在这里修改lexer
        return false;
    }

public:
    string buffer;

    void add(char c) { this->buffer += string(1, c); }
    void clear() { this->buffer = ""; }
    void reduce(vector<token>& allToken)
    {
        if (this->buffer == "")
            return;
        else if (this->isKeyword(allToken))
            return;
        else
        {
            //判断数字
            if (buffer::isNum(this->buffer))
            {
                allToken.push_back(token(this->buffer, 11));
                this->clear();
                return;
            }
            //判断letter
            else if (buffer::isLetter(this->buffer))
            {
                allToken.push_back(token(this->buffer, 10));
                this->clear();
                return;
            }
            else
                throw lexicalExcep("reduce error - " + this->buffer);
        }
    }
};

#define CHECKNULL(VAR) if(VAR==nullptr) return nullptr;

class lexer
{
private:
    string program;
    unsigned int i = 0;
    buffer b;
    void reduce() { this->b.reduce(allToken); }

    static bool test2Str(string s1, string s2, unsigned int& i)
    {
        for (int j = 0;j < s2.size();j++)
        {
            if (s1[j + i] != s2[j])
                return false;
        }
        //暂时不直接pushback
        i = i + s2.size();
        return true;
    }

    bool isOp()
    {
        if (this->program[i] == '+')
            REDUCE("+", 13)
        else if (this->program[i] == '-')
            REDUCE("-", 13)
        else if (this->program[i] == '*')
            REDUCE("*", 13)
        else if (this->program[i] == '/')
            REDUCE("/", 13)
        else if (this->program[i] == '>')
            REDUCE(">", 20)
        else if (this->program[i] == '<')
            REDUCE("<", 20)
        else if (lexer::test2Str(this->program, "<=", this->i))
        {
            this->reduce();
            this->allToken.push_back(token("<=", 20));
            return true;
        }
        else if (lexer::test2Str(this->program, ">=", this->i))
        {
            this->reduce();
            this->allToken.push_back(token(">=", 20));
            return true;
        }
        else if (lexer::test2Str(this->program, "==", this->i))
        {
            this->reduce();
            this->allToken.push_back(token("==", 20));
            return true;
        }
        else if (lexer::test2Str(this->program, "!=", this->i))
        {
            this->reduce();
            this->allToken.push_back(token("!=", 20));
            return true;
        }
        else if (lexer::test2Str(this->program, "||", this->i))
        {
            this->reduce();
            this->allToken.push_back(token("||", 21));
            return true;
        }
        else if (lexer::test2Str(this->program, "&&", this->i))
        {
            this->reduce();
            this->allToken.push_back(token("&&", 21));
            return true;
        }
        else
            return false;
    }

    bool isTerminator()
    {
        if (this->program[i] == ' ')
        {
            //看看是不是return语句
            if (this->b.buffer == "return")
            {
                this->reduce();
                this->i++; //消耗掉当前的空格
                //后面的exp留给常规流程处理
                return true;
            }
        }
        else if (this->program[i] == ';')
            REDUCE(";", 26)
        else if (this->program[i] == '=' && this->program[i+1] != '=')
            REDUCE("=", 18)
        else if (this->program[i] == ',')
            REDUCE(",", 4)
        else if (this->program[i] == ':')
            REDUCE(":", 2)
        else if (this->program[i] == '?')
            REDUCE("?", 3)
        else if (this->program[i] == '(')
            REDUCE("(", 27)
        else if (this->program[i] == ')')
            REDUCE(")", 27)
        else if (lexer::test2Str(this->program, "->", this->i))
        {
            this->reduce();
            this->allToken.push_back(token("->", 17));
            return true;
        }
        else if (this->i == program.size())
        {
            this->reduce();
            this->i++;
            return true;
        }
        else
            return this->isOp();
    }

public:
    vector<token> allToken;
    lexer(string program) : program(program) {}

    void run()
    {
        while (this->i <= this->program.size())
        {
            if (!this->isTerminator()) //这里面会消耗字符
            {
                if (this->program[i] != ' ' && this->program[i] != '\n')
                    this->b.add(this->program[i]);
                this->i++;
            }
        }
    }
};

class parser
{
private:
    static const short LETTER = 10;
    static const short TYPE = 29;
    static const short NUMLiteral = 11;
    static const short BOOLLiteral = 30;

    unsigned int i = 0;
    Scope* scope;

    static bool isOp(token t)
    {
        return t.second == 13 || t.second == 20 || t.second == 21;
    }

    static bool isLiteral(token t)
    {
        return t.second == NUMLiteral || t.second == BOOLLiteral;
    }

    BasicNode* _exp()
    {
        BasicNode* result;
        if (allToken[i].first == "(") //括号开头
        {
            this->i++;
            result = this->_exp();
            this->i++; //消耗掉右括号
        }
        else if (isLiteral(allToken[i])) //字面量
        {
            string literal = allToken[i].first;
            if (allToken[i].second == NUMLiteral)
                result = new NumNode(atof(literal.c_str()));
            else if (allToken[i].second == BOOLLiteral)
            {
                if (literal == "true")
                    result = new BoolNode(true);
                else
                    result = new BoolNode(false);
            }
            else
                throw parseExcep("_exp literal case - Literal not added");
            this->i++;
        }
        else if (allToken[i].second == LETTER) //标识符开头
        {
            result = this->scope->findVariable(allToken[i].first);
            if (result != nullptr) //变量
            {
                this->i++;
            }
            else
            {
                Function* f = this->scope->findFunction(allToken[i].first);
                if (f == nullptr)
                    throw parseExcep("_exp identifier case - Undefined identifier");
                this->i++;
                result = new FunNode(f);
                auto sonNode = this->_ParaList();
                for (BasicNode* i : sonNode)
                    result->addNode(i);
            }
        }
        else
            throw parseExcep("_exp - Unexpected beginning of expression");

        if (isOp(allToken[i])) //后面接运算符
        {
            //构造运算符对应函数节点
            Function* op = this->scope->findFunction(allToken[i].first);
            if (allToken[i].first == "==" || allToken[i].first == "!=") //bool版等于不等于得特殊处理下
            {
                if (evalHelp::typeInfer(result) == Bool)
                    op = this->scope->findFunction(allToken[i].first + "Bool");
            }
            if (op == nullptr)
                throw parseExcep("_exp - The function entity corresponding to the operator was not found");
            this->i++;
            FunNode* fn = new FunNode(op);
            //插子节点
            fn->addNode(result);
            auto expRet = this->_exp();
            fn->addNode(expRet);
            return fn;
        }
        else if (allToken[i].first == "?")
        {
            this->i++;
            //result是条件，后面还有true和false两个exp
            auto trueExp = this->_exp();
            ProNode* truePro = new ProNode;
            truePro->addNode(trueExp, true);
            this->i++; //越过:
            auto falseExp = this->_exp();
            ProNode* falsePro = new ProNode;
            falsePro->addNode(falseExp, true);

            auto in = new IfNode(result, truePro, falsePro);
            return in;
        }
        else
        {
            return result;
        }
    }

    vector<BasicNode*> _headParaList()
    {
        if (allToken[i].first == "(")
        {
            this->i++;
            vector<BasicNode*> result;
            while (allToken[i].first != ")")
            {
                string varName = allToken[i].first;
                this->i += 2; //越过中间的:，到类型标签位置
                Variable* v;
                if (allToken[i].first == "Num")
                {
                    v = new Variable(Num);
                    this->scope->addVariable(varName, v);
                }
                else if (allToken[i].first == "Bool")
                {
                    v = new Variable(Bool);
                    this->scope->addVariable(varName, v);
                }
                else
                    throw parseExcep("_headParaList - Unrecognized type tag");
                //支持新类型需要在这里修改parser
                result.push_back(v);
                this->i++; //越过类型标签
                if (allToken[i].first == ",")
                    this->i++;
            }
            this->i++;
            return result;
        }
        else
            throw parseExcep("_headParaList - ( not found");
    }

    vector<BasicNode*> _ParaList()
    {
        if (allToken[i].first == "(")
        {
            this->i++;
            vector<BasicNode*> result;
            while (allToken[i].first != ")")
            {
                auto expRet = this->_exp();
                result.push_back(expRet);
                if (allToken[i].first == ",")
                    this->i++; //消耗掉后面的逗号
            }
            this->i++;
            return result;
        }
        else
            throw parseExcep("_headParaList - ( not found");
    }

    BasicNode* _statementBlock()
    {
        ProNode* n = new ProNode;
        for (;this->i != allToken.size();this->i++) //自增用于跳语句结尾分号
        {
            if (allToken[i].first == "return")
            {
                //return语句
                this->i++;
                auto expRet = this->_exp();
                n->addNode(expRet, true);
            }
            else
            {
                //赋值语句
                if (allToken[i].second == LETTER)
                {
                    string varName = allToken[i].first;
                    auto leftVal = this->scope->findVariable(varName);
                    this->i++;
                    if (allToken[i].first == "=")
                    {
                        this->i++;
                        auto expRet = this->_exp();
                        if (leftVal == nullptr) //变量原先不存在，需要定义
                        {
                            leftVal = new Variable(evalHelp::typeInfer(expRet));
                            this->scope->addVariable(varName, leftVal);
                        }
                        //弄个赋值节点
                        auto an = new AssignNode(leftVal, expRet);
                        n->addNode(an, false);
                    }
                }
                else
                    throw parseExcep("_statementBlock assignment case - The assigned object must be a identifier");
            }
        }
        return n;
    }

    program _program()
    {
        vector<BasicNode*> paraList = this->_headParaList();
        if (allToken[i].first == "->")
        {
            this->i++;
            ProNode* body = (ProNode*)(this->_statementBlock());
            return program(paraList, body, this->scope);
        }
        else
            throw parseExcep("_program - Program syntax must be: (parameter list) -> function body");
    }

public:
    vector<token> allToken;
    parser(vector<token> allToken, Scope* scope) : allToken(allToken), scope(scope) {}
    program run() { return this->_program(); }

    static program creatProgram(string code, Scope* scope)
    {
        //Scope* scope = new Scope;
        lexer l(code);
        l.run();
        parser p(l.allToken, scope);
        return p.run();
    }
};
