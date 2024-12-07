/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MIMETypes.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/05 14:47:49 by lwoiton           #+#    #+#             */
/*   Updated: 2024/11/10 14:43:44 by lwoiton          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#ifndef MIMETYPES_HPP
# define MIMETYPES_HPP

#include <map>
#include <string>
#include "Logger.hpp"

class MIMEType
{
	private:
		std::map<std::string, std::string> _mimeTypeMap;
	public:
		MIMEType();
		~MIMEType();
		std::string getMIMEType(const std::string &ext);
		bool isValidMIMEType(const std::string &ext);	
};

#endif // MIMETYPES_HPP