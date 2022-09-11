/**
 * @file symbol.cpp
 * @author Rivers Jin (riversjin@foxmail.com)
 * @brief 符号表及符号实现
 * @version 0.1
 * @date 2021-05-11
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "symbol.h"
#include "codegen_context.hpp"
#include <fmt/core.h>
#include <typeinfo>

using namespace spc;
using std::string;
using std::shared_ptr;
using std::make_shared;

bool SymbolTable::addLocalSymbol(string name,shared_ptr<TypeNode> type,bool isConst){
    if(localSymbols.count(name)>0){
        throw CodegenException(fmt::format("Duplicate local name {}",name));
        return false;
    }
    if(localAliases.count(name)>0){
        throw CodegenException(fmt::format("When creating local variable detecting an existed alias named \"{}\"",name));
        return false;
    }
    auto localVariable = context.builder.CreateAlloca(type->get_llvm_type(context));
    localVariable->setName(name); //设置在LLVM IR中此变量的名字
    localSymbols[name]=make_shared<Symbol>(name,type,localVariable,isConst);
    return true;
}

std::shared_ptr<Symbol> SymbolTable::getLocalSymbol(string name){
    if(localSymbols.count(name)==0){
        return nullptr;
    }
    return localSymbols[name];
}

bool SymbolTable::addGlobalSymbol(string name,shared_ptr<TypeNode> type,llvm::Constant* initializer,bool isConst){
    if(globalSymbols.count(name)>0){
        throw CodegenException(fmt::format("Duplicate global name {}",name));
        return false;
    }
    if(globalAliases.count(name)>0){
        throw CodegenException(fmt::format("When creating global variable detecting an existed alias named \"{}\"",name));
        return false;
    }
    auto llvmtype=type->get_llvm_type(context);
    llvm::Constant* constant=nullptr;
    if(is_a_ptr_of<AliasTypeNode>(type)){
        auto aliasTypePtr=cast_node<AliasTypeNode>(type);
        type=getGlobalAlias(aliasTypePtr->identifier->name);
    }
    switch(llvmtype->getTypeID()){
        case llvm::Type::IntegerTyID:
            constant = (initializer==nullptr)?llvm::ConstantInt::get(llvmtype,0):initializer;
            break;
        case llvm::Type::DoubleTyID:
            constant = (initializer==nullptr)?llvm::ConstantFP::get(llvmtype,0):initializer;
            break;
        case llvm::Type::ArrayTyID:
            constant=llvm::ConstantAggregateZero::get(cast_node<ArrayTypeNode>(type)->element_type->get_llvm_type(context));
            break;
        case llvm::Type::StructTyID:
            constant=llvm::ConstantAggregateZero::get(cast_node<RecordTypeNode>(type)->innertype);
            break;
        default:
            throw CodegenException("unsupported type: " + type2string(type->type));
    }
    auto globalVariblePtr = new llvm::GlobalVariable(*context.module,type->get_llvm_type(context),isConst,llvm::GlobalVariable::InternalLinkage,constant,name);
    globalSymbols[name]=make_shared<Symbol>(name,type,globalVariblePtr);
    return true;
}

std::shared_ptr<Symbol> SymbolTable::getGlobalSymbol(std::string name){
    if(globalSymbols.count(name)==0){
        return nullptr;
    }
    return globalSymbols[name];
}

bool SymbolTable::addLocalAlias(std::string alias,std::shared_ptr<TypeNode> type){
    if(localAliases.count(alias)){
        throw CodegenException(fmt::format("Duplicate local alias {}",alias));
        return false;
    }
    if(localSymbols.count(alias)){
        throw CodegenException(fmt::format("When creating local alias detecting an existed alias named \"{}\"",alias));
        return false;
    }
    localAliases[alias]=type;
    return true;
}

std::shared_ptr<TypeNode> SymbolTable::getLocalAlias(std::string name){
    if(localAliases.count(name)==0){
        return nullptr;
    }
    return localAliases[name];
}

bool SymbolTable::addGlobalAlias(std::string alias,std::shared_ptr<TypeNode> type){
    if(globalAliases.count(alias)){
        throw CodegenException(fmt::format("Duplicate global alias {}",alias));
        return false;
    }
    if(globalAliases.count(alias)){
        throw CodegenException(fmt::format("When creating global alias detecting an existed alias named \"{}\"",alias));
        return false;
    }
    globalAliases[alias]=type;
    return true;
}

std::shared_ptr<TypeNode> SymbolTable::getGlobalAlias(std::string name){
    if(globalAliases.count(name)==0)
    {
        
        return nullptr;
    }
    return globalAliases[name];
}

void SymbolTable::resetLocals(){
    localAliases.clear();
    localSymbols.clear();
}