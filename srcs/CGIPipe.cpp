/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIPipe.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/30 19:32:14 by lwoiton           #+#    #+#             */
/*   Updated: 2024/12/12 10:32:29 by lwoiton          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGIPipe.hpp"

CGIPipe::CGIPipe(ClientConnection& parent, bool isReadEnd)
    : IOHandler(createPipe(isReadEnd, &_otherEnd))
    , _parent(parent)
    , _isReadEnd(isReadEnd)
	, _buffer()
	, _closed(false)
{
	if (_otherEnd != -1)
	{
		close(_otherEnd);
		_otherEnd = -1;
	}
}

CGIPipe::~CGIPipe()
{
	close(_fd);
}

bool CGIPipe::handleRead()
{
    if (!_isReadEnd)
        return true; // Write end doesn't read
        
    char buf[4096];
	ssize_t bytesRead = read(getFd(), buf, sizeof(buf));
	
	if (bytesRead > 0)
	{
		_buffer.insert(_buffer.end(), buf, buf + bytesRead);
		return true;
	}
	else if (bytesRead == 0)
		return false; // EOF
	else
		return true; // Error
}

bool CGIPipe::handleWrite()
{
    if (_isReadEnd || _buffer.empty())
        return true; // Read end doesn't write

	ssize_t written = write(getFd(), &_buffer[0], _buffer.size());      
	if (written > 0)
		_buffer.erase(_buffer.begin(), _buffer.begin() + written);
	return true;
}

bool CGIPipe::wantsToRead() const
{
	return _isReadEnd && !_closed;
}

bool CGIPipe::wantsToWrite() const
{
	return !_isReadEnd && !_buffer.empty() && !_closed;
}

int CGIPipe::getFd() const
{
	return _fd;
}