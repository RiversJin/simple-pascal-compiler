/**
 * @file const_value_nodes.cpp
 * @author Rivers Jin (riversjin@foxmail.com)
 * @brief 字符常量代码生成部分
 * @version 0.1
 * @date 2021-05-11
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <llvm/IR/Type.h>
#include <llvm/IR/Constants.h>
#include "utils/ast.hpp"
#include "codegen/codegen_context.hpp"

namespace spc
{
    llvm::Value *BooleanNode::codegen(CodegenContext &context)
    {

        return val ? context.builder.getTrue()
                   : context.builder.getFalse();
    }

    llvm::Value *IntegerNode::codegen(CodegenContext &context)
    {
        auto *type = context.builder.getInt32Ty();
        return llvm::ConstantInt::getSigned(type, val);
    }

    llvm::Value *RealNode::codegen(CodegenContext &context)
    {
        auto *type = context.builder.getDoubleTy();
        return llvm::ConstantFP::get(type, val);
    }

    llvm::Value *CharNode::codegen(CodegenContext &context)
    {
        return context.builder.getInt8(static_cast<uint8_t>(val));
    }

    llvm::Value *StringNode::codegen(CodegenContext &context)
    {
        return context.builder.CreateGlobalStringPtr(val);
    }
}
