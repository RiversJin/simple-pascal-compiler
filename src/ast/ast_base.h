/**
 * @file ast_base.h
 * @author Rivers Jin (riversjin@foxmail.com)
 * @brief AST基类头文件 定义所需的基类
 * @version 0.1
 * @date 2021-03-31
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef AST_BASE_H
#define AST_BASE_H

#include <cassert>
#include <string>
#include <memory>
#include <iostream>
#include <sstream>
#include <map>
#include <algorithm>
#include <locale>
#include <fmt/core.h>
#include <list>
#include <llvm/IR/Value.h>
#include <typeinfo>

/**
 * @brief simple pascal compiler 
*/
namespace spc{
    /** 前置声明区 **/

    //语义节点
    struct AbstractNode;
    struct DummyNode;
    struct ExprNode;
    struct LeftValueExprNode;
    struct StmtNode;
    struct IdentifierNode;
    struct TypeNode;
    struct StringTypeNode;
    struct AliasTypeNode;
    struct IntegerNode;
    struct RangeNode;
    struct ArrayTypeNode;
    struct RecordTypeNode;
    struct SetTypeNode;
    struct ConstValueNode;
    struct BooleanNode;
    struct IntegerNode;
    struct RealNode;
    struct CharNode;
    struct StringNode;
    struct ArrayRefNode;
    struct RecordRefNode;
    struct BinopExprNode;
    struct FuncExprNode;
    struct SysRoutineNode;
    struct SysCallNode;
    struct ArgListNode;
    struct ParamDeclNode;
    struct ParamListNode;
    struct VarDeclNode;
    struct VarListNode;
    struct ConstDeclNode;
    struct ConstListNode;
    struct TypeDefNode;
    struct TypeListNode;
    struct NameListNode;
    struct RoutineCallNode;
    struct SubroutineListNode;
    struct HeadListNode;
    struct RoutineNode;
    struct ProgramNode;
    struct SubroutineNode;
    struct CompoundStmtNode;
    struct AssignStmtNode;
    struct ProcStmtNode;
    struct IfStmtNode;
    struct RepeatStmtNode;
    struct WhileStmtNode;
    struct ForStmtNode;
    struct CaseExprNode;
    struct CaseStmtNode;
    struct StmtList;

    struct CodegenContext;


    using NodePtr = std::shared_ptr<AbstractNode>;

    //一些类型转换用的函数
    /**
     * @brief 确认给定的一个基类ptr指向的真正对象是不是想要的类型
     * 
     * @tparam NodeType 
     * @param ptr 
     * @return true 
     * @return false 
     */
    template<typename NodeType>
    bool is_a_ptr_of(const std::shared_ptr<AbstractNode> &ptr)
    {
        auto _p = ptr.get();
        return dynamic_cast<NodeType *>(_p) != nullptr;
    }

    inline std::shared_ptr<AbstractNode> wrap_node(AbstractNode *node)
    {
        return std::shared_ptr<AbstractNode>{node};
    }

    /**
     * @brief 给定基类，返回想要的目标类型指针(注意 模板参数为指定类型，而不是指定类型的指针)
     * 
     * @tparam TNode 
     * @param node 
     * @return std::enable_if<std::is_base_of<AbstractNode, TNode>::value, std::shared_ptr<TNode>>::type 
     */
    template<typename TNode>
    typename std::enable_if<std::is_base_of<AbstractNode, TNode>::value, std::shared_ptr<TNode>>::type
    cast_node(const std::shared_ptr<AbstractNode> &node)
    {
       if(is_a_ptr_of<TNode>(node))
           return std::dynamic_pointer_cast<TNode>(node);
       //std::string nodeTypeName = typeid(*node.get()).name();
       assert(is_a_ptr_of<TNode>(node));
       return nullptr;
    }
    /**
     * @brief 一个助手函数，生成指定的AST节点，并返回指向此节点的基类指针
     * 
     * @tparam NodeType 
     * @tparam Args 
     * @param args 
     * @return std::shared_ptr<AbstractNode> 
     */
    template<typename NodeType, typename...Args>
    std::shared_ptr<AbstractNode> make_node(Args &&...args)
    {
        return std::dynamic_pointer_cast<AbstractNode>(std::make_shared<NodeType>(std::forward<Args>(args)...));
    };

    /**
     * @brief 所有AST节点的基类 是一个纯虚基类
     * 
     */
    struct AbstractNode : public std::enable_shared_from_this<AbstractNode>{
        /**
         * @brief 指向此节点子节点的指针容器
         * 
         */
        std::list<std::shared_ptr<AbstractNode>> _children;
        /**
         * @brief 指向此节点的父节点，注意为了避免循环引用导致内存泄漏，这里要使用weak——ptr
         * 
         */
        std::weak_ptr<AbstractNode> _parent;

