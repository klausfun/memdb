#include "command_registry.h"

void CommandRegistry::register_command(const std::string& command_name, CommandFactory factory) {
    registry[command_name] = factory;
}

std::unique_ptr<Command> CommandRegistry::get_command(const std::string& command_name) {
    auto it = registry.find(command_name);
    if (it != registry.end()) {
        return it->second();
    }
    return nullptr;
}
