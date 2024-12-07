/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TempFile.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/30 19:26:59 by lwoiton           #+#    #+#             */
/*   Updated: 2024/11/30 20:04:14 by lwoiton          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// TempFile.cpp
#include "TempFile.hpp"

size_t TempFile::_counter = 0;

TempFile::TempFile(ClientConnection& parent)
    : IOHandler(-1)
    , _parent(parent)
    , _size(0)
    , _readPos(0)
{
    _path = generatePath();
    
    // Open file with proper permissions
    _fd = open(_path.c_str(), O_RDWR | O_CREAT | O_EXCL, 0600);
    if (_fd == -1)
        throw std::runtime_error("Failed to create temp file");

    try {
        setNonBlocking(_fd);
    }
    catch (const std::exception& e) {
        cleanup();
        throw;
    }
}

TempFile::~TempFile()
{
    cleanup();
}

void TempFile::cleanup()
{
    if (_fd != -1) {
        close(_fd);
        _fd = -1;
    }
    if (!_path.empty()) {
        remove(_path.c_str());
        _path.clear();
    }
}

std::string TempFile::generatePath() const
{
    std::stringstream ss;
    ss << "/tmp/webserv_" << getpid() << "_" << ++_counter;
    return ss.str();
}

bool TempFile::handleRead()
{
    // Reading from temp file is only done when sending response
    char buffer[4096];
    
    while (true)
    {
        ssize_t bytesRead = pread(_fd, buffer, sizeof(buffer), _readPos);
        
        if (bytesRead > 0)
        {
            _readPos += bytesRead;
            // Handle the read data (probably send to client)
            // This depends on how your ClientConnection processes responses
        }
        else if (bytesRead == 0)
        {
            // End of file
            return true;
        }
        else
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return true;
            return false;
        }
    }
}

bool TempFile::handleWrite()
{
    // Writing to temp file is usually done when receiving request body
    // This depends on how your ClientConnection provides data
    return true;
}

bool TempFile::wantsToRead() const
{
    // Only want to read when sending response and not at end of file
    return _readPos < _size;
}

bool TempFile::wantsToWrite() const
{
    // This depends on your ClientConnection implementation
    return false;
}

int TempFile::getFd() const
{
    return _fd;
}

size_t TempFile::write(const char* data, size_t len)
{
    if (_fd == -1)
        return 0;

    ssize_t written = ::write(_fd, data, len);
    if (written > 0)
    {
        _size += written;
        return static_cast<size_t>(written);
    }
    return 0;
}

size_t TempFile::read(char* buffer, size_t len)
{
    if (_fd == -1 || _readPos >= _size)
        return 0;

    ssize_t bytesRead = pread(_fd, buffer, len, _readPos);
    if (bytesRead > 0)
    {
        _readPos += bytesRead;
        return static_cast<size_t>(bytesRead);
    }
    return 0;
}

void TempFile::rewind()
{
    _readPos = 0;
}

size_t TempFile::size() const
{
    return _size;
}