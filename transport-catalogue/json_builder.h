#pragma once


#include <vector>
#include <map>
#include <deque>

#include "json.h"

namespace json{

enum Commands{
    KEY,
    VALUE,
    START_DICT,
    START_ARRAY,
    END_DICT,
    END_ARRAY,
    BUILD
};

class Builder{
private:
    class BaseContext;
    class DictValueContext;
    class DictItemContext;
    class ArrayItemContext;
public:
    
    Builder() = default;

    DictValueContext Key(std::string key);
    BaseContext Value(Node::Value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    BaseContext EndDict();
    BaseContext EndArray();
    Node Build();
private:

    Node root_ = nullptr;
    std::vector<Node*> nodes_stack_;
    std::deque<Node> nodes_;

    void CheckContext(Commands command);
    void EndConatiner();

    template<typename T>
    void StartConatiner(T obj);

    // Key() → Value(), StartDict(), StartArray()
    // StartDict() → Key(), EndDict()
    // Key() → Value() → Key(), EndDict()
    // StartArray() → Value(), StartDict(), StartArray(), EndArray() 
    // StartArray() → Value() → Value(), StartDict(), StartArray(), EndArray()

    class BaseContext {
    public:
        BaseContext(Builder& builder) : builder_(builder) {}
        Node Build() {
            return builder_.Build();
        }
        DictValueContext Key(std::string key) {
            return builder_.Key(std::move(key));
        }
        BaseContext Value(Node::Value value) {
            return builder_.Value(std::move(value));
        }
        DictItemContext StartDict() {
            return builder_.StartDict();
        }
        ArrayItemContext StartArray() {
            return builder_.StartArray();
        }
        BaseContext EndDict() {
            return builder_.EndDict();
        }
        BaseContext EndArray() {
            return builder_.EndArray();
        }
    private:
        Builder& builder_;
    };

    class DictValueContext : public BaseContext {
    public:
        DictValueContext(BaseContext base) : BaseContext(base) {}
        DictItemContext Value(Node::Value value) { return BaseContext::Value(std::move(value)); }
        Node Build() = delete;
        DictValueContext Key(std::string key) = delete;
        BaseContext EndDict() = delete;
        BaseContext EndArray() = delete;
    };

    class DictItemContext : public BaseContext {
    public:
        DictItemContext(BaseContext base) : BaseContext(base) {}
        Node Build() = delete;
        BaseContext Value(Node::Value value) = delete;
        BaseContext EndArray() = delete;
        DictItemContext StartDict() = delete;
        ArrayItemContext StartArray() = delete;
    };

    class ArrayItemContext : public BaseContext {
    public:
        ArrayItemContext(BaseContext base) : BaseContext(base) {}
        ArrayItemContext Value(Node::Value value) { return BaseContext::Value(std::move(value)); }
        Node Build() = delete;
        DictValueContext Key(std::string key) = delete;
        BaseContext EndDict() = delete;
    };

};


template<typename T>
void Builder::StartConatiner(T obj) {
    
    if (nodes_stack_.empty() || nodes_stack_.back()->IsArray() || nodes_stack_.back()->IsString()) {
        nodes_.emplace_back(obj);
        nodes_stack_.push_back(&nodes_.back());
    } else {
        throw std::logic_error("Start Conatiner in wrong place");
    }
}

}