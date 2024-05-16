#include "json_builder.h"
#include <algorithm>


namespace json{


Builder::DictValueContext Builder::Key(std::string key){
    CheckContext(Commands::KEY);

    if(nodes_stack_.back()->IsMap()){
        nodes_.emplace_back(std::move(key));
        nodes_stack_.push_back(&nodes_.back());
    }else{
        throw std::logic_error("Key in wrong place");
    }
    return BaseContext{*this};

}
Builder::BaseContext Builder::Value(Node::Value value){
    CheckContext(Commands::VALUE);

    if(root_ == nullptr && nodes_stack_.empty()){
        root_ = std::move(value);
    }else if(nodes_stack_.back()->IsArray()){
        nodes_stack_.back()->AsArray().emplace_back(std::move(value));
    }else if(nodes_stack_.back()->IsString()){
        Node* key = nodes_stack_.back();
        nodes_stack_.pop_back();
        nodes_stack_.back()->AsMap().insert({std::move((*key).AsString()), std::move(value)});
    }else{
        throw std::logic_error("Value in wrong place");
    }
    return *this;
}

Builder::DictItemContext Builder::StartDict(){
    CheckContext(Commands::START_DICT);
    StartConatiner(Dict{});
    return {*this};
}

Builder::ArrayItemContext Builder::StartArray(){
    CheckContext(Commands::START_ARRAY);
    StartConatiner(Array{});
    return {*this};
}

Builder::BaseContext Builder::EndDict(){
    CheckContext(Commands::END_DICT);

    if(nodes_stack_.back()->IsMap()){
        EndConatiner();
    }else{
        throw std::logic_error("EndDict in wrong place");
    }
    return *this;
}

Builder::BaseContext Builder::EndArray(){
    CheckContext(Commands::END_ARRAY);

    if(nodes_stack_.back()->IsArray()){
        EndConatiner();
    }else{
        throw std::logic_error("EndDict in wrong place");
    }
    return *this;
}


Node Builder::Build(){
    if(nodes_stack_.empty() && root_ != nullptr){
        return root_;
    }else{
        throw std::logic_error("Must be one object");
    }
    return root_;
}

void Builder::CheckContext(Commands command) {
    if(root_ != nullptr && (command == Commands::KEY || command == Commands::VALUE
                             || command == Commands::START_ARRAY || command == Commands::START_DICT)){
        throw std::logic_error("Object already done");
    }
    if(nodes_stack_.empty() && (command == Commands::END_ARRAY || command == Commands::END_DICT)){
        throw std::logic_error("nodes_stack_ empty for end dict or array");
    }
}

void Builder::EndConatiner(){
    Node* node = nodes_stack_.back();
    nodes_stack_.pop_back();

    if(nodes_stack_.empty()){
        root_ = std::move(*node);
    }else if(nodes_stack_.back()->IsArray()){
        nodes_stack_.back()->AsArray().emplace_back(std::move(*node));
    }else if(nodes_stack_.back()->IsString()){
        Node* key = nodes_stack_.back();
        nodes_stack_.pop_back();
        nodes_stack_.back()->AsMap().insert({std::move((*key).AsString()), std::move(*node)});
    }

}

}