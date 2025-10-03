// EnergyPlus, Copyright (c) 1996-2025, The Board of Trustees of the University of Illinois,
// The Regents of the University of California, through Lawrence Berkeley National Laboratory
// (subject to receipt of any required approvals from the U.S. Dept. of Energy), Oak Ridge
// National Laboratory, managed by UT-Battelle, Alliance for Sustainable Energy, LLC, and other
// contributors. All rights reserved.
//
// NOTICE: This Software was developed under funding from the U.S. Department of Energy and the
// U.S. Government consequently retains certain rights. As such, the U.S. Government has been
// granted for itself and others acting on its behalf a paid-up, nonexclusive, irrevocable,
// worldwide license in the Software to reproduce, distribute copies to the public, prepare
// derivative works, and perform publicly and display publicly, and to permit others to do so.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted
// provided that the following conditions are met:
//
// (1) Redistributions of source code must retain the above copyright notice, this list of
//     conditions and the following disclaimer.
//
// (2) Redistributions in binary form must reproduce the above copyright notice, this list of
//     conditions and the following disclaimer in the documentation and/or other materials
//     provided with the distribution.
//
// (3) Neither the name of the University of California, Lawrence Berkeley National Laboratory,
//     the University of Illinois, U.S. Dept. of Energy nor the names of its contributors may be
//     used to endorse or promote products derived from this software without specific prior
//     written permission.
//
// (4) Use of EnergyPlus(TM) Name. If Licensee (i) distributes the software in stand-alone form
//     without changes from the version obtained under this License, or (ii) Licensee makes a
//     reference solely to the software portion of its product, Licensee must refer to the
//     software as "EnergyPlus version X" software, where "X" is the version number Licensee
//     obtained under this License and may not use a different name for the software. Except as
//     specifically required in this Section (4), Licensee shall not use in a company name, a
//     product name, in advertising, publicity, or other promotional activities any name, trade
//     name, trademark, logo, or other designation of "EnergyPlus", "E+", "e+" or confusingly
//     similar designation, without the U.S. Department of Energy's prior written consent.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#ifndef FileSystem_hh_INCLUDED
#define FileSystem_hh_INCLUDED

#include <algorithm>
#include <fmt/format.h>
#include <fmt/os.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>
#include <nlohmann/json.hpp>
#include <string>
#ifndef __cppcheck__
#    if __has_include(<filesystem>)
#        include <filesystem>
namespace fs = std::filesystem;
#    elif __has_include(<experimental/filesystem>)
#        include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#    else
// cppcheck-suppress preprocessorErrorDirective
#        error "no filesystem support"
#    endif
#endif

#include <EnergyPlus/EnergyPlus.hh>

// If we want to allow this kind of stuff
// fs::path p = "folder/eplus";
// std::string suffixStr = "out.audit";
//
// fs::path filePath = p + suffixStr; => folder/eplusout.audit (would throw)
// std::string message = "Cannot find " + p + "." => would throw now, need p.string() instead

// inline fs::path operator+(fs::path const &left, fs::path const &right) {
//    return fs::path(left)+=right;
// }

namespace EnergyPlus {

namespace FileSystem {

    extern std::string const exeExtension;

    enum class FileTypes
    {
        Invalid = -1,
        // JSON types should go first,
        EpJSON,
        JSON,
        GLHE,
        last_json_type = GLHE,
        CBOR,
        MsgPack,
        UBJSON,
        BSON,
        last_binary_json_type = BSON,
        IDF,
        IMF,
        CSV,
        TSV,
        TXT,
        ESO,
        MTR,
        last_flat_file_type = MTR,
        DDY,
        Num
    };

    constexpr bool is_all_json_type(FileTypes t)
    {
        return t > FileTypes::Invalid && t <= FileTypes::last_binary_json_type;
    }
    constexpr bool is_json_type(FileTypes t)
    {
        return t > FileTypes::Invalid && t <= FileTypes::last_json_type;
    }
    constexpr bool is_binary_json_type(FileTypes t)
    {
        return t > FileTypes::last_json_type && t <= FileTypes::last_binary_json_type;
    }
    constexpr bool is_idf_type(FileTypes t)
    {
        return t == FileTypes::IDF || t == FileTypes::IMF;
    }
    constexpr bool is_flat_file_type(FileTypes t)
    {
        return t > FileTypes::last_binary_json_type && t <= FileTypes::last_flat_file_type;
    }

    // Similar to fs::path::make_preferred, but also does '\\' => '/' conversion on POSIX, which make_preferred does not do
    [[nodiscard]] fs::path makeNativePath(fs::path const &path);

