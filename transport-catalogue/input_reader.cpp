#include "input_reader.h"

/**
 * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
 */
Catalogue::Coordinates ParseCoordinates(std::string_view str) {
    static const double nan = std::nan("");

    auto not_space = str.find_first_not_of(' ');
    auto comma = str.find(',');

    if (comma == str.npos) {
        return {nan, nan};
    }

    auto not_space2 = str.find_first_not_of(' ', comma + 1);

    double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
    double lng = std::stod(std::string(str.substr(not_space2)));

    return {lat, lng};
}

/**
 * Удаляет пробелы в начале и конце строки
 */
std::string_view Trim(std::string_view string) {
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

/**
 * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
 */
std::vector<std::string_view> Split(std::string_view string, char delim) {
    std::vector<std::string_view> result;

    size_t pos = 0;
    while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
        auto delim_pos = string.find(delim, pos);
        if (delim_pos == string.npos) {
            delim_pos = string.size();
        }
        if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
            result.push_back(substr);
        }
        pos = delim_pos + 1;
    }

    return result;
}

/**
 * Парсит маршрут.
 * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
 * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
 */
std::vector<std::string_view> ParseRoute(std::string_view route) {
    if (route.find('>') != route.npos) {
        return Split(route, '>');
    }

    auto stops = Split(route, '-');
    std::vector<std::string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

    return results;
}

CommandDescription ParseCommandDescription(std::string_view line) {
    auto colon_pos = line.find(':');
    if (colon_pos == line.npos) {
        return {};
    }

    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos) {
        return {};
    }

    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos) {
        return {};
    }

    return {std::string(line.substr(0, space_pos)),
            std::string(line.substr(not_space, colon_pos - not_space)),
            std::string(line.substr(colon_pos + 1))};
}

InputReader::InputReader(std::istream& input_stream)
    :input_stream_(input_stream){
};

void InputReader::ReadAndApplyCommands(Catalogue::TransportCatalogue& catalogue){
    int base_request_count;
    input_stream_ >> base_request_count >> std::ws;

    for (int i = 0; i < base_request_count; ++i) {
        std::string line;
        getline(input_stream_, line);
        ParseLine(line);
    }
    ApplyCommands(catalogue);
}

void InputReader::ParseLine(std::string_view line) {
    auto command_description = ParseCommandDescription(line);
    if (command_description) {
        commands_.push_back(std::move(command_description));
    }
}

Catalogue::Distance ExtractDistanceFromWords(std::string_view line){
    unsigned int dist = 0;
    std::string name_location;
    
    auto m_pos = line.find('m');
    auto to_pos = line.find("to");
    
    dist = static_cast<unsigned int>(std::stoul(std::string(line.substr(0, m_pos))));
    
    name_location = std::string(Trim(line.substr(to_pos+2)));

    return {dist, name_location};
}

std::vector<Catalogue::Distance> ParseDistances(std::string_view str){
    std::vector<Catalogue::Distance> dists;  
    std::vector<std::string_view> stop_info = Split(str, ',');
    
    if(stop_info.size() > 2){
        dists.reserve(stop_info.size()-2);
        
        for(size_t i = 2; i < stop_info.size(); i++){
            dists.push_back(ExtractDistanceFromWords(Trim(stop_info.at(i))));
        }
    }
    return dists;
}

void InputReader::ApplyCommands([[maybe_unused]] Catalogue::TransportCatalogue& catalogue) const {
    for(auto& command : commands_){
        if(command.command == "Stop"){
            catalogue.AddStop(command.id, ParseCoordinates(command.description));
        }
    }
    for(auto& command : commands_){
        if(command.command == "Stop"){
            auto distances = ParseDistances(command.description);
            for(auto& dist:distances){
                catalogue.AddStopsDistance(command.id, dist.name_location, dist.dist);
            }
        }
    }
    for(auto& command : commands_){
        if(command.command == "Bus"){
            catalogue.AddBus(command.id, ParseRoute(command.description));
        }
    }
}

