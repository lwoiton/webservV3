/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CSocket.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/25 14:33:01 by lwoiton           #+#    #+#             */
/*   Updated: 2024/10/25 16:52:17 by lwoiton          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#ifndef CSOCKET_HPP
# define CSOCKET_HPP

#include <arpa/inet.h>

# include "Socket.hpp"

class CSocket : public Socket
{
	public:
		explicit			CSocket(int fd, const struct sockaddr_in& addr);
							~CSocket();
		
		const std::string&	getIP() const;
		uint16_t			getPort() const;
		std::string 		toString() const;
	private:
		std::string			_ip;
		uint16_t			_port;
};


#endif	// CSOCKET_HPP