/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/23 01:11:10 by lwoiton           #+#    #+#             */
/*   Updated: 2024/11/29 22:35:16 by lwoiton          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPRequest.hpp"
#include "Utils.hpp"
#include <string.h>
#include <algorithm>
#include <cstdlib>

HTTPRequest::HTTPRequest()
	: _state(REQUEST_LINE)
	, _url(NULL)
	, _multipartState(NULL)
{
	
}

HTTPRequest::~HTTPRequest()
{
	// delete _url;
	// if (_multipartState)
	// 	delete _multipartState;
}

void HTTPRequest::parse(std::vector<char> &data)
{
	if(_state == REQUEST_LINE)
		parseRequestLine(data);
	if(_state == HEADERS)
		parseHeaders(data);
    print(false, true);
	if(_state == BODY_INIT)
		determineBodyType();
	if (_state == BODY)
		parseBody(data);
}


void HTTPRequest::parseRequestLine(std::vector<char> &data)
{
    size_t index = 0;
    const size_t data_size = data.size();

    // 1. Check if we have enough data for a complete request line
    size_t eol = HTTPUtils::findEOL(data);
    if (eol == std::string::npos)
        return; // Need more data

    // 2. Parse Method (token)
    while (index < data_size && HTTPUtils::isToken(static_cast<unsigned char>(data[index]))) {
        _method += data[index++];
    }
    
    // Validate method presence and content
    if (_method.empty())
        throw HTTPError(400, "Bad Request: No Method");
	// Server only supports GET, POST and DELETE methods
    if (_method != "GET" && _method != "POST" && _method != "DELETE" && _method != "HEAD")
        throw HTTPError(405, "Method Not Allowed");

    // 3. Exactly one SP after method
    if (index >= data_size || data[index] != ' ')
        throw HTTPError(400, "Bad Request: Missing space after method");
    index++;

    // 4. Parse Request-URI
    // URI can't contain whitespace according to RFC 7230
    while (index < data_size && !std::isspace(static_cast<unsigned char>(data[index]))) {
        _uri += data[index++];
    }
    
    // Validate URI presence
    if (_uri.empty())
        throw HTTPError(400, "Bad Request: No URI");

    // 5. Exactly one SP after URI
    if (index >= data_size || data[index] != ' ')
        throw HTTPError(400, "Bad Request: Missing space after URI");
    index++;

    // 6. Parse HTTP-Version
    while (index < data_size && !std::isspace(static_cast<unsigned char>(data[index]))) {
        _version += data[index++];
    }

    // Validate version
    if (_version.empty())
        throw HTTPError(400, "Bad Request: No HTTP Version");
	// RFC 7230 section 2.6 defines the version format:
	// 					HTTP-version  = HTTP-name "/" DIGIT "." DIGIT
    //					HTTP-name     = %x48.54.54.50 ; "HTTP", case-sensitive
	// We only support Version 1.1
    if (_version != "HTTP/1.1")
        throw HTTPError(505, "HTTP Version Not Supported");

    // 7. Must end with CRLF
	if (HTTPUtils::findEOL(data, index) != index)
		throw HTTPError(400, "Bad Request: Invalid line ending");
    index += 2;

    // 8. Parse URI into URL object
    try {
		if (_url)
        	delete _url; // Clean up old URL if exists
        _url = new URL(_uri);
    } catch (const std::exception& e) {
        throw HTTPError(400, "Bad Request: Invalid URI");
    }
	
	// Check Request Line 

    // Remove parsed data from buffer
    data.erase(data.begin(), data.begin() + index);
    _state = HEADERS;
	LOG_DEBUG("Parse State change to 'HEADERS'");
}

/**
 * @brief Parses the HTTP headers from the provided data buffer.
 *
 * This function processes the HTTP headers from the given data buffer and stores them in the 
 * _headers map. It expects the headers to be in the format specified by RFC 7230, Section 3.2.
 * The function looks for the end of the header section, which is indicated by a double CRLF 
 * sequence (\r\n\r\n). Each header field is expected to have a field name followed by a colon 
 * and a field value. Optional whitespace (OWS) around the colon and at the end of the field 
 * value is ignored.
 * 
 * @param data A vector of characters containing the raw HTTP request data.
 *
 * @throws HTTPError If the header format is invalid (e.g., missing colon or invalid characters 
 *                   in the header name).
 *
 * @note This function modifies the input data buffer by removing the parsed headers.
 * 
 * @see RFC 7230, Section 3.2: https://tools.ietf.org/html/rfc7230#section-3.2
 */
