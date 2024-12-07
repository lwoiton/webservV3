/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/22 22:23:36 by lwoiton           #+#    #+#             */
/*   Updated: 2024/11/29 22:33:38 by lwoiton          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Connection.hpp"
#include "Logger.hpp"
#include "HTTPError.hpp"
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include "Utils.hpp"

Connection::Connection(CSocket *socket) 
    : _socket(socket)
    , _readBuffer() // Initialize empty
    , _writeBuffer() // Initialize empty
{
	if (!_socket)
	{
		LOG_DEBUG("Socket is NULL");
		throw HTTPError(500, "Internal Server Error");
	}
}

Connection::~Connection()
{
	delete _socket;
}

bool Connection::handleRead()
{
	char buffer[BUFFER_SIZE];
	ssize_t bytesRead = ::recv(getFd(), buffer, BUFFER_SIZE, 0);
    if (bytesRead < 0)
		return true;
	if (bytesRead == 0)
		return false;
    try
    {
		_readBuffer.insert(_readBuffer.end(), buffer, buffer + bytesRead);
        _currentRequest.parse(_readBuffer);
		return true;
    }
    catch(const std::exception& e)
    {
        LOG_ERROR("Parse error on fd " + TO_STRING(getFd()) + ": " + e.what());
        return false;
    }   
    return true;
}

bool Connection::handleWrite() {
    if (_state == WRITING_HEADERS) {
        // Send headers first
        if (!_writeBuffer.empty()) {
            ssize_t bytesWritten = ::send(getFd(), &_writeBuffer[0], 
                                        _writeBuffer.size(), MSG_NOSIGNAL);
            if (bytesWritten > 0) {
                _writeBuffer.erase(_writeBuffer.begin(), 
                                _writeBuffer.begin() + bytesWritten);
                if (_writeBuffer.empty()) {
                    _state = WRITING_BODY;
                }
            }
        }
        return true;
    }
    
    if (_state == WRITING_BODY) {
        // If buffer is empty, get next chunk
        if (_writeBuffer.empty()) {
            std::vector<char> chunk = _currentResponse.getNextChunk();
            if (chunk.empty()) {
                _state = WRITING_COMPLETE;
                return true;
            }
            _writeBuffer = chunk;
        }
        
        // Write current buffer
        ssize_t bytesWritten = ::send(getFd(), &_writeBuffer[0], 
                                    _writeBuffer.size(), MSG_NOSIGNAL);
        if (bytesWritten > 0) {
            _writeBuffer.erase(_writeBuffer.begin(), 
                            _writeBuffer.begin() + bytesWritten);
        }
    }
    
    return true;
}

bool	Connection::wantsToRead() const {
	// We want to read if we're in any reading state and haven't completed the request
	return ((_state == PENDING_REQUEST
			|| _state == READING_HEADERS
			|| _state == READING_BODY)
			&& !hasCompletedRequest());
}

bool	Connection::wantsToWrite() const {
	// We want to write if we have data in our write buffer
	// or if we're in a writing state
	return (!_writeBuffer.empty()
			|| _state == WRITING_HEADERS
			|| _state == WRITING_BODY);
}

bool Connection::hasCompletedRequest() const
{
	if (_currentRequest.getState() == HTTPRequest::COMPLETE)
	{
		LOG_DEBUG("Reaquest complete!");
	}
	else
		LOG_DEBUG("Request not complete!");
    return _currentRequest.getState() == HTTPRequest::COMPLETE;
}

bool Connection::shouldKeepAlive() const
{
	return _currentRequest.shouldKeepAlive();
}

bool Connection::hasCompletedResponse() const
{
    return _writeBuffer.empty();
}

void Connection::queueResponse(const HTTPResponse& response)
{
    std::string serialized = response.serialize();
    _writeBuffer.insert(_writeBuffer.end(), 
                       serialized.begin(), 
                       serialized.end());
}

void Connection::reset()
{
    _readBuffer.clear();
    _writeBuffer.clear();
    _currentRequest.reset();
	_currentResponse.reset();
}

Connection::State	Connection::getState() const
{
	return _state;
}


int Connection::getFd() const
{
    return _socket->getFd();
}

const std::string& Connection::getIP() const
{
	return _socket->getIP();
}

uint16_t Connection::getPort() const
{
	return _socket->getPort();
}

std::string Connection::getSocketInfoString() const
{
	return _socket->toString();
}

HTTPRequest& Connection::getCurrentRequest()
{
	return _currentRequest;
}

