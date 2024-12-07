#include "DataBase.hpp"

UserDatabase::UserDatabase() {}

UserDatabase::~UserDatabase() {}

std::string UserDatabase::addUserToDatabase(std::map<std::string, std::string> &user_details) {
    User user;
    
    std::ostringstream html;
    html << "<html><head><title>User Creation</title></head><body><h1>User Creation:</h1><ul>";
    if (user_details["username"].empty()) {
        html << "<li>Error: Username not provided</li>";
        return html.str();
    } else if (this->userExists(user_details["username"])) {
        html << "<li>Error: User already exists</li>";
        return html.str();
    }

    std::string username = user_details["username"];
    user.first_name = user_details["first_name"];
    user.last_name = user_details["last_name"];
    user.age = user_details["age"];
    user.email = user_details["email"];

    this->_users[username] = user;

    html << "<li>User added successfully</li>";
    return html.str();
}

std::string UserDatabase::deleteUserFromDatabase(const std::string &username, HTTPResponse &response) {
    std::ostringstream html;
    html << "<html><head><title>User Deletion</title></head><body><h1>User Deletion:</h1><ul>";
    if (this->userExists(username)) {
        this->_users.erase(username);
        html << "<li>User deleted successfully</li>";
        response.setStatus(200);   
    } else {
        html << "<li>Error: User " << username << " does not exist</li>";
        response.setStatus(404);
    }
    return html.str();
}

bool UserDatabase::userExists(const std::string &username) const {
    return this->_users.find(username) != this->_users.end();
}

/* print details of requested user */
void UserDatabase::printUserDetails(const std::string &username) const {
    if (this->userExists(username)) {
        User user = this->_users.at(username);
        std::cout << "First name: " << user.first_name << std::endl;
        std::cout << "Last name: " << user.last_name << std::endl;
        std::cout << "Age: " << user.age << std::endl;
        std::cout << "Email: " << user.email << std::endl;
    } else {
        // this needs to be handled as catching exception
        std::cerr << "Error: User does not exist" << std::endl;
    }
}

/* print all usernames available, without user details */
void UserDatabase::printUsers() const {
    std::map<std::string, User>::const_iterator it = this->_users.begin();
    for (; it != this->_users.end(); ++it) {
        std::cout << "Username: " << it->first << std::endl;
    }
}

/* print users in plain html text that will be returned to the server */
// create a string with html tags and return it
std::string UserDatabase::printUserDetails_html(const std::string &username) const {
    std::ostringstream html;

    html << "<html><head><title>User Details</title></head><body><h1>User Details:</h1><ul>" ;
    if (this->userExists(username)) {
        User user = _users.at(username);
        std::cout << "==========PRINT ALL USERS==============" << std::endl;
        printAllUsers();
        std::cout << "==========PRINT ALL USERS==============" << std::endl;
        html << "<li>First name: " << user.first_name << "</li>";
        html << "<li>Last name: " << user.last_name << "</li>";
        html << "<li>Age: " << user.age << "</li>";
        html << "<li>Email: " << user.email << "</li>";
    } else {
        html << "<li>Error: User " << username << " does not exist</li>";
    }
    return html.str();
}

/* print all usernames available, without user details */
std::string UserDatabase::printUsers_html() const {
    std::ostringstream html;
    html << "<html><head><title>Users</title></head><body><h1>Users:</h1><ul>";
    std::map<std::string, User>::const_iterator it = this->_users.begin();

    if (this->_users.empty()) {
        html << "<li>No users found</li>";
    }

    for (; it != this->_users.end(); ++it) {
        html << "<li>Username: " << it->first << "</li>";
    }
    html << "</ul></body></html>";
    return html.str();
}

void UserDatabase::printAllUsers() const {
    if (_users.empty()) {
        std::cout << "No users in the database." << std::endl;
        return;
    }

    for (std::map<std::string, User>::const_iterator it = _users.begin(); it != _users.end(); ++it) {
        const std::string& username = it->first;
        const User& user = it->second;

        std::cout << "Username: " << username << std::endl;
        std::cout << "  First name: " << user.first_name << std::endl;
        std::cout << "  Last name: " << user.last_name << std::endl;
        std::cout << "  Age: " << user.age << std::endl;
        std::cout << "  Email: " << user.email << std::endl;
    }
}

