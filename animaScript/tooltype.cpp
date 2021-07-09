#include "nodetype.h"
#include "toolNode.h"
#include <algorithm>
#include <iterator>

bool isNotAssignable(BasicNode* val) //warn:�Ƿ񲻿ɸ�ֵ��������֧���µĽڵ�����Ҫ�����޸�
{
    return (val->getType()==Pro||val->getType()==Fun||val->getType()==If);
    //fix:Ŀǰ�ݲ�֧�ֺ���ָ�룬��Ϊ����ʵ��ı�����ʾ��û��ƺ�
}


BasicNode::~BasicNode()
{
    copyHelp::delTree(this);
}


VarNode::VarNode(nodeType valtype)
{
    this->valtype=valtype;
    if(valtype!=-1)
        this->typeRestrictFlag=true;
}

VarNode::~VarNode()
{
    if((!this->isEmpty())&&this->ownershipFlag)
        delete this->val;
    //Ȼ��BasicNode����
}

void assignmentChecking(BasicNode *val,bool thisTypeRestrictFlag,nodeType thisvaltype)
{
    if(isNotAssignable(val))
        throw cannotAssignedExcep();
    if (thisTypeRestrictFlag && val->getType() != thisvaltype)
        throw callCheckMismatchExcep(TypeMisMatch);
}

void VarNode::setVal(BasicNode* val)
{
    assignmentChecking(val,this->typeRestrictFlag,this->getValType());
    this->clearVal();
    //warn:�����Ͻ�����Ŀǰ����ƣ�������Ӧ��Ϊ��������Ȩ��ֵ����Ϊ����Ȩ������ʱ�򣩣����ڴ��ݲ����м�顣������м�飬ֱ���ڴ˴����
    this->valtype=val->getType();
    this->ownershipFlag=true;
    this->val=val;
}

void VarNode::setBorrowVal(BasicNode *val)
{
    assignmentChecking(val, this->typeRestrictFlag, this->getValType());
    this->clearVal();
    this->valtype=val->getType();
    this->ownershipFlag=false;
    this->val=val;
}

void VarNode::setVarVal(VarNode *node)
{
    if(node->isEmpty())
        throw unassignedAssignedExcep();
    this->clearVal();
    BasicNode* oriVal=node->eval();
    //Ŀǰ����Ϊ�����������п�����������Ȩ����������Ϊ������Ȩָ�봫��
    if(copyHelp::isLiteral(oriVal))
        this->setVal(copyHelp::copyVal(oriVal));
    if(oriVal->getType()==Var)
        this->setBorrowVal(oriVal);
    //fix:֧�ֺ���ָ��֮����Ҫ�ڴ˴��������
}

BasicNode* VarNode::eval()
{
    if(this->isEmpty())
        throw unassignedEvalExcep();
    else
    {
        if(copyHelp::isLiteral(this->val))
            return copyHelp::copyVal(this->val);
        else
            return this->val;
    }
    //ע�⣬�༶ָ��Ҳֻ����һ�Ρ������ӷ���ֵ�����޷��жϷ��ص�������������ı���ָ��ֵ��������ǰ��ҪgetValType�����ж�
}

void VarNode::clearVal()
{
    if(this->ownershipFlag)
    {
        delete this->val;
        this->val=nullptr;
    }
}


void FunNode::addNode(BasicNode *node)
{
    if (this->sonNode.size() + 1 > this->funEntity->getParnum()) //�����������
        throw parameterNumExceedingExcep();
    if (this->getParType()[sonNode.size()] != evalHelp::typeInfer(node)) //���ͼ��
        throw callCheckMismatchExcep(TypeMisMatch);
    BasicNode::addNode(node);
}

BasicNode* FunNode::eval()
{
#ifdef PARTEVAL
    this->giveupEval=false;

    try
    {
#endif
        return this->funEntity->eval(this->sonNode);
#ifdef PARTEVAL
    }
    catch(callCheckMismatchExcep e) //��Ϊδ��ֵ����δ��ֵʹ�ò������Ͳ�ƥ�䣬���������������ֵ
            //�������ڵ����������ֵ���ڴ˴����У��ýڵ������ֵ�ᱻ�ϲ�������ڵ��鵽���������ڵ�Ҳ�������ֵ
    {
        if(e.getType()==TypeMisMatch)
        {
            this->giveupEval=true;
            return this;
        }
        else
            throw e;
    }
#endif
}


AssignNode::AssignNode(BasicNode* n1, BasicNode* n2)
{
    if (n1->getType() == Var)
    {
        if (evalHelp::typeInfer(n1) == evalHelp::typeInfer(n2))
        {
            BasicNode::addNode(n1);
            BasicNode::addNode(n2);
        }
    }
    else
        throw callCheckMismatchExcep(TypeMisMatch);
}

