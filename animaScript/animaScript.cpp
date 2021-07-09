#include <iostream>
#include "toolNode.h"
#include "parser.h"
#include "funinterface.h"

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

    string code = "(aaa:Num,b1:Num,c:Num) -> d=f1(aaa*c,b1); e=f1(d,c)-aaa; a=(aaa+b1)*c; d=e/a; return d;";
    string code2 = "(aaa:Num,b1:Num,c:Num) -> d=f1(aaa*c,b1); e=f1(d,c)-aaa; return ((d>5)||(e==7)) ? 1 : 2;";
    program p = parser::creatProgram(code, &scope);
    NumNode* r = (NumNode*)p.call({ new NumNode(1),new NumNode(2),new NumNode(3) });
    r = (NumNode*)p.call({ new NumNode(2),new NumNode(2),new NumNode(2) });;
    program p2 = parser::creatProgram(code2, &scope);
    r = (NumNode*)p2.call({ new NumNode(1),new NumNode(2),new NumNode(3) });
    r = (NumNode*)p2.call({ new NumNode(2),new NumNode(2),new NumNode(2) });

    /*
    lexer l(code);
    try
    {
        l.run();
    }
    catch (string s)
    {
        cout << s << endl;
    }

    for (auto i : l.allToken)
        cout << i.first << " " << i.second << endl;
    */
    cout << "ok";
}
