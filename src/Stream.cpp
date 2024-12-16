#include "Stream.h"
#define BUFFER_SIZE 65536

Stream::Stream(void) : buffer(NULL), size(0) {}

Stream::Stream(string file) : buffer(NULL), size(0) {
    if (!file.empty())
        loadFile(file);
    else
        saveFile(file);
}

Stream::Stream(Server *server, string path) : buffer(NULL), _bufferString(""), size(0) {
    ServerRef = server;
    this->path = path;
}

void    Stream::createStream(void *data, size_t len) {
    size = len;
    buffer = new char[size];
    if (!buffer)
        return ;
    memcpy(buffer, data, size);
    ((char *)buffer)[size - 1] = '\0';
}

void	*Stream::getStream(void) {
    return buffer;
}

int		Stream::streamSize(void) {
    return size;
}

bool Stream::handleErrors(string file){
    trim(file);
    string error;

    if (access(file.c_str(), F_OK) == -1) 
        error = " 404 Not Found";
    else if (access(file.c_str(), R_OK) == -1)
        error = " 403 Forbidden";
    else if(access(file.c_str(), X_OK) == -1 && (file.find(".php") != string::npos || file.find(".py") != string::npos))
        error = " 403 Forbidden";

    if(!error.empty())
    {
        cout << file << endl;
        cout << "chamado aqui: " << error << endl;
        cout << "file: " << file << endl;
    }

    if (!error.empty()) {
        loadFile(ServerRef->getPageDefault(error.substr(1, 3)));
        ServerRef->setStatusCode(error);
        return true;
    }
    return false;
}


void Stream::handleFile(string& file){
    trim(file);
    ifstream infile(file.c_str());
    if(!infile.is_open())
    {
        cerr << RED << "Could not open file\n" << RESET;
        throw(string(" 500 Internal Server Error"));
    }

    _bufferString = string((istreambuf_iterator<char>(infile)), istreambuf_iterator<char>());
    if (_bufferString.empty())
    {
        cerr << RED << "Could not read file\n" << RESET;
        throw(string(" 500 Internal Server Error"));
    }
}

string Stream::getQueryString(){
    if(path.find("?") != string::npos){
        size_t pos = path.find("?");
        return path.substr(pos + 1);
    }
    return "";
}

void Stream::handleCGI(string& file) {
    int fd[2];
    if (pipe(fd) == -1) {
        perror("Pipe creation failed");
        throw(string(" 500 Internal Server Error"));
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("Fork failed");
        throw(string(" 500 Internal Server Error"));
    }

    else if (pid == 0) {
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        dup2(fd[1], STDERR_FILENO);
        close(fd[1]);

        string body = ServerRef->getContentBody();
        int pipe_stdin[2];
        if (pipe(pipe_stdin) == -1) {
            perror("Pipe creation for STDIN failed");
            exit(EXIT_FAILURE);
        }

        if (fork() == 0) {
            close(pipe_stdin[0]);
            ssize_t return_write = write(pipe_stdin[1], body.c_str(), body.size());
            if(return_write == 0 && body.size() != 0){
                cerr << RED << "Error writing to pipe" << RESET << endl;
                exit(EXIT_FAILURE);
            }
            else if(return_write == -1){
                cerr << RED << "Error writing to pipe" << RESET << endl;
                exit(EXIT_FAILURE);
            }
            close(pipe_stdin[1]);
            exit(0);
        } else {
            close(pipe_stdin[1]);
            dup2(pipe_stdin[0], STDIN_FILENO);
            close(pipe_stdin[0]);
        }

        char **args = new char*[3];
        if (file.find(".php") != string::npos) {
            args[0] = strdup("/usr/bin/php");
            args[1] = strdup(file.c_str());
            args[2] = NULL;
        } else if (file.find(".py") != string::npos) {
            args[0] = strdup("/usr/bin/python3");
            args[1] = strdup(file.c_str());
            args[2] = NULL;
        } else {
            cerr << RED << "Unsupported script type" << RESET << endl;
            exit(EXIT_FAILURE);
        }

        string request_method = std::string("REQUEST_METHOD=") + (ServerRef->getMethod() == GET ? "GET" : "POST");  
        string query_string = ServerRef->getMethod() == GET ? "QUERY_STRING=" + getQueryString() : "";
        string content_length = ServerRef->getMethod() == POST ? "CONTENT_LENGTH=" + std::to_string(body.size()) : "";

        char* envp[] = {
            (char*)request_method.c_str(),
            (char*)(ServerRef->getMethod() == GET ? query_string.c_str() : content_length.c_str()),
            NULL
        };

        execve(args[0], args, envp);

        free(args[0]);
        free(args[1]);
        delete[] args;

        perror("Script execution failed");
        exit(EXIT_FAILURE);
    }

    else {
        close(fd[1]);
        char data[128];
        _bufferString.clear();
        ssize_t count;

        int timeout_seconds = 5;
        time_t start_time = time(NULL);

        while (true) {
            int status;
            pid_t wait_result = waitpid(pid, &status, WNOHANG);

            if (wait_result == 0) {
                if (difftime(time(NULL), start_time) >= timeout_seconds) {
                    kill(pid, SIGKILL);
                    cerr << RED << "The script took too long and was terminated." << RESET << endl;
                    throw(string(" 504 Gateway Timeout"));
                }
            } else if (wait_result == -1) {
                perror("waitpid failed");
                throw(string(" 500 Internal Server Error"));
            } else {
                if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
                    cerr << RED << "Script execution failed!" << RESET << endl;
                    throw(string(" 500 Internal Server Error"));
                }
                break;
            }

            count = read(fd[0], data, sizeof(data));
            if (count > 0) {
                _bufferString.append(data, count);
            } else if (count == -1) {
                perror("Error reading from pipe");
                throw(string(" 500 Internal Server Error"));
            } else if (count == 0) {
                break;
            }
        }

        close(fd[0]);
    }
}


void Stream::loadFile(string file) {
    _bufferString.clear();
    try{
        if(!handleErrors(file)){
            if (file.find(".php") == string::npos && file.find(".py") == string::npos) {
                handleFile(file);
            } else {
                handleCGI(file);
            }
        }
    } catch (string& e) {
        loadFile(ServerRef->getPageDefault(e.substr(1, 3)));
        ServerRef->setStatusCode(e);
    }   
}

void	Stream::saveFile(string file) {
    if (file.empty())
        return;
    ofstream out(file.c_str(), ofstream::out | ofstream::binary);

    if (!out.is_open() || out.bad() || out.fail())
        return;
    out.write(reinterpret_cast<char*>(buffer), size);
    if (!out) {
        cerr << RED << "could not write file\n" << RESET;
        return ;
    }
    out.close();
}

Stream & Stream::operator = (Stream & pointer) {
    if (this != &pointer) {
        buffer = reinterpret_cast<char*>(pointer.buffer);
        size = pointer.size;
    }
    return *this;
}

Stream::~Stream(void) {
    if (buffer)
        delete[] reinterpret_cast<char *>(buffer);
}
