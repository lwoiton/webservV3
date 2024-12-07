/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ListeningSocket.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/30 18:32:58 by lwoiton           #+#    #+#             */
/*   Updated: 2024/12/03 00:38:46 by lwoiton          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#ifndef LISTENINGSOCKET_HPP
# define LISTENINGSOCKET_HPP

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string.h>
#include <sstream>

# include "IOHandler.hpp"
# include "ClientConnection.hpp"
# include "EventLoop.hpp"

class ListeningSocket : public IOHandler 
{
	public:
		// Constructor that takes configuration
		ListeningSocket(const std::string& host, int port);
		~ListeningSocket();

		// IOHandler interface implementation
		virtual bool handleRead();
		virtual bool handleWrite();
		virtual bool wantsToRead() const;
		virtual bool wantsToWrite() const;
		virtual int getFd() const;

		// Get socket info for logging
		std::string getInfo() const;

	private:
		std::string _host;
		int         _port;
		int         _fd;

		// Prevent copying
		ListeningSocket(const ListeningSocket&);
		ListeningSocket& operator=(const ListeningSocket&);

		// Helper methods
		void setupSocket();
		void setSocketOptions();
		void bindSocket();
		void startListening();
};

#endif // LISTENINGSOCKET_HPP