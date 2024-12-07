#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include "Utils.hpp"
#include "HTTPResponse.hpp"


#define USER_ADDED "User added successfully"
#define USER_EXISTS "User already exists"
#define USER_NOT_FOUND "User does not exist"
#define USERNAME_NOT_PROVIDED "Username not provided"
#define USER_DELETED "User deleted successfully"

struct User {
    std::string first_name;
    std::string last_name;
    std::string age;
    std::string email;
};

class UserDatabase
{
    private:
        std::map<std::string, User> _users;
    public:
        UserDatabase();
        ~UserDatabase();

        // Database operations
		std::string addUserToDatabase(std::map<std::string, std::string>& user_details);
    	std::string deleteUserFromDatabase(const std::string& username, HTTPResponse& response);
        bool userExists(const std::string &username) const;
        void printUserDetails(const std::string &username) const;
        std::string printUserDetails_html(const std::string &username) const;
        void printUsers() const;
        std::string printUsers_html() const;
        void printAllUsers() const;

};

#endif