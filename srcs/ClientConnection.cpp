/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientConnection.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/30 19:46:31 by lwoiton           #+#    #+#             */
/*   Updated: 2024/12/12 00:23:11 by lwoiton          ###   ########.fr       */
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
	if (_request.shouldKeepAlive())
		_keepAlive = true;
	if (_state == PROCESSING_CGI)
		return handleCGIRead();
	return handleClientRead();
}

bool	ClientConnection::handleClientRead()
{
	char buffer[4096];
    
	ssize_t bytesRead = read(_fd, buffer, sizeof(buffer));
	if (bytesRead > 0)
	{
		_readBuffer.insert(_readBuffer.end(), buffer, buffer + bytesRead);
		try
		{
			_request.parse(_readBuffer);
			if (_request.getState() == HTTPRequest::COMPLETE)
			{
				if (_request.isCGI())
				{
					_state = PROCESSING_CGI;
					setupCGI();
				}
				else
				{
					// Process request
					_state = SENDING_RESPONSE;
				}
			}
		}
		catch(const std::exception& e)
		{
			std::cerr << e.what() << '\n';
		}
		
	}
	else if (bytesRead == 0)
	{
		// Client closed connection
		return false;
	}
    return true;
}

bool	ClientConnection::handleCGIRead()
{
	if (_cgiPipe->isReadEnd())
	{
		char buffer[4096];
		ssize_t bytesRead = read(_cgiPipe->getFd(), buffer, sizeof(buffer));
		if (bytesRead > 0)
		{
			_cgiPipe->write(buffer, bytesRead);
		}
		else if (bytesRead == 0)
		{
			_cgiPipe->closeWrite();
		}
		else if (errno != EAGAIN)
		{
			// Error reading from CGI pipe
			return false;
		}
	}
	return true;
}

bool	ClientConnection::setupCGI()
{
	_state = PROCESSING_CGI;

	// Create pipes for CGI process
	_cgi.inputPipe = new CGIPipe(*this, false);
	_cgi.outputPipe = new CGIPipe(*this, true);

	// Process CGI request through new CGIProcessor
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
				return true; // Keep Connection open
			}
			return false;  // Close connection
		}
	}
	return true;
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

void ClientConnection::reset()
{
    _state = READING_REQUEST;
    _contentLength = 0;
    _bytesRead = 0;
    _chunkedTransfer = false;
    _readBuffer.clear();
    _writeBuffer.clear();
	if (_cgi.inputPipe)
	{
		delete _cgi.inputPipe;
		_cgi.inputPipe = NULL;
	}
	if (_cgi.outputPipe)
	{
		delete _cgi.outputPipe;
		_cgi.outputPipe = NULL;
	}
}