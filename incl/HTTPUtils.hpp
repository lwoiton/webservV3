/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPUtils.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/12 02:59:13 by lwoiton           #+#    #+#             */
/*   Updated: 2024/11/12 14:45:51 by lwoiton          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#ifndef HTTPUTILS_HPP
# define HTTPUTILS_HPP

# include <vector>
# include <string>
# include <cstring>

/**
 * @namespace HTTPUtils
 * @brief Utility functions for HTTP message parsing according to RFC 7230
 */
namespace HTTPUtils
{
    bool		isToken(unsigned char c);
    bool		isOWS(unsigned char c);
    size_t		findHeaderEnd(const std::vector<char>& data);
    size_t		findNextColon(const std::vector<char>& data, size_t start);
    std::string	trimOWS(const std::string& value);
    size_t		findEOL(const std::vector<char>& data, size_t start = 0);
	bool 		hasToken(const std::string& field_value, const std::string& token);
	bool		removeToken(std::string& field_value, const std::string& token);
	size_t		findString(const std::vector<char>& data, const std::string& str, size_t start = 0);
}
#endif // HTTPTUtils_HPP