#ifndef CONFIG_HPP_
#define CONFIG_HPP_

#include <string>
#include <vector>

class Config {
public:
    Config(std::string filename);

    std::string Get(const std::string& key) const;
    bool Has(const std::string& key) const;

    std::vector<std::string>& autostart_rules();

private:
    void Parse();

    std::vector<std::string> autostart_rules_;
};

#endif
