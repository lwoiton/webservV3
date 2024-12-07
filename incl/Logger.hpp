/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/22 21:14:46 by lwoiton           #+#    #+#             */
/*   Updated: 2024/11/12 22:18:08 by lwoiton          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#ifndef LOGGER_HPP
#define LOGGER_HPP

#define COLOR_RESET   "\033[0m"
#define COLOR_BOLD    "\033[1m"

// Bright colors (more vibrant)
#define COLOR_BRIGHT_RED     "\033[91m"
#define COLOR_BRIGHT_GREEN   "\033[92m"
#define COLOR_BRIGHT_YELLOW  "\033[93m"
#define COLOR_BRIGHT_BLUE    "\033[94m"
#define COLOR_BRIGHT_MAGENTA "\033[95m"
#define COLOR_BRIGHT_CYAN    "\033[96m"

// Combined bold and bright
#define COLOR_BOLD_CYAN    "\033[1;96m"
#define COLOR_BOLD_GREEN   "\033[1;92m"
#define COLOR_BOLD_YELLOW  "\033[1;93m"
#define COLOR_BOLD_RED     "\033[1;91m"

#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <sstream>
#include <iomanip>

enum LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class Logger {
private:
    static Logger* instance;
    std::ofstream logFile;
    LogLevel currentLevel;
    std::string currentDate;

    Logger();
    Logger(const Logger&);

    std::string getTimestamp();
    std::string getDate();
    std::string getLevelString(LogLevel level);
    void openLogFile();

public:
    static Logger* getInstance();
    void setLogLevel(LogLevel level);
    void log(LogLevel level, const std::string& message, 
             const char* file = NULL, const char* function = NULL, int line = -1);
    ~Logger();
};

// Helper class for number conversion
class NumberConverter {
public:
    static std::string convert(size_t value) {
        std::ostringstream ss;
        ss << value;
        return ss.str();
    }
    
    static std::string convert(int value) {
        std::ostringstream ss;
        ss << value;
        return ss.str();
    }

    static std::string convert(long value) {
        std::ostringstream ss;
        ss << value;
        return ss.str();
    }

    static std::string convert(double value) {
        std::ostringstream ss;
        ss << value;
        return ss.str();
    }
};

#define LOG_DEBUG(message) Logger::getInstance()->log(DEBUG, message, __FILE__, __FUNCTION__, __LINE__)
#define LOG_INFO(message) Logger::getInstance()->log(INFO, message)
#define LOG_WARNING(message) Logger::getInstance()->log(WARNING, message, __FILE__, __FUNCTION__, __LINE__)
#define LOG_ERROR(message) Logger::getInstance()->log(ERROR, message, __FILE__, __FUNCTION__, __LINE__)
#define TO_STRING(x) (NumberConverter::convert(x))

#endif // LOGGER_HPP