    [[nodiscard]] fs::path getFileName(fs::path const &filePath);

    // Returns the parent directory of a path. This implementation differs from filesystem::path::parent_path because it treats trailing separators
    // differently.
    // | s      | getParentDirectoryPath(s) | fs::path(s).parent_path() |
    // |--------|---------------------------|---------------------------|
    // | a/b/c  | "a/b"                     | "a/b"                     |
    // | a/b/c/ | "a/b"                     | "a/b/c"                   |
    // | a.idf  | "./"                      | ""                        |
    [[nodiscard]] fs::path getParentDirectoryPath(fs::path const &filePath);

    [[nodiscard]] fs::path getAbsolutePath(fs::path const &filePath);

    [[nodiscard]] fs::path getProgramPath();

    // For `a/b/c.txt.idf` it returns `idf`, i.e. anything after last dot, **not including the dot** (unlike fs::path::extension() which includes it)
    [[nodiscard]] fs::path getFileExtension(fs::path const &gc);

    // Returns the FileType by looking at its extension.
    [[nodiscard]] FileTypes getFileType(fs::path const &filePath);

    // Turns a/b/c.txt.idf into a/b/c.txt, **without mutating the original object** unlike fs::path::replace_extension
    [[nodiscard]] fs::path removeFileExtension(fs::path const &filePath);

    // Replace (or append) an extension to a path **without mutating the original object** unlike fs::path::replace_extension
    [[nodiscard]] fs::path replaceFileExtension(fs::path const &filePath, fs::path const &ext);

    // Creates a directory if it doesn't already exists
    void makeDirectory(fs::path const &directoryPath);

    bool pathExists(fs::path const &path);

    bool directoryExists(fs::path const &directoryPath);

    bool fileExists(fs::path const &filePath);

    // Checks that fileExists(filePath), if so tries to rename to destination, falling back on copy+remove if failed (if trying to do move accross
    // devices for eg)
    void moveFile(fs::path const &filePath, fs::path const &destinationPath);

    int systemCall(std::string const &command);

    // Returns false if not fileExists(filePath), or if filePath cannot be removed
    bool removeFile(fs::path const &filePath);

    // On Windows, this just copies the file. On Unix, it creates a symlink
    // Starts by checking that fileExists(filePath) is true
    void linkFile(fs::path const &filePath, fs::path const &linkPath);

    // Reads the full file if it exists
    // On Windows, this must be binary input to have \r\n in the read file otherwise it will be converted to \n
    std::string readFile(fs::path const &filePath, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::binary);

    // Reads the full json file if it exists
    nlohmann::json readJSON(fs::path const &filePath, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::binary);

    template <FileTypes fileType> std::string getJSON(const nlohmann::json &data, int const indent = 4)
    {
        if constexpr (is_json_type(fileType)) {
            return data.dump(indent, ' ', false, nlohmann::json::error_handler_t::replace);
        } else if constexpr (is_binary_json_type(fileType)) {
            std::string binary_data;
            if constexpr (fileType == FileTypes::CBOR) {
                nlohmann::json::to_cbor(data, binary_data);
            } else if constexpr (fileType == FileTypes::MsgPack) {
                nlohmann::json::to_msgpack(data, binary_data);
            } else if constexpr (fileType == FileTypes::BSON) {
                nlohmann::json::to_bson(data, binary_data);
            } else if constexpr (fileType == FileTypes::UBJSON) {
                nlohmann::json::to_ubjson(data, binary_data);
            }
            return binary_data;
        } else {
            static_assert(is_all_json_type(fileType), "Must be a JSON type");
        }
    }

    template <class T, class... Ts> struct is_any : std::disjunction<std::is_same<std::remove_cv_t<T>, std::remove_cv_t<Ts>>...>
    {
    };

    template <class T>
    inline constexpr bool enable_unique_ptr_v =
        is_any<T, std::unique_ptr<fs::path>, std::unique_ptr<fmt::ostream>, std::unique_ptr<std::ostream>, std::unique_ptr<FILE *>>::value;

    template <class T, FileTypes fileType>
    inline constexpr bool enable_json_v =
        is_all_json_type(fileType) && is_any<T, nlohmann::json>::value && !is_any<T, std::string_view, std::string, char *>::value;

