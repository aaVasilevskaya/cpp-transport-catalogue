#include "json.h"

using namespace std;

namespace json {

namespace {

Node LoadNode(istream& input);
Node LoadString(std::istream& input);

std::string LoadLiteral(std::istream& input) {
    std::string s;
    while (std::isalpha(input.peek())) {
        s.push_back(static_cast<char>(input.get()));
    }
    return s;
}

Node LoadArray(std::istream& input) {
    std::vector<Node> result;

    for (char c; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }
    if (!input) {
        throw ParsingError("Array parsing error"s);
    }
    return Node(std::move(result));
}

Node LoadDict(std::istream& input) {
    Dict dict;

    for (char c; input >> c && c != '}';) {
        if (c == '"') {
            std::string key = LoadString(input).AsString();
            if (input >> c && c == ':') {
                if (dict.find(key) != dict.end()) {
                    throw ParsingError("Duplicate key '"s + key + "' have been found");
                }
                dict.emplace(std::move(key), LoadNode(input));
            } else {
                throw ParsingError(": is expected but '"s + c + "' has been found"s);
            }
        } else if (c != ',') {
            throw ParsingError(R"(',' is expected but ')"s + c + "' has been found"s);
        }
    }
    if (!input) {
        throw ParsingError("Dictionary parsing error"s);
    }
    return Node(std::move(dict));
}

Node LoadString(std::istream& input) {
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true) {
        if (it == end) {
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            ++it;
            break;
        } else if (ch == '\\') {
            ++it;
            if (it == end) {
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            switch (escaped_char) {
                case 'n':
                    s.push_back('\n');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case 'r':
                    s.push_back('\r');
                    break;
                case '"':
                    s.push_back('"');
                    break;
                case '\\':
                    s.push_back('\\');
                    break;
                default:
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        } else if (ch == '\n' || ch == '\r') {
            throw ParsingError("Unexpected end of line"s);
        } else {
            s.push_back(ch);
        }
        ++it;
    }

    return Node(std::move(s));
}

Node LoadBool(std::istream& input) {
    const auto s = LoadLiteral(input);
    if (s == "true"sv) {
        return Node{true};
    } else if (s == "false"sv) {
        return Node{false};
    } else {
        throw ParsingError("Failed to parse '"s + s + "' as bool"s);
    }
}

Node LoadNull(std::istream& input) {
    if (auto literal = LoadLiteral(input); literal == "null"sv) {
        return Node{nullptr};
    } else {
        throw ParsingError("Failed to parse '"s + literal + "' as null"s);
    }
}

Node LoadNumber(std::istream& input) {
    std::string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    } else {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return std::stoi(parsed_num);
            } catch (...) {
                // В случае неудачи, например, при переполнении
                // код ниже попробует преобразовать строку в double
            }
        }
        return std::stod(parsed_num);
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

Node LoadNode(std::istream& input) {
    char c;
    if (!(input >> c)) {
        throw ParsingError("Unexpected EOF"s);
    }
    switch (c) {
        case '[':
            return LoadArray(input);
        case '{':
            return LoadDict(input);
        case '"':
            return LoadString(input);
        case 't':
            // Атрибут [[fallthrough]] (провалиться) ничего не делает, и является
            // подсказкой компилятору и человеку, что здесь программист явно задумывал
            // разрешить переход к инструкции следующей ветки case, а не случайно забыл
            // написать break, return или throw.
            // В данном случае, встретив t или f, переходим к попытке парсинга
            // литералов true либо false
            [[fallthrough]];
        case 'f':
            input.putback(c);
            return LoadBool(input);
        case 'n':
            input.putback(c);
            return LoadNull(input);
        default:
            input.putback(c);
            return LoadNumber(input);
    }
}

}  // namespace


/*---------------Node------------------*/

Node::Node(Value data)
:data_(std::move(data)){
}

const Node::Value& Node::GetValue() const { 
    return data_;
}

const Array& Node::AsArray() const {
    if (IsArray()){
        return std::get<Array>(data_);
    }
    throw (std::logic_error("not Array"));
}
const Dict& Node::AsMap() const {
    if (IsMap()){
        return std::get<Dict>(data_);
    }
    throw (std::logic_error("not Map"));
}
int Node::AsInt() const {
   if (IsInt()){
        return std::get<int>(data_);
    }
    throw (std::logic_error("not Int"));
}
double Node::AsDouble() const{
    if (IsPureDouble()){
        return std::get<double>(data_);
    }else if(IsDouble()){
        return static_cast<double>(std::get<int>(data_));
    }
    throw (std::logic_error("not Double"));
}
bool Node::AsBool() const{
    if (IsBool()){
        return std::get<bool>(data_);
    }
    throw (std::logic_error("not Bool"));
}
const string& Node::AsString() const {
    if (IsString()){
        return std::get<std::string>(data_);
    }
    throw (std::logic_error("not String"));
}

bool Node::IsInt() const{
    return std::holds_alternative<int>(data_) ? true:false;
}
bool Node::IsDouble() const{
    return std::holds_alternative<int>(data_) 
    || std::holds_alternative<double>(data_) ? true:false;
}
bool Node::IsPureDouble() const{
    return std::holds_alternative<double>(data_) ? true:false;
}
bool Node::IsBool() const{
    return std::holds_alternative<bool>(data_) ? true:false;
}
bool Node::IsString() const{
    return std::holds_alternative<std::string>(data_) ? true:false;
}
bool Node::IsNull() const{
    return std::holds_alternative<std::nullptr_t>(data_) ? true:false;
}
bool Node::IsArray() const{
    return std::holds_alternative<Array>(data_) ? true:false;
}
bool Node::IsMap() const{
    return std::holds_alternative<Dict>(data_) ? true:false;
}

bool Node::operator == (const Node &rhs) const {
    if(data_.index() == rhs.data_.index()){
        return data_ == rhs.data_;
    }
    return false; 
}
bool Node::operator != (const Node &rhs) const{
    return !(*this == rhs);
}

struct PrintContext {
    std::ostream& out;
    int indent_step = 4;
    int indent = 0;

