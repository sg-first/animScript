animaScript 
==========
自定义类型指南
------
### 定义类型
``` cpp
struct vec4f
{
    float f1;
    float f2;
    float f3;
    float f4;
};
```

### 定义包装节点
``` cpp
typedef PackNode<vec4f, Vec4f> Vec4fNode;
```

### 修改copyHelp进行类型注册
修改` copyHelp::copyVal `
``` cpp
BasicNode* copyHelp::copyVal(BasicNode* node) //（值类型）拷贝
{
    //调用前应该对参数类型进行检查
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
    //warn:支持更多具拷贝构造函数类型（目前都是字面量）后还需要在此处进行添加
    return nullptr; //如果进行参数检查了不会走到这一步
}
```
修改`copyHelp:: isLiteral`
``` cpp
static bool isLiteral(nodeType type) { return (type == Num || type == String || type == Bool || type==Vec4f); } //warn:添加新的值类型要进行修改
```

### 定义构造函数
``` cpp
    static BasicNode* makeVec4f(vector<BasicNode*>& sonNode)
    {
        return new Vec4fNode(vec4f{ getNum(sonNode[0]),getNum(sonNode[1]),getNum(sonNode[2]),getNum(sonNode[3]) });
    }

    static BasicNode* getVec4f_1(vector<BasicNode*>& sonNode) //测试用的函数
    {
        auto v = dynamic_cast<Vec4fNode*>(sonNode[0])->getData();
        auto r = v.f1;
        return new NumNode(r);
    }
```
注：也可以不用构造函数，直接用字面量构造。不过用字面量构造需要修改parser，让它可以解析这种字面量。具体看`parser.h`中的注释

### 注册构造函数
``` cpp
scope.addFunction("makeVec4f", new Function({ Num,Num,Num,Num }, BuiltinFunc::makeVec4f, Vec4f));
scope.addFunction("getVec4f_1", new Function({ Vec4f }, BuiltinFunc::getVec4f_1, Num));
```

### 测试代码
``` cpp
string code = "(a:Num,b:Num,c:Num,d:Num)-> vec=makeVec4f(a,b,c,d); return getVec4f_1(vec);";
program p = parser::creatProgram(code, s);
NumNode* r = (NumNode*)p.call({ new NumNode(1),new NumNode(2),new NumNode(3),new NumNode(4) });
assert(r->getData() == 1);
delete r;
```
