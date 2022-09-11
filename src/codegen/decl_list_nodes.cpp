/**
 * @file decl_list_nodes.cpp
 * @author Rivers Jin (riversjin@foxmail.com)
 * @brief 变量声明,创建的代码生成部分
 * @version 0.1
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Constant.h>
#include <llvm/Support/Casting.h>
#include <typeinfo>
#include <fmt/core.h>
#include "utils/ast.hpp"
#include "utils/ast_utils.hpp"
#include "codegen/codegen_context.hpp"

namespace spc
{
    llvm::Value *ConstListNode::codegen(CodegenContext &context)
    {
        for (auto &child : children()) child->codegen(context);
        return nullptr;
    }
    /**
     * @brief 分配在内存中常量
     * 
     * @param context 
     * @return llvm::Value* 
     */
    llvm::Value *ConstDeclNode::codegen(CodegenContext &context)
    {
        if (context.is_subroutine)
        {
            //
            bool success = context.symbolTable.addLocalSymbol(name->name,value->type,true);
            //auto *local = context.builder.CreateAlloca(value->get_llvm_type(context));
            //auto success = context.set_local(name->name, local);
            if (!success) throw CodegenException("duplicate identifier in const section: " + name->name);
            auto local=context.symbolTable.getLocalSymbol(name->name);
            context.builder.CreateStore(value->codegen(context), local->get_llvmptr());
            return local->get_llvmptr();
        }
        else
        {
            if (is_a_ptr_of<StringNode>(value)) //符号表字符串的支持还有问题，先在这打个洞吧...
                return value->codegen(context);
            auto *constant = llvm::cast<llvm::Constant>(value->codegen(context)); //获得初值
            bool success = context.symbolTable.addGlobalSymbol(name->name,value->type,constant,true);
            return context.symbolTable.getGlobalSymbol(name->name)->get_llvmptr();
        }
    }

    llvm::Value *VarListNode::codegen(CodegenContext &context)
    {
        for (auto &child : children()) child->codegen(context);
        return nullptr;
    }

    llvm::Value *VarDeclNode::codegen(CodegenContext &context)
    {
        if (context.is_subroutine) //如果是子过程 分配临时变量
        {
            bool success = context.symbolTable.addLocalSymbol(name->name,type);
            if (!success) throw CodegenException("duplicate identifier in var section: " + name->name);
            auto local=context.symbolTable.getLocalSymbol(name->name);
            return local->get_llvmptr();
        }
        else
        {
            bool success = context.symbolTable.addGlobalSymbol(name->name,type,nullptr);
            if(!success) throw CodegenException("duplicate global identifier in var sectrion: "+name->name);
            auto ptr=context.symbolTable.getGlobalSymbol(name->name)->get_llvmptr();
            return ptr;
        }
    }

    llvm::Value *TypeListNode::codegen(CodegenContext &context)
    {
        for (auto &child : children()) child->codegen(context);
        return nullptr;
    }

    llvm::Value *TypeDefNode::codegen(CodegenContext &context)
    {

        if(is_a_ptr_of<RecordTypeNode>(this->type)){
            auto p=cast_node<RecordTypeNode>(this->type);
            std::vector<llvm::Type*> llvmTypes;
            std::vector<std::shared_ptr<TypeNode>> types;
            for(const auto& type:p->fields){
                llvmTypes.push_back(type.second->get_llvm_type(context));
            }
            llvm::StructType *structReg = llvm::StructType::create(context.module->getContext(),llvmTypes,name->name);
            p->innertype=structReg;
        }
        if (context.is_subroutine){
            bool success = context.symbolTable.addLocalAlias(name->name,type);
            if(!success) throw CodegenException(fmt::format("duplicate local type alias: \"{}\"",name->name));
        }
        else{
            bool success = context.symbolTable.addGlobalAlias(name->name,type);
            if(!success) throw CodegenException(fmt::format("duplicate global type alias: \"{}\"",name->name));
        }
        return nullptr;
    }
}