    template <FileTypes fileType> void writeFile(fs::path const &filePath, const std::string_view data)
    {
        static_assert(is_all_json_type(fileType) || is_flat_file_type(fileType), "Must be a valid file type");
#ifdef _WIN32
        auto filePathStr = filePath.string();
        auto path = filePathStr.c_str();
#else
        auto path = filePath.c_str();
#endif

        if constexpr (is_json_type(fileType) || is_flat_file_type(fileType)) {
            auto f = fmt::output_file(path, fmt::buffer_size = (2 << 17));
            f.print("{}", data);
        } else if constexpr (is_binary_json_type(fileType)) {
            auto close_file = [](FILE *f) { fclose(f); };
            auto holder = std::unique_ptr<FILE, decltype(close_file)>(fopen(path, "wb"), close_file);
            if (!holder) {
                throw FatalError(fmt::format("Could not open file: {}", path));
            }

            auto f = holder.get();
            fmt::print(f, "{}", data);
        }
    }

    template <FileTypes fileType> void writeFile(fmt::ostream &os, const std::string_view data)
    {
        static_assert(fileType > FileTypes::Invalid, "Must be a valid file type");
        os.print("{}", data);
    }

    template <FileTypes fileType> void writeFile(std::ostream &os, const std::string_view data)
    {
        static_assert(fileType > FileTypes::Invalid, "Must be a valid file type");
        fmt::print(os, "{}", data);
    }

    template <FileTypes fileType> void writeFile(FILE *f, const std::string_view data)
    {
        static_assert(fileType > FileTypes::Invalid, "Must be a valid file type");
        fmt::print(f, "{}", data);
    }

    template <class T, FileTypes fileType, typename = std::enable_if_t<enable_unique_ptr_v<T>>> void writeFile(T &os, const std::string_view data)
    {
        static_assert(fileType > FileTypes::Invalid, "Must be a valid file type");
        if (os) {
            writeFile<fileType>(*os, data);
        }
    }

    template <FileTypes fileType, class T, typename = std::enable_if_t<enable_json_v<T, fileType>>>
    void writeFile(fs::path const &filePath, T &data, int const indent = 4)
    {
        auto const json_str = getJSON<fileType>(data, indent);
        writeFile<fileType>(filePath, std::string_view(json_str));
    }

    template <FileTypes fileType, class T, typename = std::enable_if_t<enable_json_v<T, fileType>>>
    void writeFile(fmt::ostream &os, T &data, int const indent = 4)
    {
        auto const json_str = getJSON<fileType>(data, indent);
        writeFile<fileType>(os, std::string_view(json_str));
    }

    template <FileTypes fileType, class T, typename = std::enable_if_t<enable_json_v<T, fileType>>>
    void writeFile(std::ostream &os, T &data, int const indent = 4)
    {
        auto const json_str = getJSON<fileType>(data, indent);
        writeFile<fileType>(os, std::string_view(json_str));
    }

    template <FileTypes fileType, class T, typename = std::enable_if_t<enable_json_v<T, fileType>>>
    void writeFile(FILE *f, T &data, int const indent = 4)
    {
        auto const json_str = getJSON<fileType>(data, indent);
        writeFile<fileType>(f, std::string_view(json_str));
    }

    template <FileTypes fileType, class T, class T2, typename = std::enable_if_t<enable_json_v<T2, fileType> && enable_unique_ptr_v<T>>>
    void writeFile(T &os, T2 &data, int const indent = 4)
    {
        if (os) {
            auto const json_str = getJSON<fileType>(data, indent);
            writeFile<fileType>(*os, std::string_view(json_str));
        }
    }

    std::string toString(fs::path const &p);

    std::string toGenericString(fs::path const &p);

    fs::path appendSuffixToPath(fs::path const &outputFilePrefixFullPath, const std::string &suffix);

} // namespace FileSystem
} // namespace EnergyPlus

// Add a custom formatter for fmt
template <> struct fmt::formatter<fs::path>
{
    // Presentation format: 's' - string, 'g' - generic_string.
    char presentation = 's';

    // Parses format specifications of the form ['s' | 'g'].
    constexpr auto parse(format_parse_context &ctx) -> decltype(ctx.begin())
    {
        // Parse the presentation format and store it in the formatter:
        auto it = ctx.begin(), end = ctx.end();
        if (it != end && (*it == 's' || *it == 'g')) {
            presentation = *it++;
        }

        // Check if reached the end of the range:
        if (it != end && *it != '}') {
            throw format_error("invalid format");
        };

        // Return an iterator past the end of the parsed range:
        return it;
    }

    template <typename FormatContext> auto format(const fs::path &p, FormatContext &ctx) -> decltype(ctx.out())
    {
        return format_to(ctx.out(), "{}", presentation == 'g' ? EnergyPlus::FileSystem::toGenericString(p) : EnergyPlus::FileSystem::toString(p));
    }
};

#endif
