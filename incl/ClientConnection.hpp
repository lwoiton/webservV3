/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientConnection.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/30 18:38:02 by lwoiton           #+#    #+#             */
/*   Updated: 2024/12/17 15:04:25 by lwoiton          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#ifndef CLIENTCONNECTION_HPP
# define CLIENTCONNECTION_HPP

# include <string>
# include <vector>
# include <cstdint>
# include <sstream>
# include <algorithm>

# include "IOHandler.hpp"
# include "CGIPipe.hpp"
# include "HTTPRequest.hpp"
# include "HTTPResponse.hpp"
# include "Logger.hpp"
# include "TempFile.hpp"
# include "CGIProcessor.hpp"

class ClientConnection : public IOHandler
{
	public:
		// Constructor takes socket fd and client info
		ClientConnection(int fd, const std::string& ip, uint16_t port);
		~ClientConnection();

		// IOHandler interface implementation
		virtual bool handleRead();
		virtual bool handleWrite();
		virtual bool wantsToRead() const;
		virtual bool wantsToWrite() const;
		virtual int getFd() const;

		// Get client info for logging
		const std::string& getIP() const;
		uint16_t getPort() const;
		std::string getInfo() const;

	private:
		enum State {
			READING_REQUEST,
			PROCESSING_CGI,
			SENDING_RESPONSE,
		};
		// Socket info
		int             _fd;
		std::string     _clientIP;
		uint16_t        _clientPort;

		// State management
		State           _state;
		bool            _keepAlive;
		
		// Buffers
		std::vector<char> _readBuffer;
		std::vector<char> _writeBuffer;

		// Request processing state
		HTTPRequest	 	_request;
		size_t          _contentLength;
		size_t          _bytesRead;
		bool            _chunkedTransfer;

		// Response processing state
		HTTPResponse	_response;
		size_t          _bytesWritten;
		
		// CGI handling
		struct CGI
		{
			CGIPipe*			inputPipe;
			CGIPipe*			outputPipe;
			
			pid_t				childPid;
			std::vector<char>	inputBuffer;
			std::vector<char>	outputBuffer;
		} 				_cgi;
		void	setupCGI();
		
		// Helper methods
		bool	handleClientRead();
		bool	handleCGIRead();
		void	reset();

		// Prevent copying
		ClientConnection(const ClientConnection&);
		ClientConnection& operator=(const ClientConnection&);
};

#endif // CLIENTCONNECTION_HPP