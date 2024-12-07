/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/22 21:25:27 by lwoiton           #+#    #+#             */
/*   Updated: 2024/11/12 23:08:56 by lwoiton          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Socket.hpp"

Socket::Socket() : _fd(-1)
{
}

Socket::~Socket()
{
    if (_fd != -1)
        close();
}

void Socket::create()
{
    if (_fd != -1)
        throw std::runtime_error("Socket already created");
    _fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_fd == -1)
        throw std::runtime_error(std::string("Failed to create socket: ") + strerror(errno));
}

void Socket::setNonBlocking(bool nonBlocking)
{
    if (_fd == -1)
        throw std::runtime_error("Socket not created");
    int flags = fcntl(_fd, F_GETFL, 0);
    if (flags == -1)
        throw std::runtime_error(std::string("Failed to get socket flags: ") + strerror(errno));
    flags = nonBlocking ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK);
    if (fcntl(_fd, F_SETFL, flags) == -1)
        throw std::runtime_error(std::string("Failed to set socket flags: ") + strerror(errno));
}

void Socket::setSocketOptions(int option, int value)
{
    if (_fd == -1)
        throw std::runtime_error("Socket not created");
    if (setsockopt(_fd, SOL_SOCKET, option, &value, sizeof(value)) == -1)
        throw std::runtime_error(std::string("Failed to set socket option: ") + strerror(errno));
}

void Socket::close()
{
    if (_fd != -1)
    {
        if (::close(_fd) == -1)
            throw std::runtime_error(std::string("Failed to close socket: ") + strerror(errno));
        _fd = -1;
    }
}

int Socket::getFd() const
{
    return _fd;
}