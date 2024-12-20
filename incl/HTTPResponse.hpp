/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/22 22:27:27 by lwoiton           #+#    #+#             */
/*   Updated: 2024/12/20 13:39:53 by lwoiton          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP

#include "TempFile.hpp"
#include "Logger.hpp"
#include "HTTPError.hpp"
#include "Utils.hpp"
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <cstdlib>

class HTTPResponse
{
	public:
		HTTPResponse();
		~HTTPResponse();
		enum ResponseState {
			CREATING,
			SENDING,
			COMPLETE
		};
		ResponseState 		getState() const;
		void		setStatus(int status);
		void		setHeader(const std::string &key, const std::string &value);
		void		deleteHeader(const std::string& key);
		void		setBody(const std::vector<char> &body);
		std::vector<char>& getBody() const;
		void		appendToBody(const char* data, size_t len);
		std::string getHttpDate();
		std::vector<char>	serialize() const;
		size_t		getBodySize() const;
		void		reset();
		int			getStatus() const;
		void		print() const;
		std::vector<char>					getNextChunk();
	private:
		static const size_t					MEMORY_THRESHOLD = 1024 * 1024; // 1MB
    	static const size_t					CHUNK_SIZE; // 8KB chunks for transfer
		ResponseState						_state;
    	TempFile*							_tempFile;
    	bool								_usingTempFile;
    	size_t								_readOffset;
    	std::vector<char>					_sendBuffer;
		int									_statusCode;
		std::map<std::string, std::string>	_headers;
		std::vector<char>					_body;
		size_t								_bodySize;
		std::string							getStatusText() const;
		void								setEssentialHeaders();
		bool								hasMoreData() const;
};

#endif // HTTPRESPONSE_HPP