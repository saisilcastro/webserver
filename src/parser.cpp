/* #include "Server.h"

static const std::string WHITESPACE = " \t\n\r\f\v";
static const std::string COMMENT = "#";
static const std::string OPEN_BRACKET = "{";
static const std::string CLOSE_BRACKET = "}";

static bool skipEmptyLines(std::string& line, int &bracketCount) {
    std::string::size_type start = line.find_first_not_of(WHITESPACE);
    std::string::size_type end = line.find_last_not_of(WHITESPACE);
    if (start == std::string::npos || end == std::string::npos) {
        return true;
    }
    line = line.substr(start, end - start + 1);
    if (line.empty() || line[0] == COMMENT[0]) {
        return true;
    }
    if (line.find("server {") != std::string::npos) {
        bracketCount++;
        return true;
    }
    if (line.find(OPEN_BRACKET) != std::string::npos) {
        bracketCount++;
    }
    return false;
}

static void validateSemicolon(const std::string& line) {
    if (line.find(";") == std::string::npos && 
        line.find(OPEN_BRACKET) == std::string::npos &&
        line.find(CLOSE_BRACKET) == std::string::npos) {
        throw std::runtime_error("Missing semicolon");
    }
}

static void processDirective(const std::string& line, Server& config, Location& currentLocation, bool inLocation) {
    std::string::size_type spacePos = line.find(" ");
    std::string directive = line.substr(0, spacePos);
    std::string value = line.substr(spacePos + 1);

    if (directive == "listen") {
        config.setPort(value);
    }
    else if(directive == "max_body_size") {
        try
        {
            config.setMaxBodySize(value);
        }
        catch(const std::exception& e)
        {
            throw std::runtime_error(e.what());
        }
    }
    else if(directive == "server_name") {
        config.setHost(value);
    }
    else if (directive == "root") {
        if(value[0] == '.' && value[1] == '/')
            value = value.substr(2);
        if(value[0] == '/')
            value = value.substr(1);
        if (inLocation) {
            currentLocation.directives[directive] = value;
        } else {
            config.setRoot(value);
        }
    }
    else if (directive == "autoindex") {
        if (inLocation) {
            currentLocation.directives[directive] = value;
        } else {
            std::cerr << "Error: Unknown directive outside location: " << directive << std::endl;
            throw std::runtime_error("Unknown directive outside location");
        }
    }
    else {
        if (inLocation) {
            if(directive == "error_page") {
                std::string error = value.substr(0, value.find(" "));
                std::string path = value.substr(value.find(" ") + 1);
                config.addErrorPage(error, path, currentLocation);
            }
            else
                currentLocation.directives[directive] = value;
        } else {
            if(directive == "error_page") {
                std::string error = value.substr(0, value.find(" "));
                std::string path = value.substr(value.find(" ") + 1);
                config.addErrorPage(error, path);
            }
            else
            {
                std::cerr << "Error: Unknown directive outside location: " << directive << std::endl;
                throw std::runtime_error("Unknown directive outside location");
            }
        }
    }
}

static void erase(std::string &line) {
    if(line.find(";") != std::string::npos)
        line.erase(line.find(";"), 1);
    if(line.find(OPEN_BRACKET) != std::string::npos)
        line.erase(line.find(OPEN_BRACKET), 1);
    if(line.find(CLOSE_BRACKET) != std::string::npos)
        line.erase(line.find(CLOSE_BRACKET), 1);
    line = ft_strip(line);
}

void parser(const char *file, Server& config) {
    std::ifstream in(file);
    if (!in.is_open()) {
        throw std::runtime_error("Error: Could not open file. Using default settings.");
    }

    std::string line;
    Location currentLocation;
    bool inLocation = false;
    int bracketCount = 0;

    while (std::getline(in, line)) {
        if (skipEmptyLines(line, bracketCount))
            continue;
        if (line.find(CLOSE_BRACKET) != std::string::npos) {
            bracketCount--;
            if (inLocation) {
                config.addLocation(currentLocation);
                currentLocation = Location();
                inLocation = false;
            }
            continue;
        }
        try {
            validateSemicolon(line);
        } catch (const std::exception& e) {
            throw std::runtime_error("Error: " + std::string(e.what()) + ". Using default settings.");
        }
        erase(line);
        if (line.find("location") != std::string::npos) {
            inLocation = true;
            currentLocation.path = line.substr(line.find(" ") + 1);
            if (!currentLocation.path.empty() && currentLocation.path[currentLocation.path.size() - 1] == OPEN_BRACKET[0]) {
                currentLocation.path.erase(currentLocation.path.size() - 1);
            }
        } else {
            processDirective(line, config, currentLocation, inLocation);
        }
    }
    in.close();
    if (bracketCount != 0) {
        throw std::runtime_error("Error: Unmatched opening bracket. Using default settings.");
    }
}
 */