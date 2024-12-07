/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/22 18:47:28 by lwoiton           #+#    #+#             */
/*   Updated: 2024/11/13 12:47:40 by lwoiton          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"
#include "Utils.hpp"

Config::Config(const std::string &configPath)
{
    _parseConfig(configPath);
}

Config::~Config()
{
}

void Config::_parseConfig(const std::string &configPath)
{
    std::ifstream file(configPath.c_str());
    if (!file.is_open())
        throw std::runtime_error("Cannot open config file: " + configPath);
    
    // Add HTTP and Logging context
    bool inHttpContext = false;
    std::string token;
    
    while (file.good())
    {
        token = _getNextToken(file);
        if (token.empty())
            break;
        
        if (token == "log_level")
        {
            std::string level = _getNextToken(file);
            _expectToken(file, ";");
            if (level == "debug")
                Logger::getInstance()->setLogLevel(DEBUG);
            else if (level == "info")
                Logger::getInstance()->setLogLevel(INFO);
            else if (level == "warning")
                Logger::getInstance()->setLogLevel(WARNING);
            else if (level == "error")
                Logger::getInstance()->setLogLevel(ERROR);
            else
                throw std::runtime_error("Invalid log level: " + level);
        }
        else if (token == "http")
        {
            if (inHttpContext)
                throw std::runtime_error("Nested http context not allowed");
            _expectToken(file, "{");
            inHttpContext = true;
        }
        else if (token == "server")
        {
            if (!inHttpContext)
                throw std::runtime_error("Server block must be inside http context");
            _parseServer(file);
        }
        else if (token == "}")
        {
            if (!inHttpContext)
                throw std::runtime_error("Unexpected closing brace");
            inHttpContext = false;
        }
        else
            throw std::runtime_error("Unexpected token in config: " + token);
    }

    if (_servers.empty())
	{
        throw std::runtime_error("No server configurations found");
	}
	LOG_INFO("Webserv config: " + configPath + " parsed successfully");
}

void Config::_parseServer(std::ifstream &file)
{
    _expectToken(file, "{");
    
    ServerConfig server;
    std::string token;
    
    while (file.good())
    {
        token = _getNextToken(file);
        if (token == "}")
            break;
        else if (token == "listen")
        {
            std::string hostPort = _getNextToken(file);
            size_t colonPos = hostPort.find(':');
            if (colonPos != std::string::npos)
            {
                server.host = hostPort.substr(0, colonPos);
                std::istringstream(hostPort.substr(colonPos + 1)) >> server.port;
            }
            else
                server.host = hostPort;
            
            if (!_isValidHost(server.host) || !_isValidPort(server.port))
                throw std::runtime_error("Invalid host:port configuration");
            
            _expectToken(file, ";");
        }
        else if (token == "server_name")
        {
            while (file.good())
            {
                token = _getNextToken(file);
                if (token == ";")
                    break;
                server.serverNames.push_back(token);
            }
        }
        else if (token == "root")
        {
            server.root = _getNextToken(file);
            _expectToken(file, ";");
        }
        else if (token == "client_max_body_size")
        {
            std::string size = _getNextToken(file);
            std::istringstream(size) >> server.clientMaxBodySize;
            _expectToken(file, ";");
        }
        else if (token == "error_page")
        {
            int code;
            std::string codeStr = _getNextToken(file);
            std::istringstream(codeStr) >> code;
            std::string page = _getNextToken(file);
            server.errorPages[code] = page;
            _expectToken(file, ";");
        }
        else if (token == "location")
            _parseRoute(file, server);
        else
            throw std::runtime_error("Unexpected token in server block: " + token);
    }

    _servers.push_back(server);
}

void Config::_parseRoute(std::ifstream &file, ServerConfig &server)
{
    Route route;
    route.path = _getNextToken(file);
    _expectToken(file, "{");
    
    std::string token;
    while (file.good())
    {
        token = _getNextToken(file);
        if (token == "}")
            break;
        else if (token == "root")
        {
            route.root = _getNextToken(file);
            _expectToken(file, ";");
        }
        else if (token == "methods")
        {
            while (file.good())
            {
                token = _getNextToken(file);
                if (token == ";")
                    break;
                route.allowedMethods.insert(token);
            }
        }
        else if (token == "autoindex")
        {
            token = _getNextToken(file);
            route.autoindex = (token == "on");
            _expectToken(file, ";");
        }
		else if (token == "index")
		{
			token = _getNextToken(file);
			route.index = token;
			_expectToken(file, ";");
		}
		else if (token == "upload_dir")
		{
			token = _getNextToken(file);
			route.uploadDir = token;
			_expectToken(file, ";");
		}
        // Add more route configurations as needed
    }
    
    server.routes.push_back(route);
}

// Modified token getter to handle comments
std::string Config::_getNextToken(std::ifstream &file)
{
    std::string token;
    char c;
    bool inComment = false;

    while (file.good())
    {
        c = file.peek();
        if (c == EOF)
            break;

        // Handle comments
        if (inComment)
        {
            if (c == '\n')
                inComment = false;
            file.get();
            continue;
        }

        if (c == '#')
        {
            inComment = true;
            file.get();
            continue;
        }

        // Skip whitespace
        if (std::isspace(c))
        {
            file.get();
            continue;
        }

        // Handle special characters
        if (c == ';' || c == '{' || c == '}')
        {
            token = file.get();
            return token;
        }

        // Read regular token
        while (file.good())
        {
            c = file.peek();
            if (c == EOF || std::isspace(c) || c == ';' || c == '{' || c == '}' || c == '#')
                break;
            token += file.get();
        }

        if (!token.empty())
            return token;
    }
    return token;
}

void Config::_expectToken(std::ifstream &file, const std::string &expected)
{
    std::string token = _getNextToken(file);
    if (token != expected)
        throw std::runtime_error("Expected '" + expected + "', got '" + token + "'");
}

bool Config::_isValidHost(const std::string &host) const
{
    if (host == "localhost" || host == "0.0.0.0")
        return true;
    
    struct in_addr addr;
    return inet_pton(AF_INET, host.c_str(), &addr) == 1;
}

bool Config::_isValidPort(int port) const
{
    return port > 0 && port < 65536;
}

const std::vector<Config::ServerConfig>& Config::getServers() const
{
    return _servers;
}

/* std::ostream&   operator<<(std::ostream& out, const Config& src)
{
     
} */