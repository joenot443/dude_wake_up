//
//  Strings.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-27.
//

#ifndef Strings_h

#define Strings_h
#include <memory>
#include <iostream>
#include <tuple>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <tuple>
#include <string>
#include <iomanip>
#include <sstream>
#include "json.hpp"

class StringManager {
public:
    static void loadStrings();
    static std::string get(const std::string& key);
private:
    static nlohmann::json stringJson;
};


template<typename ... Args>
std::string formatString( const std::string& format, Args ... args )
{
    int size_s = std::snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
    if( size_s <= 0 ){ throw std::runtime_error( "Error during formatting." ); }
    auto size = static_cast<size_t>( size_s );
    std::unique_ptr<char[]> buf( new char[ size ] );
    std::snprintf( buf.get(), size, format.c_str(), args ... );
    return std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
}

static std::string idString(std::string idStr) {
  return formatString("##%s", idStr.c_str());
}

static std::string idAppendedToString(std::string str, std::string idStr) {
  return formatString("%s##%s", str.c_str(), idStr.c_str());
}

static std::vector<std::string> split_string (std::string s, std::string delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = s.find (delimiter, pos_start)) != std::string::npos) {
        token = s.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back (token);
    }

    res.push_back (s.substr (pos_start));
    return res;
}

static std::string truncateString(const std::string& str, const int x) {
    if (str.length() <= x) {
        return str;
    } else {
        return str.substr(0, x);
    }
}

static std::string formatTimeDuration(float seconds) {
    int hours = static_cast<int>(seconds) / 3600;
    int minutes = (static_cast<int>(seconds) % 3600) / 60;
    int sec = static_cast<int>(seconds) % 60;

    std::stringstream ss;
    if (hours > 0) {
        ss << hours << ":";
    }
    ss << std::setw(2) << std::setfill('0') << minutes << ":";
    ss << std::setw(2) << std::setfill('0') << sec;

    return ss.str();
}

static std::vector<std::string> mapColorsToStrings(const std::vector<std::array<float, 4>>& colors) {
    std::vector<std::string> result;
    result.reserve(colors.size()); // Reserve memory for efficiency

    for (const auto& color : colors) {
        std::ostringstream oss;
        oss << "R: " << color[0] << " G: " << color[1] << " B: " << color[2] << " A: " << color[3];
        result.push_back(oss.str());
    }

    return result;
}

static std::string formatColor(std::array<float, 4> color) {
  return formatString("R: %.2f G: %.2f B: %.2f, A: %.2f", color[0], color[1], color[2], color[3]);
}


// Removes the file extension from a string
static std::string removeFileExtension(const std::string& filename) {
    size_t lastindex = filename.find_last_of(".");
    return filename.substr(0, lastindex);
}


#endif Strings_h
