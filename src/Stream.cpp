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
        // loadErrorPage(*this, "404");
        // contentMaker(_contentMaker);
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
        else if (pid == 0) { // processo filho
            close(fd[0]);
            dup2(fd[1], STDOUT_FILENO);
            dup2(fd[1], STDERR_FILENO); // Captura stderr também
            close(fd[1]);

            if (file.find(".php") != std::string::npos) {
                const char* argv[] = {"php", file.c_str(), NULL};
                const char* envp[] = {NULL}; // Usa o ambiente atual

                execve("/usr/bin/php", const_cast<char* const*>(argv), const_cast<char* const*>(envp));
            } else if (file.find(".py") != std::string::npos) {
                const char* argv[] = {"python3", file.c_str(), NULL};
                const char* envp[] = {NULL}; // Usa o ambiente atual

                execve("/usr/bin/python3", const_cast<char* const*>(argv), const_cast<char* const*>(envp));
            }
            perror("Execução do script falhou! Certifique-se de que você tem PHP ou Python instalado.");
            exit(EXIT_FAILURE);
        } else { // processo pai
            close(fd[1]);
            char data[128];
            std::string result;
            ssize_t count;

            // Controle de timeout
            int timeout_seconds = 5; // Define o timeout desejado
            time_t start_time = time(NULL);
            bool timeout_reached = false;

            while (true) {
                // Verifica se o processo filho terminou
                int status;
                pid_t wait_result = waitpid(pid, &status, WNOHANG);

                if (wait_result == 0) { // O processo filho ainda está em execução
                    if (difftime(time(NULL), start_time) >= timeout_seconds) {
                        // Timeout alcançado, mata o processo filho
                        kill(pid, SIGKILL);
                        std::cerr << "O script demorou demais e foi terminado." << std::endl;
                        timeout_reached = true;
                        break;
                    }
                } else if (wait_result == -1) {
                    // Erro ao esperar pelo processo
                    perror("waitpid falhou");
                    break;
                } else { // O processo filho terminou
                    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
                        std::cerr << "Execução do script falhou!" << std::endl;
                    }
                    break;
                }

                // Lê dados do pipe
                count = read(fd[0], data, sizeof(data));
                if (count > 0) {
                    result.append(data, count);
                } else if (count == -1) {
                    perror("Erro ao ler do pipe");
                    break; // Lidar com erro de leitura
                } else if (count == 0) {
                    // EOF alcançado, sai do loop
                    break;
                }
            }

            close(fd[0]); // Fecha a extremidade de leitura

            if (timeout_reached) {
                // loadFile("default/defaultErrorPages/504.html");
                _contentMaker.printContent();
                return;
            }

            // Após a execução, aloca o buffer
            size = result.size();
            buffer = new char[size];
            if (!buffer)
                throw std::runtime_error("Falha na alocação do buffer!");

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
