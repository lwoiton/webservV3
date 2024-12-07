/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/23 01:13:09 by lwoiton           #+#    #+#             */
/*   Updated: 2024/11/29 22:34:46 by lwoiton          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPResponse.hpp"

const size_t HTTPResponse::CHUNK_SIZE = 8192;

HTTPResponse::HTTPResponse() 
	: _tempFile(NULL)
	, _usingTempFile(false)
	, _readOffset(0)
{
	
}

HTTPResponse::~HTTPResponse()
{
	delete _tempFile;
	_tempFile = NULL;
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

/* void HTTPResponse::setBody(const std::string& body)
{
    _body = body;
    // Automatically set Content-Length header
    std::stringstream ss;
    ss << body.length();
    _bodySize = body.length();
	if (_statusCode != 204 && _statusCode >= 200)
    	setHeader("Content-Length", ss.str());
	setHeader("Date", getHttpDate());
	setHeader("Server", "webserv/1.0");
} */

void	HTTPResponse::setBody(const std::string& body)
{
	if (body.length() > MEMORY_THRESHOLD)
	{
		_tempFile = new TempFile();
		_tempFile->write(body.c_str(), body.size());
		_usingTempFile = true;
		_body.clear();
	}
	else
	{
		_body = body;
	}
	setHeader("Content-Length", toString(body.length()));
}

// Get next chunk for sending
std::vector<char> HTTPResponse::getNextChunk()
{
	std::vector<char> chunk;
	
	if (_usingTempFile)
	{
		char buffer[CHUNK_SIZE];
		size_t bytesRead = _tempFile->read(buffer, CHUNK_SIZE);
		if (bytesRead > 0) {
			chunk.assign(buffer, buffer + bytesRead);
		}
	}
	else if (!_body.empty())
	{
		size_t remaining = _body.length() - _readOffset;
		size_t chunkSize = std::min(remaining, CHUNK_SIZE);
		chunk.assign(_body.begin() + _readOffset, 
					_body.begin() + _readOffset + chunkSize);
		_readOffset += chunkSize;
	}
	
	return chunk;
}

bool HTTPResponse::hasMoreData() const
{
	if (_usingTempFile)
		return _readOffset < _bodySize;
	return _readOffset < _body.length();
}

std::string	HTTPResponse::getBody() const
{
	if (_usingTempFile)
	{
		return std::string("Please implement retading from response body temp file!");
	}
	return _body;
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
	setHeader("Date", getHttpDate());
	setHeader("Server", "webserv/1.0");
}

std::string HTTPResponse::serialize() const
{
    std::stringstream response;

    // Status line
    response << "HTTP/1.1 " << _statusCode << " " << getStatusText() << "\r\n";

    // Headers
    std::map<std::string, std::string>::const_iterator it;
    for (it = _headers.begin(); it != _headers.end(); ++it)
    {
        response << it->first << ": " << it->second << "\r\n";
    }

    // Empty line separating headers from body
    response << "\r\n";

    // Body
    if (!_body.empty())
    {
        response << _body;
    }

    return response.str();
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
        ss << _body << "\n";
    }
    else 
    {
        ss << "[" << _bodySize << " bytes of " << contentType << "]\n";
    }
    
    ss << "==================\n";
    LOG_DEBUG(ss.str());
}