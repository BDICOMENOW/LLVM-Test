#pragma once
#include <memory>
#include <string>

// 存放表达式数据
class ExprAst
{
public:
	virtual ~ExprAst() = 0;

private:

};
// 纯虚析构函数必须提供一个定义，否则链接器会报错
inline ExprAst::~ExprAst() = default;

// 存放数字节点数据
class NumberExprAst : public ExprAst
{
public:
	NumberExprAst() {};
	NumberExprAst(std::string Val) : val(Val) {}
	~NumberExprAst() = default;

private:
	std::string val;
};

// 存放二元操作符节点数据
class BinaryExprAst : public ExprAst
{
public:
	BinaryExprAst(std::string op,std::unique_ptr<ExprAst> left, std::unique_ptr<ExprAst> right)
				 :op(op),left(std::move(left)),right(std::move(right)){};


private:
	std::string op;
	std::unique_ptr<ExprAst> left, right;

};

