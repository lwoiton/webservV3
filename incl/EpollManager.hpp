/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EpollManager.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/22 19:36:41 by lwoiton           #+#    #+#             */
/*   Updated: 2024/10/23 01:47:09 by lwoiton          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#ifndef EPOLLMANAGER_HPP
# define EPOLLMANAGER_HPP

# include <vector>
# include <sys/epoll.h>
# include <unistd.h>
# include <stdexcept>
# include <cerrno>

class EpollManager
{
    public:
                                        EpollManager();
                                        ~EpollManager();

        void                            addSocket(int fd, uint32_t events);
        void                            removeSocket(int fd);
        void                            modifySocket(int fd, uint32_t events);
        std::vector<struct epoll_event> waitEvents(int timeout = -1);

    private:
        int                             _epollFd;
        static const int                _MAX_EVENTS = 1024;

        // Prevent copying
                                        EpollManager(const EpollManager&);
        EpollManager&                   operator=(const EpollManager&);
};

#endif // EPOLLMANAGER_HPP