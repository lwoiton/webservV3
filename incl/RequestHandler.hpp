/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/20 15:25:33 by lwoiton           #+#    #+#             */
/*   Updated: 2024/12/20 15:42:57 by lwoiton          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// RequestHandler.hpp
#pragma once
#ifndef REQUEST_HANDLER_HPP
#define REQUEST_HANDLER_HPP

#include <map>
#include <string>
#include <vector>
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "Config.hpp"
#include "MIMETypes.hpp"

// RequestHandler.hpp
class RequestHandler {
private:
    static RequestHandler* _instance;
    
    // Configuration data that's constant after initialization
    const std::vector<Config::ServerConfig>& _config;
    std::map<std::string, const Config::Route*> _routingTable;
    MIMETypes _mimeTypes;
    
    // Private constructor for singleton
    explicit RequestHandler(const std::vector<Config::ServerConfig>& config);
    
    // Prevent copying
    RequestHandler(const RequestHandler&);
    RequestHandler& operator=(const RequestHandler&);

public:
    static void initialize(const std::vector<Config::ServerConfig>& config);
    static RequestHandler* getInstance();
    static void cleanup();
    
    // Main service method - remains stateless
    HTTPResponse handleRequest(const HTTPRequest& req) const;

private:
    // Helper methods - all const as they don't modify instance state
    HTTPResponse handleGETRequest(const HTTPRequest& req, const Config::Route* route) const;
    HTTPResponse handlePOSTRequest(const HTTPRequest& req, const Config::Route* route) const;
    HTTPResponse handleDELETERequest(const HTTPRequest& req, const Config::Route* route) const;
    HTTPResponse handleCGIRequest(const HTTPRequest& req, const Config::Route* route) const;
    
    // Route finding
    const Config::Route* findBestRoute(const HTTPRequest& req) const;
    
    // Environment handling
    std::map<std::string, std::string> buildCGIEnvironment(const HTTPRequest& req, const std::string& scriptPath) const;
};

#endif // REQUEST_HANDLER_HPP