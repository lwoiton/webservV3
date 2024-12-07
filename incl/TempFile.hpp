/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TempFile.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/21 17:07:53 by lwoiton           #+#    #+#             */
/*   Updated: 2024/11/30 20:04:36 by lwoiton          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#ifndef TEMPFILE_HPP
#define TEMPFILE_HPP

#include <string>
#include <fcntl.h>
#include <sstream>
#include <sys/stat.h>
#include <errno.h>

#include "IOHandler.hpp"
#include "ClientConnection.hpp"  // Forward declaration

class ClientConnection;  // Forward declaration

class TempFile : public IOHandler
{
public:
    explicit TempFile(ClientConnection& parent);
    ~TempFile();

    // IOHandler interface implementation
    virtual bool handleRead();
    virtual bool handleWrite();
    virtual bool wantsToRead() const;
    virtual bool wantsToWrite() const;
    virtual int getFd() const;

    // File operations
    size_t write(const char* data, size_t len);
    size_t read(char* buffer, size_t len);
    void rewind();  // Reset read position to start
    size_t size() const;

private:
    int _fd;
    ClientConnection& _parent;
    std::string _path;
    size_t _size;
    size_t _readPos;
    static size_t _counter;  // For generating unique names

    std::string generatePath() const;
    void cleanup();

    // Prevent copying
    TempFile(const TempFile&);
    TempFile& operator=(const TempFile&);
};

#endif // TEMPFILE_HPP