BasicNode* AssignNode::eval()
{
    VarNode* n1 = dynamic_cast<VarNode*>(this->sonNode[0]);
    n1->setVal(evalHelp::literalCopyEval(this->sonNode[1]));
    //return this->sonNode[1]->eval();
    return nullptr;
}

#ifdef PARTEVAL
bool isNotGiveupEval(BasicNode* node)
{
    if(!(node->getType()==Fun&&dynamic_cast<FunNode*>(node)->giveupEval))
        if(!(node->getType()==If&&dynamic_cast<IfNode*>(node)->giveupEval))
                //warn:֧���µĿ������ڵ��Ҫ�ڴ˴����
                return true;
    return false;
}
#endif

BasicNode* evalHelp::literalCopyEval(BasicNode* node)
{
    if (copyHelp::isLiteral(node))
        return copyHelp::copyVal(node->eval());
    else
        return node->eval();
}

nodeType evalHelp::typeInfer(BasicNode*& node)
{
    if (copyHelp::isLiteral(node))
        return node->getType();
    else if (node->getType() == Fun)
        return dynamic_cast<FunNode*>(node)->getRetType();
    else if (node->getType() == Var)
        return dynamic_cast<VarNode*>(node)->getValType();
    //�����Ǿ�̬����ʱ����Ч���Ƶ�������ֵ����Ψһʱ����ȷ��
    else if (node->getType() == Pro)
        return *(dynamic_cast<ProNode*>(node)->getRetType().begin());
    else if (node->getType() == If)
        return *(dynamic_cast<IfNode*>(node)->getRetType().begin());
    else
    throw Excep("This node type cannot infer");
}

set<nodeType> evalHelp::unionTypeInfer(BasicNode*& node)
{
    if (node->getType() == Pro)
        return dynamic_cast<ProNode*>(node)->getRetType();
    else if (node->getType() == If)
        return dynamic_cast<IfNode*>(node)->getRetType();
    else
    {
        set<nodeType> result;
        result.insert(evalHelp::typeInfer(node));
        return result;
    }
}

BasicNode* Function::eval(vector<BasicNode*> sonNode) const
{
    for (short i = 0;i < sonNode.size();i++)
        sonNode[i] = evalHelp::literalCopyEval(sonNode[i]);
    auto result = this->BEfun(sonNode);
    for (auto i : sonNode)
        copyHelp::delLiteral(i);
    return result;
}

void ProNode::addNode(BasicNode* node, bool isRet)
{
    BasicNode::addNode(node);
    this->isRet.push_back(isRet);
}

BasicNode* ProNode::eval()
{
    const vector<BasicNode*>& body = this->sonNode;
    for (unsigned int i = 0;i < body.size();i++)
    {
        auto r = evalHelp::literalCopyEval(body.at(i));
        if (this->isRet.at(i))
            return r;
        else
            copyHelp::delLiteral(r);
    }
    return nullptr;
}

set<nodeType> ProNode::getRetType()
{
    set<nodeType> result;
    vector<BasicNode*>& body = this->sonNode;
    for (unsigned int i = 0;i < body.size();i++)
    {
        if (this->isRet.at(i))
            result.insert(evalHelp::typeInfer(body.at(i)));
    }
    return result;
}

IfNode::IfNode(BasicNode* condition, BasicNode* truePro, BasicNode* falsePro)
{
    if(evalHelp::typeInfer(condition)!=Bool)
        throw Excep("IfNode condition value's type must be Bool");
    else if(truePro->getType()!=Pro || falsePro->getType()!=Pro)
        throw Excep("IfNode true/falsePro value's type must be Pro");
    else
    {
        BasicNode::addNode(condition);
        BasicNode::addNode(truePro);
        BasicNode::addNode(falsePro);
    }
}

BasicNode* IfNode::eval()
{
    BasicNode* recon;
#ifdef PARTEVAL
    try
    {
#endif
        recon = evalHelp::literalCopyEval(this->sonNode[0]);
#ifdef PARTEVAL
    }
    catch(string e) //fix:���Ҫָ������PARTEVAL�������쳣������������ϲ�throw
    {
        return this; //������ֵ��ֱ�ӷ���
    }
#endif

    BasicNode* result;
    if(dynamic_cast<BoolNode*>(recon)->getData())
        result= evalHelp::literalCopyEval(this->sonNode[1]);
    else
        result= evalHelp::literalCopyEval(this->sonNode[2]);

    copyHelp::delLiteral(recon);
    return result;
}

set<nodeType> IfNode::getRetType() 
{
    auto s1 = dynamic_cast<ProNode*>(sonNode[1])->getRetType();
    auto s2 = dynamic_cast<ProNode*>(sonNode[2])->getRetType();
    set<nodeType> result;
    set_union(s1.begin(), s1.end(), s2.begin(), s2.end(), inserter(result, result.begin()));
    return result;
}