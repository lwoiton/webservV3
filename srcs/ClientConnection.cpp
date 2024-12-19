/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientConnection.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/30 19:46:31 by lwoiton           #+#    #+#             */
/*   Updated: 2024/12/18 17:51:12 by lwoiton          ###   ########.fr       */
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
				if (!_request.shouldKeepAlive())
					_keepAlive = false;
				if (_request.isCGI())
					setupCGI();
				else
				{
					//Process request
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
		// Client closed connection -> EventLoop will handle removal of the epoll instance and the client instance through IOHnadler by returning false here.
		return false;
	}
	return true;	
}

void	ClientConnection::setupCGI()
{
	_state = PROCESSING_CGI;

	// Create pipes for CGI process
	_cgi.inputPipe = new CGIPipe(*this, false);
	_cgi.outputPipe = new CGIPipe(*this, true);

	// Process CGI request through new CGIProcessor
	_cgi.childPid = fork();
	if (_cgi.childPid == 0)
	{
		//child Process
		//setup environment , redirect stdin/stdout
		//execve
		exit(1);
	}
	else if (_cgi.childPid > 0)
	{
		//parent process
		//close unused pipe ends
		//set up IOHandlers for pipes
		//set up signal handler for SIGCHLD
		_state = PROCESSING_CGI;
	}
	else
	{
		throw HTTPError(500, "Failed to fork CGI process");
	}
	
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
		
		if (_writeBuffer.empty() && _state == SENDING_RESPONSE)
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
    return _state == READING_REQUEST && _request.getState() != HTTPRequest::COMPLETE;
}

bool ClientConnection::wantsToWrite() const
{
    return _state == SENDING_RESPONSE && _response.getState() != HTTPResponse::COMPLETE;
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