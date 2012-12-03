#pragma once

#include <string>

class Type
{
public:
    enum { INT, DOUBLE, STRING, LIST, UNKNOWN, VOID };
    Type() { id = UNKNOWN; }
    Type(int id): id(id) {}
    int id;

    std::string name()
    {
        switch(id)
        {
        case INT: return "Int";
        case DOUBLE: return "Double";
        case STRING: return "String";
        case VOID: return "Void";
        case UNKNOWN:
        default: return "unknown";
        }
    }
};
