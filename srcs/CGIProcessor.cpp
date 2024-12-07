#include "CGIProcessor.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include "Utils.hpp"

const size_t CGIProcessor::PIPE_BUF_SIZE = 65536;
const size_t CGIProcessor::MEMORY_THRESHOLD = 1024 * 1024;
const size_t CGIProcessor::CHUNK_SIZE = 8192;
const int CGIProcessor::READ_TIMEOUT = 30;

// TempFile implementation
CGIProcessor::TempFile::TempFile() : _fd(-1), _size(0), _closed(false) {
    char template_path[] = "/tmp/webserv-XXXXXX";
    _fd = mkstemp(template_path);
    if (_fd == -1) {
        throw std::runtime_error("Failed to create temp file");
    }
    _path = template_path;
}

CGIProcessor::TempFile::~TempFile() {
    close();
    if (!_path.empty()) {
        unlink(_path.c_str());
    }
}

void CGIProcessor::TempFile::write(const char* data, size_t size) {
    if (_closed) return;
    
    size_t written = 0;
    while (written < size) {
        ssize_t result = ::write(_fd, data + written, size - written);
        if (result < 0) {
            if (errno == EINTR) continue;
            throw std::runtime_error("Failed to write to temp file");
        }
        written += result;
        _size += result;
    }
}

void CGIProcessor::TempFile::streamTo(int pipe_fd) {
    if (_closed) return;
    
    if (lseek(_fd, 0, SEEK_SET) == -1) {
        throw std::runtime_error("Failed to seek temp file");
    }

    char buffer[CHUNK_SIZE];
    ssize_t bytes_read;

    while ((bytes_read = read(_fd, buffer, sizeof(buffer))) > 0) {
        size_t offset = 0;
        while (offset < static_cast<size_t>(bytes_read)) {
            ssize_t written = ::write(pipe_fd, 
                                    buffer + offset, 
                                    static_cast<size_t>(bytes_read) - offset);
            if (written < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    usleep(1000);
                    continue;
                }
                throw std::runtime_error("Failed to write to CGI pipe");
            }
            offset += written;
        }
    }
}

void CGIProcessor::TempFile::close() {
    if (!_closed && _fd != -1) {
        ::close(_fd);
        _closed = true;
    }
}

size_t CGIProcessor::TempFile::size() const {
    return _size;
}

// PipeHandler implementation
CGIProcessor::PipeHandler::PipeHandler() : _read_fd(-1), _write_fd(-1), _closed(false) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        throw std::runtime_error("Failed to create pipe");
    }
    _read_fd = pipefd[0];
    _write_fd = pipefd[1];
}

CGIProcessor::PipeHandler::~PipeHandler() {
    close();
}

void CGIProcessor::PipeHandler::close() {
    if (!_closed) {
        if (_read_fd != -1) ::close(_read_fd);
        if (_write_fd != -1) ::close(_write_fd);
        _closed = true;
    }
}

int CGIProcessor::PipeHandler::getReadFd() const {
    return _read_fd;
}

int CGIProcessor::PipeHandler::getWriteFd() const {
    return _write_fd;
}

void CGIProcessor::PipeHandler::closeRead() {
    if (_read_fd != -1) {
        ::close(_read_fd);
        _read_fd = -1;
    }
}

void CGIProcessor::PipeHandler::closeWrite() {
    if (_write_fd != -1) {
        ::close(_write_fd);
        _write_fd = -1;
    }
}

// CGIProcessor implementation
CGIProcessor::CGIProcessor(const HTTPRequest& req) 
    : _parsedRequest(req), _env(9) {
    _path_to_script = "./ubuntu_cgi_tester";
    _env.createEnv(_parsedRequest, _path_to_script);
}

CGIProcessor::~CGIProcessor() {}

void CGIProcessor::setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        throw std::runtime_error("Failed to get file descriptor flags");
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        throw std::runtime_error("Failed to set non-blocking mode");
    }
}

void CGIProcessor::handleLargeBody(const std::vector<char>& body, int pipe_fd) {
    if (body.size() <= MEMORY_THRESHOLD) {
        // For bodies under threshold, stream directly from memory
        streamBodyToCGI(pipe_fd, body);
    } else {
        // For large bodies, use temporary file
        TempFile tempFile;
        tempFile.write(body.data(), body.size());
        tempFile.streamTo(pipe_fd);
    }
}

