//
//  Strings.h
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


//template<class...Durations, class DurationIn>
//std::tuple<Durations...> break_down_durations( DurationIn d ) {
//  std::tuple<Durations...> retval;
//  using discard=int[];
//  (void)discard{0,(void((
//    (std::get<Durations>(retval) = std::chrono::duration_cast<Durations>(d)),
//    (d -= std::chrono::duration_cast<DurationIn>(std::get<Durations>(retval)))
//  )),0)...};
//  return retval;
//}
//
//std::string durationToString(float duration) {
//  auto chronoDuration = std::chrono::duration<float>(duration);
//  return break_down_durations<std::chrono::minutes, std::chrono::seconds>(chronoDuration);
//}


#endif Strings_h

