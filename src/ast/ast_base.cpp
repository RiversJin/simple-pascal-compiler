/**
 * @file ast_base.cpp
 * @brief AST语义节点的一些具体实现，为了避免递归引用，部分成员函数在这里实现。但主要还是在头文件中完成
 * @version 0.1
 * @date 2021-05-10
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <sstream>
#include <iostream>
#include <fmt/core.h>
#include "ast_base.h"

using namespace spc;
using namespace std;




void AbstractNode::print_json() const {
    clog<<this->to_json()<<endl;
}

string AbstractNode::to_json() const {
    std::stringstream ret;
    ret << "{";
    ret << this->json_head();
    if (this->should_have_children())
    {
        ret << ", \"children\": [";
        bool is_first = true;
        for (auto &node : this->_children)
        {
            if (is_first) is_first = false;
            else ret << ", ";
            ret << node->to_json();
        }
        ret << "]";
    }
    ret << "}";
    return ret.str();
}

string SysCallNode::json_head()const{
    return std::string{"\"type\": \"SysCall\", \"identifier\": \""} + to_string(this->routine->routine) +
                   "\", \"args\": " + this->args->to_json();
}

SysCallNode::SysCallNode(const NodePtr &routine, const NodePtr &args)
                : routine(cast_node<SysRoutineNode>(routine)), args(cast_node<ArgListNode>(args))
        {}