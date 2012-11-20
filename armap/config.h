#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <map>
#include <string>

class Config {

    public:
        Config();
        ~Config();
        bool load(const char *filename);
        bool save(const char *filename);
        void setString(std::string key, std::string value);
        void setFloat(std::string key, float value);
        void setInt(std::string key, int value);

        std::string getString(std::string key);
        float getFloat(std::string key);
        int getInt(std::string key);

    private:
        const char *filename;
        std::map<std::string , std::string> data;
};

#endif
