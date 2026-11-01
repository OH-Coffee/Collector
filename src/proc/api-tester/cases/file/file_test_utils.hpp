#ifndef file_test_utils_hpp
#define file_test_utils_hpp

#include <cstdlib>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

namespace file_test {

class TemporaryDirectory
{
public:
    TemporaryDirectory()
    {
        std::string template_path = (std::filesystem::temp_directory_path() / "collector-file-test-XXXXXX").string();
        std::vector<char> writable_path(template_path.begin(), template_path.end());
        writable_path.push_back('\0');

        char* created_path = mkdtemp(writable_path.data());
        if (nullptr == created_path) {
            throw std::runtime_error("failed to create temporary test directory");
        }
        _path = created_path;
    }

    ~TemporaryDirectory()
    {
        std::error_code error_code;
        std::filesystem::remove_all(_path, error_code);
    }

    std::string path(const std::string& i_relative_path = "") const
    {
        return (std::filesystem::path(_path) / i_relative_path).string();
    }

private:
    std::string _path;
};

} // namespace file_test

#endif /* file_test_utils_hpp */
