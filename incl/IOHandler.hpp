/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IOHandler.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/27 11:33:25 by lwoiton           #+#    #+#             */
/*   Updated: 2024/12/03 00:47:25 by lwoiton          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#ifndef IOHANDLER_HPP
# define IOHANDLER_HPP

# include <stdexcept>
# include <unistd.h>
# include <fcntl.h>

# include "EventLoop.hpp"

class IOHandler
{
	protected:
		explicit		IOHandler(int fd);
		void			setNonBlocking(int fd);
	public:
		virtual			~IOHandler() = 0;
		virtual bool	handleRead() = 0;
		virtual bool	handleWrite() = 0;
		virtual bool	wantsToRead() const = 0;
		virtual bool	wantsToWrite() const = 0;
		virtual int		getFd() const = 0;
	private:
		IOHandler(const IOHandler& src);
		IOHandler& operator=(const IOHandler& src);
};

# endif //IOHANDLER