void HTTPRequest::parseHeaders(std::vector<char>& data, bool isTrailer)
{
    size_t index = 0;
    
    // Check for complete header section (ends with \r\n\r\n)
    size_t header_end = HTTPUtils::findHeaderEnd(data);
    if (header_end == std::string::npos)
        return; // Need more data
    
        
    while (index < header_end) {
        // Find the colon
        size_t colon = HTTPUtils::findNextColon(data, index);
        if (colon == std::string::npos) {
            std::cout << "Header line: " << std::string(data[0], data[header_end]) << std::endl;
            std::cout << "ERROR THROWS HERE 1" << std::endl;
            throw HTTPError(400, "Invalid Header Format");
        }
            
        // Extract field name (token)
        std::string name;
        while (index < colon) {
            char c = static_cast<unsigned char>(data[index]);
            if (!HTTPUtils::isToken(static_cast<unsigned char>(c))) {   
                throw HTTPError(400, "Invalid Header Name");
            }
            name += c;
            index++;
        }
        
        // Skip colon and OWS (Optional WhiteSpace)
        index++; // skip colon
        while (index < header_end && HTTPUtils::isOWS(data[index]))
            index++;
            
        // Extract field value
        std::string value;
        while (index < header_end)
		{
            // Handle line folding (obs-fold)
            if (data[index] == '\r' && index + 1 < header_end && data[index + 1] == '\n')
			{
                if (index + 2 < header_end && (data[index + 2] == ' ' || data[index + 2] == '\t'))
				{
                    index += 2;
                    continue;
                }
                break;
            }
            value += data[index++];
        }
        
        // Trim trailing OWS
        value = HTTPUtils::trimOWS(value);

		if (isTrailer)
		{
			// RFC 7230 Section 4.1.2: Trailer must not contain certain headers
            if (name == "Transfer-Encoding" || 
                name == "Content-Length" || 
                name == "Host" || 
                name == "Cache-Control" || 
                name == "Max-Forwards" || 
                name == "TE" || 
                name == "Authorization")
                throw HTTPError(400, "Bad Request: Invalid Trailer Field");
		}
        
        // Store header
        _headers[name] = value;
        
        // Skip CRLF
        index += 2;
    }
    
    // Remove parsed headers from buffer
    data.erase(data.begin(), data.begin() + header_end + 4); // +4 for final \r\n\r\n
    
	if (!isTrailer)
	{
		LOG_DEBUG("Parse State change to 'BODY_INIT'");
    	_state = BODY_INIT;
	}
}

/**
 * @brief Determines how to handle the message body based on headers
 * @details RFC 7230 Section 3.3.3 defines message body length determination
 * 
 * @param headers Map of parsed headers
 * @return BodyLengthType indicating how to process the body
 */
