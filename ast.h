#pragma once
#include <memory>
#include <string>
#include <iostream>
#include"llvm/IR/Value.h"


class NumberExprAst;
class BinaryExprAst;
class VariableDeclAst;
class VariableAccessAst;
class AssignExprAst;
class BlockStmtAst;
class IfStmtAst;
class ForStmtAst;
class BreakStmtAst;
class ContinueStmtAst;
class UnaryExprAst;

// 1. 访问（Visitor）接口
// 定义了后端代码生成器必须实现的方法
class Visitor {
	public:
	virtual ~Visitor(){}
	virtual llvm::Value* VisitNumberExpr(NumberExprAst* expr) = 0;
	virtual llvm::Value* VisitBinaryExpr(BinaryExprAst* expr) = 0;
	virtual llvm::Value* VisitVariableDecl(VariableDeclAst* expr) = 0;
	virtual llvm::Value* VisitVariableAccess(VariableAccessAst* expr) = 0;
	virtual llvm::Value* VisitAssignExpr(AssignExprAst* expr) = 0;
	virtual llvm::Value* VisitBlockStmt(BlockStmtAst* expr) = 0;
	virtual llvm::Value* VisitIfStmt(IfStmtAst* expr) = 0;
	virtual llvm::Value* VisitForStmt(ForStmtAst* expr) = 0;
	virtual llvm::Value* VisitBreakStmt(BreakStmtAst* expr) = 0;
	virtual llvm::Value* VisitContinueStmt(ContinueStmtAst* expr) = 0;
	virtual llvm::Value* VisitUnaryExpr(UnaryExprAst* expr) = 0;
};

// 存放表达式数据的基类
class ExprAst
{
public:
	virtual ~ExprAst() = 0;
	virtual void Dump(int intent = 0) const = 0;

	// 翻译官访问自己
	virtual llvm::Value* Accept(Visitor* vis) = 0;

	// 是否是左值
	bool isLValue = false;

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

	// 实现 Accept：告诉访问接口Vis“我是一个数字”
	llvm::Value* Accept(Visitor* vis) override {
		return vis->VisitNumberExpr(this);
	}

public:
	std::string val;
};

enum class UnaryOp {
	deref,	// 指针解引用 *
	addr	// 取地址 &
};

class UnaryExprAst : public ExprAst
{
public:
	UnaryExprAst(UnaryOp _op, std::unique_ptr<ExprAst> _node) {
        this->op = _op;
        this->node = std::move(_node);
    }

	void Dump(int indent = 0) const override {
        for (int i = 0; i < indent; ++i) std::cout << " ";
        std::cout << "UnaryExpr: " << (op == UnaryOp::deref ? "*" : "&") << std::endl;
        if (node) node->Dump(indent + 1);
    }

