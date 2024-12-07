/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIPipe.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/30 19:32:14 by lwoiton           #+#    #+#             */
/*   Updated: 2024/12/01 16:05:59 by lwoiton          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGIPipe.hpp"

CGIPipe::CGIPipe(ClientConnection& parent, bool isReadEnd)
    : IOHandler(-1)
    , _parent(parent)
    , _isReadEnd(isReadEnd)
{
    int pipefd[2];
    if (pipe(pipefd) == -1)
        throw std::runtime_error("Failed to create pipe");
        
    // Set appropriate fd based on pipe end
    _fd = _isReadEnd ? pipefd[0] : pipefd[1];
    
    // Close unused end
    close(_isReadEnd ? pipefd[1] : pipefd[0]);
}

bool CGIPipe::handleRead()
{
    if (!_isReadEnd)
        return true; // Write end doesn't read
        
    char buffer[4096];
    while (true) {
        ssize_t bytesRead = read(_fd, buffer, sizeof(buffer));
        
        if (bytesRead > 0) {
            // Process CGI output
            _parent.handleCGIOutput(buffer, bytesRead);
        }
        else if (bytesRead == 0)
            return false; // EOF
        else
            return true; // Error
    }
}

bool CGIPipe::handleWrite()
{
    if (_isReadEnd)
        return true; // Read end doesn't write

    // Write data to CGI process
    while (!_parent.getCGIInputBuffer().empty())
	{
        ssize_t written = write(_fd, &_parent.getCGIInputBuffer()[0],
                              _parent.getCGIInputBuffer().size());       
        if (written > 0) {
            _parent.consumeCGIInputBuffer(written);
        }
        else if (written == -1)
           return true;
    }
    return true;
}