/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPError.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/23 00:21:37 by lwoiton           #+#    #+#             */
/*   Updated: 2024/11/18 13:50:15 by lwoiton          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPError.hpp"

HTTPError::HTTPError(int code, const std::string& message) 
    : _code(code), _message(message)
{
    std::stringstream ss;
    ss << "HTTP Error " << code;
    if (!message.empty())
        ss << ": " << message;
            
    // Log the error when it's created
    LOG_ERROR(ss.str());
}

HTTPError::HTTPError(int code) 
    : _code(code), _message()
{
    LOG_ERROR(what());
}

const char* HTTPError::what() const throw()
{
    if (_message.empty())
    {
        switch (_code)
        {
            case 400: return "Bad Request";
            case 401: return "Unauthorized";
            case 403: return "Forbidden";
            case 404: return "Not Found";
            case 405: return "Method Not Allowed";
            case 408: return "Request Timeout";
            case 411: return "Length Required";
            case 413: return "Payload Too Large";
            case 414: return "URI Too Long";
            case 415: return "Unsupported Media Type";
            case 500: return "Internal Server Error";
            case 501: return "Not Implemented";
            case 502: return "Bad Gateway";
            case 503: return "Service Unavailable";
            case 505: return "HTTP Version Not Supported";
            default: return "Unknown Error";
        }
    }
    return _message.c_str();
}

const char* WouldBlockException::what() const throw()
{
	return "Operation would block";
}

int HTTPError::getCode() const
{
    return _code;
}

HTTPResponse HTTPError::createErrorResponse(const std::string& serverRoot) const 
{
    HTTPResponse response;
    response.setStatus(_code);
    response.setHeader("Content-Type", "text/html");

    // Try to load custom error page
    std::string errorPath = serverRoot + "/error/" + TO_STRING(_code) + ".html";
    std::string errorContent = _loadErrorPage(errorPath);
    
    if (!errorContent.empty()) {
        response.setBody(errorContent);
    } else {
        // Use default error page if custom one not found
        response.setBody(_getDefaultErrorPage());
    }

    return response;
}

std::string HTTPError::_loadErrorPage(const std::string& path) const 
{
    std::ifstream file(path.c_str(), std::ios::binary);
    if (!file) {
        LOG_DEBUG("Error page not found: " + path);
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string HTTPError::_getDefaultErrorPage() const 
{
    std::stringstream ss;
    ss << "<!DOCTYPE html>\n"
       << "<html>\n"
       << "<head>\n"
       << "    <title>Error " << _code << "</title>\n"
       << "    <style>\n"
       << "        body { font-family: Arial, sans-serif; margin: 40px; }\n"
       << "        .error-container { text-align: center; }\n"
       << "        .error-code { font-size: 72px; color: #666; }\n"
       << "        .error-message { font-size: 24px; color: #333; }\n"
       << "    </style>\n"
       << "</head>\n"
       << "<body>\n"
       << "    <div class='error-container'>\n"
       << "        <div class='error-code'>" << _code << "</div>\n"
       << "        <div class='error-message'>" << what() << "</div>\n"
       << "    </div>\n"
       << "</body>\n"
       << "</html>";
    return ss.str();
}