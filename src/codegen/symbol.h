/**
 * @file symbol.h
 * @author Rivers Jin (riversjin@foxmail.com)
 * @brief 主要定义有关符号表类的信息
 * @version 0.1
 * @date 2021-04-02
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef SYMBOL_H
#define SYMBOL_H
#include "ast/ast_base.h"
#include <map>

namespace spc
{
    struct SymbolTable;
    struct CodegenContext;
    /// 符号实体 用来存放变量与其对应的指针
    struct Symbol{
        std::string name; //符号名称
        bool isConst;
        llvm::Value * ptr;
        std::shared_ptr<TypeNode> typeNode;
        /**
         * @brief 获得此符号的指针
         * 
         * @return llvm::Value* 
         */
        llvm::Value* get_llvmptr(){return ptr;}
        Symbol(std::string name,std::shared_ptr<TypeNode> type,llvm::Value* ptr,bool isConst=false):name(name),typeNode(type),ptr(ptr),isConst(isConst){}
        friend struct SymbolTable;
    };
    /**
     * @brief 一个基础款的符号表
     * 
     */
    struct SymbolTable{
        SymbolTable(CodegenContext* context):context(*context){}
        /**
         * @brief 添加局部变量 如果成功返回true 失败程序将终止
         * 
         * @param name 变量名
         * @param isConst 
         * @return true 
         * @return false 
         */
        bool addLocalSymbol(std::string name,std::shared_ptr<TypeNode>,bool isConst=false);
        /**
         * @brief 获取局部变量，如果失败将返回nullptr
         * 
         * @param name 
         * @return std::shared_ptr<Symbol> 
         */
        std::shared_ptr<Symbol> getLocalSymbol(std::string name);
        /**
         * @brief 添加全局变量 如果成功返回true 失败程序将终止
         * 
         * @param name 变量名
         * @param isConst 
         * @return true 
         * @return false 
         */
        bool addGlobalSymbol(std::string name,std::shared_ptr<TypeNode>,llvm::Constant* initializer,bool isConst=false);
        /**
         * @brief 获取全局变量 如果失败返回nullptr
         * 
         * @param name 
         * @return std::shared_ptr<Symbol> 
         */
        std::shared_ptr<Symbol> getGlobalSymbol(std::string name);
        /**
         * @brief 添加局部别名 成功返回true 失败程序终止
         * 
         * @param alias 别名
         * @param type 类型
         * @return true 
         * @return false 
         */
        bool addLocalAlias(std::string alias,std::shared_ptr<TypeNode> type);
        /**
         * @brief 获取局部别名 失败返回nullptr
         * 
         * @param name 
         * @return std::shared_ptr<TypeNode> 
         */
        std::shared_ptr<TypeNode> getLocalAlias(std::string name);
        /**
         * @brief 添加全局变量 成功返回true 失败程序终止
         * 
         * @param alias 
         * @param type 
         * @return true 
         * @return false 
         */
        bool addGlobalAlias(std::string alias,std::shared_ptr<TypeNode> type);
        /**
         * @brief 获取局部变量 失败返回nullptr
         * 
         * @param name 
         * @return std::shared_ptr<TypeNode> 
         */
        std::shared_ptr<TypeNode> getGlobalAlias(std::string name);
        /**
         * @brief 当生成另一个函数时，清空局部变量
         * 
         */
        void resetLocals();
    protected:
        /// 局部变量表
        std::map<std::string,std::shared_ptr<Symbol>> localSymbols;
        /// 全局变量表
        std::map<std::string,std::shared_ptr<Symbol>> globalSymbols;
        /// 局部别名表
        std::map<std::string,std::shared_ptr<TypeNode>> localAliases;
        /// 全局别名表
        std::map<std::string,std::shared_ptr<TypeNode>> globalAliases;
        CodegenContext& context;
    };
} // namespace spc



#endif