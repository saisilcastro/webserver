#include "Server.h"
#include "Stream.h"

void Server::response(int client, string path, string protocol) {
    int method = master.isMethod();
    Stream stream(this);
    string fullPath;
    static string LocationRoot;
    Location location;

    defineLocationPath(location, path, LocationRoot);
    defineFullPath(fullPath, location, extractURL(path));

    if (!transfer) {
        handleError(stream, "500", "500 Internal Server Error");
        return;
    }

    if (method == DELETE) {
        handleDeleteRequest(client, path, stream);
    } else if (method == INVALID_REQUEST) {
        handleError(stream, "405", "405 Method Not Allowed");
    } else {
        handleRequest(method, client, path, protocol, stream, location, fullPath);
    }

    _contentMaker = ContentMaker(client, protocol, "keep-alive", _contentMaker.getStatus(), stream.getStream(), stream.streamSize());
    contentMaker(_contentMaker);
}

void Server::handleError(Stream& stream, const string& errorCode, const string& errorMessage) {
    _contentMaker.setStatus(errorCode + " " + errorMessage);
    loadErrorPage(stream, errorCode);
    cout << _contentMaker.getStatus() << " STATUS" << endl;
}

void Server::handleDeleteRequest(int client, const string& path, Stream& stream) {
    string file = root + path;
    struct stat mStat;

    if (!access(file.c_str(), F_OK) && !access(file.c_str(), R_OK | W_OK | X_OK) && stat(file.c_str(), &mStat) == 0 && mStat.st_size > 0) {
        if (remove(file.c_str()) == -1) {
            handleError(stream, "405", "405 Method Not Allowed");
        }
    } else {
        handleError(stream, "403", "403 Forbidden");
    }
}

void Server::handleRequest(int method, int client, string& path, const string& protocol, Stream& stream, Location& location, const string& fullPath) {
    struct stat info;
    size_t pos = path.rfind(".");

    if (method == POST && !validatePostRequest(stream, path, protocol)) return;

    if (pos == string::npos) {
        handleDirectoryOrFile(stream, location, fullPath);
    } else {
        handleFileRequest(client, path, protocol, stream);
    }
}

bool Server::validatePostRequest(Stream& stream, string& path, const string& protocol) {
    if (maxBodySize < master.getFileLen()) {
        handleError(stream, "413", "413 Content Too Large");
        return false;
    }
    _contentMaker.setStatus(protocol + " 200 OK");
    return true;
}

void Server::handleDirectoryOrFile(Stream& stream, Location& location, const string& fullPath) {
    struct stat info;

    if (stat(fullPath.c_str(), &info) == 0) {
        if (location.data.find("index") != location.data.end()) {
            loadIndexPage(stream, location);
        } else {
            loadDirectoryPage(stream, location);
        }
    } else {
        handleError(stream, "404", "404 Not Found");
    }
}

void Server::handleFileRequest(int client, string& path, const string& protocol, Stream& stream) {
    if (!validateFileExtension(path)) {
        handleError(stream, "404", "404 Not Found");
    } else if (LocationRoot != "") {
        stream.loadFile(LocationRoot + path);
    } else {
        stream.loadFile(root + path);
    }
}

bool Server::validateFileExtension(string& path) {
    size_t pos = path.find(".");
    if (pos == string::npos) return true;

    if (path.find("?") != string::npos) {
        if ((path.find(".php") != string::npos && path[pos + 4] != '?') ||
            (path.find(".py") != string::npos && path[pos + 3])) {
            return false;
        }
        if (path.find(".php") != string::npos) path = path.substr(0, pos + 4);
        else if (path.find(".py") != string::npos) path = path.substr(0, pos + 3);
    } else {
        if ((path.find(".php") != string::npos && path.length() > pos + 4) ||
            (path.find(".py") != string::npos && path.length() > pos + 3)) {
            return false;
        }
    }
    return true;
}
