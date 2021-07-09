#include <iostream>
#include "toolNode.h"
#include "parser.h"
#include "funinterface.h"

void test(Scope* s)
{
    {
        string code = "(aaa:Num,b1:Num,c:Num) -> d=f1(aaa*c,b1); e=f1(d,c)-aaa; a=(aaa+b1)*c; d=e/a; return d*9;";
        program p = parser::creatProgram(code, s);
        NumNode* r = (NumNode*)p.call({ new NumNode(1),new NumNode(2),new NumNode(3) });
        assert(r->getData() == 7);
        delete r;
        r = (NumNode*)p.call({ new NumNode(2),new NumNode(2),new NumNode(2) });
        assert(r->getData() == 6.75);
        delete r;
    }
    {
        string code = "(aaa:Num,b1:Num,c:Num) -> d=f1(aaa*c,b1); e=f1(d,c)-aaa; return ((d>5)||(e==7)) ? 1 : 2;";
        program p = parser::creatProgram(code, s);
        NumNode* r = (NumNode*)p.call({ new NumNode(1),new NumNode(2),new NumNode(3) });
        assert(r->getData() == 1);
        delete r;
        r = (NumNode*)p.call({ new NumNode(2),new NumNode(2),new NumNode(2) });
        assert(r->getData() == 1);
        delete r;
    }
    {
        string code = "(a:Bool,b:Bool)->return (a==b)?1:2;";
        program p = parser::creatProgram(code, s);
        NumNode* r = (NumNode*)p.call({ new BoolNode(true),new BoolNode(true) });
        assert(r->getData() == 1);
        delete r;
        r = (NumNode*)p.call({ new BoolNode(true),new BoolNode(false) });
        assert(r->getData() == 2);
        delete r;
    }
}

int main()
{
    Scope scope;
    scope.addFunction("+", new Function({ Num,Num }, BuiltinFunc::add, Num));
    scope.addFunction("-", new Function({ Num,Num }, BuiltinFunc::sub, Num));
    scope.addFunction("*", new Function({ Num,Num }, BuiltinFunc::mul, Num));
    scope.addFunction("/", new Function({ Num,Num }, BuiltinFunc::div, Num));
    scope.addFunction("<", new Function({ Num,Num }, BuiltinFunc::less, Bool));
    scope.addFunction("<=", new Function({ Num,Num }, BuiltinFunc::lessEqual, Bool));
    scope.addFunction(">", new Function({ Num,Num }, BuiltinFunc::greater, Bool));
    scope.addFunction(">=", new Function({ Num,Num }, BuiltinFunc::greater, Bool));
    scope.addFunction("==", new Function({ Num,Num }, BuiltinFunc::equal, Bool));
    scope.addFunction("!=", new Function({ Num,Num }, BuiltinFunc::notEqual, Bool));
    scope.addFunction("==Bool", new Function({ Bool,Bool }, BuiltinFunc::equal, Bool));
    scope.addFunction("!=Bool", new Function({ Bool,Bool }, BuiltinFunc::notEqual, Bool));
    scope.addFunction("&&", new Function({ Bool,Bool }, BuiltinFunc::And, Bool));
    scope.addFunction("||", new Function({ Bool,Bool }, BuiltinFunc::Or, Bool));

    scope.addFunction("f1", new Function({ Num,Num }, BuiltinFunc::add, Num));

    test(&scope);
}
