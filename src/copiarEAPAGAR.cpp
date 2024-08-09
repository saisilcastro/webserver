/* void Server::response(int client, string path, string protocol) {
    size_t pos = path.rfind(".");
    Stream stream("");
    string status = " 200 OK";

    if (transfer) {
        if (master.isMethod() != DELETE) {
            mimeMaker(path);

            if (pos == string::npos) {
                string index = findDirectiveValue("index");

                if (MaxBodySize < master.getFileLen() && master.getFileLen() > 0) {
                    stream.loadFile(root + '/' + "413.html");
                    status = " 413 Content Too Large";
                } else if (!index.empty()) {
                    stream.loadFile(root + '/' + index);
                } else {
                    struct stat s;
                    if (stat((root + path).c_str(), &s) == -1) {
                        stream.loadFile(root + "/404.html");
                    } else {
                        stream.loadFile(root + "/index2.html");
                    }
                }
            } else {
                if (master.isMethod() == POST && MaxBodySize > master.getFileLen()) {
                    contentMaker(client, protocol + " 200 OK", "keep-alive", stream.getStream(), stream.streamSize());
                    path = "/413.html";
                    status = " 413 Content Too Large";
                } else if ((pos = path.find(".")) != string::npos) {
                    if (path.find("?") != string::npos) {
                        if ((path.find(".php") != string::npos && path[pos + 4] != '?') ||
                            (path.find(".py") != string::npos && path[pos + 3])) {
                            path = "/404.html";
                            status = " 404 Not Found";
                        } else {
                            if (path.find(".php") != string::npos)
                                path = path.substr(0, pos + 4);
                            else if (path.find(".py") != string::npos)
                                path = path.substr(0, pos + 3);
                        }
                    } else {
                        if ((path.find(".php") != string::npos && path.length() > pos + 4) ||
                            (path.find(".py") != string::npos && path.length() > pos + 3)) {
                            path = "/404.html";
                            status = " 404 Not Found";
                        }
                    }
                }

                stream.loadFile(root + path);
            }
        } else {
            struct stat mStat;
            string file = root + path;

            if (!access(file.c_str(), F_OK) && !access(file.c_str(), R_OK | W_OK | X_OK)) {
                if (!stat(file.c_str(), &mStat) && mStat.st_size > 0) {
                    if (remove(file.c_str()) == -1) {
                        status = " 405 Method Not Allowed";
                        stream.loadFile(root + "/405.html");
                    }
                }
            } else {
                status = " 403 Forbidden";
                stream.loadFile(root + "/403.html");
            }
        }
    } else {
        status = " 500 Internal Server Error";
        stream.loadFile(root + "/500.html");
    }

    contentMaker(client, protocol + status, "keep-alive", stream.getStream(), stream.streamSize());
}
 */