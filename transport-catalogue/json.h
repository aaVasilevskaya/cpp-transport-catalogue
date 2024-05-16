#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

class Node;
// Сохраните объявления Dict и Array без изменения
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node {
public:
    using Value = std::variant<std::nullptr_t, int, double, std::string, bool, Array, Dict>;
   /* Реализуйте Node, используя std::variant */
    Node() = default;

    explicit Node(Value data);

    template<typename T>
    Node(T data)
    :data_(data){}

    const Value& GetValue() const;

    int AsInt() const;
    double AsDouble() const;
    const std::string& AsString() const;
    bool AsBool() const;
    const Array& AsArray() const;
    Array& AsArray();
    const Dict& AsMap() const;
    Dict& AsMap();
    
    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsBool() const;
    bool IsString() const;
    bool IsNull() const;
    bool IsArray() const;
    bool IsMap() const;

    bool operator == (const Node &rhs) const;
    bool operator != (const Node &rhs) const;

private:
    Value data_;
};

class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;

    bool operator == (const Document &rhs) const;
    bool operator != (const Document &rhs) const;

private:
    Node root_;
};

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

}  // namespace json