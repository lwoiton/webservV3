/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ListeningSocket.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/30 19:15:09 by lwoiton           #+#    #+#             */
/*   Updated: 2024/12/03 00:59:12 by lwoiton          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ListeningSocket.hpp"

ListeningSocket::ListeningSocket(const std::string& host, int port)
    : IOHandler(_fd)  // Note: _fd will be initialized in setupSocket()
    , _host(host)
    , _port(port)
    , _fd(-1)
{
    setupSocket();
}

ListeningSocket::~ListeningSocket()
{
    if (_fd != -1)
        close(_fd);
}

void ListeningSocket::setupSocket()
{
    // Create socket
    _fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_fd == -1)
        throw std::runtime_error("Failed to create socket");

    try {
        setSocketOptions();
        bindSocket();
        startListening();
    }
    catch (const std::exception& e) {
        if (_fd != -1)
            close(_fd);
        throw;
    }
}

void ListeningSocket::setSocketOptions()
{
    // Set socket options
    int opt = 1;
    if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
        throw std::runtime_error("setsockopt failed");

	// Set non-blocking should be done in IOHandler constructor
}

void ListeningSocket::bindSocket()
{
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(_port);
    
    if (_host.empty() || _host == "0.0.0.0")
        addr.sin_addr.s_addr = INADDR_ANY;
    else if (inet_pton(AF_INET, _host.c_str(), &addr.sin_addr) <= 0)
        throw std::runtime_error("Invalid host address");

    if (bind(_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
        throw std::runtime_error("bind failed");
}

void ListeningSocket::startListening()
{
    if (listen(_fd, SOMAXCONN) == -1)
        throw std::runtime_error("listen failed");
}

bool ListeningSocket::handleRead()
{
    struct sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);
    
	int clientFd = accept(_fd, (struct sockaddr*)&clientAddr, &addrLen);
	
	if (clientFd == -1) {
		return true;  // Accept failed, try again later
	}

	try {
		char clientIP[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
		uint16_t clientPort = ntohs(clientAddr.sin_port);
		
		ClientConnection* client = new ClientConnection(clientFd, clientIP, clientPort);
		EventLoop::getInstance()->registerHandler(client);
		return true;
	}
	catch (const std::exception& e) {
		close(clientFd);
		return false;
	}
}

bool ListeningSocket::handleWrite()
{
    return true;  // Listening socket never writes
}

bool ListeningSocket::wantsToRead() const
{
    return true;  // Always want to accept new connections
}

bool ListeningSocket::wantsToWrite() const
{
    return false;  // Never writes
}

int ListeningSocket::getFd() const
{
    return _fd;
}

std::string ListeningSocket::getInfo() const
{
    std::stringstream ss;
    ss << _host << ":" << _port;
    return ss.str();
}