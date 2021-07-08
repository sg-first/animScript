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
    scope.addFunction("<", new Function({ Num,Num }, BuiltinFunc::less, Num));
    scope.addFunction("<=", new Function({ Num,Num }, BuiltinFunc::lessEqual, Num));
    scope.addFunction(">", new Function({ Num,Num }, BuiltinFunc::greater, Num));
    scope.addFunction(">=", new Function({ Num,Num }, BuiltinFunc::greater, Num));
    scope.addFunction("==", new Function({ Num,Num }, BuiltinFunc::equal, Num));
    scope.addFunction("!=", new Function({ Num,Num }, BuiltinFunc::notEqual, Num));
    scope.addFunction("==Bool", new Function({ Bool,Bool }, BuiltinFunc::equal, Bool));
    scope.addFunction("!=Bool", new Function({ Bool,Bool }, BuiltinFunc::notEqual, Bool));
    scope.addFunction("&&", new Function({ Bool,Bool }, BuiltinFunc::And, Bool));
    scope.addFunction("||", new Function({ Bool,Bool }, BuiltinFunc::Or, Bool));

    string program = "(a:Num,b:Num,c:Num)-> d=f1(a*c,b); e=f2(d,c)+a; a=(a+b)*c; return e;";
    lexer l(program);
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

    cout << endl << "parser:" << endl;

    parser p(l.allToken, &scope);
    auto tree = p.run();
    if (tree == nullptr)
        cout << "parser Error!" << endl;
}