void HTTPRequest::determineBodyType(void)
{
    LOG_DEBUG("determineBodyType called");
    
    // First check Content-Type for multipart
    if (_headers.find("Content-Type") != _headers.end() && 
        _headers.at("Content-Type").find("multipart/form-data") != std::string::npos)
    {
        LOG_DEBUG("Found multipart/form-data Content-Type - setting MULTIPART type");
        _bodyType = MULTIPART;
		_state = BODY;
        // Still need Content-Length for multipart data
        if (_headers.find("Content-Length") != _headers.end()) {
            _bodyLength = ::atoi(_headers.at("Content-Length").c_str());
            LOG_DEBUG("Multipart content length: " + toString(_bodyLength));
        }
        return;
    }
    
    // Then check Transfer-Encoding
    if (_headers.find("Transfer-Encoding") != _headers.end())
    {
        LOG_DEBUG("Found Transfer-Encoding header");
        if (HTTPUtils::hasToken(_headers.at("Transfer-Encoding"), "chunked"))
        {
            if (_headers.find("Content-Length") != _headers.end()) {
                LOG_ERROR("Both Transfer-Encoding and Content-Length present");
                throw HTTPError(400, "Bad Request: Both Transfer-Encoding and Content-Length present");
            }
            LOG_DEBUG("Setting body type to CHUNKED");
            _bodyLength = 0;
            _bodyType = CHUNKED;
			_state = BODY;
        }
        else {
            LOG_ERROR("Unsupported Transfer-Encoding");
            throw HTTPError(501, "Not Implemented");
        }
        return;
    }
    
    // Finally check Content-Length for regular body
    if (_headers.find("Content-Length") != _headers.end())
    {
        LOG_DEBUG("Found Content-Length header");
        _bodyType = CONTENT_LENGTH;
		_state = BODY;
        _bodyLength = ::atoi(_headers.at("Content-Length").c_str());
        LOG_DEBUG("Set body type to CONTENT_LENGTH with length: " + toString(_bodyLength));
        return;
    }
    
    // No body indicators found
    LOG_DEBUG("No body indicators found - setting NO_BODY type");
    _bodyType = NO_BODY;
	LOG_DEBUG("Parse State change to 'COMPLETE'");
	_state = COMPLETE;
}


void HTTPRequest::parseBody(std::vector<char> &data)
{
    LOG_DEBUG("parseBody called with data size: " + toString(data.size()));
    
    switch (_bodyType)
    {
    case CONTENT_LENGTH:
        LOG_DEBUG("CONTENT_LENGTH case - calling parseContentLengthBody");
        parseContentLengthBody(data);
        break;
        
    case CHUNKED:
        LOG_DEBUG("CHUNKED case - calling parseChunkedBody");
        parseChunkedBody(data);
        break;
        
    case MULTIPART:
        LOG_DEBUG("MULTIPART case - calling parseMultipartBody");
        LOG_DEBUG("Content-Type: " + _headers["Content-Type"]);
        LOG_DEBUG("Boundary will be extracted from: " + _headers["Content-Type"]);
        parseMultipartBody(data);
        break;
        
    default:
        LOG_ERROR("Unknown body type: " + toString(_bodyType));
        break;
    }
    LOG_DEBUG("parseBody completed for current data chunk");
}

void HTTPRequest::parseContentLengthBody(std::vector<char> &data)
{
	size_t remaining = _bodyLength - _body.size();
	size_t processable = std::min(remaining, data.size());
	
	_body.insert(_body.end(), data.begin(), data.begin() + processable);
	data.erase(data.begin(), data.begin() + processable);
	
	LOG_DEBUG("Content-Length body: " + toString(_body.size()) + " / " + toString(_bodyLength));
	if (_body.size() == _bodyLength)
		_state = COMPLETE;
}


/**
 * @brief Parse chunked transfer encoding according to RFC 7230 Section 4.1
 * 
 * @details chunked-body   = *chunk last-chunk trailer-part CRLF
 * 			chunk          = chunk-size [ chunk-ext ] CRLF chunk-data CRLF
 * 			chunk-size     = 1*HEXDIG
 * 			last-chunk     = 1*("0") [ chunk-ext ] CRLF
 * 			chunk-data     = 1*OCTET
 * 			chunk-ext      = *( ";" chunk-ext-name [ "=" chunk-ext-val ] )
 * 
 * 			RFC 7230 Section 4.1.3: includes pusdo code for parsing chunked encoding
 * @param data Raw input data buffer
 */
