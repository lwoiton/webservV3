/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileHandler.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/20 17:57:20 by lwoiton           #+#    #+#             */
/*   Updated: 2024/12/20 20:05:03 by lwoiton          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// FileHandler.cpp
#include "FileHandler.hpp"
#include <sys/stat.h>
#include <fcntl.h>

FileHandler::FileHandler(ClientConnection& parent, const std::string& path)
    : IOHandler(open(path.c_str(), O_RDONLY | O_NONBLOCK))
    , _parent(parent)
    , _bytesRead(0)
{
    struct stat st;
    if (stat(path.c_str(), &st) == -1)
        throw HTTPError(404, "File not found");
    
    _fileSize = st.st_size;
    _parent.getResponse().setHeader("Content-Length", toString(_fileSize));
}

FileHandler::~FileHandler() {}

bool FileHandler::handleRead()
{
    char buffer[BUFFER_SIZE];
    ssize_t bytesRead = read(getFd(), buffer, BUFFER_SIZE);
    
    if (bytesRead > 0) {
        _bytesRead += bytesRead;
        _parent.getResponse().appendToBody(buffer, bytesRead);
        
        if (_bytesRead >= _fileSize) {
            _parent.queueResponse();
            return false;  // Done reading - remove from event loop
        }
        return true;
    }
    else if (bytesRead == 0) {  // EOF
        _parent.queueResponse();
        return false;
    }
    
    return errno == EAGAIN || errno == EWOULDBLOCK;
}

bool FileHandler::handleWrite() {
    return true;  // We only read from files
}

bool FileHandler::wantsToRead() const {
    return _bytesRead < _fileSize;
}

bool FileHandler::wantsToWrite() const {
    return false;
}

int FileHandler::getFd() const {
    return IOHandler::getFd();
}
