#pragma once
#include <memory>
#include <string>
#include <iostream>
#include"llvm/IR/Value.h"


class NumberExprAst;
class BinaryExprAst;

// 1. 翻译官（Visitor）接口
// 定义了后端代码生成器必须实现的方法
class Visitor {
	public:
	virtual ~Visitor(){}
	virtual llvm::Value* VisitNumberExpr(NumberExprAst* expr) = 0;
	virtual llvm::Value* VisitBinaryExpr(BinaryExprAst* expr) = 0;
};

// 存放表达式数据的基类
class ExprAst
{
public:
	virtual ~ExprAst() = 0;
	virtual void Dump(int intent = 0) const = 0;

	// 锟斤拷锟绞接匡拷
	virtual llvm::Value* Accept(Visitor* vis) = 0;


private:

};
// 纯虚析构函数必须提供一个定义，否则链接器会报错
inline ExprAst::~ExprAst() = default;

// 存放数字节点数据 (树叶)
class NumberExprAst : public ExprAst
{
public:
	NumberExprAst() {};
	NumberExprAst(std::string Val) : val(Val) {}
	~NumberExprAst() = default;

	void Dump(int indent = 0) const override {
		for (int i = 0; i < indent; ++i) std::cout << " ";
		std::cout << "NumberNode: " << val << std::endl;
	}

	// 实现 Accept：告诉翻译官“我是一个数字”
	llvm::Value* Accept(Visitor* vis) override {
		return vis->VisitNumberExpr(this);
	}

public:
	std::string val;
};

// 存放二元操作符节点数据 (树枝)
class BinaryExprAst : public ExprAst
{
public:
	BinaryExprAst(std::string op,std::unique_ptr<ExprAst> left, std::unique_ptr<ExprAst> right)
				 :op(op),left(std::move(left)),right(std::move(right)){};

	void Dump(int indent = 0) const override {
		for (int i = 0; i < indent; ++i)std::cout << " ";
		std::cout << "BinaryNode: " << op << std::endl;
		// 递归打印左子树和右子树
		if (left) left->Dump(indent + 1);

		if (right) right->Dump(indent + 1);
	}

	// 实现 Accept：告诉翻译官“我是一个二元操作符”
	llvm::Value* Accept(Visitor* vis) override {
		return vis->VisitBinaryExpr(this);
	}


public:
	// 为了方便 CodeGen 读取，设为 public
	std::string op;
	std::unique_ptr<ExprAst> left, right;

};

