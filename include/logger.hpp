#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>

constexpr bool _LOGGER_DEBUG = false;
constexpr bool _LOGGER_ERROR = true; // better stay on true

template<typename... Args>
void log(const Args &... args) {
   if(_LOGGER_DEBUG) {
      (std::clog << ... << args);
      std::clog << std::endl;
   }
}

template <typename... Args>
void logError(const Args &... args) {
   if (_LOGGER_ERROR) {  
      std::cerr << "ERROR: ";
      (std::cerr << ... << args);
      std::cerr << std::endl;
   }
}

#endif // !LOGGER_HPP   
