/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/23 14:15:27 by lwoiton           #+#    #+#             */
/*   Updated: 2024/11/12 23:09:30 by lwoiton          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Logger.hpp"
#include <signal.h>
#include <iostream>

static IServer* g_server = NULL;

void signalHandler(int signum)
{
    if (g_server != NULL)
    {
        LOG_INFO("Received signal " + signum);
        g_server->stop();
    }
}

void setupSignalHandlers()
{
    signal(SIGINT, signalHandler);   // Ctrl+C
    signal(SIGTERM, signalHandler);  // termination request
}

int main(int argc, char* argv[])
{
    try
    {
        // Set default config path
        std::string configPath = "default.conf";
        
        // Parse command line arguments
        if (argc > 2)
        {
            std::cerr << "Usage: " << argv[0] << " [configuration file]" << std::endl;
            return 1;
        }
        else if (argc == 2)
        {
            configPath = argv[1];
        }

        // Setup signal handlers
        //setupSignalHandlers();

        // Initialize server
        Server server(configPath);
        g_server = &server;
        
        // Run server
        server.run();

        LOG_INFO("Server shutdown complete");
        return 0;
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("Fatal error: " + std::string(e.what()));
        return 1;
    }
}