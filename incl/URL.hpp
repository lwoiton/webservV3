/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   URL.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/07 11:56:46 by lwoiton           #+#    #+#             */
/*   Updated: 2024/11/11 18:42:52 by lwoiton          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#ifndef URL_HPP
#define URL_HPP

#include "HTTPError.hpp"
#include <string>
#include <map>
#include <stdexcept>
#include <algorithm>
#include <sstream>
#include <vector>

class URL
{
	private:
		std::string							_uri; // Original URI (encoded)
		std::string							_scheme;
		std::string							_authority;
		std::string							_path;
		std::string							_query;
		std::map<std::string, std::string>	_queryParams;
		std::string							_fragment;
		bool								_absoluteForm;
		void								_parse(const std::string& uri);
		void								_parseQueryParams(const std::string& query);
		std::string							_decodeComponent(const std::string& enocoded);
		std::string							_normalize(std::string src);
		void								_normalizePath();
		void								_validateURL() const;
		void								_validateQuery() const;
		void								_validateAuthority() const;
		void								_validatePath() const;
	public:
													URL(const std::string& uri);
													~URL();
		const std::string&							getEncoded() const;
		const std::string&							getScheme() const;
		const std::string&							getAuthority() const;
		const std::string&							getPath() const;
		const std::string&							getQuery() const;
		const std::map<std::string, std::string>&	getQueryParams() const;
		bool										isAbsoluteForm() const;
};

#endif // URL_HPP