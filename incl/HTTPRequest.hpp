/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/22 22:31:38 by lwoiton           #+#    #+#             */
/*   Updated: 2024/11/21 17:29:39 by lwoiton          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

#include <string>
#include "HTTPError.hpp"
#include "Logger.hpp"
#include "URL.hpp"
#include <map>
#include <sstream>
#include <sys/stat.h>
#include <algorithm>
#include "Config.hpp"
#include "HTTPUtils.hpp"
#include "TempFile.hpp"

class HTTPRequest
{
	public:
		HTTPRequest();
		~HTTPRequest();
		enum BodyType
		{
			NO_BODY,
			CONTENT_LENGTH,
			CHUNKED,
			MULTIPART
		};
		enum RequestState
		{
			REQUEST_LINE,
			HEADERS,
			BODY_INIT,
			BODY,
			COMPLETE
		};
		/**
		 * @brief Represents a part in multipart/form-data as defined in RFC 7578 Section 4.1
		 */
		struct MultipartPart {
			std::string name;          // Form field name
			std::string filename;      // Original filename
			std::string contentType;   // MIME type
			std::vector<unsigned char> data;  // Changed to unsigned char for binary data
			
			MultipartPart() {}
		};

		/**
		 * @brief State tracking for multipart parsing according to RFC 7578
		 */
		struct MultipartState {
			std::string boundary;      ///< Boundary string from Content-Type header
			std::vector<char> buffer;  ///< Temporary buffer for part processing
			size_t boundaryStart;      ///< Start position of current boundary
			size_t headerStart;        ///< Start position of current headers
			size_t bodyStart;          ///< Start position of current part body
			bool finalBoundary;        ///< Flag for final boundary found
			std::vector<MultipartPart> parts; ///< Parsed parts
			
			MultipartState() : boundaryStart(0), headerStart(0), bodyStart(0), finalBoundary(false) {}
		};
		struct RouteMatch {
			const Config::Route*	route;
			std::string				remainingPath;
			bool					found;

			RouteMatch() : route(NULL), found(false) {}
		};
		struct FileInfo {
			bool        exists;
			bool        isDirectory;
			std::string path;
			std::string mimeType;

			FileInfo() : exists(false), isDirectory(false) {}
		};
		void					parse(std::vector<char> &data);
		void					determineBodyType(void);
		void					setRouteMatch(const Config::Route* route, const std::string& remaining);
		void					setFileInfo(const std::string& path, const std::string& mimeType);
		bool					isCGI(void) const;
		RequestState			getState() const;
		void					setState(RequestState state);
		void					setBodyType(BodyType type);
		void					setMethod(const std::string& method);
		const std::string		&getMethod() const;
		const std::string		&getVersion() const;
		const URL				&getURL() const;
		const std::string 		&getUri() const;
		const std::string		&getHeader(const std::string &key) const;
		const std::vector<char>	&getBody() const;
		const Config::Route		*getMatchedRoute() const;
		const std::string		&getRemainingPath() const;
		const FileInfo			&getFileInfo() const;
		const MultipartState	&getMultipartState() const;
		bool 					shouldKeepAlive() const;
		bool					hasMatchedRoute() const;
		void 					reset();
		void					print(bool includeBodies = true, bool allHeaders = true, const std::set<std::string>& allowedMimeTypes = std::set<std::string>()) const;
		void 					printState() const;
		void					appendToBody(const std::vector<char>& data);
		int 					getTempFileFd();
		bool					hasFileOperationsPending() const;
		void					handleWrite();
	private:
		static const size_t					MEMORY_THRESHOLD = 1024 * 1024; // 1MB
		RequestState						_state;
		BodyType							_bodyType;
		size_t								_bodyLength;
		long								_chunkLength;
		std::string							_method;
		std::string							_uri;
		URL*								_url;
		std::string							_version;
		std::map<std::string, std::string>	_headers;
		std::string							_authorityPath;
		std::vector<char>					_body;
		MultipartState						*_multipartState;
		RouteMatch							_routeMatch;
		FileInfo							_fileInfo;
		TempFile*							_tempFile;
		bool								_usingTempFile;
		std::vector<char>					_pendingWrite;  // Buffer for data waiting to be written
		size_t								_writeOffset;  // Track position in pending write buffer
		void								parseRequestLine(std::vector<char> &data);
		void								parseHeaders(std::vector<char>& data, bool isTrailer = false);
		void								parseBody(std::vector<char>& data);
		void								parseContentLengthBody(std::vector<char> &data);
		void								parseChunkedBody(std::vector<char> &data);
		void								parseMultipartBody(std::vector<char> &data);
		void								parseMultipartHeaders(std::vector<char>& headerData, MultipartPart& part);
};

#endif // HTTPREQUEST_HPP