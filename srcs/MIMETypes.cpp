/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MIMETypes.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/05 14:50:54 by lwoiton           #+#    #+#             */
/*   Updated: 2024/11/12 22:48:20 by lwoiton          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "MIMETypes.hpp"

MIMEType::MIMEType(void)
{
	_mimeTypeMap[".html"] = "text/html";
	_mimeTypeMap[".htm"] = "text/html";
	_mimeTypeMap[".css"] = "text/css";
	_mimeTypeMap[".js"] = "text/javascript";
	_mimeTypeMap[".txt"] = "text/plain";


	_mimeTypeMap[".jpg"] = "image/jpeg";
	_mimeTypeMap[".jpeg"] = "image/jpeg";
	_mimeTypeMap[".png"] = "image/png";
	_mimeTypeMap[".gif"] = "image/gif";
	_mimeTypeMap[".svg"] = "image/svg+xml";
	_mimeTypeMap[".ico"] = "image/x-icon";


	_mimeTypeMap[".json"] = "application/json";
	_mimeTypeMap[".pdf"] = "application/pdf";
	_mimeTypeMap[".xml"] = "application/xml";


	_mimeTypeMap[".zip"] = "application/zip";
	_mimeTypeMap[".tar"] = "application/x-tar";
	_mimeTypeMap[".gz"] = "application/x-gzip";


	_mimeTypeMap[".ttf"] = "font/ttf";
	_mimeTypeMap[".woff"] = "font/woff";
	_mimeTypeMap[".woff2"] = "font/woff2";
}

MIMEType::~MIMEType(void)
{
}

std::string MIMEType::getMIMEType(const std::string &ext)
{
	size_t pos = ext.find_last_of('.');
	if (pos == std::string::npos)
		return "application/octet-stream";
	std::string extention = ext.substr(pos);
	for (size_t i = 0; i < extention.size(); i++)
		extention[i] = std::tolower(extention[i]);
	std::map<std::string, std::string>::iterator it = _mimeTypeMap.find(extention);
	if (it != _mimeTypeMap.end())
	{
		return it->second;
	}
	return "application/octet-stream";
}

bool MIMEType::isValidMIMEType(const std::string &ext)
{
	std::map<std::string, std::string>::iterator it = _mimeTypeMap.find(ext);
	if (it != _mimeTypeMap.end())
		return true;
	return false;
}