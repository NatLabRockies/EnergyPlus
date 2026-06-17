#ifndef RS_INSTANCE_FACTORY_H_
#define RS_INSTANCE_FACTORY_H_

#include <string>
#include <memory>
#include <fstream>
#include <vector>
#include <nlohmann/json.hpp>

namespace Courierr { class Courierr; }

namespace tk205 {

class RSInstanceBase;

inline void read_binary_file(const char *filename, std::vector<char> &bytes)
{
    std::ifstream is(filename, std::ifstream::binary);
    if (is) {
        // get length of file:
        is.seekg(0, is.end);
        size_t length = static_cast<size_t>(is.tellg());
        is.seekg(0, is.beg);

        bytes.resize(length);
        // read data as a block:
        is.read(bytes.data(), length);

        is.close();
    }
}

inline nlohmann::json load_json(const char *input_file)
{
    std::string filename(input_file);
    std::string::size_type idx = filename.rfind('.');

    using namespace nlohmann;

    json j;

    if (idx != std::string::npos) {
        std::string extension = filename.substr(idx + 1);

        if (extension == "cbor") {
            std::vector<char> bytearray;
            read_binary_file(input_file, bytearray);
            j = json::from_cbor(bytearray);
        }
        else if (extension == "json") {
            std::string schema(input_file);
            std::ifstream in(schema);
            in >> j;
        }
    }
    return j;
}

/// @class SchemVer rs_instance_factory.h
/// @brief SchemVer tokenizes a semantic version string (assuming no metadata tag) into a comparable set of integers

class SchemVer {
  public:
    SchemVer(const char *version_str)
    {
        int version_segments = 0;
        char ver_str[128];
        strcpy(ver_str, version_str);
        char *s = strchr(ver_str, '.');
        while (s) {
            ++s;
            ++version_segments;
            s = strchr(s, '.');
        }
        if (version_segments == 2) {
            char *tok = strtok(ver_str, ".");
            if (tok) {
                major_ = atoi(tok);
                tok = strtok(NULL, ".");
                if (tok) {
                    minor_ = atoi(tok);
                    tok = strtok(NULL, ".");
                    if (tok) {
                        patch_ = atoi(tok);
                    }
                }
            }
        }
        else
        {
            throw std::invalid_argument("Version string invalid.");
        }
    }
    friend bool operator==(const SchemVer &v1, const SchemVer &v2);
    friend bool operator!=(const SchemVer &v1, const SchemVer &v2);
    friend bool operator>(const SchemVer &v1, const SchemVer &v2);
    friend bool operator<(const SchemVer &v1, const SchemVer &v2);
    friend bool operator>=(const SchemVer &v1, const SchemVer &v2);
    friend bool operator<=(const SchemVer &v1, const SchemVer &v2);

  private:
    int major_{0};
    int minor_{0};
    int patch_{0};
};

inline bool operator==(const SchemVer &v1, const SchemVer &v2)
{
    return ((v1.major_ == v2.major_) && (v1.minor_ == v2.minor_) && (v1.patch_ == v2.patch_));
}

inline bool operator!=(const SchemVer &v1, const SchemVer &v2)
{
    return ((v1.major_ != v2.major_) || (v1.minor_ != v2.minor_) || (v1.patch_ != v2.patch_));
}

inline bool operator>(const SchemVer &v1, const SchemVer &v2)
{
    if (v1.major_ < v2.major_) {
        return false;
    }
    else if (v1.major_ > v2.major_) {
        return true;
    }
    else if (v1.major_ == v2.major_) {
        if (v1.minor_ < v2.minor_) {
            return false;
        }
        else if (v1.minor_ > v2.minor_) {
            return true;
        }
        else if (v1.minor_ == v2.minor_) {
            if (v1.patch_ < v2.patch_) {
                return false;
            }
            else if (v1.patch_ > v2.patch_) {
                return true;
            }
            else {
                return false; // equal or malformed
            }
        }
    }
    return false;
}

inline bool operator<(const SchemVer &v1, const SchemVer &v2)
{
    if (v1.major_ > v2.major_) {
        return false;
    }
    else if (v1.major_ < v2.major_) {
        return true;
    }
    else if (v1.major_ == v2.major_) {
        if (v1.minor_ > v2.minor_) {
            return false;
        }
        else if (v1.minor_ < v2.minor_) {
            return true;
        }
        else if (v1.minor_ == v2.minor_) {
            if (v1.patch_ > v2.patch_) {
                return false;
            }
            else if (v1.patch_ < v2.patch_) {
                return true;
            }
            else {
                return false; // equal or malformed
            }
        }
    }
    return false;
}

inline bool operator<=(const SchemVer &v1, const SchemVer &v2)
{
    if (v1.major_ > v2.major_) {
        return false;
    }
    else if (v1.major_ < v2.major_) {
        return true;
    }
    else if (v1.major_ == v2.major_) {
        if (v1.minor_ > v2.minor_) {
            return false;
        }
        else if (v1.minor_ < v2.minor_) {
            return true;
        }
        else if (v1.minor_ == v2.minor_) {
            if (v1.patch_ > v2.patch_) {
                return false;
            }
            else if (v1.patch_ < v2.patch_) {
                return true;
            }
            else {
                return true;
            }
        }
    }
    return false;
}

inline bool operator>=(const SchemVer &v1, const SchemVer &v2)
{
    if (v1.major_ < v2.major_) {
        return false;
    }
    else if (v1.major_ > v2.major_) {
        return true;
    }
    else if (v1.major_ == v2.major_) {
        if (v1.minor_ < v2.minor_) {
            return false;
        }
        else if (v1.minor_ > v2.minor_) {
            return true;
        }
        else if (v1.minor_ == v2.minor_) {
            if (v1.patch_ < v2.patch_) {
                return false;
            }
            else if (v1.patch_ > v2.patch_) {
                return true;
            }
            else {
                return true;
            }
        }
    }
    return false;
}

/// @class RSInstanceFactory rs_instance_factory.h
/// @brief This class is an abstract interface to support RS factory sub-classes

class RSInstanceFactory {
  public: // Interface
    RSInstanceFactory() = default;
    virtual ~RSInstanceFactory() = default;

    static bool register_factory(std::string const &RS_ID,
                                 std::shared_ptr<RSInstanceFactory> factory);

    // Universal factory interface create(). Factory::create() will, through factory lookup and
    // delegation, actually return the requested object.
    static std::shared_ptr<RSInstanceBase> create(std::string const &RS_ID,
                                                  const char *RS_instance_file,
                                                  std::shared_ptr<Courierr::Courierr> logger);

    // Derived factories override create_instance() for actual resource creation
    virtual std::shared_ptr<RSInstanceBase> create_instance(const char *RS_instance_file, 
                                                            std::shared_ptr<Courierr::Courierr> logger) const = 0;

    // Rule of five
    RSInstanceFactory(const RSInstanceFactory &other) = delete;
    RSInstanceFactory &operator=(const RSInstanceFactory &other) = delete;
    RSInstanceFactory(RSInstanceFactory &&) = delete;
    RSInstanceFactory &operator=(RSInstanceFactory &&) = delete;
};
} // namespace tk205

#endif