void HTTPRequest::parseChunkedBody(std::vector<char> &data)
{
	while (!data.empty())
	{
		// State 1 - Parse chunk size (indicated by chunkLength == -1)
		if (_chunkLength == -1)
		{
			size_t eol = HTTPUtils::findEOL(data);
			if (eol == std::string::npos)
            {
				return; // Need more data (wait for epoll)
            }
			//Parse chunk size (ignore chunk extensions as per RFC 7230 Section 4.1.1)
			std::string chunkSizeLine(data.begin(), data.begin() + eol);
			size_t	semicolon = chunkSizeLine.find(';');
			std::string chunkSize;
			if (semicolon == std::string::npos)
				chunkSize = chunkSizeLine;
			else
				chunkSize = chunkSizeLine.substr(0, semicolon);
			
			for(size_t i = 0; i < chunkSize.size(); i++)
			{
				if (!std::isxdigit(chunkSize[i]) && !HTTPUtils::isOWS(static_cast<unsigned char>(chunkSize[i])))
					throw HTTPError(400, "Bad Request: Invalid Chunk Size (non-hex characters)");
			}

			// Convert chunk size string (in hex) to size
			char *endptr;
			_chunkLength = strtol(chunkSize.c_str(), &endptr, 16);
			if ( *endptr != '\0' || _chunkLength < 0)
				throw HTTPError(400, "Bad Request: Invalid Chunk Size");
			// Remove chunk size line from data
			data.erase(data.begin(), data.begin() + eol + 2); // +2 for CRLF
            LOG_DEBUG("CHUNK LENGTH - new: " + toString(_chunkLength));   
		}
        LOG_DEBUG("CHUNK LENGTH - current:" + toString(_chunkLength));
		// State 2: Reading Chunk Data (indicated by chunkLength > 0), Try to read everything in data
		if (_chunkLength > 0)
		{
            size_t eol = HTTPUtils::findEOL(data);
			if (eol == std::string::npos) {
				return; // Need more data (wait for epoll)
            }
			// Calculate how much data we can process
			// size_t processable = std::min(static_cast<size_t>(_chunkLength), data.size());
            
			// Must have at least 1 byte to process (wait for more data from epoll)
			// if (processable == 0)
			// 	return;
			
			// Append chunk data to body
			_body.insert(_body.end(), data.begin(), data.begin() + _chunkLength);
			_bodyLength += _chunkLength;
			
			data.erase(data.begin(), data.begin() + _chunkLength + 2); // Remove processed data
			_chunkLength = -1; // Reset chunk length for next chunk
			
		}
		// State 3: Chunk complete, check for CRLF and optional trailer fields
		if (_chunkLength == 0)
		{
			if (data.size() < 2) {
                LOG_DEBUG("We are here");
				return; // Need more data for CRLF from epoll
            }
			LOG_DEBUG("DATA SIZE: " + toString(data.size()));
            LOG_DEBUG("DATA to HEx: " + stringToHex(std::string(data.begin() -2, data.begin() +2)));
			if (data[0] != '\r' || data[1] != '\n')
				throw HTTPError(400, "Bad Request: Missing chunk CRLF or invalid chunk size");
			
			// Remove CRLF
			data.erase(data.begin(), data.begin() + 2);

            if (data.size() == 0) {
                
                HTTPUtils::removeToken(_headers["Transfer-Encoding"], "chunked");
                _headers.erase("Trailer");
                _headers["Content-Length"] = toString(_body.size());
                
                _bodyType = CONTENT_LENGTH;
                _state = COMPLETE;
                _chunkLength = -1;
                return;
            }
                
			// Look for trailer fields (if any) ending with CRLF CRLF
			size_t trailerEnd = HTTPUtils::findHeaderEnd(data);
			if (data.size() > 0 && trailerEnd == std::string::npos)
				return; // Need more data for trailers
			
            // Parse trailer fields
			parseHeaders(data, true);
			
			// Update headers as per RFC 7230 Section 4.1.2
			return;
		}
	}
}

