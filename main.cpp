#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
namespace fs = std::filesystem;

#include "in_stream.hpp"
#include "tag_tree.hpp"

//https://www.hackerrank.com/challenges/attribute-parser/problem

void print_usage(const std::string &prg_name)
{

    std::cout << "Usage: " << prg_name << " [--parser|-p <parser type>] <input filename>" << std::endl;
    std::cout << R"VVV(
    <parser type> could be "tag_tree" or "in_stream"
        various algorithms for finding attribute value

    <input filename> file has following format:

        Lines in HRML format
        Empty line
        Lines with queries in following format
            <tag>.<tag>.<tag>~<attribute>
        Empty line

)VVV";
    std::cout << "HRML is artifical markup language. Each line has only one opening or closing tag."
                 " Opening tags can also contain attributes in form <attribute name> = \"<attribute value>\"."
                 " Tags can be nested.";
    std::cout << R"VVV(

For example:

    <tag1 value = "Hello World">
    <tag2 name = "Name1">
    </tag2>
    <tag3 another="another" final="final">
    </tag3>
    </tag1>

    tag1.tag2~name
    tag1~name
    tag1~value

output should be:

    tag1.tag2~name="Name1"
    tag1~name not found
    tag1~value="Hello World"
)VVV";
}

struct Input
{
    std::stringstream text_stream;
    std::stringstream query_stream;
};

struct Config
{
    std::string parser{"tag_tree"};
    std::string filename;
};

Input prepare_input(const Config &config)
{
    Input input;
    std::ifstream input_file(config.filename);
    if (!input_file)
        throw std::runtime_error(std::string("Error opening file \"") + config.filename + "\" for reading");
    std::string line;
    while (getline(input_file, line) && !line.empty())
        input.text_stream << line << std::endl;
    while (getline(input_file, line) && !line.empty())
        input.query_stream << line << std::endl;
    return input;
}

std::optional<Config> make_config(int argc, char **argv)
{
    auto prg_name = fs::path(argv[0]).filename();
    Config config;
    if (argc < 2)
    {
        print_usage(prg_name);
        return std::nullopt;
    }
    for (int i = 1; i < argc; i++)
    {
        std::string_view arg(argv[i]);
        if (arg == "--parser" || arg == "-p")
        {
            config.parser = argv[++i];
        }
        else if (arg == "--help" || arg == "-h")
        {
            print_usage(prg_name);
            return std::nullopt;
        }
        else
        {
            config.filename = arg;
        }
    }
    return {config};
}

std::unique_ptr<ITagValue> make_parser(const Config &config, std::istream &input_stream)
{
    std::unique_ptr<ITagValue> res;
    if (config.parser == "tag_tree")
        res = std::make_unique<TagTree>(input_stream);
    else if (config.parser == "in_stream")
        res = std::make_unique<InStream>(input_stream);
    else
        throw std::runtime_error(std::string("unknown parser: ") + config.parser);
    return res;
}

int main(int argc, char **argv)
try
{
    auto config = make_config(argc, argv);
    if (!config)
        return 0;

    auto input = prepare_input(*config);

    std::unique_ptr<ITagValue> parser = make_parser(*config, input.text_stream);

    std::string query;
    while (getline(input.query_stream, query))
    {
        std::cout << parser->get_value(query) << std::endl;
    }

    return 0;
}
catch (const std::runtime_error &ex)
{
    std::cerr << ex.what() << std::endl;
    return -1;
}
