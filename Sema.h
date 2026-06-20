#pragma once
#include <string>
#include <map>
#include <iostream>
#include "Type.h"

class Sema {
public:
    // 符号表：本质是一个字典，用来记录变量名是否被声明过
    
	std::map<std::string, std::shared_ptr<CType>> symbolTable;

    // 检查变量是否已经声明
    bool CheckVariableDecl(const std::string& name,std::shared_ptr<CType> type) {
		
        if(symbolTable.count(name) > 0) {
            std::cout<<"语义错误：变量 "<<name<<" 重复定义！"<<std::endl;
            return false;
        }
        // 没问题，记录在符号表
        symbolTable[name] = type;
        return true;
    }

    // 检查变量是否定义
    bool CheckVariableUse(const std::string& name) {
        if(symbolTable.count(name) == 0) {
            std::cout<<"语义错误：变量 "<<name<<" 未定义就使用了！"<<std::endl;
            return false;
        }
        return true;
    }
        
};
