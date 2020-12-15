#include <cassert>
#include <map>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "alg_interface.h"

// tree solution
// parse tag struct from stream into tree and then search path with it

class TagTree : public ITagValue
{
private:
    using AttrValueMap = std::map<std::string, std::string>;
    struct Tag;
    using TagMap = std::map<std::string, Tag>;
    struct Tag
    {
        std::string name;
        Tag *parent = nullptr;
        AttrValueMap attr_value_map;
        TagMap nested_tags;
    };

    Tag root;

    static inline bool isidentchar(int c)
    {
        return isalnum(c) || c == '_';
    }

    static std::string read_word(const std::string &next_line, size_t &pos)
    {
        // skip space before word
        while (pos < next_line.size() && !isidentchar(next_line[pos]))
            pos++;
        auto start = pos;
        while (pos < next_line.size() && isidentchar(next_line[pos]))
            pos++;
        return next_line.substr(start, pos - start);
    }

    static AttrValueMap::value_type get_attr_value_pair(const std::string &next_line, size_t &pos)
    {
        // read attr
        AttrValueMap::value_type res;
        auto end = pos; // move from =
        // skip spaces
        while (end > 0 && isspace(next_line[end - 1]))
            --end;
        auto start = end;
        while (start > 0 && isidentchar(next_line[start - 1]))
            --start;
        auto attribute = next_line.substr(start, end - start);
        ++pos; //skip '='
        start = next_line.find('"', pos);
        end = next_line.find('"', start + 1);
        if (start == next_line.npos || end == next_line.npos)
            throw std::runtime_error(std::string("wrong attribute syntax attribute:") + attribute);
        auto value = next_line.substr(start, end - start + 1);
        pos = end + 1;
        return std::make_pair(std::move(attribute), std::move(value));
    }

    struct TagRes
    {
        Tag tag;
        bool is_open;
    };

    static std::optional<TagRes> next_tag(std::istream &text)
    {
        std::string next_line;

        while (getline((text >> std::skipws), next_line) && next_line.empty())
            ;
        if (text.eof())
            return {};

        TagRes tag_res;
        size_t pos = 0;

        assert(next_line.substr(pos, 1) == "<");
        tag_res.is_open = !(next_line.substr(pos, 2) == "</");
        pos = (tag_res.is_open) ? 1 : 2;
        auto &tag = tag_res.tag;
        tag.name = read_word(next_line, pos);
        // read tag attrs
        while ((pos = next_line.find(" = ", pos)) != std::string::npos)
        {
            auto attr_value = get_attr_value_pair(next_line, pos);
            tag.attr_value_map.emplace(std::move(attr_value));
        }

        return {tag_res};
    }

public:
    TagTree(std::istream &text)
    {
        Tag *context = &root;
        while (auto tag_res = next_tag(text))
        {
            auto &tag = tag_res->tag;

            if (tag_res->is_open)
            {
                tag.parent = context;
                auto insres = context->nested_tags.emplace(tag.name, std::move(tag));
                assert(insres.second); // actually insterted
                context = &(insres.first->second);
            }
            else
            {
                // closing tag has the same name as open
                assert(context->name == tag.name);
                context = context->parent;
            }
        }
    }

    std::string get_value(const std::string &str_path) override
    try
    {
        std::string res;
        size_t start = 0;
        const Tag *context = &root;
        bool read_tag = true;
        size_t end;
        do
        {
            end = str_path.find_first_of(".~", start);
            if (end != str_path.npos)
            {
                auto tagname = str_path.substr(start, end - start);
                context = &(context->nested_tags.at(tagname));
                read_tag = (str_path[end] == '.'); // switch to read attribute mode
                start = ++end;
            }
            else
            {
                if (read_tag) // end of string but attribute doesn't read
                {
                    throw std::out_of_range(str_path + (" not found"));
                }
                else
                {
                    auto attrname = read_word(str_path, start);
                    auto value = context->attr_value_map.at(attrname);
                    std::ostringstream out_str;
                    out_str << str_path << "=" << value;
                    res = out_str.str();
                    break;
                }
            }

        } while (end != str_path.npos);

        return res;
    }
    catch (const std::out_of_range &ex)
    {
        return str_path + (" not found");
    }
};
