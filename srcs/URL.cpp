/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   URL.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/07 11:59:27 by lwoiton           #+#    #+#             */
/*   Updated: 2024/11/11 18:52:38 by lwoiton          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "URL.hpp"

URL::URL(const std::string& uri)
{
    _uri = uri;
    _scheme.clear();
    _authority.clear();
    _path.clear();
    _query.clear();
    _queryParams.clear();
    _fragment.clear();
    _absoluteForm = false;

	// 1. Basic structural validation
	if (_uri.empty())
		throw HTTPError(400, "Invalid URL");
	if (_uri.length() > 2048)
		throw HTTPError(414, "URI Too Long");

	// 2. Parse URI into URL components
	_parse(uri);

	// 3. Validate URL components
	_validateURL();
}

URL::~URL()
{
	
}

void URL::_parse(const std::string& uri)
{
    size_t currentPos = 0;
    size_t endPos;

    // Parse scheme
    endPos = uri.find("://");
    if (endPos != std::string::npos) {
        _scheme = _normalize(uri.substr(0, endPos));
        currentPos = endPos + 3;
        _absoluteForm = true;
    }

    // Parse authority (if absolute form)
    if (_absoluteForm) {
        endPos = uri.find("/", currentPos);
        if (endPos != std::string::npos) {
            _authority = _normalize(uri.substr(currentPos, endPos - currentPos));
            currentPos = endPos;
        } else {
            _authority = _normalize(uri.substr(currentPos));
            return;
        }
    }

    // Parse path
    endPos = uri.find_first_of("?#", currentPos);
    if (endPos != std::string::npos)
	{
		_path = _decodeComponent(uri.substr(currentPos, endPos - currentPos));
		_normalizePath();
        currentPos = endPos;
    }
	else
	{
		_path = _decodeComponent(uri.substr(currentPos));
		_normalizePath();
        return;
    }

    // Parse query
    if (currentPos < uri.length() && uri[currentPos] == '?') {
        ++currentPos;
        endPos = uri.find('#', currentPos);
        
        if (endPos != std::string::npos) {
            _query = uri.substr(currentPos, endPos - currentPos);
            currentPos = endPos;
        } else {
            _query = uri.substr(currentPos);
            return;
        }
        _parseQueryParams(_query);
    }

    // Parse fragment
    if (currentPos < uri.length() && uri[currentPos] == '#') {
        _fragment = _decodeComponent(uri.substr(currentPos + 1));
    }
}

void URL::_parseQueryParams(const std::string& query) {
    size_t start = 0;
    size_t end;

	_query = query;
    while (start < query.length()) {
        // Find the end of the current parameter
        end = query.find('&', start);
        if (end == std::string::npos) {
            end = query.length();
        }

        // Find the equals sign separating key and value
        size_t equals = query.find('=', start);
        if (equals != std::string::npos && equals < end) {
            std::string key = _decodeComponent(query.substr(start, equals - start));
            std::string value = _decodeComponent(query.substr(equals + 1, end - equals - 1));
            _queryParams[key] = value;
        }
        start = end + 1;
    }
}

std::string URL::_normalize(std::string src)
{
    std::transform(src.begin(), src.end(), src.begin(), ::tolower);
    return (src);
}

void URL::_normalizePath()
{
	if (_path.empty())
	{
		_path = "/";
		return;
	}
	
	std::string normalized;
	bool lastWasSlash = false;
	for (std::string::const_iterator it = _path.begin(); it != _path.end(); it++)
	{
		if (*it == '/')
		{
			if (!lastWasSlash)
			{
				normalized += '/';
				lastWasSlash = true;
			}
		}
		else
		{
			normalized += *it;
			lastWasSlash = false;
		}
	}
	
	//Handle dot segments
	std::vector<std::string> segments;
	std::stringstream pathStream(normalized);
	std::string segment;

	while (std::getline(pathStream, segment, '/'))
	{
		if (segment == "..")
		{
			if (!segments.empty())
			{
				segments.pop_back();
			}
		}
		else if (segment != "." && !segment.empty())
		{
			segments.push_back(segment);
		}
	}

	//Reconstruct path
	_path.clear();
	_path = "/";
	for (size_t i = 0; i < segments.size(); i++)
	{
		if (i > 0)
		{
			_path += "/";
		}
		_path += segments[i];
	}
}

std::string	URL::_decodeComponent(const std::string& enocoded)
{
	std::string decoded;
	for (size_t i = 0; i < enocoded.length(); i++)
	{
		if (enocoded[i] == '%' && i + 2 < enocoded.length())
		{
			std::string hex = enocoded.substr(i + 1, 2);
			std::istringstream iss(hex);
			int value;
			iss >> std::hex >> value;
			decoded += static_cast<char>(value);
			i += 2;
		}
		else if (enocoded[i] == '+')
		{
			decoded += ' ';
		}
		else
		{
			decoded += enocoded[i];
		}
	}
	return (decoded);
}

void URL::_validateURL() const
{

    // 1. Scheme validation for absolute URLs
    if (_absoluteForm)
	{
        if (_scheme != "http")
		{
            throw HTTPError(400, "Only HTTP scheme is supported");
        }
        if (_authority.empty())
		{
            throw HTTPError(400, "Absolute URL requires authority");
        }
    }

    // 2. Path validation
    _validatePath();

    // 3. Authority validation (if present)
    if (!_authority.empty())
	{
        _validateAuthority();
    }

    // 4. Query validation (if present)
    if (!_query.empty())
	{
        _validateQuery();
    }
}

void	URL::_validatePath() const {
    // Path must be empty or start with '/' in absolute form
    if (_absoluteForm && !_path.empty() && _path[0] != '/')
	{
        throw HTTPError(400, "Absolute URL path must start with '/'");
    }

    // Path must start with '/' in relative form without authority
    if (!_absoluteForm && _authority.empty() && !_path.empty() && _path[0] != '/')
    {
        throw HTTPError(400, "Path must start with '/' when no authority is present");
    }

    // Check path components
    for (std::string::const_iterator it = _path.begin(); it != _path.end(); ++it)
	{
        // Check for control characters
        if (*it < 32 || *it == 127)
		{
            throw HTTPError(400, "Invalid control character in path");
        }
    }

    // Check path segments
    std::string segment;
    std::istringstream pathStream(_path);
    while (std::getline(pathStream, segment, '/'))
	{
        // Check segment length
        if (segment.length() > 255)
		{
            throw HTTPError(414, "Path segment too long");
        }
        
        // Check for invalid characters in segment
        for (std::string::const_iterator it = segment.begin(); it != segment.end(); ++it)
		{
            if (*it == '\\' || *it == '<' || *it == '>' || *it == '"' || *it == '`')
			{
                throw HTTPError(400, "Invalid character in path segment");
            }
        }
    }
}

void URL::_validateAuthority() const {
    size_t colonPos = _authority.find(':');
    
    // Validate hostname
    std::string hostname = colonPos != std::string::npos ? 
        _authority.substr(0, colonPos) : _authority;
        
    // Check hostname length
    if (hostname.length() > 255) {
        throw HTTPError(414, "Hostname too long");
    }

    // Check hostname characters
    for (std::string::const_iterator it = hostname.begin(); it != hostname.end(); ++it) {
        if (!isalnum(*it) && *it != '-' && *it != '.') {
            throw HTTPError(400, "Invalid character in hostname");
        }
    }

    // Validate port if present
    if (colonPos != std::string::npos) {
        std::string portStr = _authority.substr(colonPos + 1);
        
        // Check if port is numeric
        for (std::string::const_iterator it = portStr.begin(); it != portStr.end(); ++it) {
            if (!isdigit(*it)) {
                throw HTTPError(400, "Invalid port number");
            }
        }

        // Check port range
        int port = std::atoi(portStr.c_str());
        if (port <= 0 || port > 65535) {
            throw HTTPError(400, "Port number out of range");
        }
    }
}

void URL::_validateQuery() const {
    // Check total query length
    if (_query.length() > 2048) {
        throw HTTPError(414, "Query string too long");
    }

    // Validate each query parameter
    for (std::map<std::string, std::string>::const_iterator it = _queryParams.begin();
         it != _queryParams.end(); ++it) {
        // Check key
        if (it->first.empty()) {
            throw HTTPError(400, "Empty query parameter key");
        }
        if (it->first.length() > 64) {
            throw HTTPError(414, "Query parameter key too long");
        }

        // Check value
        if (it->second.length() > 1024) {
            throw HTTPError(414, "Query parameter value too long");
        }

        // Check for invalid characters in key and value
        std::string::const_iterator c;
        for (c = it->first.begin(); c != it->first.end(); ++c) {
            if (*c < 32 || *c == 127) {
                throw HTTPError(400, "Invalid character in query parameter key");
            }
        }
        for (c = it->second.begin(); c != it->second.end(); ++c) {
            if (*c < 32 || *c == 127) {
                throw HTTPError(400, "Invalid character in query parameter value");
            }
        }
    }
}

const std::string& URL::getEncoded() const {
    return _uri;
}

const std::string& URL::getScheme() const {
    return _scheme;
}

const std::string& URL::getAuthority() const {
    return _authority;
}

const std::string& URL::getPath() const {
    return _path;
}

const std::string& URL::getQuery() const {
    return _query;
}

const std::map<std::string, std::string>& URL::getQueryParams() const {
    return _queryParams;
}

bool URL::isAbsoluteForm() const {
    return _absoluteForm;
}