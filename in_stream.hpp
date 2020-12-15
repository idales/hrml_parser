#include <regex>
#include <stdexcept>
#include <stdexcept>
#include <string>
#include <tuple>

#include "alg_interface.h"

// instream solution
// get next tag from query and immediately find it in the input stream

class InStream : public ITagValue
{
    struct Exception : public std::runtime_error
    {
        Exception(char const *const message) throw()
            : std::runtime_error(message) {}
        const char *what() const throw()
        {
            return std::runtime_error::what();
        }
    };

    std::istream &text;
    std::string line;

    inline void next_line()
    {
        if (!std::getline(text, line))
            throw Exception("End of stream");
    }

    std::tuple<std::string, bool> get_next_word(const std::string &str_path, size_t &path_pos)
    {
        bool is_tag = (path_pos == 0 || str_path[path_pos] == '.');
        auto start_pos = path_pos ? path_pos + 1 : 0;
        path_pos = str_path.find_first_of(".~", start_pos);
        return std::make_tuple(str_path.substr(start_pos, path_pos - start_pos), is_tag);
    }

    void reset_input_stream()
    {
        line.clear();
        text.seekg(0);
    }

    void find_close_tag(const std::string &tag)
    {
        auto t = std::string(R"(</\s*)") + tag + R"(\s*>)";
        std::regex regex(t);
        do
        {
            next_line();
        } while (!std::regex_match(line, regex));
    }

    std::tuple<std::string, bool> parse_tag_name()
    { //
        std::regex regex(R"(<(\/?)\s*(\w+).*>)");
        std::smatch match;
        std::string tag_name;
        bool is_open;
        if (std::regex_match(line, match, regex))
        {
            tag_name = match[2];
            is_open = match.length(1) == 0;
        }
        else
        {
            throw Exception("not tag");
        }

        return std::make_tuple(tag_name, true);
    }

    void find_open_tag(const std::string &tag)
    {
        std::string found_tag;
        do
        {
            bool is_open;
            std::tie(found_tag, is_open) = parse_tag_name();
            if (!is_open)
                throw Exception("should be open tag ");
            if (found_tag != tag)
            {
                find_close_tag(found_tag);
                next_line(); // move to next open tag
            }
        } while (tag != found_tag);
    }

    std::string find_attr(const std::string &word)
    {
        std::string res;
        std::string t(word + R"V(\s*=\s*"([^"]+))V");
        std::regex regex(t);
        std::smatch match;
        std::string tag_name;
        if (regex_search(line, match, regex))
        {
            res = match[1];
        }
        else
        {
            throw Exception("attr not found");
        }
        return res;
    }

public:
    InStream(std::istream &text)
        : text(text)
    {
    }

    std::string get_value(const std::string &str_path) override
    try
    {
        size_t path_pos = 0;
        reset_input_stream();
        std::string res;
        do
        {
            auto [word, is_tag] = get_next_word(str_path, path_pos);
            if (is_tag)
            {
                next_line(); // move into tag or read first line
                find_open_tag(word);
            }
            else
            {
                res = str_path + "=\"" + find_attr(word)+"\"";
                break;
            }
        } while (path_pos != str_path.npos);
        return res;
    }
    catch (const Exception &ex)
    {
        return str_path + (" not found");
    }
};
