#pragma once
#include <string>
#include <vector>
#include <array>
#include <variant>
#include <cstdint>

class DataType {
public:
    using Value = std::variant<
            std::monostate,
            int32_t,
            std::string,
            bool
    >;

    enum class Type {
        INT32,
        STRING,
        BYTES,
        BOOLEAN
    };

    DataType(Type type) : type_(type) {}

    Type getType() const { return type_; }

    static std::string typeToString(Type type) {
        switch (type) {
            case Type::INT32: return "INT32";
            case Type::STRING: return "STRING";
            case Type::BYTES: return "BYTES";
            case Type::BOOLEAN: return "BOOLEAN";
            default: return "UNKNOWN";
        }
    }

private:
    Type type_;
};