void HTTPRequest::parseMultipartHeaders(std::vector<char>& headerData, MultipartPart& part)
{
    std::map<std::string, std::string> headers;
    size_t pos = 0;
    
    // Parse headers until we hit empty line
    while (pos < headerData.size()) {
        // Find end of line
        size_t lineEnd = pos;
        while (lineEnd < headerData.size() && headerData[lineEnd] != '\r')
            lineEnd++;
            
        // If we've hit an empty line, we're done with headers
        if (lineEnd == pos)
            break;
            
        // Extract header line
        std::string line(headerData.begin() + pos, headerData.begin() + lineEnd);
        
        // Find the colon
        size_t colon = line.find(':');
        if (colon != std::string::npos) {
            std::string name = line.substr(0, colon);
            std::string value = line.substr(colon + 1);
            
            // Trim whitespace
            value = HTTPUtils::trimOWS(value);
            headers[name] = value;
        }
        
        // Move past this line and its CRLF
        pos = lineEnd + 2;
    }
    
    // Process Content-Disposition header
    std::string disposition = headers["Content-Disposition"];
    if (disposition.empty() || disposition.find("form-data") == std::string::npos)
        throw HTTPError(400, "Bad Request: Invalid Content-Disposition in multipart");
        
    // Extract name parameter
    size_t namePos = disposition.find("name=\"");
    if (namePos != std::string::npos) {
        namePos += 6;  // length of 'name="'
        size_t nameEnd = disposition.find("\"", namePos);
        if (nameEnd != std::string::npos)
            part.name = disposition.substr(namePos, nameEnd - namePos);
    }
    
    // Extract filename parameter if present
    size_t filenamePos = disposition.find("filename=\"");
    if (filenamePos != std::string::npos) {
        filenamePos += 10;  // length of 'filename="'
        size_t filenameEnd = disposition.find("\"", filenamePos);
        if (filenameEnd != std::string::npos)
            part.filename = disposition.substr(filenamePos, filenameEnd - filenamePos);
    }
    
    // Get Content-Type if present
    std::map<std::string, std::string>::iterator it = headers.find("Content-Type");
    if (it != headers.end())
        part.contentType = it->second;

    // Debug logging
    LOG_DEBUG("Parsed multipart part - Name: " + part.name + 
              ", Filename: " + part.filename + 
              ", Content-Type: " + part.contentType);
}


void HTTPRequest::parseMultipartBody(std::vector<char>& data)
{
    LOG_DEBUG("=== Starting Multipart Parsing ===");
    LOG_DEBUG("Data size: " + toString(data.size()) + " bytes");
    
    // First time initialization
    if (!_multipartState) {
        _multipartState = new MultipartState();
        
        // Extract boundary
        std::string contentType = _headers["Content-Type"];
        _multipartState->boundary = "--" + contentType.substr(contentType.find("boundary=") + 9);
        LOG_DEBUG("Boundary: " + _multipartState->boundary);
        
        // Skip preamble
        _multipartState->boundaryStart = HTTPUtils::findString(data, _multipartState->boundary);
        if (_multipartState->boundaryStart == std::string::npos) {
            return;
        }
        
        data.erase(data.begin(), data.begin() + _multipartState->boundaryStart);
    }
    
    while (!data.empty()) {
        // Find next boundary
        size_t boundaryPos = HTTPUtils::findString(data, _multipartState->boundary);
        if (boundaryPos == std::string::npos) {
            return;
        }
        
        // Check for final boundary
        if (boundaryPos + _multipartState->boundary.length() + 2 <= data.size()) {
            if (data[boundaryPos + _multipartState->boundary.length()] == '-' && 
                data[boundaryPos + _multipartState->boundary.length() + 1] == '-') {
                LOG_DEBUG("Final boundary found - parsing complete");
                _state = COMPLETE;
                return;
            }
        }
        
        // Parse headers
        std::vector<char> partData(data.begin() + boundaryPos + _multipartState->boundary.length() + 2, data.end());
        size_t headerEnd = HTTPUtils::findHeaderEnd(partData);
        if (headerEnd == std::string::npos) {
            return;
        }
        
        // Create and parse part
        MultipartPart part;
        std::vector<char> headerData(partData.begin(), partData.begin() + headerEnd);
        parseMultipartHeaders(headerData, part);
        
        // Find body boundaries
        size_t nextBoundary = HTTPUtils::findString(data, _multipartState->boundary, 
                                                  boundaryPos + _multipartState->boundary.length() + headerEnd + 4);
        if (nextBoundary == std::string::npos) {
            return;
        }
        
        // Extract body
		size_t bodyStart = boundaryPos + _multipartState->boundary.length() + headerEnd + 4;
        // Add check for CRLF after headers
    	while (bodyStart < data.size() && (data[bodyStart] == '\r' || data[bodyStart] == '\n'))
        	bodyStart++;
        size_t bodyEnd = nextBoundary - 2;
        
        if (bodyEnd > bodyStart && bodyEnd <= data.size()) {
            part.data.reserve(bodyEnd - bodyStart);
            part.data.insert(part.data.begin(), 
                           data.begin() + bodyStart, 
                           data.begin() + bodyEnd);
        }
        
        _multipartState->parts.push_back(part);
        data.erase(data.begin(), data.begin() + nextBoundary);
    }
}

