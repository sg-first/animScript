#include "nodetype.h"
#include "toolNode.h"

bool copyHelp::isLiteral(int type) //warn:�Ƿ�Ϊ������������µ�������Ҫ�����޸�
{
    return (type==Num||type==String||type==Bool);
}

bool copyHelp::isLiteral(BasicNode* node)
{
    return copyHelp::isLiteral(node->getType());
}

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
    this->sonNode.push_back(node);
}

BasicNode* FunNode::eval()
{
#ifdef PARTEVAL
    this->giveupEval=false;
#endif

    if(this->funEntity==nullptr)
        throw Excep("funEntity is null");

#ifdef PARTEVAL
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
    n1->setVal(this->sonNode[1]->eval());
    return this->sonNode[1]->eval();
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

void evalHelp::recursionEval(BasicNode* &node)
{
    if(copyHelp::isLiteral(node))
        return; //��������������Լ�������ֵ��������������¸�ֵһ�ξ��ظ���
    else
    {
        BasicNode* result;
#ifdef PARTEVAL
        try
        {
#endif
            result=node->eval();
#ifdef PARTEVAL
        }
        catch(unassignedEvalExcep) //��δ��ֵ������ֵ������ԭ��
        {result=node;}
#endif

        if (node->getType() != Var)
#ifdef PARTEVAL
            if(isNotGiveupEval(node)) //�Է�����ֵ�Ľڵ㣬������ɾ��
#endif
                delete node;
        node=result; //�ڵ���滻��������ڵ㣩��ɣ��ӽڵ�ֻ��Ҫ���ؼ���
        //�����Ѿ���ֵ�ı����������������ֵ������������AST�е�λ�ã�������������û�б���������Ϊ����������Ȩ��scope��������ܻ�Ҫ���ʣ�
    }
}

nodeType evalHelp::typeInfer(BasicNode*& node)
{
    if (copyHelp::isLiteral(node))
        return node->getType();
    else if (node->getType() == Fun)
        return dynamic_cast<FunNode*>(node)->getRetType();
    else if (node->getType() == Var)
        return dynamic_cast<VarNode*>(node)->getValType();
    else
        return Null;
}

BasicNode* Function::eval(vector<BasicNode*> &sonNode)
{
    //�����в�����ֵ
    for (short i = 0;i < sonNode.size();i++)
        evalHelp::recursionEval(sonNode[i]);
    return this->BEfun(sonNode);
}

void ProNode::addNode(BasicNode* node, bool isRet)
{
    BasicNode::addNode(node);
    this->isRet.push_back(isRet);
}

BasicNode* ProNode::eval()
{
    vector<BasicNode*>& body = this->sonNode;
    for (unsigned int i = 0;i < body.size();i++)
    {
        evalHelp::recursionEval(body.at(i));
        if (this->isRet.at(i))
            return body.at(i);
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

BasicNode* conditionalControlNode::evalCondition()
{
#ifdef PARTEVAL
    this->giveupEval=false;
#endif

    BasicNode* recon;
#ifdef PARTEVAL
    try
    {
#endif
        recon=this->condition->eval();
#ifdef PARTEVAL
    }
    catch(unassignedEvalExcep) //conditionֱ�Ӿ��Ǹ����ű�����������ֵ��������
    {throw Excep("conditionalControlNode return");}
#endif

#ifdef PARTEVAL
    if(recon->getType()==Fun&&dynamic_cast<FunNode*>(recon)->giveupEval) //��һ�����������з�����ֵ�ı���
    {
        this->giveupEval=true; //���������ڵ�Ҳ������ֵ
        throw Excep("conditionalControlNode return");
    }
#endif

    return recon;
}

BasicNode* IfNode::eval()
{
    BasicNode* recon;
#ifdef PARTEVAL
    try
    {
#endif
        recon=this->evalCondition();
#ifdef PARTEVAL
    }
    catch(string e)
    {
        if(e=="conditionalControlNode return")
            return this; //������ֵ��ֱ�ӷ���
        else
            throw e;
    }
#endif

    if(recon->getType()!=Num)
        throw Excep("IfNode condition value's type mismatch");
    BasicNode* result;
    if(dynamic_cast<NumNode*>(recon)->getData()==0) //�����ж�false
        result=this->falsePro->eval();
    else
        result=this->truePro->eval();

    delete recon;
    return result;
}

IfNode::~IfNode()
{
    delete this->condition;
    delete this->truePro;
    delete this->falsePro;
}
