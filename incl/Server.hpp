/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/22 19:33:22 by lwoiton           #+#    #+#             */
/*   Updated: 2024/11/30 00:33:05 by lwoiton          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#ifndef SERVER_HPP
# define SERVER_HPP

# include "Config.hpp"
# include "Logger.hpp"
# include "EpollManager.hpp"
# include "LSocket.hpp"
# include "Connection.hpp"
# include "DataBase.hpp"
# include "RequestProcessor.hpp"
# include "CGIProcessor.hpp"
# include <map>
# include <memory>

# define BACKLOG 4096

class Server
{
    public:
									Server(const std::string &configPath);
        virtual                     ~Server();

        virtual void                run();
        virtual void                stop();

    private:
        std::auto_ptr<Config>		_config;
        std::auto_ptr<EpollManager>	_epoll;
        std::map<int, LSocket*>		_listenSockets;
        std::map<int, Connection*>	_connections;
        RequestProcessor            _reqProc;
        bool                        _isRunning;

        void                        _setupListeners();
        void                        _handleEvents();
        void                        _acceptConnection(LSocket* socket);
        void                        _handleConnection(Connection* conn, uint32_t events);
        void                        _cleanupConnection(int fd);
                                    
                                    Server();
                                    Server(const Server&);
        Server&                     operator=(const Server&);
};

#endif // SERVER_HPP