#pragma once
#include <string>
#include <map>
#include <vector>
#include <memory>
#include <iostream>
#include "Type.h"

class Sema {
public:
    // 符号表
    std::vector<std::map<std::string, std::shared_ptr<CType>>> symbolTable;

    Sema() {
        // 编译器启动时，将全局作用域入栈！
        symbolTable.push_back(std::map<std::string, std::shared_ptr<CType>>());
    }

    void EnterScope() {
        // 遇到 '{'，入栈
        symbolTable.push_back(std::map<std::string, std::shared_ptr<CType>>());
    }

    void ExitScope() {
        // 遇到 '}'，出栈
        symbolTable.pop_back();
    }

    bool CheckVariableDecl(const std::string& name, std::shared_ptr<CType> type) {
        // 获取当前作用域！vector 的尾部元素就是当前作用域！
        auto& currentScope = symbolTable.back();
        
        if(currentScope.count(name) > 0) {
            std::cout << "语义错误：变量 " << name << " 在当前作用域重复定义！" << std::endl;
            return false;
        }
        currentScope[name] = type;
        return true;
    }

    std::shared_ptr<CType> GetVariableType(const std::string& name) {
        // 就近原则：从顶层（尾部）到底层（头部）倒序搜索！
        for (auto it = symbolTable.rbegin(); it != symbolTable.rend(); ++it) {
            if (it->count(name) > 0) {
                return (*it)[name]; // 在某一层找到了，立刻返回！
            }
        }
        return nullptr;
    }

    bool CheckVariableUse(const std::string& name) {
        if(GetVariableType(name) == nullptr) {
            std::cout << "语义错误：变量 " << name << " 未定义就使用了！" << std::endl;
            return false;
        }
        return true;
    }
};