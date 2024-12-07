/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/22 22:23:36 by lwoiton           #+#    #+#             */
/*   Updated: 2024/11/29 22:36:09 by lwoiton          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#ifndef CONNECTION_HPP
# define CONNECTION_HPP

# include <vector>
# include <string>
# include <stdint.h>

# include "IOHandler.hpp"
# include "CSocket.hpp"
# include "HTTPRequest.hpp"
# include "HTTPResponse.hpp"

class Connection : public IOHandler
{
    public:
		enum State
			{
				PENDING_REQUEST,
				READING_HEADERS,
				READING_BODY,
				WRITING_HEADERS,
				WRITING_BODY,
				FILE_OPERATION_PENDING,
				WRITING_COMPLETE
			};
        explicit					Connection(CSocket *socket);
                                    ~Connection();
		
    	virtual bool       			handleRead();
        virtual bool       			handleWrite();
		virtual bool				wantsToRead() const;
		virtual bool				wantsToWrite() const;
        bool                        hasCompletedRequest() const;
        bool                        hasCompletedResponse() const;
        void                        queueResponse(const HTTPResponse& response);
        int                  		getFd() const;
		const std::string&			getIP() const;
		uint16_t					getPort() const;
		std::string					getSocketInfoString() const;
		State						getState() const;
		HTTPRequest& 				getCurrentRequest();
		bool						shouldKeepAlive() const;
		void						reset();
    private:
		CSocket						*_socket;
		static const size_t			BUFFER_SIZE = 4096;
		State						_state;
        std::vector<char>			_readBuffer;
        std::vector<char>			_writeBuffer;
        HTTPRequest					_currentRequest;
		HTTPResponse				_currentResponse;

									Connection(const Connection&);
        Connection&					operator=(const Connection&);
};

#endif // CONNECTION_HPP