/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIPipe.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/30 19:32:14 by lwoiton           #+#    #+#             */
/*   Updated: 2024/12/20 15:19:46 by lwoiton          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// CGIPipe.cpp
#include "CGIPipe.hpp"

CGIPipe::CGIPipe(ClientConnection& parent, bool isReadEnd) //ReadEnd aka OutputPipe
    : IOHandler(createPipe(isReadEnd, &_fd, &_childFd))  // Will be set after pipe creation
    , _parent(parent)
    , _isReadEnd(isReadEnd)
    , _closed(false)
{
    // IOHandler constructor will handle making fd non-blocking
    // and registering with event loop
}

int CGIPipe::createPipe(bool isReadEnd, int *_fd, int *_childFd)
{
	int fds[2];
	if (pipe(fds) == -1)
		throw std::runtime_error("Failed to create pipe");
	
	// Store the end we want to return in _fd and the other in _otherEnd
	if (isReadEnd) // OutputPipe
	{
        *_fd = fds[0];       // Read end
        *_childFd = fds[1]; // Write end
	}
	else // InputPipe
	{
        *_fd = fds[1];       // Write end
        *_childFd = fds[0]; // Read end
	}

	// The webserver end shoudl be made non blocking by IOHandler and the 
	// other end can be blocking since it is a seperate process which will
	// not interfere with other clients of the webserver
	return *_fd;
}

CGIPipe::~CGIPipe()
{
	closePipe();
}

bool CGIPipe::handleRead()
{
    if (_isReadEnd || _closed)
        return true;

    char buffer[4096];
    ssize_t bytesRead = read(_fd, buffer, sizeof(buffer));
    
    if (bytesRead > 0) {
        // Add to parent's response body
        _parent.getResponse().appendToBody(buffer, bytesRead);
        return true;
    }
    else if (bytesRead == 0) {
        // EOF - close pipe
		closePipe();
		//parent queque the response
        _parent.queueResponse();
        return true;
    }
    return true;
}



bool CGIPipe::handleWrite() //Writing into the child process
{
    if (_isReadEnd || _closed)
        return true;

    // Get data from parent's request body at current offset
    const std::vector<char>& body = _parent.getRequest().getBody();
    size_t offset = _parent.getCGI().writeOffset;
    size_t remaining = body.size() - offset;

    if (remaining > 0)
	{
        ssize_t written = write(_fd, body.data() + offset, remaining);
        if (written > 0)
		{
            _parent.getCGI().writeOffset += written;
            if (offset + written == body.size())
			{
                // Done writing - close pipe
                closePipe();
                
            }
        }
    }

    return true;
}

bool CGIPipe::wantsToRead() const
{
	if (!_isReadEnd || _closed)
		return false;
	return true;
}

bool CGIPipe::wantsToWrite() const
{
    if (_isReadEnd || _closed)
        return false;
    const std::vector<char>& body = _parent.getRequest().getBody();
    return _parent.getCGI().writeOffset < body.size();
}

int CGIPipe::getFd() const
{
    return _fd;
}

void	CGIPipe::closePipe()
{
	if (!_closed && _fd != -1)
	{
		close(_fd);
		_fd = -1;
		_closed = true;
	}
}