        virtual ~AbstractNode() noexcept = default;
        /**
         * @brief 调用此函数进行代码生成，每个节点（即此类的派生类）都应重写此虚函数以实现各种语义的代码生成
         * 
         * @param context 代码生成的上下文
         * @return llvm::Value* 
         */
        virtual llvm::Value *codegen(CodegenContext& context)=0;
        /**
         * @brief 将AST打印至clog（standard logging stream),它实际上会调用print_json函数
         * 
         */
        void print_json()const;
        /**
         * @brief 将AST转换成json形式字符串
         * 
         * @return std::string 
         */
        std::string to_json()const;
        /**
         * @brief 如果此节点可以有子节点的话,返回存放子节点的容器
         * 
         * @return std::list<std::shared_ptr<AbstractNode>>& 
         */
        std::list<std::shared_ptr<AbstractNode>>& children() noexcept{
            assert(this->should_have_children());
            return this->_children;
        }
        /**
         * @brief 返回此节点的父节点
         * 
         * @return std::weak_ptr<AbstractNode>
         */
        auto& parent() noexcept{
            return this->_parent;
        }
        /**
         * @brief 添加子节点
         * 
         * @param node 指向子节点的指针
         */
        virtual void add_child(const std::shared_ptr<AbstractNode> &node)
        {
            this->_children.push_back(node);
            node->parent() = this->shared_from_this();
        }
        /**
         * @brief 添加子节点
         * 
         * @param node 指向子节点的指针
         */
        virtual void add_child(std::shared_ptr<AbstractNode> &&node)
        {
            this->_children.push_back(node);
            node->parent() = this->shared_from_this();
        }
        /**
         * @brief 子节点合并.给定节点容器内的节点添加到此节点的子节点列表
         * 
         * @param children 存放子节点指针的容器
         */
        void merge_children(const std::list<std::shared_ptr<AbstractNode>> &children)
        {
            for (const auto &e : children)
            {
                this->add_child(e);
            }
        }
        /**
         * @brief 子节点合并.将传入的节点的子节点添加到此节点的子节点列表
         * 
         * @param node 传入的语义节点
         */
        void lift_children(const std::shared_ptr<AbstractNode> &node)
        {
            this->merge_children(node->children());
        }
        /**
         * @brief 此节点是否具有子节点
         * 
         * @return true 
         * @return false 
         */
        virtual bool should_have_children() const
        { return true; }
        /**
         * @brief 打印json用的一个函数
         * @return std::string
         */
        virtual std::string json_head() const = 0;
    };

    /**
     * @brief 语义节点基类 它是"AbstractNode"与实际语义节点中间的一层抽象，用来简化语义节点的设计
     */
    struct DummyNode : public AbstractNode
    {
    public:
        /**
         * @brief 代码生成函数
         * @param context 
         * @return llvm::Value* 
         */
        llvm::Value *codegen(CodegenContext &context) override
        {
            std::cout << typeid(*this).name() << std::endl;
            assert(false);
            return nullptr;
        }
        
        std::string json_head() const override
        {
            return std::string{"\"type\": \"<unspecified-from-dummy>\""};
        }
    };
    /**
     * @brief 表达式节点,这是个纯虚类
     */
    struct ExprNode : public DummyNode
    {
        std::shared_ptr<TypeNode> type;
        //virtual llvm::Value* get_ptr(CodegenContext& context)=0;
        
    protected:
        ExprNode() = default;
    };
    /**
     * @brief 左值表达式(即可被赋值的东西)
     * 
     */
    struct LeftValueExprNode : public ExprNode{
        /**
         * @brief Get the ptr object 获得左值目标指针为了赋值使用
         * 
         * @param context 代码生成上下文
         * @return llvm::Value* 指这个左值在llvm IR中的指针
         */
        virtual llvm::Value* get_ptr(CodegenContext& context)=0;
        /**
         * 左值名
         */
        std::string name;
    };
    /**
     * @brief Statement语句(纯虚基类)
     * 
     */
    struct StmtNode : public DummyNode
    {
    protected:
        StmtNode() = default;
    };

