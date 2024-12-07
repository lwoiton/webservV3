#include "../incl/Environment.hpp"

Environment::Environment(int n) : size(n) {
    env = new char*[size];
    for (int i = 0; i < size; ++i) {
        env[i] = NULL;
    }
}

Environment::~Environment()
{
    for (int i = 0; i < size; ++i) {
        if (env[i] != NULL)
            delete[] env[i];
    } 
    delete[] env;
}

void Environment::AddEnvVar(int i, const char *key, const char *value) {
    if (i >= 0 && i < size) {
        int len = std::strlen(key) + std::strlen(value) + 2; // For "name" + "=" + "value" + '\0'
        env[i] = new char[len];
        std::strcpy(env[i], key);
        std::strcat(env[i], "=");
        std::strcat(env[i], value);
    }
}

void Environment::printEnv() const {
    for (int i = 0; i < size; ++i) {
        std::cout << env[i] << std::endl;
    }
}

std::string Environment::parseQueryString(std::string uri) {
    // I only need to get a string after the '?' character
    size_t pos = uri.find("?");
    if (pos != std::string::npos) {
        std::string query = uri.substr(pos + 1);
        std::cout << "Query string: " << query << std::endl;
        return query;
    }
    return "";
}

void Environment::createEnv(HTTPRequest &req, std::string _path_to_script) {
	
	AddEnvVar(0, "REQUEST_METHOD", req.getMethod().c_str());
	AddEnvVar(1, "SCRIPT_FILENAME", _path_to_script.c_str());
	AddEnvVar(2, "SERVER_NAME", "localhost"); // remove hardcoding
	AddEnvVar(3, "SERVER_PROTOCOL", req.getVersion().c_str());
	AddEnvVar(4, "SERVER_PORT", "8080"); // remove hardcoding
	
	if (req.getMethod() == "GET") {
		AddEnvVar(5, "QUERY_STRING", parseQueryString(req.getUri()).c_str());
	}
	else if (req.getMethod() == "POST") {
		AddEnvVar(5, "CONTENT_TYPE", req.getHeader("Content-Type").c_str());
		AddEnvVar(6, "CONTENT_LENGTH", req.getHeader("Content-Length").c_str());
        LOG_DEBUG("Content-Length: " + req.getHeader("Content-Length"));
	}
}

