
#ifndef string_spliter_h
#define string_spliter_h

#include <functional>
#include <memory>
#include <utility>

#include "string/string_split.hpp"


class StringSpliter;
class ReverseStringSpliter;


typedef std::shared_ptr<StringSpliter>          StringSpliterPtr;
typedef std::shared_ptr<ReverseStringSpliter>   ReverseStringSpliterPtr;

typedef enum StringSpliterType : uint64_t
{
    UNKNOWN_SPLITER = 0,
    STRING_FORWARD_SPLITER,
    STRING_REVERSE_SPLITER,
} StringSpliterType;


class StringSpliter
{
public:
    template<typename ... T> static StringSpliterPtr create(T && ... i_args) {
        return std::make_shared<StringSpliter>(std::forward<T>(i_args)...);
    }
public:
    virtual void split(const std::string_view &i_str,
                       const std::string_view &i_delim,
                       const std::function<bool(const std::string_view &)> &i_callback);
public:
    virtual void split(const std::string_view &i_str, const std::string_view &i_delim, bool i_drop_empty, std::set<std::string>    &o_data);
    virtual void split(const std::string_view &i_str, const std::string_view &i_delim, bool i_drop_empty, std::list<std::string>   &o_data);
    virtual void split(const std::string_view &i_str, const std::string_view &i_delim, bool i_drop_empty, std::vector<std::string> &o_data);
};



class ReverseStringSpliter : public StringSpliter
{
public:
    template<typename ... T> static ReverseStringSpliterPtr create(T && ... i_args) {
        return std::make_shared<ReverseStringSpliter>(std::forward<T>(i_args)...);
    }
public:
    void split(const std::string_view &i_str,
               const std::string_view &i_delim,
               const std::function<bool(const std::string_view &)> &i_callback) override;
public:
    void split(const std::string_view &i_str, const std::string_view &i_delim, bool i_drop_empty, std::set<std::string>    &o_data) override;
    void split(const std::string_view &i_str, const std::string_view &i_delim, bool i_drop_empty, std::list<std::string>   &o_data) override;
    void split(const std::string_view &i_str, const std::string_view &i_delim, bool i_drop_empty, std::vector<std::string> &o_data) override;
};


#endif /* string_spliter_h */
