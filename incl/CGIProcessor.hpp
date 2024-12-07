#ifndef CGIPROCESSOR_HPP
#define CGIPROCESSOR_HPP

#include <iostream>
#include <string>
#include <vector>
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "Environment.hpp"

class CGIProcessor {
private:
    static const size_t PIPE_BUF_SIZE;
    static const size_t MEMORY_THRESHOLD;
    static const size_t CHUNK_SIZE;
    static const int READ_TIMEOUT;
    
    HTTPRequest _parsedRequest;
    Environment _env;
    std::string _path_to_script;
    
    class TempFile {
    private:
        std::string _path;
        int _fd;
        size_t _size;
        bool _closed;

    public:
        TempFile();
        ~TempFile();
        void write(const char* data, size_t size);
        void streamTo(int pipe_fd);
        void close();
        size_t size() const;

    private:
        TempFile(const TempFile&);
        TempFile& operator=(const TempFile&);
    };

    class PipeHandler {
    private:
        int _read_fd;
        int _write_fd;
        bool _closed;

    public:
        PipeHandler();
        ~PipeHandler();
        void close();
        int getReadFd() const;
        int getWriteFd() const;
        void closeRead();
        void closeWrite();

    private:
        PipeHandler(const PipeHandler&);
        PipeHandler& operator=(const PipeHandler&);
    };

    HTTPResponse handleGETCGI(char* argv[]);
    HTTPResponse handlePOSTCGI(char* argv[]);
    void streamBodyToCGI(int pipe_fd, const std::vector<char>& body);
    void handleLargeBody(const std::vector<char>& body, int pipe_fd);
    void setNonBlocking(int fd);
    std::vector<char> readCGIResponse(int fd);

public:
    explicit CGIProcessor(const HTTPRequest& req);
    ~CGIProcessor();
    HTTPResponse handleCGIRequest();
};

#endif