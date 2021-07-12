#pragma once
#include <string>
#include <vector>
#include <functional>
#include "marco.h"
#include "excep.h"
#include <assert.h>
using namespace std;

enum nodeType { Num, String, Bool, Vec4f,
    Var, Pro, Fun, If, Null, Assign };

class BasicNode
{
public:
    virtual nodeType getType() const = 0;
    virtual void addNode(BasicNode* node)
    {
        assert(node != nullptr);
        this->sonNode.push_back(node);
    }
    virtual BasicNode* eval()=0;
    virtual ~BasicNode();
    BasicNode(const BasicNode& n);
    BasicNode(){}

    vector<BasicNode*> sonNode;
};
typedef function<BasicNode*(vector<BasicNode*>&sonNode)>BE; //进行基础求值


class nullNode : public BasicNode
{
public:
    virtual nodeType getType() const { return Null; }
    virtual void addNode(BasicNode*) { throw addSonExcep(Null); }
    virtual BasicNode* eval() {throw cannotEvaledExcep();}
};

template<typename T, nodeType TAG>
class PackNode : public BasicNode
{
protected:
    T data;
public:
    virtual nodeType getType() const { return TAG; }
    virtual void addNode(BasicNode*) { throw addSonExcep(TAG); }
    virtual BasicNode* eval() { return this; }
    PackNode(T data) : data(data) {}
    PackNode(const PackNode& n) : BasicNode(n), data(n.getData()) {}
    T getData() const { return this->data; }
};

struct vec4f
{
    float f1;
    float f2;
    float f3;
    float f4;
};

typedef PackNode<double, Num> NumNode;
typedef PackNode<string, String> StringNode;
typedef PackNode<bool, Bool> BoolNode;
typedef PackNode<vec4f, Vec4f> Vec4fNode;