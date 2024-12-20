/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIPipe.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/30 18:39:58 by lwoiton           #+#    #+#             */
/*   Updated: 2024/12/20 12:13:48 by lwoiton          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// CGIPipe.hpp
#pragma once
#ifndef CGIPIPE_HPP
#define CGIPIPE_HPP

#include "IOHandler.hpp"
#include "ClientConnection.hpp"

class ClientConnection; // Forward declaration

class CGIPipe : public IOHandler {
public:
    // Constructor takes parent and specifies if this is read end
    CGIPipe(ClientConnection& parent, bool isReadEnd);
    ~CGIPipe();

    // IOHandler interface
    virtual bool handleRead() override;
    virtual bool handleWrite() override;
    virtual bool wantsToRead() const override;
    virtual bool wantsToWrite() const override;
    virtual int getFd() const override;

    // CGI-specific functions
    static int createPipe(bool isReadEnd, int *_fd, int *_childFd);
    bool isClosed() const { return _closed; }

private:
    int _fd;       // File descriptor for webserv end of pipe
	int _childFd;  // File descriptor towards the CGI process
    ClientConnection& _parent;
    bool _isReadEnd;
    bool _closed;

	// Helper function to close the pipe
	void closePipe();
    
    // No copying
    CGIPipe(const CGIPipe&);
    CGIPipe& operator=(const CGIPipe&);
};

#endif // CGIPIPE_HPP