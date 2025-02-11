#include "Stream.h"
#include "Server.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <filesystem>

using namespace std;

void Server::setError(const string& error, const string& msg, bool& readyToWrite){
    cerr << RED << msg << RESET << endl;
    master.setMethod(error);
    readyToWrite = true;
}

void Server::checkServerName(Protocol &master){
    string client_host = master.getHost();
    string server_host = host;

    if(client_host != "localhost" && server_host != "" && client_host != server_host)
        master.setMethod("INVALID_HOST");
}

string Server::createPacket(int client) {
    fd_set read_fd, write_fd;
    struct timeval timeout;
    bool packetCreated = false;
    bool creating = true;
    bool readyToWrite = false;
    bool receivingPost = false;
    bool createFile = false;
    char buffer[65535];
    size_t currentSize = 0;
    size_t writtenByte = 0;
    size_t dataLen;
    size_t offset = 0;
    int piece;
    string path("");
    ofstream out;
    vector<char> tmpHeader;
    master.reset();

    while (creating) {
        FD_ZERO(&read_fd);
        FD_ZERO(&write_fd);
        if(master.isMethod() != GET && master.isMethod() != POST && master.isMethod() != DELETE && master.isMethod() != EMPTY && !readyToWrite){
            readyToWrite = true;
        }
        if (!readyToWrite) {
            FD_SET(client, &read_fd);
        } else {
            FD_SET(client, &write_fd);
        }

      if (!packetCreated) {
			timeout.tv_sec = 10;
			timeout.tv_usec = 0;
		}
		else {
            timeout.tv_sec = 0;
            timeout.tv_usec = master.getFileLen() / 65535;
		}

        int ready = select(client + 1, &read_fd, &write_fd, NULL, &timeout);
        if (ready < 0) {
            setError("INTERNAL_SERVER_ERROR", "Error during select", readyToWrite);
        } else if (ready == 0) {
            readyToWrite = true;
            FD_SET(client, &write_fd);
        }

        if (!readyToWrite && FD_ISSET(client, &read_fd)) {
            memset(buffer, 0, sizeof(buffer));
            piece = recv(client, buffer, sizeof(buffer), 0);
            if (piece > 0) {
                currentSize += piece;
                if (!createFile) {
                    packetCreated = true;
                    if (!master.extract(buffer, this)) {
                        tmpHeader.insert(tmpHeader.end(), buffer, buffer + piece);
                        if (!master.extract(&tmpHeader[0], this))
                            continue;
                        else {
                            memcpy(buffer, &tmpHeader[0], tmpHeader.size());
                            tmpHeader.clear();
                            createFile = true;
                        }
                    }
                    else
                        createFile = true;
                    if (master.getFileLen() && master.getFileLen() <= maxBodySize && !master.getFileName().empty()) {
                        std::string upload = root + "/upload";
                        struct stat st;
                        if (stat(upload.c_str(), &st) == -1)
                            mkdir(upload.c_str(), 0766);
                        if(access(upload.c_str(), F_OK | W_OK) == -1){
                            setError("INTERNAL_SERVER_ERROR", "Error during access", readyToWrite);
                            continue;
                        }
                        path = upload + "/" + master.getFileName();
                        std::cout << path << std::endl;
                        if (!master.getFileName().empty()) {
                            out.open(path.c_str(), ios::out | ios::binary);
                            receivingPost = true;
                        }
                        if (!out.is_open()) {
                            cerr << RED << "Failed to open file: " << path << RESET << endl;
                            continue;
                        }
                        offset = static_cast<size_t>(master.getHeaderLen());
                    }
                }
                if(master.isMethod() != GET && master.isMethod() != POST && master.isMethod() != DELETE){
                    cout << RED << "Entrou\n" << RESET;
                    continue;
                }
                dataLen = piece - offset;
                size_t remainingLen = static_cast<size_t>(master.getFileLen()) - writtenByte;

                if (remainingLen <= dataLen)
                    dataLen = remainingLen;
                if (dataLen > 0) {
                    char *sub = strstr(buffer + offset, master.getBoundary().c_str());
                    if (sub)
                        dataLen -= master.getBoundary().length() + 8;
                    out.write(buffer + offset, dataLen);
                    writtenByte += dataLen;

                    if (writtenByte > maxBodySize) // check if max body size is exceeded
                        setError("ENTITY_TOO_LARGE", "Entity too large", readyToWrite);

                    cout << BLUE "Uploaded " << writtenByte << " of " << master.getFileLen() << RESET << endl;
                }

                if (writtenByte >= master.getFileLen()) {
                    cout << "File transfer complete.\n";
                    readyToWrite = true;
                }
                offset = 0;
            } else if (piece == 0 || writtenByte >= master.getFileLen()) {
                cout << "Received entire file.\n";
                readyToWrite = true;
            } else {
                setError("INTERNAL_SERVER_ERROR", "Error during recv", readyToWrite);
                break;
            }
        }

        if (readyToWrite && FD_ISSET(client, &write_fd)) {
            if(master.isMethod() == POST && !receivingPost && master.getContentBody().length() > maxBodySize){
               master.setMethod("ENTITY_TOO_LARGE");
               cerr << RED << "Entity too large" << RESET << endl;
            }

            out.close();
            if(master.isMethod() == ENTITY_TOO_LARGE || (master.isMethod() == INVALID_REQUEST && master.getFileLen() != 0)){
                remove(path.c_str());
                usleep(master.getFileLen() / 1000);
            }
            response(client, master.getPath(), master.getType());
            break;
        }
    }
    return "";
}

