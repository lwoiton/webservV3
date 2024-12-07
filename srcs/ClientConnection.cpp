/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientConnection.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/30 19:46:31 by lwoiton           #+#    #+#             */
/*   Updated: 2024/12/02 18:27:43 by lwoiton          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// ClientConnection.cpp
#include "ClientConnection.hpp"

ClientConnection::ClientConnection(int fd, const std::string& ip, uint16_t port)
    : IOHandler(fd)
    , _fd(fd)
    , _clientIP(ip)
    , _clientPort(port)
    , _state(READING_REQUEST)
    , _keepAlive(true)
    , _contentLength(0)
    , _bytesRead(0)
    , _chunkedTransfer(false)
{
}

ClientConnection::~ClientConnection()
{
    if (_fd != -1)
        close(_fd);
}

bool ClientConnection::handleRead()
{
	if (_state == PROCESSING_CGI)
		return handleCGIRead();
	return handleClientRead();
}

bool	ClientConnection::handleCGIRead()
{
	return _cgiOutputPipe->handleRead();
}

bool	ClientConnection::handleClientRead()
{
	char buffer[4096];
    
	ssize_t bytesRead = read(_fd, buffer, sizeof(buffer));
	if (bytesRead > 0)
	{
		_readBuffer.insert(_readBuffer.end(), buffer, buffer + bytesRead);
	
		// Process based on current state
		switch (_state)
		{
			case READING_HEADERS:
				if (processHeaders())
					return true;
				break;
				
			case READING_BODY:
				if (_chunkedTransfer)
				{
					if (processChunkedBody())
						return true;
				}
				else
				{
					if (processBody())
						return true;
				}
				break;
			default:
				break;
		}
	}
	else if (bytesRead == 0)
	{
		// Client closed connection
		return false;
	}
    return true;
}

bool ClientConnection::handleWrite()
{
	if (_writeBuffer.empty())
		return true;
	ssize_t bytesWritten = write(_fd, &_writeBuffer[0], _writeBuffer.size());
	
	if (bytesWritten > 0)
	{
		_writeBuffer.erase(_writeBuffer.begin(), 
							_writeBuffer.begin() + bytesWritten);
		
		if (_writeBuffer.empty() && _state == WRITING_RESPONSE)
		{
			if (_keepAlive)
			{
				reset();
				return true;
			}
			return false;  // Close connection
		}
	}
}

bool ClientConnection::wantsToRead() const
{
    return _state == READING_HEADERS || _state == READING_BODY;
}

bool ClientConnection::wantsToWrite() const
{
    return !_writeBuffer.empty() || _state == WRITING_RESPONSE;
}

int ClientConnection::getFd() const
{
    return _fd;
}

const std::string& ClientConnection::getIP() const
{
    return _clientIP;
}

uint16_t ClientConnection::getPort() const
{
    return _clientPort;
}

std::string ClientConnection::getInfo() const
{
    std::stringstream ss;
    ss << _clientIP << ":" << _clientPort;
    return ss.str();
}

bool ClientConnection::processHeaders()
{
    // Look for end of headers (CRLFCRLF)
    std::vector<char>::iterator it = std::search(
        _readBuffer.begin(), _readBuffer.end(),
        "\r\n\r\n", "\r\n\r\n" + 4);

    if (it != _readBuffer.end())
    {
        // Found end of headers
        size_t headerLength = it - _readBuffer.begin() + 4;
        
        // Process headers here...
        // Set _contentLength, _chunkedTransfer, _keepAlive based on headers
        
        // Remove processed headers from buffer
        _readBuffer.erase(_readBuffer.begin(), _readBuffer.begin() + headerLength);
        
        // Determine next state
        if (_contentLength > 0 || _chunkedTransfer)
        {
            _state = READING_BODY;
            return processBody();  // Process any remaining data
        }
        else
        {
            _state = PROCESSING;
            return true;
        }
    }
    return true;  // Need more data
}

bool ClientConnection::processBody()
{
    if (!_chunkedTransfer)
    {
        _bytesRead += _readBuffer.size();
        
        if (_bytesRead >= _contentLength)
        {
            _state = PROCESSING;
            return true;
        }
    }
    return true;
}

bool ClientConnection::processChunkedBody()
{
    // Chunked transfer handling...
    // This will need proper implementation according to RFC 7230
    return true;
}

void ClientConnection::reset()
{
    _state = READING_HEADERS;
    _contentLength = 0;
    _bytesRead = 0;
    _chunkedTransfer = false;
    _readBuffer.clear();
    _writeBuffer.clear();
}