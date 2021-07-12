#include "toolNode.h"

BasicNode* copyHelp::copyVal(BasicNode* node) //��ֵ���ͣ�����
{
    //����ǰӦ�öԲ������ͽ��м��
    if (node->getType() == Num)
        return new NumNode(*dynamic_cast<NumNode*>(node));
    if (node->getType() == String)
        return new StringNode(*dynamic_cast<StringNode*>(node));
    if (node->getType() == Bool)
        return new BoolNode(*dynamic_cast<BoolNode*>(node));
    if (node->getType() == Vec4f)
        return new Vec4fNode(*dynamic_cast<Vec4fNode*>(node));
    if(node->getType()==Null)
        return new nullNode();
    //warn:֧�ָ���߿������캯�����ͣ�Ŀǰ����������������Ҫ�ڴ˴��������
    return nullptr; //������в�������˲����ߵ���һ��
}

BasicNode* copyHelp::copyNode(BasicNode* node) //���������ӽڵ㣬warn:֧���µĳ����ӽڵ����ͺ�Ҫ�����޸�
{
    if(copyHelp::isLiteral(node))
        return copyHelp::copyVal(node);
    if(node->getType()==Var) //Var����Ȩ���򣬴˴������и��ƣ�ֱ�ӷ���
        return node;
    if(node->getType()==Fun)
        return new FunNode(*dynamic_cast<FunNode*>(node));
    if(node->getType()==If)
        return new IfNode(*dynamic_cast<IfNode*>(node));
    throw string("The type is not regular son nodes to copy"); //Pro����Ϊ�����ӽڵ㣬���ڴ˿���
}

void copyHelp::delTree(BasicNode *n)
{
    for(BasicNode* node:n->sonNode)
    {
        if(node->getType()!=Var)
        {
            delete node;
            node=nullptr;
        }
    }
}

void copyHelp::delLiteral(BasicNode* n)
{
    if (n == nullptr)
        return;
    else if (copyHelp::isLiteral(n))
        delete n;
}

BasicNode::BasicNode(const BasicNode &n)
{
    for(BasicNode* i:n.sonNode)
        this->sonNode.push_back(copyHelp::copyNode(i));
}

VarNode::VarNode(VarNode &n):BasicNode(n)
{
    this->typeRestrictFlag=n.istypeRestrict();
    this->ownershipFlag=n.isOwnership();
    this->valtype=n.getValType();
    if(this->ownershipFlag) //������Ȩ������û��ֱ�Ӹ�ֵ
        this->val=copyHelp::copyVal(n.eval()); //�˴�����Ĭ�ϵĲ��ԡ���ֻ����������Ϊ������Ȩ��ֵ������ֱ�ӵ��ÿ����������ķ������п���
    else
        this->val=n.eval();
}