void HTTPRequest::handleWrite() {
    if (_pendingWrite.empty())
        return;
    
    try {
        size_t written = _tempFile->write(
            _pendingWrite.data() + _writeOffset, 
            _pendingWrite.size() - _writeOffset
        );
        
        _writeOffset += written;
        
        if (_writeOffset == _pendingWrite.size()) {
            _pendingWrite.clear();
            _writeOffset = 0;
        }
    } catch (const std::exception& e) {
        throw HTTPError(500, "Failed to write to temporary file: " + std::string(e.what()));
    }
}

void	HTTPRequest::appendToBody(const std::vector<char>& data)
{
    if (!_usingTempFile && (_body.size() + data.size()) > MEMORY_THRESHOLD)
	{
        _tempFile = new TempFile();
        if (!_body.empty())
		{
            _pendingWrite = _body;
            _writeOffset = 0;
            _body.clear();
        }
        _usingTempFile = true;
    }

    if (_usingTempFile)
	{
        // Add new data to pending write buffer
        _pendingWrite.insert(_pendingWrite.end(), data.begin(), data.end());
        
    }
	else
	{
		// The actual write will be handled by epoll
        _body.insert(_body.end(), data.begin(), data.end());
    }
}

void	HTTPRequest::setRouteMatch(const Config::Route* route, const std::string& remaining)
{
	_routeMatch.route = route;
	_routeMatch.remainingPath = remaining;
	_routeMatch.found = true;
}

void	HTTPRequest::setFileInfo(const std::string& path, const std::string& mimeType)
{
	struct stat st;
	if (::stat(path.c_str(), &st) == 0)
	{

		_fileInfo.exists = true;
		_fileInfo.isDirectory = S_ISDIR(st.st_mode);
		_fileInfo.path = path;
		_fileInfo.mimeType = mimeType;
	}
	else
	{
		throw HTTPError(404, "Not Found");
	}
}

bool HTTPRequest::isCGI(void) const
{
	return (this->_uri.find(".bla") != std::string::npos);
}

bool	HTTPRequest::shouldKeepAlive() const
{
	if (_headers.find("Connection") != _headers.end())
	{
		if (_headers.at("Connection") == "close")
			return false;
	}
	return true;
}

// Accessor methods
HTTPRequest::RequestState	HTTPRequest::getState() const
{
	return _state;
}

void	HTTPRequest::setState(RequestState state)
{
	_state = state;
}

void	HTTPRequest::setBodyType(BodyType type)
{
	_bodyType = type;
}

void	HTTPRequest::setMethod(const std::string& method)
{
	_method = method;
}

const std::string& HTTPRequest::getMethod() const
{ 
    return _method; 
}

const URL& HTTPRequest::getURL() const
{ 
    if (!_url)
		throw HTTPError(500, "URL not initialized");
	return *_url;
}

const std::string& HTTPRequest::getVersion() const
{ 
    return _version;
}

const std::string& HTTPRequest::getUri() const
{ 
    return _uri;
}


const std::vector<char>& HTTPRequest::getBody() const
{ 
    return _body; 
}

const std::string& HTTPRequest::getHeader(const std::string& key) const
{
    static const std::string empty;
    std::map<std::string, std::string>::const_iterator it = _headers.find(key);
    return it != _headers.end() ? it->second : empty;
}

const Config::Route		*HTTPRequest::getMatchedRoute() const
{
	return _routeMatch.route;
}

const std::string		&HTTPRequest::getRemainingPath() const
{
	return _routeMatch.remainingPath;
}

const HTTPRequest::FileInfo	&HTTPRequest::getFileInfo() const
{
	return _fileInfo;
}

const HTTPRequest::MultipartState	&HTTPRequest::getMultipartState() const
{
	return *_multipartState;
}

