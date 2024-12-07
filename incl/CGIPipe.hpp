/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIPipe.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/30 18:39:58 by lwoiton           #+#    #+#             */
/*   Updated: 2024/11/30 20:08:22 by lwoiton          ###   ########.fr       */
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

		// IOHandler interface
		virtual bool handleRead();
		virtual bool handleWrite();
		virtual bool wantsToRead() const;
		virtual bool wantsToWrite() const;
		virtual int getFd() const;

		// Pipe operations
		size_t write(const char* data, size_t len);
		void closeWrite();
		bool isReadEnd() const;
		bool hasData() const;
		
		// Access to buffer for parent connection
		const std::vector<char>& getBuffer() const;
		void clearBuffer();

	private:
		int _fd;
		ClientConnection& _parent;
		bool _isReadEnd;
		std::vector<char> _buffer;
		bool _closed;

		// Prevent copying
		CGIPipe(const CGIPipe&);
		CGIPipe& operator=(const CGIPipe&);
};

#endif // CGIPIPE_HPP