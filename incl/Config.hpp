/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/22 18:47:28 by lwoiton           #+#    #+#             */
/*   Updated: 2024/10/30 21:19:46 by lwoiton          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#ifndef CONFIG_HPP
# define CONFIG_HPP

# include "Logger.hpp"

# include <string>
# include <vector>
# include <map>
# include <set>

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <arpa/inet.h>
#include <algorithm>

class Config
{
    public:
        struct Route
        {
            std::string                 path; 
            std::string                 root;
            std::set<std::string>      allowedMethods;
            bool                        autoindex;
            std::string                index;
            std::string                redirect;
            std::string                uploadDir;
            std::set<std::string>      cgiExtensions;

            Route() : autoindex(false) {}
        };

        struct ServerConfig
        {
            std::string                         host;
            int                                 port;
            std::vector<std::string>           serverNames;
            std::string                        root;
            size_t                             clientMaxBodySize;
            std::map<int, std::string>         errorPages;
            std::vector<Route>                 routes;

            ServerConfig() : port(80), clientMaxBodySize(1024 * 1024) {}  // Default 1MB
        };

                                        explicit Config(const std::string &configPath);
                                        ~Config();
        
        const std::vector<ServerConfig> &getServers() const;
        
    private:
        std::vector<ServerConfig>      _servers;
        void                           _parseConfig(const std::string &configPath);
        void                           _parseServer(std::ifstream &file);
        void                           _parseRoute(std::ifstream &file, ServerConfig &server);
        std::string                    _getNextToken(std::ifstream &file);
        void                           _expectToken(std::ifstream &file, const std::string &expected);
        bool                           _isValidHost(const std::string &host) const;
        bool                           _isValidPort(int port) const;

        // Prevent copying
                                        Config(const Config&);
        Config&                         operator=(const Config&);
};

std::ostream&   operator<<(std::ostream& out, const Config& src);

#endif // CONFIG_HPP