/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/23 00:20:43 by lwoiton           #+#    #+#             */
/*   Updated: 2024/11/12 22:18:52 by lwoiton          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"

Logger* Logger::instance = NULL;

Logger::Logger() : currentLevel(DEBUG)
{
    openLogFile();
}

Logger* Logger::getInstance() {
    if (instance == NULL) {
        instance = new Logger();
    }
    return instance;
}

void Logger::setLogLevel(LogLevel level) {
    currentLevel = level;
}

std::string Logger::getLevelString(LogLevel level) {
    switch(level) {
        case DEBUG:   return "DEBUG";
        case INFO:    return "INFO";
        case WARNING: return "WARNING";
        case ERROR:   return "ERROR";
        default:      return "UNKNOWN";
    }
}

std::string Logger::getDate() {
    time_t now = time(0);
    struct tm* timeinfo = localtime(&now);
    char buffer[11];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d", timeinfo);
    return std::string(buffer);
}

void Logger::openLogFile() {
    currentDate = getDate();
    std::string filename = "logs/webserv_" + currentDate + ".log";
    logFile.open(filename.c_str(), std::ios::app);
    if (!logFile.is_open()) {
        std::cerr << "Failed to open log file: " << filename << std::endl;
    }
}

std::string Logger::getTimestamp() {
    time_t now = time(0);
    struct tm* timeinfo = localtime(&now);
    char buffer[9];
    strftime(buffer, sizeof(buffer), "%H:%M:%S", timeinfo);
    return std::string(buffer);
}

void Logger::log(LogLevel level, const std::string& message, 
                const char* file, const char* function, int line) {
    if (level < currentLevel) return;

    std::ostringstream outputStream;
    std::string levelStr = getLevelString(level);
    
    // Basic log message
    outputStream << "[" << getTimestamp() << "] [";
    
    // Apply appropriate styling based on log level
    switch(level) {
        case DEBUG:
            outputStream << COLOR_BOLD_CYAN << levelStr << COLOR_RESET << "] " << message;
            break;
            
        case INFO:
            outputStream << COLOR_BOLD_GREEN << levelStr << COLOR_RESET << "] " << message;
            break;
            
        case WARNING:
            outputStream << COLOR_BOLD_YELLOW << levelStr << COLOR_RESET << "] " << message;
            break;
            
        case ERROR:
            outputStream << COLOR_BOLD_RED << levelStr << COLOR_RESET << "] " 
                        << COLOR_BRIGHT_RED << message
                        << COLOR_RESET;
            break;
            
        default:
            outputStream << levelStr << "] " << message;
    }

    // Add file information with proper spacing for DEBUG and ERROR
    if ((level == DEBUG || level == ERROR) && file && function && line != -1) {
        std::string baseMessage = outputStream.str();
        
        // Strip color codes for length calculation
        std::string plainMessage = baseMessage;
        size_t pos;
        while ((pos = plainMessage.find("\033")) != std::string::npos) {
            size_t endPos = plainMessage.find('m', pos);
            if (endPos != std::string::npos) {
                plainMessage.erase(pos, endPos - pos + 1);
            }
        }
        
        // Calculate padding needed to reach column 85
        int padding = 85 - plainMessage.length();
        if (padding > 0) {
            outputStream << std::string(padding, ' ');
        }
        
        // Add the file information
        outputStream << "(" << file << ":" << function << ":" << line << ")";
    }

    std::string output = outputStream.str();
    
    // Strip color codes for file logging
    std::string plainOutput = output;
    size_t pos;
    while ((pos = plainOutput.find("\033")) != std::string::npos) {
        size_t endPos = plainOutput.find('m', pos);
        if (endPos != std::string::npos) {
            plainOutput.erase(pos, endPos - pos + 1);
        }
    }
    
    if (logFile.is_open()) {
        logFile << plainOutput << std::endl;
    }

    // Console output with colors
    if (level == INFO || level == DEBUG)
        std::cout << output << std::endl;
    else
        std::cerr << output << std::endl;
}

Logger::~Logger() {
    if (logFile.is_open()) {
        logFile.close();
    }
}