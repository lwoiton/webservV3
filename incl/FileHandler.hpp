/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileHandler.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/20 18:36:47 by lwoiton           #+#    #+#             */
/*   Updated: 2024/12/20 18:37:05 by lwoiton          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// FileHandler.hpp
#pragma once
#ifndef FILEHANDLER_HPP
#define FILEHANDLER_HPP

#include "IOHandler.hpp"
#include "ClientConnection.hpp"
#include "HTTPResponse.hpp"

class ClientConnection;  // Forward declaration

class FileHandler : public IOHandler {
public:
    FileHandler(ClientConnection& parent, const std::string& path);
    ~FileHandler();

    // IOHandler interface
    virtual bool handleRead();
    virtual bool handleWrite();
    virtual bool wantsToRead() const;
    virtual bool wantsToWrite() const;
    virtual int getFd() const;

private:
    ClientConnection& _parent;
    size_t _bytesRead;
    size_t _fileSize;
    static const size_t BUFFER_SIZE = 8192;

    // Prevent copying
    FileHandler(const FileHandler&);
    FileHandler& operator=(const FileHandler&);
};

#endif