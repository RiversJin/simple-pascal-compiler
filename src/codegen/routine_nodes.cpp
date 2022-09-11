/**
 * @file routine_nodes.cpp
 * @author Rivers Jin (riversjin@foxmail.com)
 * @brief 函数实现代码生成
 * @version 0.1
 * @date 2021-05-11
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <vector>
#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Verifier.h>
#include "utils/ast.hpp"
#include "codegen/codegen_context.hpp"
#include "utils/ast_utils.hpp"

namespace spc
{
    /**
     * @brief 开始生成代码的主入口
     * 
     * @param context 
     * @return llvm::Value* 
     */
    llvm::Value *ProgramNode::codegen(CodegenContext &context)
    {
        context.is_subroutine = false; //设置不是子过程 所以接下来的const，type，var会定义到全局变量中
        head_list->const_list->codegen(context);
        head_list->type_list->codegen(context);
        head_list->var_list->codegen(context);

        context.is_subroutine = true; //子过程是局部的
        head_list->subroutine_list->codegen(context);

        context.is_subroutine = false;

        auto *func_type = llvm::FunctionType::get(context.builder.getInt32Ty(), false);
        auto *main_func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage,
                                                "main", context.module.get());
        auto *block = llvm::BasicBlock::Create(context.module->getContext(), "entry", main_func);
        context.builder.SetInsertPoint(block);
        for (auto &stmt : children()) stmt->codegen(context);
        context.builder.CreateRet(context.builder.getInt32(0));

        llvm::verifyFunction(*main_func);
        if (context.fpm)
        { context.fpm->run(*main_func); }
        if (context.mpm)
        { context.mpm->run(*context.module); }
        return nullptr;
    }

    llvm::Value *SubroutineNode::codegen(CodegenContext &context)
    {
        std::vector<llvm::Type*> llvmTypes;
        std::vector<std::string> names;
        std::vector<std::shared_ptr<TypeNode>> types;
        for (auto child : params->children())
        {
            auto decl = cast_node<ParamDeclNode>(child);
            types.push_back(decl->type);
            llvmTypes.push_back(decl->type->get_llvm_type(context));
            names.push_back(decl->name->name);
        }
        auto *func_type = llvm::FunctionType::get(return_type->get_llvm_type(context), llvmTypes, false);
        auto *func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage,
                                            name->name, context.module.get());
        auto *block = llvm::BasicBlock::Create(context.module->getContext(), "entry", func);
        context.builder.SetInsertPoint(block);
        
        //将形参匹配到实参
        auto index = 0;
        for (auto &arg : func->args())
        {
            std::string argName=names[index];
            auto& argType=types[index];
            context.symbolTable.addLocalSymbol(argName,argType);
            auto ptr=context.symbolTable.getLocalSymbol(argName)->get_llvmptr();
            context.builder.CreateStore(&arg,ptr);
            index+=1;
        }
        if (return_type->type != Type::VOID)
        {
            context.symbolTable.addLocalSymbol(name->name,return_type);
        }

        head_list->codegen(context);
        for (auto &stmt : children()) stmt->codegen(context);

        if (return_type->type == Type::VOID)
        {
            context.builder.CreateRetVoid();
        }
        else //处理返回值
        {
            auto local = context.symbolTable.getLocalSymbol(name->name); //根据Pascal的规则，对函数名的赋值即为返回值
            //auto *local = context.get_local(name->name);  //根据Pascal的规则，对函数名的赋值即为返回值  
            auto *ret = context.builder.CreateLoad(local->get_llvmptr());
            context.builder.CreateRet(ret);
        }

        llvm::verifyFunction(*func);
        if (context.fpm)
        { context.fpm->run(*func); }

        context.symbolTable.resetLocals(); //清除局部信息 为下一个函数的生成做准备
        return nullptr;
    }

    llvm::Value *HeadListNode::codegen(CodegenContext &context)
    {
        const_list->codegen(context);
        type_list->codegen(context);
        var_list->codegen(context);
        subroutine_list->codegen(context);
        return nullptr;

    }

    llvm::Value *SubroutineListNode::codegen(CodegenContext &context)
    {
        for (auto &routine : children()) routine->codegen(context);
        return nullptr;
    }
}
