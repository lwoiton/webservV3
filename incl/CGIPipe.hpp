/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIPipe.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/30 18:39:58 by lwoiton           #+#    #+#             */
/*   Updated: 2024/12/12 01:08:23 by lwoiton          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#ifndef CGIPIPE_HPP
# define CGIPIPE_HPP

# include <string>
# include <vector>

# include "IOHandler.hpp"
# include "ClientConnection.hpp"


class ClientConnection;  // Forward declaration

class CGIPipe : public IOHandler
{
	public:
		// Constructor takes a reference to parent and specifies if this is read or write end
		CGIPipe(ClientConnection& parent, bool isReadEnd);
		~CGIPipe();
		
		static int createPipe(bool isReadEnd);

		// IOHandler interface
		virtual bool handleRead();
		virtual bool handleWrite();
		virtual bool wantsToRead() const;
		virtual bool wantsToWrite() const;
		virtual int getFd() const;
		
	private:
		int _fd; // The file descriptor for webserv to read/write
		int _otherEnd; // The other end of the pipe towards the CGI process
		ClientConnection& _parent;
		bool _isReadEnd;
		std::vector<char> _buffer;
		bool _closed;

		// Prevent copying
		CGIPipe(const CGIPipe&);
		CGIPipe& operator=(const CGIPipe&);
};

#endif // CGIPIPE_HPP