bool	HTTPRequest::hasMatchedRoute() const
{
	return _routeMatch.found;
}

int		HTTPRequest::getTempFileFd()
{
	if (_tempFile)
		return _tempFile->getFd();
	return -1;
}

bool	HTTPRequest::hasFileOperationsPending() const
{
	return _tempFile != NULL;
}

void HTTPRequest::reset()
{
    // Reset state
    _state = REQUEST_LINE;
    _bodyType = NO_BODY;
    _bodyLength = 0;
    _chunkLength = -1;

    // Clear strings
    _method.clear();
    _uri.clear();
    _version.clear();

    // Handle pointers
    delete _url;
    _url = NULL;
    delete _multipartState;
    _multipartState = NULL;
	delete _tempFile;
	_tempFile = NULL;

    // Clear collections
    _headers.clear();
    _body.clear();

    // Reset structs
    _routeMatch = RouteMatch();
    _fileInfo = FileInfo();
}

void HTTPRequest::print(bool includeBodies, bool allHeaders, const std::set<std::string>& allowedMimeTypes) const 
{
    std::stringstream ss;
    ss << "\n=== HTTP Request ===\n";
    ss << _method << " " << _uri << " " << _version << "\n";

    // Print headers
    ss << "--- Headers ---\n";
    if (allHeaders) {
        // Print all headers
        for (std::map<std::string, std::string>::const_iterator it = _headers.begin();
             it != _headers.end(); ++it) {
            ss << it->first << ": " << it->second << "\n";
        }
    } else {
        // Print only essential headers
        const std::string essentialHeaders[] = {
            "Content-Type", "Content-Length", "Host", "Connection"
        };
        for (size_t i = 0; i < sizeof(essentialHeaders)/sizeof(essentialHeaders[0]); ++i) {
            std::map<std::string, std::string>::const_iterator it = _headers.find(essentialHeaders[i]);
            if (it != _headers.end()) {
                ss << it->first << ": " << it->second << "\n";
            }
        }
    }

    // Handle multipart data with mime type filtering
    if (_multipartState && includeBodies) {
        ss << "\n--- Multipart Data ---\n";
        ss << "Boundary: " << _multipartState->boundary << "\n";
        
        for (size_t i = 0; i < _multipartState->parts.size(); ++i) {
            const MultipartPart& part = _multipartState->parts[i];
            
            // Skip if mime type filtering is active and type doesn't match
            if (!allowedMimeTypes.empty() && 
                allowedMimeTypes.find(part.contentType) == allowedMimeTypes.end()) {
                continue;
            }
            
            ss << "\nPart " << (i + 1) << ":\n";
            if (!part.filename.empty()) {
                ss << " Filename: " << part.filename << "\n";
                ss << " Content-Type: " << part.contentType << "\n";
                ss << " Size: " << part.data.size() << " bytes\n";
                ss << " Data: " << std::string(part.data.begin(), part.data.end()) << "\n";
            }
        }
    }
    // Handle regular body
    else if (includeBodies) {
        std::string contentType = getHeader("Content-Type");
        
        // Check if content type is allowed when filtering is active
        if (allowedMimeTypes.empty() || 
            allowedMimeTypes.find(contentType) != allowedMimeTypes.end()) {
            ss << "\n--- Body ---\n";
            ss << "Size: " << _body.size() << " bytes\n";
            ss << "Data: " << std::string(_body.begin(), _body.end()) << "\n";
        }
    }
    else {
        ss << "\n--- Body ---\n";
        ss << "Size: " << _body.size() << " bytes\n";
    }
    
    ss << "==================\n";
    LOG_DEBUG(ss.str());
}

// print State
void	HTTPRequest::printState() const
{
    std::string state;
    switch (_state)
    {
    case REQUEST_LINE:
        state = "REQUEST_LINE";
        break;
    case HEADERS:
        state = "HEADERS";
        break;
    case BODY_INIT:
        state = "BODY_INIT";
        break;
    case BODY:
        state = "BODY";
        break;
    case COMPLETE:
        state = "COMPLETE";
        break;
    }
    LOG_DEBUG("Request State: " + state);   
}