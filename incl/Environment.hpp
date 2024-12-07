#ifndef CGI_HPP
# define CGI_HPP

#include <cstring>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "HTTPRequest.hpp"

class Environment {

    public: 
        char **env;
        int size;

    Environment(int n);
    ~Environment();

    void	createEnv(HTTPRequest &req, std::string _path_to_script);
    void AddEnvVar(int index, const char *key, const char *value);
    std::string parseQueryString(std::string _uri);
    void printEnv() const;
    // getter
    char **getEnv() const { return env; }
};


#endif