    /**
     * @brief 表达式语义节点
     */
    struct IdentifierNode : public LeftValueExprNode
    {
    public:
        /**
         * @brief Construct a new Identifier Node object
         * 
         * @param c 字符串
         */
        explicit IdentifierNode(const char *c)
        {
            name=std::string(c);
            std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c){ return std::tolower(c); }); //为了忽略大小写，所以全部转为小写
        }

        llvm::Value *get_ptr(CodegenContext &context) override;
        llvm::Value *codegen(CodegenContext &context) override;

    protected:
        std::string json_head() const override
        {
            return std::string{"\"type\": \"Identifier\", \"name\": \""}
                   + this->name + "\"";
        }

        bool should_have_children() const override
        { return false; }
    };

    struct IdentifierNode;
    /**
     * @brief 定义数据类型
     * 
     */
    enum class Type
    {
        /// 未定义
        UNDEFINED, 
        /// 空
        VOID,
        /// 布尔
        BOOLEAN, 
        /// 整数
        INTEGER, 
        /// 浮点数
        REAL, 
        /// 字符
        CHAR,
        /// 字符串
        STRING, 
        /// 数组
        ARRAY, 
        /// 记录
        RECORD, 
        /// 集合
        SET //这个set大概率是不会实现了，不过来都来了，先留个坑吧
    };
    
    /**
     * @brief 输出Type枚举的字符串形式 用来解释此类型是什么用
     * 
     * @param type 
     * @return std::string 
     */
    std::string type2string(Type type);
    /**
     * @brief 类型语义基础节点 是纯虚类
     * 
     */
    struct TypeNode : public DummyNode{
        Type type = Type::UNDEFINED;
        llvm::Type *get_llvm_type(CodegenContext& context)const; 

        TypeNode() = default;
        virtual std::string json_head() const = 0;
        virtual bool should_have_children() const = 0;
    };
    /**
     * @brief 简单数据类型的语义节点 也就是不包括array,record,set这些的
     * 
     */
    struct  SimpleTypeNode : public TypeNode{
        using TypeNode::type;
        SimpleTypeNode(Type type){
            this->type=type;
        }
        virtual std::string json_head() const override;
        virtual bool should_have_children() const override {
            return false;
        }
    };
    /**
     * @brief 字符串数据类型节点
     * 
     */
    struct StringTypeNode : public TypeNode
    {
    public:
        StringTypeNode()
        { type = Type::STRING; }
        virtual std::string json_head() const override;
        virtual bool should_have_children() const override {
            return false;
        }
    };

    /**
     * @brief 别名类型节点
     * 
     */
    struct AliasTypeNode : public TypeNode
    {
    public:
        std::shared_ptr<IdentifierNode> identifier;

        AliasTypeNode(const NodePtr &identifier)
                : identifier(cast_node<IdentifierNode>(identifier))
        {
            // TODO: assert the identifier is a type
            // emm...也许不应该在构建语法树的时候做？
        }
        virtual std::string json_head() const override;
        virtual bool should_have_children() const override {
            return false;
        }
    };

    struct IntegerNode; //前置声明 类型本身应该是数据的元信息，但在这里类型又需要数据，有点扭曲...
    /**
     * @brief 子界节点
     * 
     */
    struct RangeNode : public DummyNode{
    public:
        /// 数组上界
        std::shared_ptr<IntegerNode> low;
        /// 数组下界
        std::shared_ptr<IntegerNode> high;
        /// 整个数组的长度
        int length;
        RangeNode(const NodePtr& min,const NodePtr& max);
        std::string json_head() const override;
    };
    /**
     * @brief 数组类型节点
     * 
     */
    struct ArrayTypeNode : public TypeNode
    {
    public:
        /**
         * @brief 数组范围
         * @see RangeNode
         */ 
        std::shared_ptr<RangeNode> range;
        /**
         * @brief 数组元素类型
        */
        std::shared_ptr<TypeNode> element_type;

        ArrayTypeNode(const NodePtr &range, const NodePtr &element_type)
                : range(cast_node<RangeNode>(range)), element_type(cast_node<TypeNode>(element_type))
        {
            type = Type::ARRAY;
        }

        virtual std::string json_head() const override;
        virtual bool should_have_children() const override {
            return false;
        }
    };
    /**
     * @brief 记录类型语义节点
     */ 
    struct RecordTypeNode : public TypeNode
    {
    public:
        std::map<std::string,std::shared_ptr<TypeNode>> fields;
        std::map<std::string,int> indexes;
        llvm::Type* innertype;
        RecordTypeNode();
        void add_child(const std::shared_ptr<AbstractNode> &node) override;
        void add_child(std::shared_ptr<AbstractNode> &&node) override;
        virtual std::string json_head() const override{
            return fmt::format("\"type\": \"Type\", \"name\":\"{}\"",type2string(this->type));
        }
        virtual bool should_have_children() const override {
            return true;
        }
    private:
        int i=0;
    };
    /**
     * @brief 集合类型语义节点
     */
    struct SetTypeNode : public TypeNode
    {
    public:
        SetTypeNode()
        {
            type = Type::SET;
        }
        virtual std::string json_head() const override{
            return fmt::format("\"type\": \"Type\", \"name\":\"{}\"",type2string(this->type));
        }
        virtual bool should_have_children() const override {
            return false;
        }
    };
    /**
     * @brief 常量语义节点 这是一个纯虚类
     */
    struct ConstValueNode : public ExprNode
    {
    public:
        /**
         * @brief Get the llvm type object
         * 
         * @param context 代码生成上下文
         * @return llvm::Type* 
         */
        llvm::Type *get_llvm_type(CodegenContext &context) const;

    protected:
        ConstValueNode(){
            type=nullptr;
        }

        bool should_have_children() const final
        { return false; }
    };
    /**
     * @brief 布尔字面量语义节点
     * 
     */
    struct BooleanNode : public ConstValueNode
    {
    public:
        /// 布尔值
        bool val;

        BooleanNode(bool val) : val(val)
        {
            type = std::make_shared<SimpleTypeNode>(Type::BOOLEAN);
        }

        llvm::Value *codegen(CodegenContext &context) override;

    protected:
        std::string json_head() const override
        {
            return std::string{"\"type\": \"Boolean\", \"value\": \""} +
                   (val ? "true" : "false") + "\"";
        }
    };
    /**
     * @brief 整数字面量语义节点
     * 
     */
    struct IntegerNode : public ConstValueNode
    {
    public:
        /// 值
        int val;

        IntegerNode(int val) : val(val)
        {
            type = std::make_shared<SimpleTypeNode>(Type::INTEGER);
        }
        /**
         * @brief Construct a new Integer Node object
         * 
         * @param val 可以是int类型，也可以传入字符串解析
         */
        IntegerNode(const char *val)
        {
            std::stringstream ss;
            ss << val;
            ss >> this->val;
            type = std::make_shared<SimpleTypeNode>(Type::INTEGER);
        }

        llvm::Value *codegen(CodegenContext &context) override;

    protected:
        std::string json_head() const override
        {
            return std::string{"\"type\": \"Integer\", \"value\": \""} + std::to_string(val) + "\"";
        }
    };
    /**
     * @brief 浮点数字面量语义节点
     * 
     */
    struct RealNode : public ConstValueNode
    {
    public:
        double val;
        /**
         * @brief Construct a new Real Node object
         * 
         * @param val 可以是double类型，也可以传入字符串解析
         * @see IntegerNode
         */
        RealNode(double val) : val(val)
        {
            type = std::make_shared<SimpleTypeNode>(Type::REAL);
        }

        RealNode(const char *val)
        {
            std::stringstream ss;
            ss << val;
            ss >> this->val;
            type = std::make_shared<SimpleTypeNode>(Type::REAL);
        }

        llvm::Value *codegen(CodegenContext &context) override;

    protected:
        std::string json_head() const override
        {
            return std::string{"\"type\": \"Real\", \"value\": \""} + std::to_string(val) + "\"";
        }
    };
    /**
     * @brief 字符字面量语义节点
     * 
     */
    struct CharNode : public ConstValueNode
    {
    public:
        char val;

        CharNode(const char val) : val(val)
        {
            type = std::make_shared<SimpleTypeNode>(Type::CHAR);
        }

        CharNode(const char *val)
        {
            this->val = val[1];
            type = std::make_shared<SimpleTypeNode>(Type::CHAR);
        }

        llvm::Value *codegen(CodegenContext &context) override;

    protected:
        std::string json_head() const override
        {
            return std::string{"\"type\": \"Char\", \"value\": \""} + std::to_string(val) + "\"";
        }
    };
    /**
     * @brief 字符串语义节点
     * 
     */
    struct StringNode : public ConstValueNode
    {
    public:
        std::string val;

        StringNode(const char *val) : val(val)
        {
            this->val.erase(this->val.begin());
            this->val.pop_back();
            type = std::make_shared<SimpleTypeNode>(Type::STRING);
        }

        llvm::Value *codegen(CodegenContext &context) override;

    protected:
        std::string json_head() const override
        {
            return std::string{"\"type\": \"String\", \"value\": \""} + val + "\"";
        }
    };
    /**
     * @brief 数组引用语义节点 比如nums[8]
     */
    struct ArrayRefNode : public LeftValueExprNode
    {
    public:
        /// 数组名称
        std::shared_ptr<IdentifierNode> identifier;
        /// 想要获取元素的下标
        std::shared_ptr<ExprNode> index;

        ArrayRefNode(const NodePtr &identifier, const NodePtr &index)
                : identifier(cast_node<IdentifierNode>(identifier)), index(cast_node<ExprNode>(index))
        {
            name=this->identifier->name;
        }
        llvm::Value *get_ptr(CodegenContext& context)override;
        llvm::Value *codegen(CodegenContext& context)override;

    protected:
        std::string json_head() const override
        {
            return std::string{"\"type\": \"ArrayRef\", \"identifier\": "} +
                   this->identifier->to_json() +
                   ", \"index\": " +
                   this->index->to_json();
        }

        bool should_have_children() const override
        { return false; }
    };
    /**
     * @brief 记录引用语义节点
     * @see ArrayRefNode
     * 
     */
    struct RecordRefNode : public LeftValueExprNode
    {
    public:
        /// 记录名
        std::shared_ptr<IdentifierNode> identifier;
        /// 字段名
        std::shared_ptr<IdentifierNode> field;


        RecordRefNode(const NodePtr &identifier, const NodePtr &field)
                : identifier(cast_node<IdentifierNode>(identifier)), field(cast_node<IdentifierNode>(field))
        {}
        llvm::Value *get_ptr(CodegenContext& context)override;
        llvm::Value *codegen(CodegenContext& context)override;

    protected:
        std::string json_head() const override
        {
            return std::string{"\"type\": \"RecordRef\", \"identifier\": "} +
                   this->identifier->to_json() +
                   ", \"field\": " +
                   this->field->to_json();
        }

        bool should_have_children() const override
        { return false; }
    };
    /// 二元运算符
    enum class BinaryOperator
    {
        /// Greater than
        GT, 
        /// Greater or equal
        GE, 
        /// Less than
        LT, 
        /// Less or equal
        LE, 
        /// Euqual
        EQ, 
        /// Not
        NE, 
        /// 加
        ADD, 
        /// 减
        SUB, 
        /// 乘
        MUL, 
        /// 除
        TRUEDIV, 
        DIV, 
        /// 取余
        MOD, 
        /// 逻辑运算 且
        AND, 
        /// 或
        OR, 
        /// 异或
        XOR
    };
    /**
     * @brief 给定二元运算符枚举变量，返回它的字符串形式
     * @see BinaryOperator
     * 
     * @param binop 
     * @return std::string 
     */
    inline std::string to_string(BinaryOperator binop)
    {
        std::map<BinaryOperator, std::string> binop_to_string{
                {BinaryOperator::GT,      ">"},
                {BinaryOperator::GE,      ">="},
                {BinaryOperator::LT,      "<"},
                {BinaryOperator::LE,      "<="},
                {BinaryOperator::EQ,      "="},
                {BinaryOperator::NE,      "<>"},
                {BinaryOperator::ADD,     "+"},
                {BinaryOperator::SUB,     "-"},
                {BinaryOperator::MUL,     "*"},
                {BinaryOperator::TRUEDIV, "/"},
                {BinaryOperator::DIV,     "div"},
                {BinaryOperator::MOD,     "mod"},
                {BinaryOperator::AND,     "and"},
                {BinaryOperator::OR,      "or"},
                {BinaryOperator::XOR,     "xor"}
        };
        // TODO: bound checking
        return binop_to_string[binop];
    }
    /**
     * @brief 二元表达式语义节点
     * 
     */
    struct BinopExprNode : public ExprNode
    {
    public:
        /// 运算符
        BinaryOperator op;
        /// 二元运算符左部
        std::shared_ptr<ExprNode> lhs;
        /// 二元运算符右部
        std::shared_ptr<ExprNode> rhs;

        BinopExprNode(BinaryOperator op, const NodePtr &lhs, const NodePtr &rhs)
                : op(op), lhs(cast_node<ExprNode>(lhs)), rhs(cast_node<ExprNode>(rhs))
        {}

        llvm::Value *codegen(CodegenContext &context) override;

    protected:
        bool should_have_children() const override
        { return false; }

        std::string json_head() const override
        {
            return std::string{"\"type\": \"BinopExpr\", \"op\": \""} +
                   to_string(this->op) +
                   "\", \"lhs\": " +
                   this->lhs->to_json() +
                   ", \"rhs\": " +
                   this->rhs->to_json();
        }
    };

    /**
     * @brief 函数调用表达式语义节点 比如 foo(233)
     * 
     */
    struct FuncExprNode : public ExprNode
    {
    public:
        NodePtr func_call;

        FuncExprNode(const NodePtr &func_call) : func_call(func_call)
        {
            assert(is_a_ptr_of<RoutineCallNode>(func_call) || is_a_ptr_of<SysCallNode>(func_call));
        }

        llvm::Value *codegen(CodegenContext &context) override;

    protected:
        std::string json_head() const override
        {
            return std::string{"\"type\": \"FuncExpr\", \"call\": "} + this->func_call->to_json();
        }

        bool should_have_children() const override
        { return false; }
    };
    /// 系统函数枚举
    enum class SysRoutine
    {
        /// 求绝对值
        ABS, 
        /// 返回给定数字的字符串形式
        CHR, 
        /// 将字符转换为ascii码
        ORD, 
        /// 返回指定字符ascii的前驱
        PRED,
        /// 返回指定字符ascii的后继 
        SUCC,
        /// 开方
        SQRT, 
        /// 读取 不丢弃回车
        READ, 
        /// 读取 丢弃回车
        READLN, 
        /// 输出 
        WRITE, 
        /// 输出并回车
        WRITELN
    };
    /**
     * @brief 给定一个系统函数枚举变量，返回它的字符串说明
     * 
     * @param routine 
     * @return std::string 
     */
    inline std::string to_string(SysRoutine routine)
    {
        std::map<SysRoutine, std::string> routine_to_string{
                {SysRoutine::ABS,     "abs"},
                {SysRoutine::CHR,     "chr"},
                {SysRoutine::ORD,     "ord"},
                {SysRoutine::PRED,    "pred"},
                {SysRoutine::SQRT,    "sqrt"},
                {SysRoutine::SUCC,    "succ"},
                {SysRoutine::READ,    "read"},
                {SysRoutine::READLN,  "readln"},
                {SysRoutine::WRITE,   "write"},
                {SysRoutine::WRITELN, "writeln"}
        };
        // TODO: bound checking
        return routine_to_string[routine];
    }
    /// 系统函数调用语义节点
    struct SysRoutineNode : public DummyNode
    {
    public:
        SysRoutine routine;

        explicit SysRoutineNode(SysRoutine routine) : routine(routine)
        {}

    protected:
        bool should_have_children() const override
        { return false; }
    };
    /**
     * @brief 系统函数调用语义节点
     * 
     */
    struct SysCallNode : public DummyNode
    {
    public:
        std::shared_ptr<SysRoutineNode> routine;
        std::shared_ptr<ArgListNode> args;

        SysCallNode(const NodePtr &routine, const NodePtr &args);

        explicit SysCallNode(const NodePtr &routine)
                : SysCallNode(routine, make_node<ArgListNode>())
        {}

        llvm::Value *codegen(CodegenContext &context) override;

    protected:
        std::string json_head() const override;

        bool should_have_children() const override
        { return false; }
    };
    /**
     * @brief 参数列表语义节点
     * 
     */
    struct ArgListNode : public DummyNode
    {
    protected:
        std::string json_head() const override
        {
            return std::string{"\"type\": \"ArgList\""};
        }

        bool should_have_children() const override
        { return true; }
    };
    /**
     * @brief 程序（函数）声明语义节点
     * 
     */
    struct ParamDeclNode : public DummyNode
    {
    public:
        /// 程序名
        std::shared_ptr<IdentifierNode> name;
        /// 返回类型
        std::shared_ptr<TypeNode> type;

        ParamDeclNode(const NodePtr &name, const NodePtr &type)
                : name(cast_node<IdentifierNode>(name)), type(cast_node<TypeNode>(type))
        {
            assert(is_a_ptr_of<SimpleTypeNode>(type) || is_a_ptr_of<AliasTypeNode>(type));
        }

    protected:
        std::string json_head() const override
        {
            return std::string{"\"type\": \"ParamDecl\", \"name\": "} +
                   this->name->to_json() +
                   ", \"decl\": " +
                   this->type->to_json();
        }

        bool should_have_children() const override
        { return false; }
    };
    /// 程序列表语义节点
    struct ParamListNode : public DummyNode
    {
    protected:
        std::string json_head() const override
        {
            return std::string{"\"type\": \"ParamList\""};
        }

        bool should_have_children() const override
        { return true; }
    };
    /// 变量声明语义节点
    struct VarDeclNode : public DummyNode
    {
    public:
        /// 变量名
        std::shared_ptr<IdentifierNode> name;
        /// 变量类型
        std::shared_ptr<TypeNode> type;

        VarDeclNode(const NodePtr &name, const NodePtr &type)
                : name(cast_node<IdentifierNode>(name)), type(cast_node<TypeNode>(type))
        {}

        llvm::Value *codegen(CodegenContext &context) override;

    protected:
        std::string json_head() const override
        {
            return std::string{"\"type\": \"VarDecl\", \"name\": "} +
                   this->name->to_json() +
                   ", \"decl\": " +
                   this->type->to_json();
        }

        bool should_have_children() const override
        { return false; }
    };
    /// 变量列表语义节点
    struct VarListNode : public DummyNode
    {
    public:
        llvm::Value *codegen(CodegenContext &context) override;

    protected:
        std::string json_head() const override
        {
            return std::string{"\"type\": \"VarList\""};
        }

        bool should_have_children() const override
        { return true; }
    };
    /// 常量声明语义节点
    struct ConstDeclNode : public DummyNode
    {
    public:
        std::shared_ptr<IdentifierNode> name;
        std::shared_ptr<ConstValueNode> value;

        ConstDeclNode(const NodePtr &name, const NodePtr &value)
                : name(cast_node<IdentifierNode>(name)), value(cast_node<ConstValueNode>(value))
        {}

        llvm::Value *codegen(CodegenContext &context) override;

    protected:
        std::string json_head() const override
        {
            return std::string{"\"type\": \"ConstDecl\", \"name\": "} +
                   this->name->to_json() +
                   ", \"value\": " +
                   this->value->to_json();
        }

        bool should_have_children() const override
        { return false; }
    };
    /// 常量列表语义节点
    struct ConstListNode : public DummyNode
    {
    public:
        llvm::Value *codegen(CodegenContext &context) override;

    protected:
        std::string json_head() const override
        {
            return std::string{"\"type\": \"ConstList\""};
        }

        bool should_have_children() const override
        { return true; }
    };
    /// 类型定义语义节点
    struct TypeDefNode : public DummyNode
    {
    public:
        std::shared_ptr<IdentifierNode> name;
        std::shared_ptr<TypeNode> type;

        TypeDefNode(const NodePtr &name, const NodePtr &type)
                : name(cast_node<IdentifierNode>(name)), type(cast_node<TypeNode>(type))
        {}

        llvm::Value *codegen(CodegenContext &context) override;

    protected:
        std::string json_head() const override
        {
            return std::string{"\"type\": \"TypeDef\", \"name\": "} +
                   this->name->to_json() +
                   ", \"alias\": " +
                   this->type->to_json();
        }

        bool should_have_children() const override
        { return false; }
    };
    /// 类型列表语义节点
    struct TypeListNode : public DummyNode
    {
    public:
        llvm::Value *codegen(CodegenContext &context) override;

    protected:
        std::string json_head() const override
        {
            return std::string{"\"type\": \"TypeList\""};
        }

        bool should_have_children() const override
        { return true; }
    };

    // Intermediate container, would be removed
    struct NameListNode : public DummyNode
    {
    };
    /// 过程调用语义节点
    struct RoutineCallNode : public DummyNode
    {
    public:
        /// 函数名
        std::shared_ptr<IdentifierNode> identifier;
        /// 实参
        std::shared_ptr<ArgListNode> args;

        RoutineCallNode(const NodePtr &identifier, const NodePtr &args)
                : identifier(cast_node<IdentifierNode>(identifier)), args(cast_node<ArgListNode>(args))
        {}

        explicit RoutineCallNode(const NodePtr &identifier)
                : RoutineCallNode(identifier, make_node<ArgListNode>())
        {}

        llvm::Value *codegen(CodegenContext &context) override;

    protected:
        std::string json_head() const override
        {
            return std::string{"\"type\": \"RoutineCall\", \"identifier\": "} + this->identifier->to_json() +
                   ", \"args\": " + this->args->to_json();
        }

        bool should_have_children() const final
        { return false; }
    };
    /// 子过程列表
    struct SubroutineListNode : public DummyNode
    {
    public:
        llvm::Value *codegen(CodegenContext &context) override;

    protected:
        std::string json_head() const override
        {
            return std::string{"\"type\": \"SubroutineList\""};
        }

        bool should_have_children() const final
        { return true; }
    };
    /// 存放头部（如子过程，变量，常量）的容器节点
    struct HeadListNode : public DummyNode
    {
    public:
        std::shared_ptr<ConstListNode> const_list;
        std::shared_ptr<TypeListNode> type_list;
        std::shared_ptr<VarListNode> var_list;
        std::shared_ptr<SubroutineListNode> subroutine_list;

        HeadListNode(const NodePtr &consts, const NodePtr &types, const NodePtr &vars, const NodePtr &subroutines)
                : const_list(cast_node<ConstListNode>(consts)), type_list(cast_node<TypeListNode>(types)),
                  var_list(cast_node<VarListNode>(vars)), subroutine_list(cast_node<SubroutineListNode>(subroutines))
        {}

        llvm::Value *codegen(CodegenContext &context) override;

    protected:
        std::string json_head() const override
        {
            return std::string{"\"type\": \"HeadList\", \"consts\": "} +
                   this->const_list->to_json() +
                   ", \"types\": " +
                   this->type_list->to_json() +
                   ", \"vars\": " +
                   this->var_list->to_json() +
                   ", \"subroutines\": " +
                   this->subroutine_list->to_json();
        }

        bool should_have_children() const final
        { return false; }
    };
    /// 过程语义节点
    struct RoutineNode : public DummyNode
    {
    public:
        std::shared_ptr<IdentifierNode> name;
        std::shared_ptr<HeadListNode> head_list;

        RoutineNode(const NodePtr &name, const NodePtr &head_list)
                : name(cast_node<IdentifierNode>(name)), head_list(cast_node<HeadListNode>(head_list))
        {}

    protected:
        RoutineNode() = default;

        bool should_have_children() const final
        { return true; }
    };
    /// 函数语义节点
    struct ProgramNode : public RoutineNode
    {
    public:
        using RoutineNode::RoutineNode;

        llvm::Value *codegen(CodegenContext &context) override;

    protected:
        std::string json_head() const override
        {
            return std::string{"\"type\": \"Program\", \"name\": "} +
                   this->name->to_json() +
                   ", \"head\": " +
                   this->head_list->to_json();
        }
    };
    /// 子过程语义节点
    struct SubroutineNode : public RoutineNode
    {
    public:
        std::shared_ptr<ParamListNode> params;
        std::shared_ptr<TypeNode> return_type;

        SubroutineNode(const NodePtr &name, const NodePtr &params, const NodePtr &type, const NodePtr &head_list)
                : RoutineNode(name, head_list), params(cast_node<ParamListNode>(params)),
                  return_type(cast_node<TypeNode>(type))
        {
            assert(is_a_ptr_of<SimpleTypeNode>(type) || is_a_ptr_of<AliasTypeNode>(type));
        }

        llvm::Value *codegen(CodegenContext &context) override;

    protected:
        std::string json_head() const override
        {
            return std::string{"\"type\": \"Subroutine\", \"name\": "} +
                   this->name->to_json() +
                   ", \"params\": " +
                   this->params->to_json() +
                   ", \"return\": " +
                   this->return_type->to_json() +
                   ", \"head\": " +
                   this->head_list->to_json();
        }
    };
    /// 语句块语义节点 （比如begin...end之内的语句都在这里)
    struct CompoundStmtNode : public StmtNode
    {
    public:
        llvm::Value *codegen(CodegenContext &context) override;

    protected:
        std::string json_head() const override
        {
            return std::string{"\"type\": \"CompoundStmt\""};
        }

        bool should_have_children() const override
        { return true; }
    };
    /// 赋值语句语义节点
    struct AssignStmtNode : public StmtNode
    {
    public:
        /// 被赋值量
        std::shared_ptr<ExprNode> lhs;
        /// 赋值量
        std::shared_ptr<ExprNode> rhs;

        AssignStmtNode(const NodePtr &lhs, const NodePtr &rhs)
                : lhs(cast_node<ExprNode>(lhs)), rhs(cast_node<ExprNode>(rhs))
        {
            assert(is_a_ptr_of<IdentifierNode>(lhs)
                   || is_a_ptr_of<ArrayRefNode>(lhs) || is_a_ptr_of<RecordRefNode>(lhs));
        }

        llvm::Value *codegen(CodegenContext &context) override;

    protected:
        std::string json_head() const override
        {
            return std::string{"\"type\": \"AssignStmt\", \"lhs\": "} +
                   this->lhs->to_json() +
                   ", \"rhs\": " +
                   this->rhs->to_json();
        }

        bool should_have_children() const override
        { return false; }
    };
    /// 过程调用语义节点
    struct ProcStmtNode : public StmtNode
    {
    public:
        NodePtr proc_call;

        ProcStmtNode(const NodePtr &proc_call) : proc_call(proc_call)
        {
            assert(is_a_ptr_of<RoutineCallNode>(proc_call) || is_a_ptr_of<SysCallNode>(proc_call));
        }

        llvm::Value *codegen(CodegenContext &context) override;

    protected:
        std::string json_head() const override
        {
            return std::string{"\"type\": \"ProcStmt\", \"call\": "} +
                   this->proc_call->to_json();
        }

        bool should_have_children() const override
        { return false; }
    };
    /// IF语义节点
    struct IfStmtNode : public StmtNode
    {
    public:
        std::shared_ptr<ExprNode> expr;
        std::shared_ptr<StmtNode> stmt;
        std::shared_ptr<StmtNode> else_stmt;

        IfStmtNode(const NodePtr &expr, const NodePtr &stmt, const NodePtr &else_stmt)
                : expr(cast_node<ExprNode>(expr)), stmt(cast_node<StmtNode>(stmt)),
                  else_stmt(cast_node<StmtNode>(else_stmt))
        {}

        llvm::Value *codegen(CodegenContext &context) override;

    protected:
        std::string json_head() const override
        {
            return std::string{"\"type\": \"IfStmt\", \"expr\": "} +
                   this->expr->to_json() +
                   ", \"stmt\": " +
                   this->stmt->to_json() +
                   ", \"else_stmt\": " +
                   this->else_stmt->to_json();
        }

        bool should_have_children() const override
        { return false; }
    };
    /// repeat 语义节点
    struct RepeatStmtNode : public StmtNode
    {
    public:
        std::shared_ptr<ExprNode> expr;

        RepeatStmtNode(const NodePtr &expr) : expr(cast_node<ExprNode>(expr))
        {}

        llvm::Value *codegen(CodegenContext &context) override;

    protected:
        std::string json_head() const override
        {
            return std::string{"\"type\": \"RepeatStmt\", \"expr\": "} +
                   this->expr->to_json();
        }

        bool should_have_children() const override
        { return true; }
    };
    /// while 语义节点
    struct WhileStmtNode : public StmtNode
    {
    public:
        std::shared_ptr<ExprNode> expr;
        std::shared_ptr<StmtNode> stmt;

        WhileStmtNode(const NodePtr &expr, const NodePtr &stmt)
                : expr(cast_node<ExprNode>(expr)), stmt(cast_node<StmtNode>(stmt))
        {}

        llvm::Value *codegen(CodegenContext &context) override;

    protected:
        std::string json_head() const override
        {
            return std::string{"\"type\": \"WhileStmt\", \"expr\": "} +
                   this->expr->to_json() +
                   ", \"stmt\": " +
                   this->stmt->to_json();
        }

        bool should_have_children() const override
        { return false; }
    };
    /// for语句中 方向枚举
    enum class DirectionEnum
    {
        TO, DOWNTO
    };
    /// for语义语义节点
    struct ForStmtNode : public StmtNode
    {
    public:
        /// 计数方向
        DirectionEnum direction;
        /// 计数变量
        std::shared_ptr<IdentifierNode> identifier;
        /// 开始
        std::shared_ptr<ExprNode> start;
        /// 结束
        std::shared_ptr<ExprNode> finish;
        /// for循环体内语句
        std::shared_ptr<StmtNode> stmt;

        ForStmtNode(DirectionEnum direction, const NodePtr &identifier,
                    const NodePtr &start, const NodePtr &finish, const NodePtr &stmt)
                : direction(direction), identifier(cast_node<IdentifierNode>(identifier)),
                  start(cast_node<ExprNode>(start)), finish(cast_node<ExprNode>(finish)),
                  stmt(cast_node<StmtNode>(stmt))
        {}

        llvm::Value *codegen(CodegenContext &context) override;

    protected:
        std::string json_head() const override
        {
            return std::string{"\"type\": \"ForStmt\", \"direction\": \""} +
                   (direction == DirectionEnum::TO ? "TO" : "DOWNTO") +
                   "\", \"identifier\": " + this->identifier->to_json() +
                   ", \"start\": " + this->start->to_json() +
                   ", \"finish\": " + this->finish->to_json() +
                   ", \"stmt\": " + this->stmt->to_json();
        }

        bool should_have_children() const override
        { return false; }
    };
    /// case表达式语义节点
    struct CaseExprNode : public StmtNode
    {
    public:
        std::shared_ptr<ExprNode> branch;
        std::shared_ptr<StmtNode> stmt;

        CaseExprNode(const NodePtr &branch, const NodePtr &stmt)
                : branch(cast_node<ExprNode>(branch)), stmt(cast_node<StmtNode>(stmt))
        {
            assert(is_a_ptr_of<IdentifierNode>(branch) || is_a_ptr_of<ConstValueNode>(branch));
        }

    protected:
        std::string json_head() const override
        {
            return std::string{"\"type\": \"CaseExpr\", \"branch\": "} +
                   this->branch->to_json() +
                   ", \"stmt\": " +
                   this->stmt->to_json();
        }

        bool should_have_children() const override
        { return false; }
    };
    /// case语句语义节点
    struct CaseStmtNode : public StmtNode
    {
    public:
        std::shared_ptr<ExprNode> expr;

        void add_expr(const NodePtr &expr)
        {
            this->expr = cast_node<ExprNode>(expr);
        }

        llvm::Value *codegen(CodegenContext &context) override;

    protected:
        std::string json_head() const override
        {
            return std::string{"\"type\": \"CaseStmt\", \"expr\": "} +
                   this->expr->to_json();
        }

        bool should_have_children() const override
        { return true; }
    };

    // 语句的临时容器，在语法分析构建AST的时候会用到
    struct StmtList : public StmtNode
    {
    };
};
#endif