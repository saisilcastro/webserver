#include "Stream.h"
#define BUFFER_SIZE 65536

Stream::Stream(void) : buffer(NULL), size(0) {}

Stream::Stream(string file) : buffer(NULL), size(0) {
    if (!file.empty())
        loadFile(file);
    else
        saveFile(file);
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

void Stream::loadFile(std::string file) {
    if (access(file.c_str(), F_OK) == -1) {
        loadFile("default/defaultErrorPages/404.html");
        return;
    }
    
    if (file.find(".php") == std::string::npos && file.find(".py") == std::string::npos) {
        std::ifstream in(file.c_str(), std::ios::binary | std::ios::ate);
        
        if (!in.is_open() || in.bad() || in.fail()) {
            return;
        }
        
        size = in.tellg();
        in.seekg(0, std::ios::beg);
        buffer = new char[size];
        if (!buffer)
            return;
        
        in.read(reinterpret_cast<char *>(buffer), size);
        in.close();
    } else {
        int fd[2];
        
        if (pipe(fd) == -1)
            throw std::runtime_error("pipe() failed!");
        
        pid_t pid = fork();
        if (pid == -1)
            throw std::runtime_error("fork() failed!");
        else if (pid == 0) { // child process
            close(fd[0]);
            dup2(fd[1], STDOUT_FILENO);
            dup2(fd[1], STDERR_FILENO); // Capture stderr as well
            close(fd[1]);
            
            if (file.find(".php") != std::string::npos) {
                const char* argv[] = {"php", file.c_str(), NULL};
                const char* envp[] = {NULL}; // Use the current environment
                
                execve("/usr/bin/php", const_cast<char* const*>(argv), const_cast<char* const*>(envp));
            } else if (file.find(".py") != std::string::npos) {
                const char* argv[] = {"python3", file.c_str(), NULL};
                const char* envp[] = {NULL}; // Use the current environment
                
                execve("/usr/bin/python3", const_cast<char* const*>(argv), const_cast<char* const*>(envp));
            }
            perror("Script execution failed! Make sure that you have PHP or Python installed.");
            exit(EXIT_FAILURE);
        } else { // parent process
            close(fd[1]);
            char data[128];
            std::string result;
            ssize_t count;
            
            // Timeout control
            int timeout_seconds = 5; // Define the desired timeout
            time_t start_time = time(NULL);
            bool timeout_reached = false;
            
            while (true) {
                // Check if the child process has finished
                int status;
                pid_t wait_result = waitpid(pid, &status, WNOHANG);
                
                if (wait_result == 0) { // Child process is still running
                    if (difftime(time(NULL), start_time) >= timeout_seconds) {
                        // Timeout reached, kill the child process
                        kill(pid, SIGKILL);
                        std::cerr << "The script took too long and was terminated." << std::endl;
                        timeout_reached = true;
                        break;
                    }
                } else if (wait_result == -1) {
                    // Error waiting for the process
                    perror("waitpid failed");
                    break;
                } else { // The child process has finished
                    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
                        std::cerr << "Script execution failed!" << std::endl;
                    }
                    break;
                }
                
                // Read data from the pipe
                count = read(fd[0], data, sizeof(data));
                if (count > 0) {
                    result.append(data, count);
                }
            }
            
            close(fd[0]); // Close the read end

            if (timeout_reached) {
                loadFile("default/defaultErrorPages/405.html");
                return;
            }

            // After execution, allocate the buffer
            size = result.size();
            buffer = new char[size];
            if (!buffer)
                throw std::runtime_error("Buffer allocation failed!");

            memcpy(buffer, result.c_str(), size);
        }
    }
}

void	Stream::saveFile(string file) {
    if (file.empty())
        return;
    ofstream out(file.c_str(), std::ofstream::out | std::ofstream::binary);

    if (!out.is_open() || out.bad() || out.fail())
        return;
    out.write(reinterpret_cast<char*>(buffer), size);
    if (!out) {
        cerr << "could not write file\n";
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
