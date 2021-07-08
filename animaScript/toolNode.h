#pragma once
#include "nodetype.h"
#include <set>

class copyHelp
{
public:
    static BasicNode* copyNode(BasicNode* node);
    static BasicNode* copyVal(BasicNode* node);
    static bool isLiteral(BasicNode* node);
    static bool isLiteral(int type);
    static void delTree(BasicNode* n);
};

class evalHelp
{
public:
    static void recursionEval(BasicNode*& node);
    static nodeType typeInfer(BasicNode*& node);
};

class VarNode : public BasicNode
{
protected:
    BasicNode* val = nullptr;
    bool typeRestrictFlag = true;
    nodeType valtype;
    bool ownershipFlag;

public:
    virtual nodeType getType() { return Var; }
    virtual void addNode(BasicNode*) { throw addSonExcep(Var); }
    virtual BasicNode* eval(); //字面量会拷贝
    virtual ~VarNode();
    VarNode(nodeType valtype);
    VarNode(VarNode& n);

    bool isEmpty() const { return (this->val == nullptr); }
    bool istypeRestrict() const { return this->typeRestrictFlag; }
    nodeType getValType() const { return this->valtype; }
    bool isOwnership() const { return this->ownershipFlag; }
    void setVal(BasicNode* val); //直接对值进行赋值，用这个传进来意味着转移所有权到本类（一般赋值为字面量用）
    void setBorrowVal(BasicNode* val); //直接对值进行赋值，用这个不转移所有权（一般赋值为变量指针用）
    void setVarVal(VarNode* node); //传递变量的值到this的值，即需要进行一次解包
    void clearVal();
    BasicNode* getVal() { return this->val; } //不会拷贝字面量
#ifdef READABLEGEN
    string NAME;
#endif
};
typedef VarNode Variable; //内存实体是Variable，其指针才作为节点（不像某些节点一样是遇到一个就new一次），参考函数实体和函数节点的思想


class ProNode : public BasicNode
{
protected:
    vector<bool> isRet;
public:
    virtual nodeType getType() { return Pro; }
    virtual BasicNode* eval();
    ProNode() {}
    ProNode(const ProNode& n) :BasicNode(n) {}
    //fix:该节点现在可以求值，实际应该做成逗号表达式一类的结构，支持PARTEVAL。但现在pro eval完了都释放，所以没啥用
    //BasicNode* getHeadNode() {return this->sonNode.at(0);}
    BasicNode* getSen(unsigned int sub) { return this->sonNode.at(sub); }
    virtual void addNode(BasicNode* node) { throw addSonExcep(Null); }
    void addNode(BasicNode* node, bool isRet); //ProNode用这个添加子节点才是对的
    set<nodeType> getRetType();
};


class Function
{
private:
    short parnum; //参数个数
    //基础求值（包装C++函数）
    vector<nodeType> parType;
    BE BEfun;
    nodeType retType;

public:
    Function(const vector<nodeType>& parType, BE BEfun, nodeType retType) :
        parnum(parType.size()), BEfun(BEfun), parType(parType), retType(retType) {}

    short getParnum() { return this->parnum; }
    const vector<nodeType>& getParType() { return this->parType; }
    nodeType getRetType() { return this->retType; }
    BasicNode* eval(vector<BasicNode*>& sonNode);

#ifdef READABLEGEN
    string NAME;
#endif
};

class FunNode : public BasicNode
{
protected:
    Function* funEntity; //所有权在scope，不在这里析构

public:
    virtual nodeType getType() { return Fun; }
    virtual void addNode(BasicNode* node);
    virtual BasicNode* eval();
    FunNode(Function* funEntity) :funEntity(funEntity) {}
    FunNode(const FunNode& n) :BasicNode(n) { this->funEntity = n.funEntity; } //函数实体所有权不在此，所以可以放心不复制

    const vector<nodeType>& getParType() { return this->funEntity->getParType(); }
    nodeType getRetType() { return this->funEntity->getRetType(); }
    bool haveEntity() { return this->funEntity != nullptr; }
    void setEntity(Function* funEntity) { this->funEntity = funEntity; }
    Function* getEntity() { return this->funEntity; }

#ifdef PARTEVAL
    bool giveupEval; //如果里边有符号变量，暂时放弃对此节点（基本为函数节点）的求值，并在此做标记防止根函数节点被视为求值结束而delete
    //所有控制流节点也要有该成员（若控制流条件中含有符号变量，放弃对整个控制流节点的执行（求值））
#endif
};

class AssignNode : public BasicNode
{
public:
    virtual nodeType getType() { return Assign; }
    virtual void addNode(BasicNode* node) { throw addSonExcep(Null); }
    virtual BasicNode* eval();
    AssignNode(BasicNode* n1, BasicNode* n2); //直接在构造时添加子节点
};


class conditionalControlNode : public BasicNode
{
protected:
    BasicNode* condition;
    BasicNode* evalCondition();
public:
    conditionalControlNode(BasicNode* condition) :condition(condition) {}
    conditionalControlNode(const conditionalControlNode& n) :BasicNode(n) {}

#ifdef PARTEVAL
    bool giveupEval;
#endif
};

class IfNode : public conditionalControlNode
{
protected:
    ProNode* truePro;
    ProNode* falsePro;
public:
    virtual nodeType getType() { return If; }
    virtual void addNode(BasicNode*) { throw addSonExcep(If); }
    virtual BasicNode* eval();
    IfNode(const IfNode& n);
    IfNode(BasicNode* condition, ProNode* truePro, ProNode* falsePro) :conditionalControlNode(condition), truePro(truePro), falsePro(falsePro) {}
    virtual ~IfNode();
};
