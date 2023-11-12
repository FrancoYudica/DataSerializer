#include <iostream>
#include "data_file.hpp"



struct Vector3
{
    float x, y, z;
};


std::ostream& operator<<(std::ostream& stream, const Vector3& vec)
{
    return stream << "[" << vec.x << ", " << vec.y << ", " << vec.z << "]";
}


namespace DataSerializer
{

// Implements custom serialization and deserialization
bool deserialize(const DataNode& node, Vector3& value)
{
    if (node.content_size() != 3)
        return false;

    value.x = atof(node.get_property(0).c_str());
    value.y = atof(node.get_property(1).c_str());
    value.z = atof(node.get_property(2).c_str());
    return true;
}


void serialize(DataNode& node, const Vector3& value)
{
    node.add_property(std::to_string(value.x));
    node.add_property(std::to_string(value.y));
    node.add_property(std::to_string(value.z));
}

}

using namespace DataSerializer;

int main()
{

    // Creates data file
    DataNode root;

    auto& some_node = root["some_node"];
    some_node["name"] << "Javid, Yudica";
    some_node["age"] << 24;
    some_node["height"] << 1.88;

    auto& code = some_node["code"];
    code << "C++" << "C#" << "Assembly";

    auto& pc = some_node["pc"];
    pc["processor"] << "intel";
    pc["ram"] << 32;

    size_t similar_thing_count = 10;
    some_node["similar_thing_count"] << similar_thing_count;
    for (size_t i = 0; i < similar_thing_count; i++)
        some_node["similar_thing_[" + std::to_string(i) + "]"] << rand();

    if (DataNode::write(root, "test_file.file"))
        std::cout << "Serialization and write success" << std::endl;
    
    // Reads from disk
    DataNode from_disk_node;
    if (DataNode::read(from_disk_node, "test_file.file"))
        std::cout << "Deserialization and read success" << std::endl;


    // Adds new property
    from_disk_node["some_node"]["code"] << "GDScript";

    // Writes to disk
    if (DataNode::write(from_disk_node, "read_and_modified.file"))
    {
        std::cout << "File modification success" << std::endl;
    }


    return 0;
}