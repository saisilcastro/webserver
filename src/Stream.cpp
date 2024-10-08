#include "Stream.h"
#define BUFFER_SIZE 65536

Stream::Stream(void) : buffer(NULL), size(0) {}

Stream::Stream(string file) : buffer(NULL), size(0) {
    if (!file.empty())
        loadFile(file);
    else
        saveFile(file);
}

Stream::Stream(Server* server) : buffer(NULL), size(0), serverRef(server) { loadFile("");}

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
    if(access(file.c_str(), F_OK) == -1)
    {
        loadFile("www/defaultPages/404.html");
        serverRef->getContentMaker().setStatus(" 404 Not Found");
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
            while ((count = read(fd[0], data, sizeof(data))) > 0)
                result.append(data, count);
            close(fd[0]); // Close read end

            // Wait for child process to finish
            int status;
            waitpid(pid, &status, 0);
            if (status != 0)
                cout << "Script execution failed!" << endl;

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
