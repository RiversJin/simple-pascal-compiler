/**
 * @file type_nodes.cpp
 * @brief 有关类型语义节点代码生成的实现
 * @version 0.1
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <llvm/IR/Type.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include "utils/ast.hpp"
#include "codegen/codegen_context.hpp"

namespace spc
{
    static llvm::Type *llvm_type(Type type, CodegenContext &context)
    {
        switch (type)
        {
            case Type::BOOLEAN: return context.builder.getInt1Ty();
            case Type::INTEGER: return context.builder.getInt32Ty();
            case Type::REAL: return context.builder.getDoubleTy();
            case Type::CHAR: return context.builder.getInt8Ty();
            case Type::VOID: return context.builder.getVoidTy();
            default: return nullptr;
        }
    }

    llvm::Type *ConstValueNode::get_llvm_type(CodegenContext &context) const
    {
        return this->type->get_llvm_type(context);
    }

    llvm::Type *TypeNode::get_llvm_type(CodegenContext &context) const
    {
        if (auto *simple_type = dynamic_cast<const SimpleTypeNode*>(this)) // 如果是简单数据类型 直接返回即可
        { 
            return llvm_type(simple_type->type, context); 
        }
        else if (auto *array_type = dynamic_cast<const ArrayTypeNode*>(this)) //如果是数组类型
        {
            auto itemtype=array_type->element_type->get_llvm_type(context);
            return llvm::ArrayType::get(itemtype,array_type->range->length);
        }
        else if (auto *alias = dynamic_cast<const AliasTypeNode*>(this)) //或者是别名
        {
            return context.symbolTable.getGlobalAlias(alias->identifier->name)->get_llvm_type(context);
            //return context.get_alias(alias->identifier->name); 
        }
        else if (auto *record_type = dynamic_cast<const RecordTypeNode*>(this)) //如果是结构体的话
        {
            return record_type->innertype;
        }
        
        throw CodegenException("unsupported type: " + type2string(type));
        return nullptr;//这里永远不会运行
    }
}
