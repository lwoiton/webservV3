/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LSocket.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/22 21:58:14 by lwoiton           #+#    #+#             */
/*   Updated: 2024/11/30 18:32:28 by lwoiton          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#ifndef LSOCKET_HPP
# define LSOCKET_HPP

# include "Socket.hpp"
# include "CSocket.hpp"
# include <string>
# include <netdb.h>
# include <string.h>
# include <sstream>
# include <stdexcept>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>

class LSocket : public Socket
{
    public:
        LSocket();
        ~LSocket();

        void    	setup(const std::string &host, int port);
        void    	startListen(int backlog = SOMAXCONN);  // Renamed from listen
        CSocket*    acceptClient();

    private:
        static const uint16_t    MIN_PORT = 1024;
        static const uint16_t    MAX_PORT = 65535;
        struct sockaddr_in       _addr;
        
        bool    isValidPort(int port) const;
};

#endif // LSOCKET_HPP