    void PrintIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    PrintContext Indented() const {
        return {out, indent_step, indent_step + indent};
    }
};

/*----------------Document-----------*/
Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

bool Document::operator == (const Document &rhs) const{
    return root_ == rhs.GetRoot();
}
bool Document::operator != (const Document &rhs) const{
    return !(root_ == rhs.GetRoot());
}

Document Load(istream& input) {
    return Document{LoadNode(input)};
}

void PrintNode(const Node& node, const PrintContext& context);

struct NodePrinter {
    const PrintContext& context;
    void operator()(std::nullptr_t) const {
        context.out  << "null"sv;
    }
    void operator()(int data) const {
        context.out  << data;
    }
    void operator()(double data) const {
        context.out  << data;
    }
    void operator()(bool data) const {
        context.out << std::boolalpha << data;
    }
    void operator()(std::string data) const {
        context.out.put('"');
        for (const char c : data) {
            switch (c) {
                case '\r':
                    context.out << "\\r"sv;
                    break;
                case '\n':
                    context.out << "\\n"sv;
                    break;
                case '\t':
                    context.out << "\\t"sv;
                    break;
                case '"':
                    // Символы " и \ выводятся как \" или \\, соответственно
                    [[fallthrough]];
                case '\\':
                    context.out.put('\\');
                    [[fallthrough]];
                default:
                    context.out.put(c);
                    break;
            }
        }
        context.out.put('"');
    }
    void operator()(Array data) const {
        context.out << "[\n"sv;
        bool first = true;
        auto inner_ctx = context.Indented();
        for (const Node& node : data) {
            if (first) {
                first = false;
            } else {
                context.out << ",\n"sv;
            }
            inner_ctx.PrintIndent();
            PrintNode(node, inner_ctx);
        }
        context.out.put('\n');
        context.PrintIndent();
        context.out.put(']');
    }
    void operator()(Dict data) const {
        context.out << "{\n"sv;
        bool first = true;
        auto inner_ctx = context.Indented();
        for (const auto& [key, node] : data) {
            if (first) {
                first = false;
            } else {
                context.out << ",\n"sv;
            }
            inner_ctx.PrintIndent();
            PrintNode(key, inner_ctx);
            context.out << ": "sv;
            PrintNode(node, inner_ctx);
        }
        context.out.put('\n');
        context.PrintIndent();
        context.out.put('}');
    }
};

void Print(const Document& doc, std::ostream& output) {
    PrintNode(doc.GetRoot(), PrintContext{output});
}

void PrintNode(const Node& node, const PrintContext& context) {
    std::visit(NodePrinter{context}, node.GetValue());
}

}  // namespace json