/**
 * @file identifier_node.cpp
 * @author Rivers Jin (riversjin@foxmail.com)
 * @brief 标识符代码生成
 * @version 0.1
 * @date 2021-05-11
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "ast/ast_base.h"
#include "codegen/codegen_context.hpp"

namespace spc
{
    llvm::Value *IdentifierNode::get_ptr(CodegenContext &context)
    {
        auto value = context.symbolTable.getLocalSymbol(name);
        if(value==nullptr) value = context.symbolTable.getGlobalSymbol(name);
        if(value==nullptr) throw CodegenException("identifier not found: " + name);
        return value->get_llvmptr();
    }

    llvm::Value *IdentifierNode::codegen(CodegenContext &context)
    {
        return context.builder.CreateLoad(get_ptr(context));
    }
}
