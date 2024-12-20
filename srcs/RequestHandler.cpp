/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/20 15:28:04 by lwoiton           #+#    #+#             */
/*   Updated: 2024/12/20 15:55:26 by lwoiton          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// RequestHandler.cpp
#include "RequestHandler.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

// RequestHandler.cpp
RequestHandler* RequestHandler::_instance = NULL;

void RequestHandler::initialize(const std::vector<Config::ServerConfig>& config)
{
    if (_instance == NULL) {
        _instance = new RequestHandler(config);
    }
}

RequestHandler* RequestHandler::getInstance()
{
    if (_instance == NULL) {
        throw std::runtime_error("RequestHandler not initialized");
    }
    return _instance;
}

void RequestHandler::cleanup() {
    delete _instance;
    _instance = NULL;
}

RequestHandler::RequestHandler(const std::vector<Config::ServerConfig>& config)
    : _config(config)
    , _mimeTypes()
{
    // Initialize routing table and other constant data
    createRoutingTable(config);
}

HTTPResponse RequestHandler::handleRequest(const HTTPRequest& req) const
{
	HTTPResponse response;  
    try {
		// Find and set the best route (saved in HTTPRequest since it a Request related data)
		findAndSetBestRoute(req);

		// Check allowed methods
        const Config::Route* route = req.getMatchedRoute();
        if (route->allowedMethods.find(req.getMethod()) == route->allowedMethods.end())
		{
            throw HTTPError(405, "Method Not Allowed");
        }

        if (req.isCGI())
		{
            return handleCGIRequest(req, route);
        }

        // Handle request based on method
		if (req.getMethod() == "HEAD") {
			HTTPResponse response = handleGETRequest(req);
 			response.setBody("");
			return response;
		}
       	if (req.getMethod() == "GET") {
            return handleGETRequest(req);
        } else if (req.getMethod() == "POST") {
            return handlePOSTRequest(req);
        } else if (req.getMethod() == "DELETE") {
            return handleDELETERequest(req);
        }
    }
    catch (const HTTPError& e) {
		response = e.createErrorResponse(req.getMatchedRoute()->root);
		if (req.getMethod() == "HEAD")
			response.setBody("");
    }
    catch (const std::exception& e) {
        LOG_DEBUG("process request error: " + std::string(e.what()));
		response = HTTPError(500, "Internal Server Error").createErrorResponse(req.getMatchedRoute()->root);
    }
    return response;
}

HTTPResponse RequestProcessor::handleGETRequest(HTTPRequest &req)
{
    const Config::Route* route = req.getMatchedRoute();
    if (!route) {
        LOG_ERROR("No route found for request");
        throw HTTPError(500, "Internal Server Error");
    }
    
    // Build the full filesystem path
    std::string fullPath = route->root;
    if (!fullPath.empty() && fullPath[fullPath.length() - 1] != '/' && req.getRemainingPath()[0] != '/')
        fullPath += "/";
    fullPath += req.getRemainingPath();

    // Set file info in request
    req.setFileInfo(fullPath, _mimeTypes.getMIMEType(fullPath));
    
    const HTTPRequest::FileInfo& fileInfo = req.getFileInfo();
    if (!fileInfo.exists)
        throw HTTPError(404, "Not Found");
    
    // Handle directory
    if (fileInfo.isDirectory)
    {
        // Check for index file
        if (!route->index.empty())
        {
            std::string indexPath = fullPath;
            if (indexPath[indexPath.length() - 1] != '/')
                indexPath += "/";
            indexPath += route->index;

			try
			{
            	req.setFileInfo(indexPath, _mimeTypes.getMIMEType(indexPath));
            	const HTTPRequest::FileInfo& indexInfo = req.getFileInfo();
            	if (indexInfo.exists && !indexInfo.isDirectory)
                	return serveFile(indexPath, indexInfo.mimeType);
			}
			catch(const HTTPError& e)
			{
				LOG_DEBUG("Index file not found: " + indexPath);
                throw HTTPError(404, "Not Found");
				// Ignore error and continue with directory listing
			}
			
        }

		// Show directory listing if autoindex is enabled
        if (route->autoindex)
            return handleDirectory(fullPath, *route);
        
        throw HTTPError(403, "Forbidden");
    }
    // Serve regular file
    return serveFile(fileInfo.path, fileInfo.mimeType);
}

HTTPResponse RequestProcessor::serveFile(const std::string& filePath, const std::string& mimeType) const
{
    HTTPResponse response;
    std::ifstream file(filePath.c_str(), std::ios::binary);
    
    if (!file.is_open())
        throw HTTPError(404, "Not Found");

    // Get file size
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);

    // Read file content
    std::vector<char> buffer(size);
    if (size > 0)
        file.read(&buffer[0], size);

    response.setStatus(200);
    response.setHeader("Content-Type", mimeType);
    response.setBody(std::string(buffer.begin(), buffer.end()));
    return response;
}

HTTPResponse RequestProcessor::handlePOSTRequest(HTTPRequest &req)
{
    HTTPResponse response;
    const Config::Route* route = req.getMatchedRoute();
    
    try {
        // Handle file upload if configured
        if (!route->uploadDir.empty()) {
            // Verify content type
            std::string contentType = req.getHeader("Content-Type");
            if (contentType.find("multipart/form-data") != std::string::npos)
			{
				if (route->uploadDir.empty())
				{
					throw HTTPError(403, "Forbidden: File uploads not allowed");
				}
            	// Handle multipart data through dedicated method
				return handleFileUpload(req, route);
			} 
            //return handleFileUpload(req, route);
        }
        // check if chunked request
        if (req.getHeader("Content-Length") != "") {
            response.setStatus(200);
            response.setHeader("Content-Type", "text/html");
            response.setBody("");
            return response;
        }

        // Handle other POST requests (database operations etc.)
        if (req.getURL().getPath() == "/user_create") {
            std::string body(req.getBody().begin(), req.getBody().end());
            std::map<std::string, std::string> query = parseQueryParamsPOST(body);
            std::string res = _usersDB.addUserToDatabase(query);
            response.setStatus(200);
            response.setHeader("Content-Type", "text/html");
            response.setBody(res);
            return response;
        }


        throw HTTPError(404, "Not Found");
    }
    catch (const std::exception& e) {
        throw HTTPError(500, "Internal Server Error: " + std::string(e.what()));
    }
}

HTTPResponse RequestProcessor::handleFileUpload(HTTPRequest &req, const Config::Route* route)
{
    HTTPResponse response;
    LOG_DEBUG("Processing file upload");
    
    try {
        const HTTPRequest::MultipartState& state = req.getMultipartState();
        if (state.parts.empty()) {
            throw HTTPError(400, "No file data received");
        }

        for (size_t i = 0; i < state.parts.size(); ++i)
		{
            const HTTPRequest::MultipartPart& part = state.parts[i];
            if (part.filename.empty()) continue;

            std::string uploadPath = route->uploadDir;
            if (uploadPath[uploadPath.length() - 1] != '/') {
                uploadPath += '/';
            }
            uploadPath += part.filename;

            std::ofstream file(uploadPath.c_str(), std::ios::binary);
            // Fixed operator precedence with parentheses
            if (!file || (!part.data.empty() && !file.write(
                reinterpret_cast<const char*>(&part.data[0]), 
                part.data.size()))) {
                throw HTTPError(500, "Failed to write file: " + part.filename);
            }
            file.close();

            LOG_DEBUG("Uploaded: " + part.filename + " (" + toString(part.data.size()) + " bytes)");
		}
		// Instead of plain text, return JSON response
		response.setStatus(201);
		response.setHeader("Content-Type", "application/json");
		std::string jsonResponse = "{"
			"\"status\": \"success\","
			"\"message\": \"Files uploaded successfully\","
			"\"files\": [";
		
		bool first = true;
		for (size_t i = 0; i < state.parts.size(); ++i) {
			if (!first) jsonResponse += ",";
			first = false;
			jsonResponse += "\"" + state.parts[i].filename + "\"";
		}
		
		jsonResponse += "]}";
		response.setBody(jsonResponse);
        return response;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Upload failed: " + std::string(e.what()));
        throw;
    }
}


std::string	RequestProcessor::decodeComponentPOST(const std::string& enocoded)
{
	std::string decoded;
	for (size_t i = 0; i < enocoded.length(); i++)
	{
		if (enocoded[i] == '%' && i + 2 < enocoded.length())
		{
			std::string hex = enocoded.substr(i + 1, 2);
			std::istringstream iss(hex);
			int value;
			iss >> std::hex >> value;
			decoded += static_cast<char>(value);
			i += 2;
		}
		else if (enocoded[i] == '+')
		{
			decoded += ' ';
		}
		else
		{
			decoded += enocoded[i];
		}
	}
	return (decoded);
}

std::map<std::string, std::string> RequestProcessor::parseQueryParamsPOST(const std::string &query) {
    std::map<std::string, std::string> queryParams;
    size_t start = 0;
    size_t end;

    while (start < query.length()) {
        // Find the end of the current parameter
        end = query.find('&', start);
        if (end == std::string::npos) {
            end = query.length();
        }

        // Find the equals sign separating key and value
        size_t equals = query.find('=', start);
        if (equals != std::string::npos && equals < end) {
            std::string key = decodeComponentPOST(query.substr(start, equals - start));
            std::string value = decodeComponentPOST(query.substr(equals + 1, end - equals - 1));
            queryParams[key] = value;
        }
        start = end + 1;
    }
    return (queryParams);
}

HTTPResponse RequestProcessor::handleDELETERequest(HTTPRequest &req)
{
    HTTPResponse response;
    const Config::Route* route = req.getMatchedRoute();
    
    try {
        // Build the full filesystem path
        std::string fullPath = route->root;
        if (!fullPath.empty() && fullPath[fullPath.length() - 1] != '/')
            fullPath += "/";
        fullPath += req.getRemainingPath();

        // Set and check file info
        req.setFileInfo(fullPath, _mimeTypes.getMIMEType(fullPath));
        const HTTPRequest::FileInfo& fileInfo = req.getFileInfo();
        
        if (!fileInfo.exists)
            throw HTTPError(404, "Not Found");

        // Handle file deletion
        if (remove(fileInfo.path.c_str()) != 0)
            throw HTTPError(500, "Failed to delete file");

        response.setStatus(204); // No Content
        return response;

        // Handle database operations
        if (req.getURL().getPath() == "/user") {
			std::map<std::string, std::string> params = req.getURL().getQueryParams();
            std::map<std::string, std::string>::const_iterator it = params.find("username");
            if (it == params.end())
			{
            	throw HTTPError(400, "Username required");
			}
    		std::string res = _usersDB.deleteUserFromDatabase(it->second, response);
			response.setHeader("Content-Type", "text/html");
            response.setBody(res);
            return response;
        }

        throw HTTPError(404, "Not Found");
    }
    catch (const std::exception& e) {
        throw HTTPError(500, "Internal Server Error: " + std::string(e.what()));
    }
}

void RequestProcessor::createRoutingTable(const std::vector<Config::ServerConfig> &servers)
{
    std::vector<Config::ServerConfig>::const_iterator scit;
    for (scit = servers.begin(); scit != servers.end(); ++scit)
    {
        // Create authority string (host:port)
        std::stringstream ss;
        ss << scit->host << ":" << scit->port;
        std::string authority = ss.str();
        
        // Add routes for IP-based hosting
        std::vector<Config::Route>::const_iterator rit;
        for (rit = scit->routes.begin(); rit != scit->routes.end(); ++rit)
        {
            std::string key = createRouteKey(authority, rit->path);
            _routingTable[key] = &(*rit);
        }

        // Add routes for name-based virtual hosting
        std::vector<std::string>::const_iterator snit;
        for (snit = scit->serverNames.begin(); snit != scit->serverNames.end(); ++snit)
        {
            std::stringstream ss2;
            ss2 << *snit << ":" << scit->port;
            std::string serverAuthority = ss2.str();
            
            for (rit = scit->routes.begin(); rit != scit->routes.end(); ++rit)
            {
                std::string key = createRouteKey(serverAuthority, rit->path);
                _routingTable[key] = &(*rit);
            }
        }
    }
}

std::string RequestProcessor::createRouteKey(const std::string& authority, const std::string& path) const
{
    return authority + "|" + path;
}

void	RequestProcessor::findAndSetBestRoute(HTTPRequest &req) const
{
	// Get authority and path from URL
	std::string authority;
	std::string path = req.getURL().getPath();

	// Determine authority from absolute form URL or Host header
	if (req.getURL().isAbsoluteForm())
		authority = req.getURL().getAuthority();
	else
		authority = req.getHeader("Host");

	// Search for longest matching prefix
	std::string searchPath = path;
	while (!searchPath.empty()) {
		std::string key = createRouteKey(authority, searchPath);
		std::map<std::string, const Config::Route*>::const_iterator it = _routingTable.find(key);
		
		if (it != _routingTable.end()) {
			// Found a match - store in request
			req.setRouteMatch(it->second, path.substr(searchPath.length()));
			return;
		}

 		// Try parent path
		size_t lastSlash = searchPath.find_last_of('/');
		if (lastSlash == std::string::npos)
			break;
		searchPath = searchPath.substr(0, lastSlash);
	}
 
// Try root path as fallback
	std::string rootKey = createRouteKey(authority, "/");
	std::map<std::string, const Config::Route*>::const_iterator it = _routingTable.find(rootKey);
	if (it != _routingTable.end()) {
		req.setRouteMatch(it->second, path);
		return;
	}

	// No route found
	throw HTTPError(404, "Not Found");
}

HTTPResponse RequestProcessor::handleFileList(const HTTPRequest& req) {
    HTTPResponse response;
    const Config::Route* route = req.getMatchedRoute();
    std::string uploadDir = route->uploadDir;

    DIR* dir = opendir(uploadDir.c_str());
    if (!dir) {
        throw HTTPError(500, "Failed to open directory");
    }

    // Create JSON array of files
    std::stringstream json;
    json << "[";
    bool first = true;

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip . and .. directories
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        std::string fullPath = uploadDir + "/" + entry->d_name;
        struct stat st;
        if (stat(fullPath.c_str(), &st) == 0) {
            if (!first) json << ",";
            first = false;

            json << "{"
                 << "\"name\":\"" << entry->d_name << "\","
                 << "\"size\":" << st.st_size << ","
                 << "\"modified\":" << st.st_mtime * 1000 // Convert to milliseconds for JS
                 << "}";
        }
    }
    json << "]";

    closedir(dir);

    response.setStatus(200);
    response.setHeader("Content-Type", "application/json");
    response.setBody(json.str());
    return response;
}

HTTPResponse RequestProcessor::handleDirectory(const std::string& dirPath, 
                                            const Config::Route& route) const
{
    HTTPResponse response;
    DIR* dir = opendir(dirPath.c_str());
    if (!dir) {
        LOG_DEBUG("Failed to open directory: " + dirPath);
        throw HTTPError(500, "Internal Server Error");
    }

    std::stringstream html;
    html << "<!DOCTYPE html>\n"
         << "<html><head>\n"
         << "<title>Index of " << route.path << "</title>\n"
         << "<style>\n"
         << "body { font-family: Arial, sans-serif; margin: 40px; }\n"
         << "table { width: 100%; border-collapse: collapse; }\n"
         << "th, td { text-align: left; padding: 8px; }\n"
         << "tr:nth-child(even) { background-color: #f2f2f2; }\n"
         << "</style></head><body>\n"
         << "<h1>Index of " << route.path << "</h1>\n"
         << "<table>\n"
         << "<tr><th>Name</th><th>Size</th><th>Last Modified</th></tr>\n";

    // Use std::vector for files
    std::vector<std::pair<std::string, struct stat> > files;
    struct dirent* entry;
    while ((entry = readdir(dir)))
    {
        if (entry->d_name[0] != '.') // Skip hidden files
        {
            std::string fullPath = dirPath + "/" + entry->d_name;
            struct stat st;
            if (stat(fullPath.c_str(), &st) == 0)
            {
                files.push_back(std::make_pair(std::string(entry->d_name), st));
            }
        }
    }
    closedir(dir);

    // Sort files (directories first, then alphabetically)
    // C++98 compatible comparison function
    struct CompareFiles {
        static bool compare(const std::pair<std::string, struct stat>& a, 
                          const std::pair<std::string, struct stat>& b) {
            bool aIsDir = S_ISDIR(a.second.st_mode);
            bool bIsDir = S_ISDIR(b.second.st_mode);
            if (aIsDir != bIsDir)
                return aIsDir > bIsDir;
            return a.first < b.first;
        }
    };
    
    std::sort(files.begin(), files.end(), CompareFiles::compare);

    // Generate table rows
    for (std::vector<std::pair<std::string, struct stat> >::const_iterator it = files.begin(); 
         it != files.end(); ++it)
    {
        const std::string& name = it->first;
        const struct stat& st = it->second;
        bool isDir = S_ISDIR(st.st_mode);

        html << "<tr><td><a href=\"" << name << (isDir ? "/" : "") << "\">"
             << name << (isDir ? "/" : "") << "</a></td><td>"
             << (isDir ? "-" : toString(st.st_size)) << "</td><td>"
             << ctime(&st.st_mtime) << "</td></tr>\n";
    }

    html << "</table></body></html>";
    
    response.setStatus(200);
    response.setHeader("Content-Type", "text/html");
    response.setBody(html.str());
    return response;
}

std::string RequestProcessor::generateDirectoryListing(const std::string& dirPath, const std::string& requestPath) const
{
    std::stringstream html;
    DIR* dir = opendir(dirPath.c_str());
    if (!dir) {
        LOG_DEBUG("Failed to open directory 2: " + dirPath);   
        throw HTTPError(500, "Internal Server Error");
    }

    html << "<html><head><title>Index of " << requestPath << "</title></head><body>\n";
    html << "<h1>Index of " << requestPath << "</h1><hr><pre>\n";

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL)
    {
        std::string name = entry->d_name;
        if (name != "." && name != "..")
        {
            std::string fullPath = dirPath + "/" + name;
            struct stat st;
            if (stat(fullPath.c_str(), &st) == 0)
            {
                bool isDir = S_ISDIR(st.st_mode);
                html << "<a href=\"" << name << (isDir ? "/" : "") << "\">" 
                     << name << (isDir ? "/" : "") << "</a>\n";
            }
        }
    }

    closedir(dir);
    html << "</pre><hr></body></html>";
    return html.str();
}


/* Destructor */
RequestProcessor::~RequestProcessor() {}

void RequestProcessor::printRoutingTable() const
{
    std::map<std::string, const Config::Route*>::const_iterator it;
    std::cout << "============== Routing Table: ==============" << std::endl;
    for (it = _routingTable.begin(); it != _routingTable.end(); ++it)
    {
        std::cout << it->first << " -> " << it->second->root << " " << it->second->path << std::endl;
    }
    std::cout << std::endl;
}