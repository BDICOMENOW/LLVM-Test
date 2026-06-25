#pragma once
#include <string>
#include <memory>
#include "llvm/IR/Type.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/LLVMContext.h"

// 静态类型系统基础：种类枚举
enum class TypeKind {
    INT,
    POINTER,
    ARRAY, // 数组
};

// ==========================================================
// CType 基类：静态类型系统
// ==========================================================
class CType {
public:
    virtual ~CType() = default;
    virtual TypeKind GetKind() const = 0;
    
    // 1. ToString：返回字符串
    virtual std::string ToString() const = 0;
    
    // 2. 传入 llvm::LLVMContext& context，返回 llvm::Type*
    virtual llvm::Type* ToLLVMType(llvm::LLVMContext& context) const = 0;
};

// ==========================================================
// IntType：最基础的 32位 整数类型 (int)
// ==========================================================
class IntType : public CType {
public:
    TypeKind GetKind() const override { return TypeKind::INT; }
    
    std::string ToString() const  { 
        return "int"; 
    }
    
    llvm::Type* ToLLVMType(llvm::LLVMContext& context) const {
        // 架构师提供：告诉 LLVM 这是一个 32位整数 (i32)
        return llvm::Type::getInt32Ty(context);
    }
};

// ==========================================================
// PointerType：代表指针类型 (如 int*, int**)
// ==========================================================
class PointerType : public CType {
public:

    std::shared_ptr<CType> type;

public:
    PointerType(std::shared_ptr<CType> type) : type(type) {};

    TypeKind GetKind() const override { return TypeKind::POINTER; }
    
    
    // 如果当前套娃里面是 int，它应该返回 "int*"。
    // 如果里面是 int*，它应该返回 "int**"。
    std::string ToString() const  { 
        return type->ToString() + "*"; 
    }
    
    llvm::Type* ToLLVMType(llvm::LLVMContext& context) const  {
        
        // 所有指针统一返回 ptr 类型，不再区分 i32* 还是 i32**。
        // 因此这里直接返回 llvm::Type::getPointerType
        
        return llvm::PointerType::get(context, 0);
    }
};

class ArrayType : public CType {
public:
    std::shared_ptr<CType> elementType;
    int numElements;

public:
    ArrayType(std::shared_ptr<CType> elementType, int numElements) : elementType(elementType), numElements(numElements) {};
    TypeKind GetKind() const override { return TypeKind::ARRAY; }
    std::string ToString() const  {
        return elementType->ToString() + "[" + std::to_string(numElements) + "]";
    }
    llvm::Type* ToLLVMType(llvm::LLVMContext& context) const  {
        // 指定元素类型和元素个数
        return llvm::ArrayType::get(elementType->ToLLVMType(context), numElements); 
    }
};