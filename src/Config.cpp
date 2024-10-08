#include "Config.h"

Config::Config(void) {}

void locationPrint(vector<Location>::iterator local) {
    cout << "\npath: " << local->path << "\n";
    for (map<string, string>::iterator data = local->data.begin(); data != local->data.end(); ++data)
        cout << "name: " << data->first << " value: " << data->second << "\n";
}

size_t bodySize(string size) {
    int	iSize = std::atoi(size.c_str());

    if (iSize == 0)
        throw std::runtime_error("Invalid body size format, using default file .conf");
    if (size.find_first_of("Gg") != string::npos)
        return (size_t)iSize * 1024 * 1024 * 1024;
    else if (size.find_first_of("Mm") != string::npos)
        return (size_t)iSize * 1024 * 1024;
    else if (size.find_first_of("Kk") != string::npos)
        return (size_t)iSize * 1024;
    else if (size.find_first_not_of("0123456789") == string::npos)
        return (size_t)iSize;
    else
        throw std::runtime_error("Invalid body size, using default file .conf");
}

static void extractInfo(string line, ServerInfo & one, Location & local, int bracket) {
    std::string::size_type start = line.find_first_not_of(" \t\n\r\f\v");
    std::string::size_type end = line.find_last_not_of(" \t\n\r\f\v");
    string keyword[] = {"server_name ", "root ", "listen ", "max_body_size ", "error_page ", "location "};

    if (start != string::npos && end != string::npos) {
        if (bracket == 2) {
            if (line.find(";") != string::npos) {
                size_t first = line.find_first_not_of(" \t\n\r\f\v");
                size_t last = line.substr(first).find_first_of(" \t\n\r\f\v");
                string name = line.substr(first, last);

                line = line.substr(line.find(name) + name.length());
                first = line.find_first_not_of(" \t\n\r\f\v");
                if (line.find(";") != string::npos) {
                    line.erase(line.find(";", 1));
                    string value = line.substr(first);
                    local.data.insert(make_pair(name, value));
                }
            }
            if (line.find("}") != string::npos) {
                one.location.push_back(local);
                local.path.clear();
                local.data.clear();
            }
        }
        for (int i = 0; i < 6; i++) {
            size_t pos = line.find(keyword[i]);
            if (pos != string::npos) {
                line = line.substr(pos + keyword[i].length());
                if (keyword[i] == "server_name ")
                    one.name = line.substr(0, line.find(";"));
                if (keyword[i] == "root ")
                    one.root = line.substr(0, line.find(";"));
                if (keyword[i] == "listen ")
                    one.port = line.substr(0, line.find(";"));
                if (keyword[i] == "max_body_size ")
                    one.maxBodySize = bodySize(line.substr(0, line.find(";")));
                if (keyword[i] == "error_page ")
                {
                    string name = line.substr(0, line.find_first_of(" \t\n\r\f\v"));
                    string value = line.substr(line.find_last_of(" \t\n\r\f\v"), line.substr(line.find_last_of(" \t\n\r\f\v")).find(";"));
                    one.error.insert(make_pair(name, value));
                }
                if (keyword[i] == "location ")
                {
                    local.path = line.substr(0, line.find("{"));
                    local.path = ft_strip(local.path);
                }
            }
        }
    }	
}

Config::Config(char *file) {
    ifstream 	in(file);
    int		 	bracket = 0;
    string	 	line;
    ServerInfo	one;
    Location 	local;

    if (!in.is_open() || in.fail())
        throw std::runtime_error("Error while trying to open file!");
    while (getline(in, line)) {
        if (line.find("{") != string::npos)
            bracket++;
        extractInfo(line, one, local, bracket);
        if (line.find("}") != string::npos) {
            bracket--;
            if (bracket == 0) {
                if(one.root.empty() || one.port.empty())
                    throw std::runtime_error("Invalid server configuration, using default file .conf");
                info.push_back(one);
                one.name.clear();
                one.root.clear();
                one.port.clear();
                one.location.clear();
            }
        }
    }
    in.close();
}

vector<ServerInfo> Config::infoGet(void) {
    return info;
}

string Config::getName(int pos) {
    int i = 0;
    for (vector<ServerInfo>::iterator it = info.begin(); it != info.end(); ++it) {
        if (i == pos) {
            cout << it->name << endl;
            return it->name;
        }
        i++;
    }
    return "localhost";
}

bool Config::isPortRepeated(void) {
    for (vector<ServerInfo>::iterator one = info.begin(); one != info.end(); ++one) {
        for (vector<ServerInfo>::iterator two = one + 1; two != info.end(); ++two) {
            if (one->port == two->port)
                return true;
        }
    }
    return false;
}

void Config::print(void) {
    for (vector<ServerInfo>::iterator it = info.begin(); it != info.end(); ++it) {
        cout << "host_name: " << it->name << endl;
        cout << "root: " << it->root << endl;
        cout << "port: " << it->port << endl;
        cout << "MaxBodySize: " << it->maxBodySize << endl;
        for (map<string, string>::iterator error = it->error.begin(); error != it->error.end(); ++error)
            cout << "ErrorName: " << error->first << "ErrorPath: " << error->second << endl;
        for (vector<Location>::iterator location = it->location.begin(); location != it->location.end(); ++location)
            locationPrint(location);
        cout << "}\n\n";	
    }
}

Config::~Config(){}
