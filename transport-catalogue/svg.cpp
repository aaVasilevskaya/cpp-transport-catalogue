#include "svg.h"

namespace svg {

using namespace std::literals;

void HtmlEncodeString(std::ostream& out, std::string_view sv) {
    for (char c : sv) {
        switch (c) {
            case '"':
                out << "&quot;"sv;
                break;
            case '<':
                out << "&lt;"sv;
                break;
            case '>':
                out << "&gt;"sv;
                break;
            case '&':
                out << "&amp;"sv;
                break;
            case '\'':
                out << "&apos;"sv;
                break;
            default:
                out.put(c);
        }
    }
}

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

std::ostream& operator<<(std::ostream& os, const StrokeLineCap& obj){
    switch(obj){
        case StrokeLineCap::BUTT:
            os << "butt";
            break;
        case StrokeLineCap::ROUND:
            os << "round";
            break;
        case StrokeLineCap::SQUARE:
            os << "square";
            break;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const StrokeLineJoin& obj){
    switch(obj){
        case StrokeLineJoin::ARCS:
            os << "arcs";
            break;
        case StrokeLineJoin::BEVEL:
            os << "bevel";
            break;
        case StrokeLineJoin::MITER:
            os << "miter";
            break;
        case StrokeLineJoin::MITER_CLIP:
            os << "miter-clip";
            break;
        case StrokeLineJoin::ROUND:
            os << "round";
            break;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const Rgb& color){
    os << "rgb(" << static_cast<int>(color.red) << "," 
    << static_cast<int>(color.green) << "," 
    << static_cast<int>(color.blue) << ")";
    return os;
}
std::ostream& operator<<(std::ostream& os, const Rgba& color){
    os << "rgba(" << static_cast<int>(color.red) << "," 
    << static_cast<int>(color.green) << "," 
    << static_cast<int>(color.blue) << "," << color.opacity <<")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const Color& color){
    if (std::holds_alternative<std::monostate>(color)) {
        os << "none"sv;
        return os;
    }
    if (std::holds_alternative<std::string>(color)) {
        os << std::get<std::string>(color);
        return os;
    }
    if (std::holds_alternative<svg::Rgb>(color)) {
        os << std::get<svg::Rgb>(color);
        return os;
    }
    if (std::holds_alternative<svg::Rgba>(color)) {
        os << std::get<svg::Rgba>(color);
        return os;
    }
    return os;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(context.out);
    out << "/>"sv;
}

// ---------- Polyline ------------------

Polyline& Polyline::AddPoint(Point point){
    points_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;
    for(size_t i = 0; i < points_.size(); ++i){
        out << points_[i].x << ","sv << points_[i].y;
        if (i < points_.size() - 1) {
            out << " "sv;
        }
    }
    out << "\"";
    RenderAttrs(context.out);
    out << "/>"sv;
}

// ---------- Text ------------------

Text& Text::SetPosition(Point pos){
    pos_ = pos;
    return *this;
}

// Задаёт смещение относительно опорной точки (атрибуты dx, dy)
Text& Text::SetOffset(Point offset){
    offset_ = offset;
    return *this;
}

// Задаёт размеры шрифта (атрибут font-size)
Text& Text::SetFontSize(uint32_t size){
    size_ = size;
    return *this;
}

// Задаёт название шрифта (атрибут font-family)
Text& Text::SetFontFamily(std::string font_family){
    font_family_ = std::move(font_family);
    return *this;
}

// Задаёт толщину шрифта (атрибут font-weight)
Text& Text::SetFontWeight(std::string font_weight){
    font_weight_ = std::move(font_weight);
    return *this;
}

// Задаёт текстовое содержимое объекта (отображается внутри тега text)
Text& Text::SetData(std::string data){
    data_ = std::move(data);
    return *this;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text";
    RenderAttrs(context.out);
    out << " x=\""sv << pos_.x <<"\" y=\""sv << pos_.y << "\" "sv;
    out << "dx=\""sv << offset_.x <<"\" dy=\""sv << offset_.y << "\" "sv;
    out << "font-size=\""sv << size_ << "\""sv;
    if(font_family_ != ""){
        out << " font-family=\""sv << font_family_ << "\""sv;
    }
    if(font_weight_ != ""){
        out << " font-weight=\""sv << font_weight_ << "\""sv;
    }
    out << ">"sv;
    HtmlEncodeString(out, data_); 
    out <<"</text>"sv;
}

// ---------- Document ------------------

void Document::AddPtr(std::unique_ptr<Object>&& obj){
    objects_.push_back(std::move(obj));
}

// Выводит в ostream svg-представление документа
void Document::Render(std::ostream& out) const{
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
    RenderContext ctx{out, 2, 2};
    for(const auto& obj:objects_){
        obj.get()->Render(ctx);
    }
    out << "</svg>"sv;
}

}  // namespace svg