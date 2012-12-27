#include "config.h"
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>

template <typename T> std::string tostr(const T& t) { std::ostringstream os; os << t; return os.str(); }

Config::Config()
{
}

Config::~Config()
{
}

bool Config::load(const char *filename)
{
    FILE *f = fopen(filename, "rb");
    char buf[4096];
    if (!f) return false;
    data.clear();
    while ( fgets(buf, sizeof(buf), f) ) {
        char *e = strchr(buf, '=');
        if (!e) continue;
        *e = 0;
        const char *k = buf;
        const char *v = e + 1;
        data[std::string(k)] = std::string(v);
    }
    fclose(f);
    printf("Config loaded from %s\n", filename);
    return true;
}

bool Config::save(const char *filename)
{
    FILE *f = fopen(filename, "wb");
    if (!f) return false;
    std::map<std::string, std::string>::iterator iter;
    for (iter = data.begin(); iter != data.end(); iter++) {
        fprintf(f, "%s=%s\n", iter->first.c_str(), iter->second.c_str());
    }
    fclose(f);
    printf("Config saved to %s\n", filename);
    return true;
}

void Config::setString(std::string key, std::string value)
{
    data[key] = value;
}

void Config::setFloat(std::string key, float value)
{
    data[key] = tostr(value);
}

void Config::setInt(std::string key, int value)
{
    data[key] = tostr(value);
}

std::string Config::getString(std::string key)
{
    return data[key];
}

float Config::getFloat(std::string key)
{
    return atof(data[key].c_str());
}

int Config::getInt(std::string key)
{
    return atoi(data[key].c_str());
}
