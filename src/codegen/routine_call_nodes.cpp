/**
 * @file routine_call_nodes.cpp
 * @author Rivers Jin (riversjin@foxmail.com)
 * @brief 过程调用代码生成
 * @version 0.1
 * @date 2021-05-11
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <vector>
#include "utils/ast.hpp"
#include "codegen/codegen_context.hpp"

namespace spc
{
    llvm::Value *RoutineCallNode::codegen(CodegenContext &context)
    {
        auto *func = context.module->getFunction(identifier->name);
        if (func->arg_size() != args->children().size())
        { throw CodegenException("wrong number of arguments: " + identifier->name + "()"); }
        std::vector<llvm::Value*> values;
        // TODO: check type compatibility between arguments and parameters
        for (auto &arg : args->children()) values.push_back(arg->codegen(context));
        return context.builder.CreateCall(func, values);
    }
}
