/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/23 01:13:09 by lwoiton           #+#    #+#             */
/*   Updated: 2024/12/20 14:32:28 by lwoiton          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPResponse.hpp"

const size_t HTTPResponse::CHUNK_SIZE = 8192;

HTTPResponse::HTTPResponse() 
	: _tempFile(NULL)
	, _usingTempFile(false)
	, _readOffset(0)
	, _bodySize(0)
{
	
}

HTTPResponse::~HTTPResponse()
{
}

HTTPResponse::ResponseState	HTTPResponse::getState() const
{
	return (_state);
}

void HTTPResponse::setStatus(int code)
{
    _statusCode = code;
}

int HTTPResponse::getStatus() const
{
	return _statusCode;
}

void HTTPResponse::setHeader(const std::string& key, const std::string& value)
{
    _headers[key] = value;
}

void HTTPResponse::deleteHeader(const std::string& key)
{
	_headers.erase(key);
}

void HTTPResponse::setBody(const std::vector<char>& body)
{
    _body = body;
    _bodySize = body.size();
}

void	HTTPResponse::appendToBody(const char* data, size_t len)
{
	_body.insert(_body.end(), data, data + len);
	_bodySize += len;
}

size_t HTTPResponse::getBodySize() const
{
	return _bodySize;
}

void HTTPResponse::reset()
{
	if (_tempFile)
		delete _tempFile;
	_tempFile = NULL;
	_usingTempFile = false;
	_readOffset = 0;
	_sendBuffer.clear();
	_statusCode = 0;
	_headers.clear();
	_body.clear();
	_bodySize = 0;
}

std::string HTTPResponse::getHttpDate() {
    time_t now = time(0);
    struct tm* gmt = gmtime(&now);
    
    const char* days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", 
                           "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    
    std::ostringstream ss;
    ss << days[gmt->tm_wday] << ", "
       << std::setfill('0') << std::setw(2) << gmt->tm_mday << " "
       << months[gmt->tm_mon] << " "
       << (1900 + gmt->tm_year) << " "
       << std::setfill('0') << std::setw(2) << gmt->tm_hour << ":"
       << std::setfill('0') << std::setw(2) << gmt->tm_min << ":"
       << std::setfill('0') << std::setw(2) << gmt->tm_sec << " GMT";
    
    return ss.str();
}

std::string HTTPResponse::getStatusText() const
{
    switch (_statusCode)
    {
		case 100: return "Continue";
		case 101: return "Switching Protocols";
		case 102: return "Processing";
		case 103: return "Early Hints";
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 400: return "Bad Request";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 413: return "Payload Too Large";
        case 500: return "Internal Server Error";
        case 501: return "Not Implemented";
        case 505: return "HTTP Version Not Supported";
        default: return "Unknown";
    }
}

void	HTTPResponse::setEssentialHeaders()
{
	if (_statusCode != 204 && _statusCode >= 200)
		setHeader("Content-Length", std::to_string(_bodySize));
	setHeader("Date", getHttpDate());
	setHeader("Server", "webserv/1.0");
}

std::vector<char> HTTPResponse::serialize() const
{
    // First create the header string
    std::stringstream headerStream;
    
    // Status line
    headerStream << "HTTP/1.1 " << _statusCode << " " << getStatusText() << "\r\n";
    
    // Headers
    for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); 
         it != _headers.end(); ++it) {
        headerStream << it->first << ": " << it->second << "\r\n";
    }
    
    // Empty line separating headers and body
    headerStream << "\r\n";
    
    std::string headerString = headerStream.str();
    
    // Create the final response vector
    std::vector<char> response;
    
    // Reserve space for both headers and body to avoid reallocations
    response.reserve(headerString.size() + _body.size());
    
    // Copy headers
    response.insert(response.end(), headerString.begin(), headerString.end());
    
    // Copy body
    response.insert(response.end(), _body.begin(), _body.end());
    
    return response;
}

void HTTPResponse::print() const 
{
    std::stringstream ss;
    ss << "\n=== HTTP Response ===\n";
    ss << "HTTP/1.1 " << _statusCode << " " << getStatusText() << "\n";
    
    // Print headers
    ss << "--- Headers ---\n";
    std::map<std::string, std::string>::const_iterator it;
    for (it = _headers.begin(); it != _headers.end(); ++it)
    {
        ss << it->first << ": " << it->second << "\n";
    }

    // Print body based on Content-Type
    std::string contentType = _headers.find("Content-Type") != _headers.end() ? 
                             _headers.at("Content-Type") : "";

    ss << "\n--- Body ---\n";
    if (contentType.find("text/html") != std::string::npos)
    {
		ss << std::string(_body.begin(), _body.end()) << "\n";
	}
    else 
    {
        ss << "[" << _bodySize << " bytes of " << contentType << "]\n";
    }
    
    ss << "==================\n";
    LOG_DEBUG(ss.str());
}