/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/22 21:25:27 by lwoiton           #+#    #+#             */
/*   Updated: 2024/10/24 16:29:35 by lwoiton          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#ifndef SOCKET_HPP
# define SOCKET_HPP

#include "Logger.hpp"

#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdexcept>

class Socket
{
	public:
			Socket();
			~Socket();
	
	void	setNonBlocking(bool nonBlocking);
	void	close();
	int		getFd() const;
	protected:
		int		_fd;
		void	create();
		void	setSocketOptions(int option, int value);
};

#endif // SOCKET_HPP