#pragma once
#include <map>
#include "toolNode.h"

class Scope //�����Ǽ�¼��ͬ�����ں�������������ʵ���ӳ�䣬��������ʱջ
{
private:
    BasicNode* topASTNode=nullptr; //������ȫ������������ʵ�����õĸ��ڵ㣬������ͷ��ⲿ��Ҫʹ�õ�ʵ��

public:
    ~Scope();
    map<string,Variable*> variableList;
    map<string,Function*> functionList;
    vector<Scope*> sonScope;
    Scope* fatherScope; //��Ϊ����˳�����������ϲ��ұ�����������Ҫ�ϼ��ڵ��ָ��s
    Scope(Scope* fatherScope=nullptr):fatherScope(fatherScope){}

    void addVariable(string name,Variable* var);
    void addFunction(string name,Function* fun);
    Variable* findVariable(string name, bool thisScope=true);
    Function* findFunction(string name,bool thisScope=true);
    //ֱ��ɾ��ʵ�壬���Ѿ�������ʵ���������������ʱʹ�ã�Ŀǰ��û������ʲô�ã�
    //void deleteVariable(string name);
    //void deleteFunction(string name);
    //ɾ��ʵ��ָ�������еĴ洢�����Ѿ�������ʵ���������������ʱʹ�����Ѿ�deleteʱʹ��
    void deleteVariable(Variable* var);
    void deleteFunction(Function* fun);
    void settopASTNode(BasicNode* topnode) {this->topASTNode=topnode;}
};
