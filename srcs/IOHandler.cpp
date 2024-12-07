/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IOHandler.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/29 22:16:02 by lwoiton           #+#    #+#             */
/*   Updated: 2024/12/03 00:47:04 by lwoiton          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "IOHandler.hpp"

explicit	IOHandler::IOHandler(int fd)
{
	setNonBlocking(fd);
	EventLoop::getInstance()->registerHandler(this);
}

IOHandler::~IOHandler()
{
	EventLoop::getInstance()->removeHandler(this);
}

void	IOHandler::setNonBlocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
		throw std::runtime_error("Failed to get file descriptor flags");
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
		throw std::runtime_error("Failed to set file descriptor to non-blocking mode");
}