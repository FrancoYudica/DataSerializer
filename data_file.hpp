#ifndef __DATA_FILE__
#define __DATA_FILE__
#include <vector>
#include <string>
#include <unordered_map>
#include <fstream>
#include <functional>
#include <stack>
#include <iostream>

namespace DataSerializer
{

class DataNode;

// Generic serializer and deserializer definitions
void serialize(DataNode& node, const std::string& str);
void serialize(DataNode& node, const double& value);

bool deserialize(const DataNode& node, std::string& value);
bool deserialize(const DataNode& node, double& value);


class DataNode{

public:

    DataNode() = default;

    template<typename T>
    T get() const
    {
        T value;
        deserialize(*this, value);
        return value;
    }

    static bool write(
        const DataNode& root_node,
        const std::string& file_name,
        const std::string& indent = "\t",
        const char list_separator = ','
    )
    {
        std::ofstream file(file_name);
        if (!file.is_open())
            return false;

        std::string separator = std::string(1, list_separator) + " ";

        // Keeps track of current level of indentation
        size_t n_indent_count = 0;

        std::function<void(const DataNode&, std::ofstream&)> write = [&](const DataNode& n, std::ofstream& stream)
        {
            
            // Creates indentation string
            auto indent_func = [&](const std::string& indent_str, const size_t n_count)
            {
                std::string total_indent;
                for (size_t n = 0; n < n_count; n++) total_indent += indent_str;
                return total_indent;
            };

            // For each child
            for (auto const& property : n._children)
            {
                const std::string& child_name = property.first;
                const DataNode& child_node = property.second;

                size_t n_items = child_node.content_size();

                // Doesn't have any children
                if (child_node._children.empty())
                {
                    stream << indent_func(indent, n_indent_count) << child_name << " = ";
                    for (size_t i = 0; i < n_items; i++)
                    {
                        const std::string& item = child_node.get_property(i);

                        // If item contains a list separator, the item is serialized with
                        // quotes
                        size_t x = item.find_first_of(list_separator);
                        if (x != std::string::npos)
                        {
                            // Value contains list separator, so it wraps in quotes
                            stream << "\"" << item << "\"" << ((i < n_items - 1) ? separator : "");
                        }
                        else
                        {
                            // Writes item without quotes
                            stream << item << ((i < n_items - 1) ? separator : "");
                        }
                    }
                    stream << "\n";
                }

                // Has children
                else
                {
                    // New line and writes child's name
                    stream << "\n" << indent_func(indent, n_indent_count) << child_name << "\n";

                    // Open braces
                    stream << indent_func(indent, n_indent_count) << "{\n";

                    // Updates indentation, since a new child is added
                    n_indent_count++;

                    // Recursively writes child node
                    write(child_node, stream);

                    // Node written, sot close braces
                    stream << indent_func(indent, n_indent_count) << "}\n\n";
                }
            }

            // After writing node, decreases indentation level
            if (n_indent_count > 0)
                n_indent_count--;
        };
        write(root_node, file);
        file.close();
        return true;
    }

    static bool read(
        DataNode& root_node,
        const std::string& file_name,
        const char list_separator = ','
    )
    {
        std::ifstream stream(file_name);
        if (!stream.is_open())
            return false;

        std::string property_name;
        std::string property_value;

        std::stack<std::reference_wrapper<DataNode>> node_stack;
        node_stack.push(root_node);

        // Read file line by line and process
        while(!stream.eof())
        {
            // Read line
            std::string line;
            std::getline(stream, line);

            // Lambda used to remove beginning and end whitespaces
            auto trim = [](std::string& str)
            {
                str.erase(0, str.find_first_not_of(" \t\n\r\f\v"));
                str.erase(str.find_last_not_of(" \t\n\r\f\v") + 1);
            };

            trim(line);

            // Skips empty lines
            if (line.empty())
                continue;
            
            DataNode& current_node = node_stack.top();

            // If the line contains '=', it has items
            size_t equals_index = line.find_first_of('=');
            if (equals_index != std::string::npos)
            {
                // Gets property name and values trimmed in different strings
                property_name = line.substr(0, equals_index);
                property_value = line.substr(equals_index + 1, line.size());
                trim(property_name);
                trim(property_value);

                // Loops for each character of the value string, concatenating
                // the string 
                std::string token;

                // Used to ignore separators in case value contains the separator symbol
                // inside a string
                bool quotes_opened = false;

                for (const char c : property_value)
                {
                    if (c == '\"')
                        quotes_opened = !quotes_opened;

                    else if (c == list_separator && !quotes_opened)
                    {
                        trim(token);
                        current_node[property_name] << token;

                        // Rests token
                        token.clear();
                    }

                    // Concatenates string
                    else
                    {
                        token += c;
                    }
                }

                // Any residual characters at this point just make up the final token,
                // so clean it up and add it to the contents of current_node
                if (!token.empty())
                {
                    trim(token);
                    current_node[property_name] << token;
                }
            }

            // No '='
            else
            {
                // Creates new node
                if (line[0] == '{')
                {
                    // Creates and pushes new node to be populated
                    node_stack.push(current_node[property_name]);
                }

                // End of node
                else if (line[0] == '}')
                {
                    // Close brace, so this node is completed
                    node_stack.pop();
                }

                // New node name
                else 
                {
                    property_name = line;
                }

            }
        }

        stream.close();
        return true;
    }

    inline size_t content_size() const { return _properties.size(); }

    inline const std::string& get_property(size_t i) const { return _properties[i]; }

    inline void add_property(const std::string& property)
    {
        _properties.push_back(property);
    }

    template<typename T>
    inline DataNode& operator<<(const T& value)
    {
        serialize(*this, value);
        return *this;
    }

    inline DataNode& operator[](const std::string& child_name)
    {
        // Node doesn't exists
        if (_names_map.count(child_name) == 0)
        {
            _names_map[child_name] = _children.size();
            _children.push_back( {child_name, DataNode() } );
        }
        return _children[_names_map[child_name]].second;
    }
    
private:

    /// @brief Node can contain properties
    std::vector<std::string> _properties;

    /// @brief Node can contain children DataNode
    std::vector<std::pair<std::string, DataNode>> _children;

    /// @brief Maps name to index in children vector
    std::unordered_map<std::string, size_t> _names_map;

};
}


#endif