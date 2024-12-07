/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CSocket.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/25 10:00:00 by your_login        #+#    #+#             */
/*   Updated: 2024/11/13 12:41:39 by lwoiton          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CSocket.hpp"

// Constructor takes ownership of file descriptor and extracts client info
CSocket::CSocket(int clientFd, const struct sockaddr_in& clientAddr)
    : Socket()  // Initialize base class
    , _ip()     // Empty string initialization
    , _port(0)  // Initialize port to 0
{
    if (clientFd < 0)
        throw std::runtime_error("Invalid client socket descriptor");
        
    _fd = clientFd;  // Set the base class fd
    
    // Convert IP address to string representation
    char ipBuffer[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &(clientAddr.sin_addr), ipBuffer, INET_ADDRSTRLEN) == NULL)
    {
        // If conversion fails, close socket and throw
        ::close(_fd);
        _fd = -1;
        throw std::runtime_error(std::string("Failed to convert IP address: ") 
                               + strerror(errno));
    }
    
    _ip = ipBuffer;
    _port = ntohs(clientAddr.sin_port);
    
    try {
        setNonBlocking(true);  // Inherited from Socket
    }
    catch (const std::exception& e) {
        ::close(_fd);
        _fd = -1;
        throw;  // Re-throw the exception after cleanup
    }
}

CSocket::~CSocket()
{
}

const std::string& CSocket::getIP() const
{
	return _ip;
}

uint16_t CSocket::getPort() const
{
	return _port;
}

// Returns a string representation of the socket for logging/debugging
std::string CSocket::toString() const
{
    std::stringstream ss;
	ss << _ip << ":" << _port << " -> socket " << _fd << " (O_NONBLOCK)";
    return ss.str();
}