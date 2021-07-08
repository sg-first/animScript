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
    virtual BasicNode* eval(); //�������´��
    virtual ~VarNode();
    VarNode(nodeType valtype);
    VarNode(VarNode& n);

    bool isEmpty() const { return (this->val == nullptr); }
    bool istypeRestrict() const { return this->typeRestrictFlag; }
    nodeType getValType() const { return this->valtype; }
    bool isOwnership() const { return this->ownershipFlag; }
    void setVal(BasicNode* val); //ֱ�Ӷ�ֵ���и�ֵ���������������ζ��ת������Ȩ�����ࣨһ�㸳ֵΪ�������ã�
    void setBorrowVal(BasicNode* val); //ֱ�Ӷ�ֵ���и�ֵ���������ת������Ȩ��һ�㸳ֵΪ����ָ���ã�
    void setVarVal(VarNode* node); //���ݱ�����ֵ��this��ֵ������Ҫ����һ�ν��
    void clearVal();
    BasicNode* getVal() { return this->val; } //���´��������
#ifdef READABLEGEN
    string NAME;
#endif
};
typedef VarNode Variable; //�ڴ�ʵ����Variable����ָ�����Ϊ�ڵ㣨����ĳЩ�ڵ�һ��������һ����newһ�Σ����ο�����ʵ��ͺ����ڵ��˼��


class ProNode : public BasicNode
{
protected:
    vector<bool> isRet;
public:
    virtual nodeType getType() { return Pro; }
    virtual BasicNode* eval();
    ProNode() {}
    ProNode(const ProNode& n) :BasicNode(n) {}
    //fix:�ýڵ����ڿ�����ֵ��ʵ��Ӧ�����ɶ��ű��ʽһ��Ľṹ��֧��PARTEVAL��������pro eval���˶��ͷţ�����ûɶ��
    //BasicNode* getHeadNode() {return this->sonNode.at(0);}
    BasicNode* getSen(unsigned int sub) { return this->sonNode.at(sub); }
    virtual void addNode(BasicNode* node) { throw addSonExcep(Null); }
    void addNode(BasicNode* node, bool isRet); //ProNode���������ӽڵ���ǶԵ�
    set<nodeType> getRetType();
};


class Function
{
private:
    short parnum; //��������
    //������ֵ����װC++������
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
    Function* funEntity; //����Ȩ��scope��������������

public:
    virtual nodeType getType() { return Fun; }
    virtual void addNode(BasicNode* node);
    virtual BasicNode* eval();
    FunNode(Function* funEntity) :funEntity(funEntity) {}
    FunNode(const FunNode& n) :BasicNode(n) { this->funEntity = n.funEntity; } //����ʵ������Ȩ���ڴˣ����Կ��Է��Ĳ�����

    const vector<nodeType>& getParType() { return this->funEntity->getParType(); }
    nodeType getRetType() { return this->funEntity->getRetType(); }
    bool haveEntity() { return this->funEntity != nullptr; }
    void setEntity(Function* funEntity) { this->funEntity = funEntity; }
    Function* getEntity() { return this->funEntity; }

#ifdef PARTEVAL
    bool giveupEval; //�������з��ű�������ʱ�����Դ˽ڵ㣨����Ϊ�����ڵ㣩����ֵ�����ڴ�����Ƿ�ֹ�������ڵ㱻��Ϊ��ֵ������delete
    //���п������ڵ�ҲҪ�иó�Ա���������������к��з��ű����������������������ڵ��ִ�У���ֵ����
#endif
};

class AssignNode : public BasicNode
{
public:
    virtual nodeType getType() { return Assign; }
    virtual void addNode(BasicNode* node) { throw addSonExcep(Null); }
    virtual BasicNode* eval();
    AssignNode(BasicNode* n1, BasicNode* n2); //ֱ���ڹ���ʱ����ӽڵ�
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
