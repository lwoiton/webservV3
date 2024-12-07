/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPUtils.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwoiton <lwoiton@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/12 03:01:40 by lwoiton           #+#    #+#             */
/*   Updated: 2024/11/12 14:47:54 by lwoiton          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPUtils.hpp"


/**
 * @brief Finds the end of HTTP headers in a data buffer
 * @details Searches for the double CRLF sequence that separates headers from body
 *          according to RFC 7230:
 *          HTTP-message = start-line
 *                        *( header-field CRLF )
 *                        CRLF
 *                        [ message-body ]
 * 
 * @param data Vector containing the HTTP message data
 * @return Position of the first CRLF in the double CRLF sequence, or std::string::npos if not found
 */
size_t HTTPUtils::findHeaderEnd(const std::vector<char>& data)
{
	for (size_t i = 0; i < data.size() - 3; ++i) {
		if (static_cast<unsigned char>(data[i]) == '\r' && 
			static_cast<unsigned char>(data[i + 1]) == '\n' && 
			static_cast<unsigned char>(data[i + 2]) == '\r' && 
			static_cast<unsigned char>(data[i + 3]) == '\n')
			return i;
	}
	return std::string::npos;
}

/**
 * @brief Finds the next colon in a header field
 * @details According to RFC 7230 section 3.2:
 *          header-field = field-name ":" OWS field-value OWS
 *          No whitespace is allowed between field-name and colon
 * 
 * @param data Vector containing the HTTP message data
 * @param start Starting position for the search
 * @return Position of the colon, or std::string::npos if not found or invalid
 */
size_t HTTPUtils::findNextColon(const std::vector<char>& data, size_t start)
{
	for (size_t i = start; i < data.size(); ++i) {
		unsigned char c = static_cast<unsigned char>(data[i]);
		if (c == '\r' || c == '\n')
			return std::string::npos;
		if (c == ':')
			return i;
	}
	return std::string::npos;
}

/**
 * @brief Trims optional whitespace (OWS) from start and end of string
 * @details RFC 7230 defines OWS as optional whitespace:
 *          OWS = *( SP / HTAB )
 *          Where SP = %x20 and HTAB = %x09
 * 
 * @param value String to trim
 * @return Trimmed string with leading and trailing OWS removed
 */
std::string HTTPUtils::trimOWS(const std::string& value)
{
	size_t start = 0;
	size_t end = value.length();

	while (start < end && isOWS(static_cast<unsigned char>(value[start])))
		++start;

	while (end > start && isOWS(static_cast<unsigned char>(value[end - 1])))
		--end;

	return value.substr(start, end - start);
}

/**
 * @brief Finds the end of line (CRLF sequence) in data
 * @details RFC 7230 defines CRLF as:
 *          CR = %x0D (carriage return)
 *          LF = %x0A (line feed)
 * 
 * @param data Vector containing the HTTP message data
 * @param start Optional starting position for the search
 * @return Position of CR in CRLF sequence, or std::string::npos if not found
 */
size_t HTTPUtils::findEOL(const std::vector<char>& data, size_t start)
{
	for (size_t i = start; i < data.size() - 1; ++i) {
		if (static_cast<unsigned char>(data[i]) == '\r' && 
			static_cast<unsigned char>(data[i + 1]) == '\n')
			return i;
	}
	return std::string::npos;
}

/**
 * @brief Checks if a character is a valid token character
 * @details RFC 7230 defines token as: 1*tchar
 *          tchar = "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" / "-" / "." /
 *                  "^" / "_" / "`" / "|" / "~" / DIGIT / ALPHA
 * 
 * @param c Character to check
 * @return true if character is a valid token character, false otherwise
 */
bool HTTPUtils::isToken(unsigned char c)
{
	static const std::string token_chars = "!#$%&'*+-.^_`|~";
	return (std::isalnum(c) || token_chars.find(c) != std::string::npos);
}

/**
 * @brief Checks if a character is optional whitespace
 * @details RFC 7230 defines OWS as: *( SP / HTAB )
 *          SP = %x20 (space)
 *          HTAB = %x09 (horizontal tab)
 * 
 * @param c Character to check
 * @return true if character is space or horizontal tab, false otherwise
 */
bool HTTPUtils::isOWS(unsigned char c)
{
	return (c == ' ' || c == '\t');
}

/**
 * @brief Check if a header field value contains a specific token
 * @details RFC 7230 Section 3.2.6 defines field-value parsing rules
 *          Tokens are case-insensitive and separated by commas
 * 
 * @param field_value The header field value to search in
 * @param token The token to look for
 * @return true if token is found, false otherwise
 */
bool HTTPUtils::hasToken(const std::string& field_value, const std::string& token)
{
	size_t pos = 0;
	while (pos < field_value.length())
	{
		// Skip leading whitespace
		while (pos < field_value.length() && isOWS(field_value[pos]))
			pos++;
			
		// Find end of current token (comma or end of string)
		size_t end = field_value.find(',', pos);
		if (end == std::string::npos)
			end = field_value.length();
			
		// Extract and trim the token
		std::string current_token = field_value.substr(pos, end - pos);
		current_token = trimOWS(current_token);
		
		// Case-insensitive comparison
		if (strcasecmp(current_token.c_str(), token.c_str()) == 0)
			return true;
			
		pos = end + 1;
	}
	return false;
}

/**
 * @brief Remove a specific token from a header field value
 * @details Maintains proper comma separation between remaining tokens
 * 
 * @param field_value The header field value to modify
 * @param token The token to remove
 * @return true if token was found and removed, false otherwise
 */
bool HTTPUtils::removeToken(std::string& field_value, const std::string& token)
{
	size_t pos = 0;
	bool found = false;
	std::string result;
	
	while (pos < field_value.length())
	{
		// Skip leading whitespace
		while (pos < field_value.length() && isOWS(field_value[pos]))
			pos++;
			
		// Find end of current token
		size_t end = field_value.find(',', pos);
		if (end == std::string::npos)
			end = field_value.length();
			
		// Extract and trim the token
		std::string current_token = field_value.substr(pos, end - pos);
		current_token = trimOWS(current_token);
		
		// If this isn't the token we're removing, add it to result
		if (strcasecmp(current_token.c_str(), token.c_str()) != 0)
		{
			if (!result.empty())
				result += ", ";
			result += current_token;
		}
		else
			found = true;
			
		pos = end + 1;
	}
	
	field_value = result;
	return found;
}

/**
 * @brief Find a string in a vector of chars
 * 
 * @param data Vector to search in
 * @param str String to find
 * @param start Starting position
 * @return Position of found string or std::string::npos
 */
size_t HTTPUtils::findString(const std::vector<char>& data, const std::string& str, size_t start)
{
	for (size_t i = start; i <= data.size() - str.length(); ++i) {
		bool found = true;
		for (size_t j = 0; j < str.length(); ++j) {
			if (data[i + j] != str[j]) {
				found = false;
				break;
			}
		}
		if (found)
			return i;
	}
	return std::string::npos;
	}
