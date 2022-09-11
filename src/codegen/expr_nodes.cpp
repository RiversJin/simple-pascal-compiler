/**
 * @file expr_nodes.cpp
 * @author Rivers Jin (riversjin@foxmail.com)
 * @brief 表达式节点代码生成
 * @version 0.1
 * @date 2021-05-11
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "utils/ast.hpp"
#include "codegen/codegen_context.hpp"
#include<typeinfo>

namespace spc
{
    llvm::Value *BinopExprNode::codegen(CodegenContext &context) {
        auto *lhs = this->lhs->codegen(context);
        auto *rhs = this->rhs->codegen(context);
        if (lhs->getType()->isIntegerTy(1) && rhs->getType()->isIntegerTy(1))
        {
            llvm::CmpInst::Predicate cmp;
            switch (op)
            {
                case BinaryOperator::GT: cmp = llvm::CmpInst::ICMP_SGT; break;
                case BinaryOperator::GE: cmp = llvm::CmpInst::ICMP_SGE; break;
                case BinaryOperator::LT: cmp = llvm::CmpInst::ICMP_SLT; break;
                case BinaryOperator::LE: cmp = llvm::CmpInst::ICMP_SLE; break;
                case BinaryOperator::EQ: cmp = llvm::CmpInst::ICMP_EQ; break;
                case BinaryOperator::NE: cmp = llvm::CmpInst::ICMP_NE; break;
                default: cmp = llvm::CmpInst::FCMP_FALSE;
            }
            if (cmp != llvm::CmpInst::FCMP_FALSE) return context.builder.CreateICmp(cmp, lhs, rhs);
            llvm::Instruction::BinaryOps binop;
            switch (op)
            {
                case BinaryOperator::AND: binop = llvm::Instruction::And; break;
                case BinaryOperator::OR:  binop = llvm::Instruction::Or; break;
                case BinaryOperator::XOR: binop = llvm::Instruction::Xor; break;
                default: throw CodegenException("operator is invalid: boolean " + to_string(op) + " boolean");
            }
            return context.builder.CreateBinOp(binop, lhs, rhs);
        }
        else if (lhs->getType()->isIntegerTy(32) && rhs->getType()->isIntegerTy(32))
        {
            llvm::CmpInst::Predicate cmp;
            switch (op)
            {
                case BinaryOperator::GT: cmp = llvm::CmpInst::ICMP_SGT; break;
                case BinaryOperator::GE: cmp = llvm::CmpInst::ICMP_SGE; break;
                case BinaryOperator::LT: cmp = llvm::CmpInst::ICMP_SLT; break;
                case BinaryOperator::LE: cmp = llvm::CmpInst::ICMP_SLE; break;
                case BinaryOperator::EQ: cmp = llvm::CmpInst::ICMP_EQ; break;
                case BinaryOperator::NE: cmp = llvm::CmpInst::ICMP_NE; break;
                default: cmp = llvm::CmpInst::FCMP_FALSE;
            }
            if (cmp != llvm::CmpInst::FCMP_FALSE) return context.builder.CreateICmp(cmp, lhs, rhs);
            llvm::Instruction::BinaryOps binop;
            switch (op)
            {
                case BinaryOperator::ADD: binop = llvm::Instruction::Add; break;
                case BinaryOperator::SUB: binop = llvm::Instruction::Sub; break;
                case BinaryOperator::MUL: binop = llvm::Instruction::Mul; break;
                case BinaryOperator::DIV: binop = llvm::Instruction::SDiv; break;
                case BinaryOperator::MOD: binop = llvm::Instruction::SRem; break;
                case BinaryOperator::AND: binop = llvm::Instruction::And; break;
                case BinaryOperator::OR: binop = llvm::Instruction::Or; break;
                case BinaryOperator::XOR: binop = llvm::Instruction::Xor; break;
                case BinaryOperator::TRUEDIV:
                    lhs = context.builder.CreateSIToFP(lhs, context.builder.getDoubleTy());
                    rhs = context.builder.CreateSIToFP(rhs, context.builder.getDoubleTy());
                    binop = llvm::Instruction::FDiv; break;
                default: throw CodegenException("operator is invalid: integer " + to_string(op) + " integer");
            }
            return context.builder.CreateBinOp(binop, lhs, rhs);
        }
        else if (lhs->getType()->isDoubleTy() || rhs->getType()->isDoubleTy())
        {
            if (lhs->getType()->isIntegerTy(32)) lhs = context.builder.CreateSIToFP(lhs, context.builder.getDoubleTy());
            if (rhs->getType()->isIntegerTy(32)) rhs = context.builder.CreateSIToFP(rhs, context.builder.getDoubleTy());
            llvm::CmpInst::Predicate cmp;
            switch (op)
            {
                case BinaryOperator::GT: cmp = llvm::CmpInst::FCMP_OGT; break;
                case BinaryOperator::GE: cmp = llvm::CmpInst::FCMP_OGE; break;
                case BinaryOperator::LT: cmp = llvm::CmpInst::FCMP_OLT; break;
                case BinaryOperator::LE: cmp = llvm::CmpInst::FCMP_OLE; break;
                case BinaryOperator::EQ: cmp = llvm::CmpInst::FCMP_OEQ; break;
                case BinaryOperator::NE: cmp = llvm::CmpInst::FCMP_ONE; break;
                default: cmp = llvm::CmpInst::FCMP_FALSE;
            }
            if (cmp != llvm::CmpInst::FCMP_FALSE) return context.builder.CreateFCmp(cmp, lhs, rhs);
            llvm::Instruction::BinaryOps binop;
            switch (op)
            {
                case BinaryOperator::ADD: binop = llvm::Instruction::FAdd; break;
                case BinaryOperator::SUB: binop = llvm::Instruction::FSub; break;
                case BinaryOperator::MUL: binop = llvm::Instruction::FMul; break;
                case BinaryOperator::TRUEDIV: binop = llvm::Instruction::FDiv; break;
                default: throw CodegenException("operator is invalid: real " + to_string(op) + " real");
            }
            return context.builder.CreateBinOp(binop, lhs, rhs);
        }
        else if (lhs->getType()->isIntegerTy(8) && rhs->getType()->isIntegerTy(8))
        {
            llvm::CmpInst::Predicate cmp;
            switch (op)
            {
                case BinaryOperator::GT: cmp = llvm::CmpInst::ICMP_SGT; break;
                case BinaryOperator::GE: cmp = llvm::CmpInst::ICMP_SGE; break;
                case BinaryOperator::LT: cmp = llvm::CmpInst::ICMP_SLT; break;
                case BinaryOperator::LE: cmp = llvm::CmpInst::ICMP_SLE; break;
                case BinaryOperator::EQ: cmp = llvm::CmpInst::ICMP_EQ; break;
                case BinaryOperator::NE: cmp = llvm::CmpInst::ICMP_NE; break;
                default: throw CodegenException("operator is invalid: char " + to_string(op) + " char");
            }
            return context.builder.CreateICmp(cmp, lhs, rhs);
        }
        else
        {
            throw CodegenException("operator is invalid: " + to_string(op) + " between different types");
        }
    }

    llvm::Value *FuncExprNode::codegen(CodegenContext &context)
    {
        return func_call->codegen(context);
    }
    
    llvm::Value* ArrayRefNode::get_ptr(CodegenContext& context){
        auto symbol = context.symbolTable.getLocalSymbol(identifier->name);
        if(symbol==nullptr)symbol = context.symbolTable.getGlobalSymbol(identifier->name);
        if(symbol==nullptr){
            throw CodegenException(fmt::format("Undefined Identifier named \"{}\"",identifier->name));
        }
        if(symbol->typeNode->type!=Type::ARRAY){
            throw CodegenException(fmt::format("Identifier \"{}\" is not a array!",identifier->name));
        }
        auto ptr=symbol->get_llvmptr();
        auto array=cast_node<ArrayTypeNode>(symbol->typeNode);
        int base=array->range->low->val;

        auto rawindex=context.builder.CreateSExt(index->codegen(context),context.builder.getInt64Ty()); //先扩展到64位
        auto index=context.builder.CreateSub(rawindex,context.builder.getInt64(base),"index");
        auto targetPtr=context.builder.CreateInBoundsGEP(ptr,{context.builder.getInt64(0),index},"targetPtr");
        return targetPtr;
    }

    llvm::Value* ArrayRefNode::codegen(CodegenContext& context){
        return context.builder.CreateLoad(get_ptr(context));
    }

    llvm::Value* RecordRefNode::get_ptr(CodegenContext& context){
        auto symbol=context.symbolTable.getLocalSymbol(identifier->name);
        if(symbol==nullptr)symbol = context.symbolTable.getGlobalSymbol(identifier->name);
        if(symbol==nullptr){
            throw CodegenException(fmt::format("Undefined Identifier named \"{}\"",identifier->name));
        }
        if(symbol->typeNode->type!=Type::RECORD){
            throw CodegenException(fmt::format("Identifier \"{}\" is not a record!",identifier->name));
        }
        auto llvm_ptr=symbol->get_llvmptr();
        auto record=cast_node<RecordTypeNode>(symbol->typeNode);
        std::string field_name=field->name;
        if(record->indexes.count(field_name)==0){
            throw CodegenException(fmt::format("Record \"{}\" has no field named \"{}\"",identifier->name,field->name));
        }
        int index=record->indexes[field_name];
        auto targetPtr=context.builder.CreateInBoundsGEP(llvm_ptr,{context.builder.getInt32(0),context.builder.getInt32(index)},"targetPtr");
        return targetPtr;
    }
    llvm::Value* RecordRefNode::codegen(CodegenContext& context){
        return context.builder.CreateLoad(get_ptr(context));
    }
}
