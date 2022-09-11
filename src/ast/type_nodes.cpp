/**
 * @file type_nodes.cpp
 * @brief 实现了类型节点的一些代码生成函数 严格来说应该归于codegen文件夹，但是在AST树构建的时候也用到了，所以放在这里
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "ast_base.h"
#include <fmt/core.h>
#include <map>
using namespace spc;

std::string spc::type2string(Type type){
    const std::map<Type, std::string> type_to_string{
        {Type::UNDEFINED, "<undefined-type>"},
        {Type::VOID,      "void"},
        {Type::BOOLEAN,   "boolean"},
        {Type::INTEGER,   "integer"},
        {Type::REAL,      "real"},
        {Type::CHAR,      "char"},
        {Type::STRING,    "string"},
        {Type::ARRAY,     "array"},
        {Type::RECORD,    "record"},
        {Type::SET,       "set"}
    };
    return type_to_string.at(type);
}

std::string SimpleTypeNode::json_head() const {
    return fmt::format("\"type\": \"Type\", \"name\":\"{}\"",type2string(this->type));
}

std::string StringTypeNode::json_head() const {
    return fmt::format("\"type\": \"Type\", \"name\":\"{}\"",type2string(this->type));
}

std::string AliasTypeNode::json_head() const {
    return fmt::format("\"type\": \"Type\", \"name\": \"alias\", \"identifier\":{{{0}}}",this->identifier->to_json());
}

RangeNode::RangeNode(const NodePtr& min,const NodePtr& max):low(cast_node<IntegerNode>(min)),high(cast_node<IntegerNode>(max)){
    length=high->val-low->val+1;
    assert(length>0); //这个暂时用assert 以后可能要使用更人性化的错误提示
}
std::string RangeNode::json_head()const{
    return fmt::format("\"lowerbound\":{0}, \"upperbound\":{1}, \"length\":{2}",low->val,high->val,length);
}

std::string ArrayTypeNode::json_head() const {
    return fmt::format("\"type\": \"Type\", \"name\":\"{0}\", \"inner_type\":{{{1}}},\"range\":{{{2}}} ",type2string(this->type),element_type->json_head(),range->json_head());
}

RecordTypeNode::RecordTypeNode(){
    type = Type::RECORD;
    int i=0;
    for(const auto& p:children()){
        auto fieldPtr=cast_node<VarDeclNode>(p);
        fields[fieldPtr->name->name]=fieldPtr->type;
        indexes[fieldPtr->name->name]=i++;
        std::cout<<fieldPtr->name->name<<std::endl;
    }

}
void RecordTypeNode::add_child(const std::shared_ptr<AbstractNode> &node){
    auto fieldPtr=cast_node<VarDeclNode>(node);
    fields[fieldPtr->name->name]=fieldPtr->type;
    indexes[fieldPtr->name->name]=i++;
    AbstractNode::add_child(node);
}

void RecordTypeNode::add_child(std::shared_ptr<AbstractNode> &&node){
    auto fieldPtr=cast_node<VarDeclNode>(node);
    fields[fieldPtr->name->name]=fieldPtr->type;
    indexes[fieldPtr->name->name]=i++;
    AbstractNode::add_child(node);
}