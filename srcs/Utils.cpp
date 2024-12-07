/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/23 03:00:33 by lwoiton           #+#    #+#             */
/*   Updated: 2024/11/29 14:26:18 by lwoiton          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Utils.hpp"

std::string toUpper(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    return str;
}

std::string joinSet(const std::set<std::string>& set, const std::string& delim) {
	std::string result;
	for (std::set<std::string>::const_iterator it = set.begin(); it != set.end(); ++it) {
		const std::string& s = *it;
		result += s;
		result += delim;
	}
	if (!result.empty()) {
		result.erase(result.size() - delim.size());
	}
	return result;
}

std::string stringToHex(const std::string& input)
{
    std::stringstream ss;
    ss << "hex:";
    for (size_t i = 0; i < input.length(); ++i)
    {
        ss << " " << std::hex << std::setw(2) << std::setfill('0') 
           << static_cast<int>(static_cast<unsigned char>(input[i]));
    }
    return ss.str();
}

std::string readFile(const std::string& filename)
{
	std::ifstream file(filename.c_str());
	if (!file.is_open()) {
		return "<html><body><h1>404 Not Found</h1></body></html>";
	}
	return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

std::string _sizeToString(size_t value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

int stringToNumber(const std::string& str)
{
	int result;
	std::istringstream iss(str);
	iss >> result;
	return result;
}

void setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) throw std::runtime_error("fcntl GET failed");
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) 
        throw std::runtime_error("fcntl SET failed");
}

void setPipeBufferSize(int pipefd) {
    int pipe_size = 1048576;  // 1 MB buffer size
    if (fcntl(pipefd, F_SETPIPE_SZ, pipe_size) == -1) {
        throw std::runtime_error("Failed to increase pipe buffer size");
    }
}