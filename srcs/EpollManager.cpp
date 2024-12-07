/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EpollManager.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/22 19:36:41 by lwoiton           #+#    #+#             */
/*   Updated: 2024/10/23 01:01:12 by lwoiton          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "EpollManager.hpp"
#include <stdexcept>
#include <string.h>
#include <sstream>

EpollManager::EpollManager()
{
    _epollFd = ::epoll_create1(0);
    if (_epollFd == -1)
        throw std::runtime_error(std::string("Failed to create epoll: ") + strerror(errno));
}

EpollManager::~EpollManager()
{
    if (_epollFd != -1)
        ::close(_epollFd);
}

void EpollManager::addSocket(int fd, uint32_t events)
{
    if (_epollFd == -1)
        throw std::runtime_error("EpollManager not initialized");

    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;

    if (::epoll_ctl(_epollFd, EPOLL_CTL_ADD, fd, &ev) == -1)
    {
        std::stringstream ss;
        ss << "Failed to add fd " << fd << " to epoll: " << strerror(errno);
        throw std::runtime_error(ss.str());
    }
}

void EpollManager::modifySocket(int fd, uint32_t events)
{
    if (_epollFd == -1)
        throw std::runtime_error("EpollManager not initialized");

    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;

    if (::epoll_ctl(_epollFd, EPOLL_CTL_MOD, fd, &ev) == -1)
    {
        std::stringstream ss;
        ss << "Failed to modify fd " << fd << " in epoll: " << strerror(errno);
        throw std::runtime_error(ss.str());
    }
}

void EpollManager::removeSocket(int fd)
{
    if (_epollFd == -1)
        throw std::runtime_error("EpollManager not initialized");

    if (::epoll_ctl(_epollFd, EPOLL_CTL_DEL, fd, NULL) == -1)
    {
        std::stringstream ss;
        ss << "Failed to remove fd " << fd << " from epoll: " << strerror(errno);
        throw std::runtime_error(ss.str());
    }
}

std::vector<struct epoll_event> EpollManager::waitEvents(int timeout)
{
    if (_epollFd == -1)
        throw std::runtime_error("EpollManager not initialized");

    std::vector<struct epoll_event> events(_MAX_EVENTS);
    int nfds = ::epoll_wait(_epollFd, &events[0], _MAX_EVENTS, timeout);

    if (nfds == -1)
    {
        if (errno == EINTR)  // Interrupted system call
            return std::vector<struct epoll_event>();
        throw std::runtime_error(std::string("Epoll wait failed: ") + strerror(errno));
    }

    events.resize(nfds);  // Resize to actual number of events
    return events;
}