void Server::loadError(int client, string filePath, const string& errorCode)
{
    trim(filePath);
    ifstream file(filePath.c_str());

    string errorPage;
    string statusCode;
    
    if (access(filePath.c_str(), F_OK) == -1) {
        errorPage = getPageDefault("404");
        statusCode = "404 Not Found";
    }
    else if (access(filePath.c_str(), R_OK) == -1) {
        errorPage = getPageDefault("403");
        statusCode = "403 Forbidden";
    }
    else if (!file) {
        errorPage = getPageDefault("500");
        statusCode = "500 Internal Server Error";
    }

    if (!statusCode.empty()) {
        loadError(client, errorPage, statusCode);
        return;
    }

    string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());

    ostringstream responseStream;
    responseStream << "HTTP/1.1 " << errorCode << " Error\r\n"
                   << "Connection: keep-alive\r\n"
                   << "Content-Type: text/html\r\n"
                   << "Content-Length: " << content.size() << "\r\n\r\n"
                   << content;
    
    string response = responseStream.str();

    ssize_t send_return = send(client, response.c_str(), response.size(), 0);
    if (send_return == -1){
        cerr << RED << "Error sending response" << RESET << endl;
    }
    else if(send_return == 0){
        cerr << RED << "Connection closed" << RESET << endl;
    }
}

void Server::handleDelete(Stream &stream) {
    struct stat mStat;
    string file = root + master.getPath();
    if (stat(file.c_str(), &mStat) != 0)
        _statusCode = " 404 Not Found";
    else if (access(file.c_str(), R_OK | W_OK) != 0)
        _statusCode = " 403 Forbidden";
    else if (remove(file.c_str()) != 0)
        _statusCode = " 500 Internal Server Error";
    if(_statusCode != " 200 OK")
        stream.loadFile(getPageDefault(_statusCode.substr(1, 3)));
}

void Server::handleGetPost(string& path, Stream &stream)
{   
    static string LocationRoot;
    struct stat info;
    Location location;
    string fullPath;
    size_t pos = path.rfind(".");
    mimeMaker(path);
    defineLocationPath(location, path, LocationRoot);
    defineFullPath(fullPath, location, extractURL(path));
    if(stat(fullPath.c_str(), &info) == 0)
    {
        if(pos != string::npos){
            if(LocationRoot != "")
                stream.loadFile(LocationRoot + path);
            else
                stream.loadFile(root + path);
        }
        else if(location.data.find("index") != location.data.end())
            loadIndexPage(stream, location);
        else if(S_ISDIR(info.st_mode))
            loadDirectoryPage(stream, fullPath);
    }
    else {
        _statusCode = " 404 Not Found";
        stream.loadFile(getPageDefault("404"));
    }
}

void Server::contentMaker(int client, string protocol, string connection, string buffer)
{
    if(_statusCode[0] != ' ')
        _statusCode = ' ' + _statusCode;
    ostringstream responseStream;
    responseStream << protocol + _statusCode << "\r\n"
                   << "Connection: " << connection << "\r\n"
                   << "Content-Type: text/html\r\n"
                   << "Content-Length: " << buffer.size() << "\r\n\r\n"
                   << buffer;

    string response = responseStream.str();
    ssize_t send_return = send(client, response.c_str(), response.size(), 0);
    if (send_return == -1)
        cerr << RED << "Error sending response" << RESET << endl;
    else if(send_return == 0)
        cerr << RED << "Connection closed" << RESET << endl;
}

void Server::response(int client, string path, string protocol){
    static string LocationRoot;
    Stream stream(this, path);
    Location location;
    string fullPath;
    _statusCode = " 200 OK";

    defineLocationPath(location, path, LocationRoot);
    defineFullPath(fullPath, location, extractURL(path));
    if(HandleErrors(protocol, stream) == false){
        if(master.isMethod() == GET || master.isMethod() == POST)
            handleGetPost(path, stream);
        else
            handleDelete(stream);
    }
    contentMaker(client, protocol, "keep-alive", stream.getBufferString());
}

void Server::execute(int socket) {
    int client = accept(socket, NULL, NULL);
    if (client == -1)
        return ;
    fcntl(client, F_SETFL, O_NONBLOCK);
    createPacket(client);
    close(client);
}

void Server::run(void) {
	int		sock;

	if ((sock = serverSocket(SOCK_STREAM)) == -1)
		exit(-1);
	while (1)
        execute(sock);
}

void    Run(Server *server, int max) {
    int sock[max];
    for (int i = 0; i < max; i++) {
        if ((sock[i] = server[i].serverSocket(SOCK_STREAM)) == -1)
            exit(-1);
    }
    while (1) {
        for (int i = 0; i < max; i++)
            server[i].execute(sock[i]);
    }
}