	llvm::Value* Accept(Visitor* vis) override {
		return vis->VisitUnaryExpr(this);
	}

public:
	UnaryOp op;
	std::unique_ptr<ExprAst> node;
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

class VariableDeclAst : public ExprAst
{
public:
	VariableDeclAst(std::string name) :name(name) {};
	~VariableDeclAst() = default;
	void Dump(int indent = 0) const override {
		for (int i = 0; i < indent; ++i)std::cout << " ";
		std::cout << "VariableDecl: int " << name << std::endl;
	}
	llvm::Value* Accept(Visitor* vis) override {
		return vis->VisitVariableDecl(this);
	}
public:
	std::string name;	// 记录名字
};

class VariableAccessAst : public ExprAst
{
public:
	VariableAccessAst(std::string name) :name(name) {};
	~VariableAccessAst() = default;
	void Dump(int indent = 0) const override {
		for (int i = 0; i < indent; ++i)std::cout << " ";
		std::cout << "VariableAccess: " << name << std::endl;
	}
	llvm::Value* Accept(Visitor* vis) override {
		return vis->VisitVariableAccess(this);
	}
public:
	std::string name;	// 记录读取变量的名字
};

class AssignExprAst : public ExprAst
{
public:
	AssignExprAst(std::unique_ptr<ExprAst> left, std::unique_ptr<ExprAst> right) 
				 :left(std::move(left)), right(std::move(right)) {};
	~AssignExprAst() = default;
	void Dump(int indent = 0) const override {
		for (int i = 0; i < indent; ++i)std::cout << " ";
		std::cout << "AssignNode: " << std::endl;
		// 递归打印左子树和右子树
		if (left) left->Dump(indent + 1);
		if (right) right->Dump(indent + 1);
	}
	llvm::Value* Accept(Visitor* vis) override {
		return vis->VisitAssignExpr(this);
	}
public:
	// 为了方便 CodeGen 读取，设为 public
	std::unique_ptr<ExprAst> left, right; // 左边是左值，右边是数值或者算式
};

// 新增代码块功能代表大括号{}
class BlockStmtAst : public ExprAst
{
public:
	BlockStmtAst() = default;
	~BlockStmtAst() = default;
	void Dump(int indent = 0) const override {
		for (int i = 0; i < indent; ++i)std::cout << " ";
		std::cout << "BlockStmt: {" << std::endl;
		for (const auto& stmt : stmtVec) {
			if (stmt) stmt->Dump(indent + 1);
		}
		for (int i = 0; i < indent; ++i) std::cout << " ";
		std::cout << "}" << std::endl;
	}
	llvm::Value* Accept(Visitor* vis) override {
		return vis->VisitBlockStmt(this);
	}
public:
	// 用 unique_ptr 数组装大括号里面的所有语句
	std::vector<std::unique_ptr<ExprAst>> stmtVec;
};

class IfStmtAst : public ExprAst
{
public:
	IfStmtAst(std::unique_ptr<ExprAst> condStmt, std::unique_ptr<ExprAst> thenStmt, std::unique_ptr<ExprAst> elseStmt) 
				 :condNode(std::move(condStmt)), thenNode(std::move(thenStmt)), elseNode(std::move(elseStmt)) {};
	~IfStmtAst() = default;
	void Dump(int indent = 0) const override {
		for (int i = 0; i < indent; ++i) std::cout << " ";
		std::cout << "IfStmt: " << std::endl;
		if (condNode) {
			for (int i = 0; i < indent + 1; ++i) std::cout << " ";
			std::cout << "Condition:" << std::endl;
			condNode->Dump(indent + 2);
		}
		if (thenNode) {
			for (int i = 0; i < indent + 1; ++i) std::cout << " ";
			std::cout << "Then:" << std::endl;
			thenNode->Dump(indent + 2);
		}
		if (elseNode) {
			for (int i = 0; i < indent + 1; ++i) std::cout << " ";
			std::cout << "Else:" << std::endl;
			elseNode->Dump(indent + 2);
		}
	}
	llvm::Value* Accept(Visitor* vis) override {
		return vis->VisitIfStmt(this);
	}
public:
	
	std::unique_ptr<ExprAst> condNode;	  // 条件
	std::unique_ptr<ExprAst> thenNode;// 成立时执行
	std::unique_ptr<ExprAst> elseNode;// 不成立时执行 (可能为空)
};

class ForStmtAst : public ExprAst
{
	
public:
	ForStmtAst(std::unique_ptr<ExprAst> initNode, std::unique_ptr<ExprAst> condNode, 
               std::unique_ptr<ExprAst> incNode, std::unique_ptr<ExprAst> bodyNode) 
				 :initNode(std::move(initNode)), condNode(std::move(condNode)), 
                  incNode(std::move(incNode)), bodyNode(std::move(bodyNode)) {};
	~ForStmtAst() = default;

	void Dump(int indent = 0) const override {
		for (int i = 0; i < indent; ++i) std::cout << " ";
		std::cout << "ForStmt: " << std::endl;
		if (initNode) { initNode->Dump(indent + 2); }
		if (condNode) { condNode->Dump(indent + 2); }
		if (incNode) { incNode->Dump(indent + 2); }
		if (bodyNode) { bodyNode->Dump(indent + 2); }
	}

	llvm::Value* Accept(Visitor* vis) override {
		return vis->VisitForStmt(this);
	}

public:
	std::unique_ptr<ExprAst> initNode; // 初始化 (例如 i = 0)
	std::unique_ptr<ExprAst> condNode; // 条件 (例如 i < 100)
	std::unique_ptr<ExprAst> incNode;  // 步进 (例如 i = i + 1)
	std::unique_ptr<ExprAst> bodyNode; // 循环体

};

// 越狱门 Break：自带寻路器
class BreakStmtAst : public ExprAst
{
public:
	BreakStmtAst() = default;
	~BreakStmtAst() = default;

	void Dump(int indent = 0) const override {
		for (int i = 0; i < indent; ++i) std::cout << " ";
		std::cout << "BreakStmt" << std::endl;
	}

	llvm::Value* Accept(Visitor* vis) override {
		return vis->VisitBreakStmt(this);
	}

public:
	// 寻路指针！不拥有所有权，只做标记，用裸指针！
	ExprAst* target = nullptr; 
};

// 跃迁门 Continue：自带寻路器
class ContinueStmtAst : public ExprAst
{
public:
	ContinueStmtAst() = default;
	~ContinueStmtAst() = default;

	void Dump(int indent = 0) const override {
		for (int i = 0; i < indent; ++i) std::cout << " ";
		std::cout << "ContinueStmt" << std::endl;
	}

	llvm::Value* Accept(Visitor* vis) override {
		return vis->VisitContinueStmt(this);
	}

public:
	ExprAst* target = nullptr; // 寻路指针
};


