#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <functional>
#include "command.hpp"

class CommandRegistry {
public:
    using CommandFactory = std::function<std::unique_ptr<Command>()>;

    void register_command(const std::string& command_name, CommandFactory factory);
    std::unique_ptr<Command> get_command(const std::string& command_name);

private:
    std::unordered_map<std::string, CommandFactory> registry;
};
