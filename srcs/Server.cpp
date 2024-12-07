/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/22 19:33:22 by lwoiton           #+#    #+#             */
/*   Updated: 2024/11/29 22:04:36 by lwoiton          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <sstream>
#include <sys/epoll.h>

Server::Server(const std::string &configPath) 
    : _config(new Config(configPath)) // Parsing config file
    , _epoll(new EpollManager()) // Init Epoll
	, _reqProc(RequestProcessor(_config->getServers()))
    , _isRunning(false)
{
	try
    {
        _setupListeners();
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("Server initialization failed: " + std::string(e.what()));
        throw;
    }
}

Server::~Server()
{
    std::map<int, LSocket*>::iterator lit;
    for (lit = _listenSockets.begin(); lit != _listenSockets.end(); ++lit)
        delete lit->second;

    std::map<int, Connection*>::iterator cit;
    for (cit = _connections.begin(); cit != _connections.end(); ++cit)
        delete cit->second;
}

void Server::_setupListeners()
{
    const std::vector<Config::ServerConfig>& servers = _config->getServers();
    std::vector<Config::ServerConfig>::const_iterator it;
    
    for (it = servers.begin(); it != servers.end(); ++it)
    {
        LSocket* socket = new LSocket();
        try
        {
            socket->setup(it->host, it->port);
            socket->startListen();
            socket->setNonBlocking(true);
            _epoll->addSocket(socket->getFd(), EPOLLIN);
            _listenSockets[socket->getFd()] = socket;
            LOG_INFO("Listening on " + it->host + ":" + TO_STRING(it->port) + " -> socket " + TO_STRING(socket->getFd()) + " (O_NONBLOCK | backlog 4096)" );
        }
        catch (const std::exception& e)
        {
            delete socket;
            LOG_ERROR("Failed to setup listener on " + it->host + ":" + TO_STRING(it->port) + " -> " + e.what());
            throw;
        }
    }
}

void Server::run()
{
    _isRunning = true;
    LOG_INFO("Server running...");

    while (_isRunning)
    {
        try
        {
            _handleEvents();
        }
        catch (const std::exception& e)
        {
			std::stringstream ss;
			ss << "Error in event loop: " << e.what();
            LOG_ERROR(ss.str());
        }
    }
}

void Server::_handleEvents()
{
    std::vector<struct epoll_event> events = _epoll->waitEvents();
    std::vector<struct epoll_event>::iterator it;
    
    for (it = events.begin(); it != events.end(); ++it)
    {
        try
        {
            if (_listenSockets.find(it->data.fd) != _listenSockets.end())
            {
                if (it->events & EPOLLIN)
                    _acceptConnection(_listenSockets[it->data.fd]);
            }
            else if (_connections.find(it->data.fd) != _connections.end())
            {
                _handleConnection(_connections[it->data.fd], it->events);
            }
        }
        catch (const std::exception& e)
        {
            LOG_ERROR("Error handling event: " + std::string(e.what()));
            _cleanupConnection(it->data.fd);
        }
    }
}

void Server::_acceptConnection(LSocket* socket)
{
    CSocket* clientSocket = NULL;
	Connection* conn = NULL;
	
	try
    {	
		clientSocket = socket->acceptClient();
    	if (!clientSocket)
        	return;  // No pending connections
        Connection* conn = new Connection(clientSocket);
        _epoll->addSocket(clientSocket->getFd(), EPOLLIN);
        _connections[clientSocket->getFd()] = conn;
		LOG_INFO("Connection " + clientSocket->toString() + " accepted");
    }
    catch (const std::exception& e)
    {
		delete clientSocket;
		delete conn;
		std::stringstream ss;
		ss << "Failed to setup connection: " << e.what();
        LOG_ERROR(ss.str());
    }
}

void Server::_handleConnection(Connection* conn, uint32_t events)
{
    try {
		HTTPRequest& req = conn->getCurrentRequest();
		// Handle temp file operations
        if (req.hasFileOperationsPending() && (events & EPOLLOUT)) {
            req.handleWrite();
            
            // If no more pending operations, modify epoll events
            if (!req.hasFileOperationsPending()) {
                _epoll->modifySocket(conn->getFd(), EPOLLIN);
            }
        }
		if (events & EPOLLOUT)
		{
			// Write response if we have data to write
			if (!conn->hasCompletedResponse())
			{
				if (!conn->handleWrite())
				{
					_cleanupConnection(conn->getFd());
					return;
				}
				// If write is complete, reset for next request
				if (conn->hasCompletedResponse())
				{
					if (conn->shouldKeepAlive())
					{
						conn->reset();
						_epoll->modifySocket(conn->getFd(), EPOLLIN);
					}
					else
					{
						_cleanupConnection(conn->getFd());
					}
				}
			}
		}
		
		if (events & EPOLLIN)
		{		
			// Handle reading
			if (!conn->hasCompletedRequest())
			{
				if (!conn->handleRead())
				{
					_cleanupConnection(conn->getFd());
					return;
				}
			}
			// Process request if complete and response not yet queued
			if (conn->hasCompletedRequest() && conn->hasCompletedResponse())
			{
                HTTPResponse response;                  
				response = _reqProc.processRequest(conn->getCurrentRequest());

				// Set some minimum headers
				response.setHeader("Host", conn->getCurrentRequest().getHeader("Host"));
				if (!conn->shouldKeepAlive())
					response.setHeader("Connection", "close");
				else
					response.setHeader("Connection", "keep-alive");
                
                // print response in yellow
                std::cout << "\033[1;33m" << "Response: " << "\033[0m" << std::endl;
                response.print();
                std::cout << "\033[1;33m" << "Response end " << "\033[0m" << std::endl;
                    
				conn->queueResponse(response);
				_epoll->modifySocket(conn->getFd(), EPOLLOUT);
			}
		}
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("Connection handling error for fd " + TO_STRING(conn->getFd()) + ": " + e.what());
        _cleanupConnection(conn->getFd());
    }
}

void Server::_cleanupConnection(int fd)
{
    if (_connections.find(fd) != _connections.end())
    {
        _epoll->removeSocket(fd);
        delete _connections[fd];
        _connections.erase(fd);
		std::stringstream ss;
		ss << "Connection cleaned up: " << fd;
        LOG_INFO(ss.str());
    }
}

void Server::stop()
{
    _isRunning = false;
    LOG_INFO("Server stopping...");
}