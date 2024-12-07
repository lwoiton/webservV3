/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventLoop.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/30 00:39:40 by lwoiton           #+#    #+#             */
/*   Updated: 2024/12/03 00:34:41 by lwoiton          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#ifndef EVENTLOOP_HPP
# define EVENTLOOP_HPP

#include <sys/epoll.h>
#include <map>
#include <stdexcept>
#include <iostream>

#include "IOHandler.hpp"


/**
 * @file EventLoop.hpp
 * @brief Event loop implementation using epoll for handling I/O events.
 *
 * This class implements an event loop using the epoll system call to manage I/O events.
 * It follows the singleton design pattern to ensure that only one instance of the event loop exists.
 * This is particularly useful in server applications like nginx, where a single event loop can efficiently
 * handle multiple connections.
 *
 * The singleton pattern is used here to:
 * - Ensure that only one instance of the EventLoop exists throughout the application.
 * - Provide a global point of access to the EventLoop instance.
 * - Control concurrent access to the EventLoop instance.
 *
 * The EventLoop class manages a collection of IOHandler objects, each representing a file descriptor
 * and its associated events. It provides methods to register, remove, and update handlers, and to run
 * the event loop.
 *
 * @note This class is not copyable or assignable to enforce the singleton pattern.
 *
 * @class EventLoop
 * @brief Singleton class for managing I/O events using epoll.
 */
class EventLoop
{
	public:
							~EventLoop();
		static EventLoop*	getInstance();
		void				registerHandler(IOHandler* handler);
		void				removeHandler(IOHandler* handler);
		void				run();
	private:
		static EventLoop*			_instance;
		int							_epollFd;
		bool						_running;
		std::map<int, IOHandler*>	_handlers;
		void						updateHandlerEvents(IOHandler* handler);
									EventLoop(); // Prevent instantiation for singleton pattern
									EventLoop(const EventLoop& src); // Prevent copy-construction
									EventLoop& operator=(const EventLoop& src); // Prevent assignment
};

#endif // EVENTLOOP_HPP