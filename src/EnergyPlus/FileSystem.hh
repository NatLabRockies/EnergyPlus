// EnergyPlus, Copyright (c) 1996-present, The Board of Trustees of the University of Illinois,
// The Regents of the University of California, through Lawrence Berkeley National Laboratory
// (subject to receipt of any required approvals from the U.S. Dept. of Energy), Oak Ridge
// National Laboratory, managed by UT-Battelle, Alliance for Energy Innovation, LLC, and other
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

// C++ Headers
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
#include <format>
#include <string>

// Third Party Headers
#include <nlohmann/json.hpp>

// EnergyPlus Headers
#include <EnergyPlus/EnergyPlus.hh>

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

    // Checks that fileExists(filePath), if so tries to rename to destination, falling back on copy+remove if failed (if trying to do move across
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

    template <FileTypes fileType> void writeFile(fs::path const &filePath, const std::string_view data)
    {
        static_assert(is_all_json_type(fileType) || is_flat_file_type(fileType), "Must be a valid file type");
#ifdef _WIN32
        auto filePathStr = filePath.string();
        auto path_c_str = filePathStr.c_str();
#else
        auto path_c_str = filePath.c_str();
#endif

        auto close_file = [](FILE *f) { std::fclose(f); };
        constexpr const char *mode = is_binary_json_type(fileType) ? "wb" : "w";
        auto holder = std::unique_ptr<FILE, decltype(close_file)>(std::fopen(path_c_str, mode), close_file);
        if (!holder) {
            throw FatalError(std::format("Could not open file: {}", filePath.string()));
        }
        std::fwrite(data.data(), 1, data.size(), holder.get());
    }

    template <FileTypes fileType, std::same_as<nlohmann::json> T>
        requires(is_all_json_type(fileType))
    void writeFile(fs::path const &filePath, const T &data, int const indent = 4)
    {
        auto const json_str = getJSON<fileType>(data, indent);
        writeFile<fileType>(filePath, std::string_view(json_str));
    }

    std::string toString(fs::path const &p);

    std::string toGenericString(fs::path const &p);

    fs::path appendSuffixToPath(fs::path const &outputFilePrefixFullPath, const std::string &suffix);
} // namespace FileSystem
} // namespace EnergyPlus

#if __cpp_lib_format_path >= 202403L
#    error                                                                                                                                           \
        "std::formatter specialization for std::filesystem::path is available in the STL, so the custom specialization in FileSystem.hh should be removed"
#endif
template <> struct std::formatter<fs::path>
{
    bool generic_string = false;

    // parse is inherited from formatter<string_view>.
    constexpr auto parse(std::format_parse_context &ctx) -> std::format_parse_context::iterator
    {
        // Parse the presentation format and store it in the formatter:
        auto it = ctx.begin();
        auto end = ctx.end();
        if (it != end && (*it == 's' || *it == 'g')) {
            generic_string = (*it++) == 'g';
        }

        // Check if reached the end of the range:
        if (it != end && *it != '}') {
            throw std::format_error("invalid format");
        };

        // Return an iterator past the end of the parsed range:
        return it;
    }

    // For older clang/apple-clang, use a templated FormatContext and no trailing return
    // https://github.com/llvm/llvm-project/issues/66466#issuecomment-1720807809
    // auto format(const fs::path &p, std::format_context &ctx) const -> std::format_context::iterator
    template <typename FormatContext> auto format(const fs::path &p, FormatContext &ctx) const
    {
        return std::format_to(ctx.out(), "{}", generic_string ? EnergyPlus::FileSystem::toGenericString(p) : EnergyPlus::FileSystem::toString(p));
    }
};

#endif
