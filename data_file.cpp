#include "data_file.hpp"

namespace DataSerializer
{

// Generic serializer --------------------------------------
void serialize(DataNode& node, const std::string& str)
{
    node.add_property(str);
}


void serialize(DataNode& node, const double& value)
{
    node.add_property(std::to_string(value));
}


// Generic deserializer -------------------------------------
bool deserialize(const DataNode& node, std::string& value)
{
    if (node.content_size() != 1)
        return false;

    value = node.get_property(0);
    return true;
}


bool deserialize(const DataNode& node, double& value)
{
    if (node.content_size() != 1)
        return false;

    value = atof(node.get_property(0).c_str());
    return true;
}

}