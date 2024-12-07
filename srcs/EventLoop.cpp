/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventLoop.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/30 17:49:38 by lwoiton           #+#    #+#             */
/*   Updated: 2024/12/03 00:29:28 by lwoiton          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "EventLoop.hpp"

EventLoop*	EventLoop::_instance = NULL;

EventLoop::EventLoop()
	: _epollFd(-1)
	, _running(false)
{
	_epollFd = epoll_create1(0);
	if (_epollFd == -1)
		throw std::runtime_error("Failed to create epoll instance");
}

EventLoop::~EventLoop()
{
	if (_epollFd != -1)
		close(_epollFd);
	
	std::map<int, IOHandler*>::iterator it;
	for (it = _handlers.begin(); it != _handlers.end(); it++)
		delete it->second;
}

EventLoop*	EventLoop::getInstance()
{
	if (!_instance)
		_instance = new EventLoop();
	return _instance;
}

void	EventLoop::removeHandler(IOHandler* handler)
{
	if (!handler)
		return;
	epoll_ctl(_epollFd, EPOLL_CTL_DEL, handler->getFd(), NULL);
	_handlers.erase(handler->getFd());

	delete handler;
}

// Register a new I/O handler
void	EventLoop::registerHandler(IOHandler* handler)
{
	if (!handler)
		return;
	struct epoll_event ev;
	ev.data.ptr = handler;  // Store pointer to handler instead of just fd
	ev.events = 0;    // We need to use level-triggered mode because of the subject
	
	// Set events based on handler's needs
	if (handler->wantsToRead())
		ev.events |= EPOLLIN;
	if (handler->wantsToWrite())
		ev.events |= EPOLLOUT;
		
	epoll_ctl(_epollFd, EPOLL_CTL_ADD, handler->getFd(), &ev);
	_handlers[handler->getFd()] = handler;
}
    
// Main event loop
void	EventLoop::run()
{
	const int MAX_EVENTS = 64;
	struct epoll_event events[MAX_EVENTS];
	
	while (_running) {
		int nfds = epoll_wait(_epollFd, events, MAX_EVENTS, -1);
		
		for (int i = 0; i < nfds; i++) {
			IOHandler* handler = static_cast<IOHandler*>(events[i].data.ptr);
			
			try {
				if (events[i].events & (EPOLLERR | EPOLLHUP)) {
					// Handle error events
					removeHandler(handler);
					updateHandlerEvents(handler);
					continue;
				}
				
				// Handle read events
				if (events[i].events & EPOLLIN && handler->wantsToRead()) {
					if (!handler->handleRead()) {
						removeHandler(handler);
						continue;
					}
					updateHandlerEvents(handler);
				}
				
				// Handle write events
				if (events[i].events & EPOLLOUT && handler->wantsToWrite()) {
					if (!handler->handleWrite()) {
						removeHandler(handler);
						continue;
					}
					updateHandlerEvents(handler);
				}
			}
			catch (const std::exception& e) {
				// Log error and remove handler
				removeHandler(handler);
			}
		}
	}
}
	
void	EventLoop::updateHandlerEvents(IOHandler* handler)
{
	struct epoll_event ev;
	ev.data.ptr = handler;
	ev.events = 0;   // We need to use level-triggered mode because of the subject
	
	if (handler->wantsToRead())
		ev.events |= EPOLLIN;
	if (handler->wantsToWrite())
		ev.events |= EPOLLOUT;
		
	epoll_ctl(_epollFd, EPOLL_CTL_MOD, handler->getFd(), &ev);
}