void CGIProcessor::streamBodyToCGI(int pipe_fd, const std::vector<char>& body) {
    size_t offset = 0;
    while (offset < body.size()) {
        size_t chunk_size = std::min(PIPE_BUF_SIZE, body.size() - offset);
        ssize_t written = write(pipe_fd, body.data() + offset, chunk_size);
        
        if (written < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            throw std::runtime_error("Write to CGI failed");
        }
        offset += written;
    }
}

HTTPResponse CGIProcessor::handleCGIRequest()
{
	HTTPResponse response;

	char * _argv[] = {const_cast<char*>(_path_to_script.c_str()), NULL}; 

	if (_parsedRequest.getMethod() == "GET") {
		return handleGETCGI(_argv);
	} else if (_parsedRequest.getMethod() == "POST") {
		return handlePOSTCGI(_argv);
	}
	return response;
}

std::string sizeToString_CGI(size_t value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

HTTPResponse CGIProcessor::handleGETCGI(char * _argv[])
{
	HTTPResponse response;
	// pipe for communication between parent and child
	int pipefd[2];
	if (pipe(pipefd) == -1) {
		throw std::runtime_error("pipe");
	}
	pid_t pid = fork();
	if (pid < 0) {
		throw std::runtime_error("fork");
	}
	else if (pid == 0)  // child
	{
		LOG_DEBUG("this is a GET child process\n");
		// GET request 
		std::cout << "PATH: " << _path_to_script << std::endl;
		close(pipefd[0]);
		dup2(pipefd[1], STDOUT_FILENO);
		close(pipefd[1]);
		if (execve(_path_to_script.c_str(), _argv, _env.getEnv()) == -1)
			LOG_ERROR("EXECVE FAILURE\n");
	}

	close(pipefd[1]);
	char buf[10000];
	int nbytes = read(pipefd[0], buf, sizeof(buf)); // don't forget to do it in a loop later
	if (nbytes == -1) {
		throw std::runtime_error("read");
	}
	close(pipefd[0]);
	int status;
	waitpid(pid, &status, 0);
	buf[nbytes] = '\0';
	// printf buf in yellow color
	// printf("\033[1;33m");
	// printf("buf: {%s}\n", buf);
	// printf("\033[0m");
	response.setStatus(200);
	response.setHeader("Content-Type", "text/html");
	response.setHeader("Content-Length", sizeToString_CGI(nbytes));
	response.setBody(buf);
	return response;
}

HTTPResponse CGIProcessor::handlePOSTCGI(char * _argv[]) {
    HTTPResponse response;
    int pipefd[2], cgi_pipe[2];
    
    if (pipe(pipefd) == -1 || pipe(cgi_pipe) == -1) {
        throw std::runtime_error("Failed to create pipes");
    }
    
    pid_t pid = fork();
    if (pid < 0) {
        close(pipefd[0]); close(pipefd[1]);
        close(cgi_pipe[0]); close(cgi_pipe[1]);
        throw std::runtime_error("Fork failed");
    }
    
    if (pid == 0) { // Child process

		fflush(stdout);
        close(pipefd[1]);  // Close write end of input pipe
        close(cgi_pipe[0]); // Close read end of output pipe
        
        if (dup2(pipefd[0], STDIN_FILENO) == -1)
			exit(1);
        if (dup2(cgi_pipe[1], STDOUT_FILENO) == -1)
			exit(1);
        
        close(pipefd[0]);
        close(cgi_pipe[1]);
        
        if (execve(_path_to_script.c_str(), _argv, _env.getEnv()) == -1) {
            exit(1);
        }
    }
    
    // Parent process
    close(pipefd[0]);
    close(cgi_pipe[1]);
    
    try {
        handleLargeBody(_parsedRequest.getBody(), pipefd[1]);
        close(pipefd[1]);
        
        std::vector<char> response_data;
        char buffer[CHUNK_SIZE];
        ssize_t bytes_read;
        
        while ((bytes_read = read(cgi_pipe[0], buffer, sizeof(buffer))) > 0) {
            response_data.insert(response_data.end(), buffer, buffer + bytes_read);
        }
        
        if (bytes_read < 0 && errno != EAGAIN) {
            throw std::runtime_error("Failed to read CGI response");
        }
        
        close(cgi_pipe[0]);
        
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            throw std::runtime_error("CGI script failed");
        }
        
        response.setStatus(200);
        response.setHeader("Content-Type", "text/html");
        response.setHeader("Content-Length", toString(response_data.size()));
        response.setBody(std::string(response_data.begin(), response_data.end()));
        
    } catch (const std::exception& e) {
        close(pipefd[1]);
        close(cgi_pipe[0]);
        throw;
    }
    
    return response;
}
