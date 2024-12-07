/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LSocket.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/22 21:58:14 by lwoiton           #+#    #+#             */
/*   Updated: 2024/11/13 12:48:48 by lwoiton          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "LSocket.hpp"

LSocket::LSocket() : Socket()
{
    ::memset(&_addr, 0, sizeof(_addr));
}

LSocket::~LSocket()
{
    // Socket base class destructor will handle cleanup
}

void LSocket::setup(const std::string &host, int port)
{
    if (!isValidPort(port))
    {
        std::stringstream ss;
        ss << "Invalid port number: " << port;
        throw std::runtime_error(ss.str());
    }

    create();

    int opt = 1;
    setSocketOptions(SO_REUSEADDR, opt);

    ::memset(&_addr, 0, sizeof(_addr));
    _addr.sin_family = AF_INET;
    _addr.sin_port = htons(port);

    if (host == "0.0.0.0" || host.empty())
	    _addr.sin_addr.s_addr = INADDR_ANY;
    else if (::inet_pton(AF_INET, host.c_str(), &_addr.sin_addr) <= 0)
        throw std::runtime_error("Invalid address");

    if (::bind(_fd, (struct sockaddr*)&_addr, sizeof(_addr)) < 0)
        throw std::runtime_error(std::string("Bind failed: ") + strerror(errno));

    setNonBlocking(true);
}

void LSocket::startListen(int backlog)
{
    if (_fd == -1)
        throw std::runtime_error("Socket not created");

    if (::listen(_fd, backlog) < 0)
        throw std::runtime_error(std::string("Listen failed: ") + strerror(errno));
}

CSocket* LSocket::acceptClient()
{
    if (_fd == -1)
        throw std::runtime_error("Socket not created");

    struct sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);

    int clientFd = ::accept(_fd, (struct sockaddr*)&clientAddr, &addrLen);
    
    if (clientFd < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return NULL;  // No pending connections
        throw std::runtime_error(std::string("Accept failed: ") + strerror(errno));
    }

    try {
        return new CSocket(clientFd, clientAddr);
    }
    catch (const std::exception& e) {
        ::close(clientFd);  // Clean up on error
        throw;
    }
}

bool LSocket::isValidPort(int port) const
{
    return port >= MIN_PORT && port <= MAX_PORT;
}