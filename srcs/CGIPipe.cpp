/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIPipe.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/30 19:32:14 by lwoiton           #+#    #+#             */
/*   Updated: 2024/12/18 16:23:34 by lwoiton          ###   ########.fr       */
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
	if (isReadEnd) {
        *_fd = fds[0];       // Read end
        *_childFd = fds[1]; // Write end
	} else {
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
    if (!_closed && _fd != -1) {
        close(_fd);
        _fd = -1;
        _closed = true;
    }
}

bool CGIPipe::handleRead()
{
    if (!_isReadEnd || _closed)
        return true;

    char buffer[4096];
    ssize_t bytesRead = read(_fd, buffer, sizeof(buffer));
    
    if (bytesRead > 0) {
        // Add to parent's response body
        _parent.appendResponseBody(buffer, bytesRead);
        return true;
    }
    else if (bytesRead == 0) {
        // EOF - close pipe
        _closed = true;
        close(_fd);
        _fd = -1;
        _parent.handleCGIComplete();
        return true;
    }
    
    return true;
}

bool CGIPipe::handleWrite()
{
    if (_isReadEnd || _closed)
        return true;

    // Get data from parent's request body at current offset
    const std::vector<char>& body = _parent.getRequestBody();
    size_t offset = _parent.getCGIWriteOffset();
    size_t remaining = body.size() - offset;

    if (remaining > 0) {
        ssize_t written = write(_fd, body.data() + offset, remaining);
        if (written > 0) {
            _parent.advanceCGIWriteOffset(written);
            if (offset + written == body.size()) {
                // Done writing - close pipe
                _closed = true;
                close(_fd);
                _fd = -1;
                _parent.handleCGIInputComplete();
            }
        }
    }

    return true;
}

bool CGIPipe::wantsToRead() const
{
    return _isReadEnd && !_closed;
}

bool CGIPipe::wantsToWrite() const
{
    if (_isReadEnd || _closed ||)
        return false;
    
    const std::vector<char>& body = _parent.getRequestBody();
    return _parent.getCGIWriteOffset() < body.size();
}

int CGIPipe::getFd() const
{
    return _fd;
}