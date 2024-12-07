/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/23 02:57:37 by lwoiton           #+#    #+#             */
/*   Updated: 2024/11/30 00:14:13 by lwoiton          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#ifndef UTILS_HPP
# define UTILS_HPP

# include <string>
# include <algorithm>
# include <set>
# include <iomanip>
# include <sstream>
# include <fstream>
#include <cstring>
#include <fcntl.h>

std::string toUpper(std::string str);
std::string joinSet(const std::set<std::string>& set, const std::string& delim);
std::string stringToHex(const std::string& input);
std::string readFile(const std::string& filename); 
std::string _sizeToString(size_t value);
template<typename T>
std::string toString(T value) {
	std::ostringstream oss;
	oss << value;
	return oss.str();
}
void setNonBlocking(int fd);
void setPipeBufferSize(int pipefd);
#endif // UTILS_HPP