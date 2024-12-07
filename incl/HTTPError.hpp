/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPError.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/22 23:09:51 by lwoiton           #+#    #+#             */
/*   Updated: 2024/11/29 22:13:53 by lwoiton          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#ifndef HTTPERROR_HPP
# define HTTPERROR_HPP

#include <exception>
#include <string>
#include <sstream>

#include "Logger.hpp"
#include "HTTPResponse.hpp"

class HTTPResponse;

class HTTPError : public std::exception
{
	public:
		explicit			HTTPError(int code, const std::string& message = "");
		explicit			HTTPError(int code);
		virtual				~HTTPError() throw() {}
		virtual const char*	what() const throw();
		int					getCode() const;
		HTTPResponse		createErrorResponse(const std::string& serveRoot) const;
	private:
		int 			_code;
		std::string		_message;
		std::string		_loadErrorPage(const std::string& path) const;
		std::string		_getDefaultErrorPage() const;
};

class WouldBlockException : public std::exception
{
    public:
        virtual const char* what() const throw();
};

#endif // HTTPERROR_HPP