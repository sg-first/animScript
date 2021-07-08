#pragma once
#include "toolNode.h"

class BuiltinFunc
{
private:
    static double getNum(BasicNode* node)
    {
        return dynamic_cast<NumNode*>(node)->getData();
    }

    static double getBool(BasicNode* node)
    {
        return dynamic_cast<BoolNode*>(node)->getData();
    }

public:
    static BasicNode* add(vector<BasicNode*>& sonNode)
    {
        return new NumNode(getNum(sonNode[0]) + getNum(sonNode[1]));
    }

    static BasicNode* sub(vector<BasicNode*>& sonNode)
    {
        return new NumNode(getNum(sonNode[0]) - getNum(sonNode[1]));
    }

    static BasicNode* mul(vector<BasicNode*>& sonNode)
    {
        return new NumNode(getNum(sonNode[0]) * getNum(sonNode[1]));
    }

    static BasicNode* div(vector<BasicNode*>& sonNode)
    {
        return new NumNode(getNum(sonNode[0]) / getNum(sonNode[1]));
    }

    static BasicNode* pow(vector<BasicNode*>& sonNode)
    {
        return new NumNode(std::pow(getNum(sonNode[0]), getNum(sonNode[1])));
    }

    static BasicNode* sin(vector<BasicNode*>& sonNode)
    {
        return new NumNode(std::sin(getNum(sonNode[0])));
    }

    static BasicNode* cos(vector<BasicNode*>& sonNode)
    {
        return new NumNode(std::cos(getNum(sonNode[0])));
    }

    static BasicNode* log(vector<BasicNode*>& sonNode)
    {
        return new NumNode(std::log(getNum(sonNode[1])) / std::log(getNum(sonNode[0])));
    }

    static BasicNode* ln(vector<BasicNode*>& sonNode)
    {
        return new NumNode(std::log(getNum(sonNode[0])));
    }

    static BasicNode* greater(vector<BasicNode*>& sonNode)
    {
        return new NumNode(getNum(sonNode[0]) > getNum(sonNode[1]));
    }

    static BasicNode* greaterEqual(vector<BasicNode*>& sonNode)
    {
        return new NumNode(getNum(sonNode[0]) >= getNum(sonNode[1]));
    }

    static BasicNode* less(vector<BasicNode*>& sonNode)
    {
        return new NumNode(getNum(sonNode[0]) < getNum(sonNode[1]));
    }

    static BasicNode* lessEqual(vector<BasicNode*>& sonNode)
    {
        return new NumNode(getNum(sonNode[0]) <= getNum(sonNode[1]));
    }

    static BasicNode* equal(vector<BasicNode*>& sonNode)
    {
        if(sonNode[0]->getType()==Num)
            return new NumNode(getNum(sonNode[0]) == getNum(sonNode[1]));
        else
            return new BoolNode(getBool(sonNode[0]) == getBool(sonNode[1]));
    }

    static BasicNode* notEqual(vector<BasicNode*>& sonNode)
    {
        if (sonNode[0]->getType() == Num)
            return new NumNode(getNum(sonNode[0]) != getNum(sonNode[1]));
        else
            return new BoolNode(getBool(sonNode[0]) != getBool(sonNode[1]));
    }

    static BasicNode* And(vector<BasicNode*>& sonNode)
    {
        return new BoolNode(getBool(sonNode[0]) && getBool(sonNode[1]));
    }

    static BasicNode* Or(vector<BasicNode*>& sonNode)
    {
        return new BoolNode(getBool(sonNode[0]) || getBool(sonNode[1]));
    }
};