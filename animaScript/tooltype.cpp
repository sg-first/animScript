#include "nodetype.h"
#include "toolNode.h"
#include <algorithm>
#include <iterator>

bool isNotAssignable(BasicNode* val) //warn:是否不可赋值给变量，支持新的节点类型要进行修改
{
    return (val->getType()==Pro||val->getType()==Fun||val->getType()==If);
    //fix:目前暂不支持函数指针，因为函数实体的变量表示还没设计好
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
    //然后BasicNode析构
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
    //warn:理论上讲按照目前的设计，变量不应作为具有所有权的值（因为所有权在运行时域），但在此暂不进行检查。如果进行检查，直接在此处添加
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
    //目前策略为：字面量进行拷贝（有所有权），变量作为无所有权指针传递
    if(copyHelp::isLiteral(oriVal))
        this->setVal(copyHelp::copyVal(oriVal));
    if(oriVal->getType()==Var)
        this->setBorrowVal(oriVal);
    //fix:支持函数指针之后还需要在此处进行添加
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
    //注意，多级指针也只会解包一次。不过从返回值基本无法判断返回的是自身还是自身的变量指针值，所以先前需要getValType进行判断
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
    if (this->sonNode.size() + 1 > this->funEntity->getParnum()) //参数个数检查
        throw parameterNumExceedingExcep();
    if (this->getParType()[sonNode.size()] != evalHelp::typeInfer(node)) //类型检查
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
    catch(callCheckMismatchExcep e) //因为未赋值变量未求值使得参数类型不匹配，放弃对这个函数求值
            //控制流节点对条件的求值会在此处进行，该节点放弃求值会被上层控制流节点检查到，控制流节点也会放弃求值
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
                //warn:支持新的控制流节点后要在此处添加
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
    //以下是静态类型时才有效的推导（返回值类型唯一时才正确）
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
    catch(string e) //fix:这个要指定捕获PARTEVAL引发的异常，其它情况向上层throw
    {
        return this; //放弃求值，直接返回
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