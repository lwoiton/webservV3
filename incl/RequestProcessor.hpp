#ifndef REQUESTPROCESSOR_HPP
#define REQUESTPROCESSOR_HPP

#include <iostream>
#include <string>
#include <fstream>
#include <dirent.h>
#include <sstream>
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "DataBase.hpp"
#include "HTTPError.hpp"
#include "Utils.hpp"
# include "Config.hpp"
#include "MIMETypes.hpp"
#include "CGIProcessor.hpp"

// RequestProcessor.hpp
class RequestProcessor 
{
	private:
		std::map<std::string, const Config::Route*>	_routingTable;
		UserDatabase								_usersDB;
		MIMEType									_mimeTypes;

		void										createRoutingTable(const std::vector<Config::ServerConfig> &servers);
		void										findAndSetBestRoute(HTTPRequest &req) const;
		std::string									createRouteKey(const std::string& authority, const std::string& path) const;
		
		// Modify function signatures to use HTTPRequest's FileInfo
		HTTPResponse								handleGETRequest(HTTPRequest &req);
		std::string									decodeComponentPOST(const std::string& enocoded);
		std::map<std::string, std::string> 			parseQueryParamsPOST(const std::string &query);
		HTTPResponse								handlePOSTRequest(HTTPRequest &req);
		HTTPResponse								handleDELETERequest(HTTPRequest &req);
		
		// Helper methods
		HTTPResponse								handleDirectory(const std::string& dirPath, const Config::Route& route) const;
		HTTPResponse								serveFile(const std::string& filePath, const std::string& mimeType) const;
		HTTPResponse								handleFileUpload(HTTPRequest &req, const Config::Route* route);
//		HTTPResponse								handleListFiles(HTTPRequest &req, const Config::Route* route);
		HTTPResponse								handleFileList(const HTTPRequest& req);

		std::string									generateDirectoryListing(const std::string& dirPath, const std::string& requestPath) const;

	public:
													RequestProcessor(const std::vector<Config::ServerConfig> &servers);
													~RequestProcessor();
		HTTPResponse								processRequest(HTTPRequest &req);
		void printRoutingTable() const;
};

#endif