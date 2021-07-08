#include "nodetype.h"
#include "toolNode.h"

bool copyHelp::isLiteral(int type) //warn:是否为字面量，添加新的字面量要进行修改
{
    return (type==Num||type==String||type==Bool);
}

bool copyHelp::isLiteral(BasicNode* node)
{
    return copyHelp::isLiteral(node->getType());
}

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
    n1->setVal(this->sonNode[1]->eval());
    return this->sonNode[1]->eval();
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

void evalHelp::recursionEval(BasicNode* &node)
{
    if(copyHelp::isLiteral(node))
        return; //如果是字面量，自己就是求值结果，下面再重新赋值一次就重复了
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
        catch(unassignedEvalExcep) //对未赋值变量求值，保持原样
        {result=node;}
#endif

        if (node->getType() != Var)
#ifdef PARTEVAL
            if(isNotGiveupEval(node)) //对放弃求值的节点，不进行删除
#endif
                delete node;
        node=result; //节点的替换在这里（父节点）完成，子节点只需要返回即可
        //对于已经赋值的变量，整体过程是用值替代本身变量在AST中的位置，不过变量本身并没有被析构，因为变量的所有权在scope（后面可能还要访问）
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
    //对所有参数求值
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
    catch(unassignedEvalExcep) //condition直接就是个符号变量，放弃求值返回自身
    {throw Excep("conditionalControlNode return");}
#endif

#ifdef PARTEVAL
    if(recon->getType()==Fun&&dynamic_cast<FunNode*>(recon)->giveupEval) //是一个函数里面有放弃求值的变量
    {
        this->giveupEval=true; //本控制流节点也放弃求值
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
            return this; //放弃求值，直接返回
        else
            throw e;
    }
#endif

    if(recon->getType()!=Num)
        throw Excep("IfNode condition value's type mismatch");
    BasicNode* result;
    if(dynamic_cast<NumNode*>(recon)->getData()==0) //这里判断false
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
