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

#ifndef IOFiles_hh_INCLUDED
#define IOFiles_hh_INCLUDED

// C++ Headers
#include <array>
#include <cassert>
#include <fstream>
#include <iostream>
#include <limits>
#include <ostream>
#include <vector>

// EnergyPlus Headers
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/FileSystem.hh>

// Third Party Headers
#include <fmt/compile.h>
#include <fmt/format.h>
#include <fmt/os.h>
#include <fmt/ostream.h>
#include <fmt/printf.h>
#include <fmt/ranges.h>
#include <nlohmann/json.hpp>

namespace {
struct DoubleWrapper
{
    // this cannot be marked explicit
    // we need the implicit conversion for it to work
    // clang-format off
    DoubleWrapper(double val) : value(val) {};
    // clang-format on
    operator double() const
    {
        return value;
    };
    DoubleWrapper &operator=(const double &other)
    {
        value = other;
        return *this;
    }

private:
    double value;
};
} // namespace

namespace fmt {
template <> struct formatter<DoubleWrapper>
{
private:
    fmt::detail::dynamic_format_specs<char> specs_;
    const char *format_str_;
    fmt::memory_buffer buffer = fmt::memory_buffer();

    struct null_handler : detail::error_handler
    {
        void on_align(align_t)
        {
        }
        void on_sign(sign_t)
        {
        }
        void on_hash()
        {
        }
    };

    static constexpr bool should_be_fixed_output(const double value)
    {
        return (value >= 0.099999999999999995 || value <= -0.099999999999999995) || (value == 0.0) || (value == -0.0);
    }

    static constexpr bool fixed_will_fit(const double value, const int places)
    {
        if (value < 1.0 && value > -1.0) {
            return true;
        } else {
            return static_cast<int>(std::log10(std::abs(value))) < places;
        }
    }

    static std::string &zero_pad_exponent(std::string &str)
    {
        // if necessary, pad the exponent with a 0 to match the old formatting from Objexx
        if (str.size() > 3) {
            if (!std::isdigit(str[str.size() - 3])) {
                // wants a 0 inserted
                str.insert(str.size() - 2, "0");
            }
        }
        return str;
    }

    std::string_view spec_builder()
    {
        buffer.clear();
        buffer.push_back('{');
        buffer.push_back(':');
        //    [[fill]align][sign]["#"]["0"][width]["." precision]["L"][type]

        //    [[fill]align]
        switch (specs_.align) {
        case align_t::left:
            if (specs_.fill.size()) {
                buffer.append(specs_.fill);
            }
            buffer.push_back('<');
            break;
        case align_t::right:
            if (specs_.fill.size()) {
                buffer.append(specs_.fill);
            }
            buffer.push_back('>');
            break;
        case align_t::center:
            if (specs_.fill.size()) {
                buffer.append(specs_.fill);
            }
            buffer.push_back('^');
            break;
        case align_t::none:
        case align_t::numeric:
            break;
        default:
            throw fmt::format_error("Bad alignment");
        }

        //    [sign]
        switch (specs_.sign) {
        case sign_t::plus:
            buffer.push_back('+');
            break;
        case sign_t::minus:
            buffer.push_back('-');
            break;
        case sign_t::space:
            buffer.push_back(' ');
            break;
        case sign_t::none:
            break;
        default:
            throw fmt::format_error("Bad sign");
        }

        //    [alt]
        if (specs_.alt) {
            buffer.push_back('#');
        }

        //    [width]
        if (specs_.width >= 0) {
            if (specs_.fill[0] == '0') {
                buffer.push_back('0');
            }
            auto fmt_int = fmt::format_int(specs_.width);
            buffer.append(fmt_int.data(), fmt_int.data() + fmt_int.size());
        }

        //    [precision]
        if (specs_.precision >= 0) {
            buffer.push_back('.');

            auto fmt_int = fmt::format_int(specs_.precision);
            buffer.append(fmt_int.data(), fmt_int.data() + fmt_int.size());
        }

        //    [locale]
        if (specs_.localized) {
            buffer.push_back('L');
        }

        //    [type]
        buffer.push_back(specs_.type);

        buffer.push_back('}');

        return {buffer.data(), buffer.size()};
    }

    template <typename Context> void handle_specs(Context &ctx)
    {
        detail::handle_dynamic_spec<detail::width_checker>(specs_.width, specs_.width_ref, ctx);
        detail::handle_dynamic_spec<detail::precision_checker>(specs_.precision, specs_.precision_ref, ctx);
    }

public:
    template <typename ParseContext> constexpr auto parse(ParseContext &ctx)
    {
        auto begin = ctx.begin(), end = ctx.end();
        format_str_ = begin;
        if (begin == end) {
            return begin;
        }
        using handler_type = fmt::detail::dynamic_specs_handler<ParseContext>;
        auto it = fmt::detail::parse_format_specs(begin, end, handler_type(specs_, ctx));
        return it;
    }

    template <typename FormatContext> auto format(const DoubleWrapper &doubleWrapper, FormatContext &ctx)
    {
        const auto next_float = [](const double value) {
            if (std::signbit(value)) {
                if (value == -0.0) {
                    return value;
                } else {
                    return std::nextafter(value, std::numeric_limits<decltype(value)>::lowest());
                }
            } else {
                if (value == 0.0) {
                    return value;
                } else {
                    return std::nextafter(value, std::numeric_limits<decltype(value)>::max());
                }
            }
        };

        double val = doubleWrapper;

        handle_specs(ctx);
        detail::specs_checker<null_handler> checker(null_handler(), detail::mapped_type_constant<double, FormatContext>::value);
        checker.on_align(specs_.align);
        if (specs_.sign != sign::none) {
            checker.on_sign(specs_.sign);
        }
        if (specs_.alt) {
            checker.on_hash();
        }
        if (specs_.precision >= 0) {
            checker.end_precision();
        }

        if (specs_.type == 'R') { // matches RoundSigDigits() behavior
            // push the value up a tad to get the same rounding behavior as Objexx
            const auto fixed_output = should_be_fixed_output(val);

            if (fixed_output) {
                specs_.type = 'F';

                if (val > 100000.0) {
                    const auto digits10 = static_cast<int>(std::log10(val));
                    // we cannot represent this val to the required precision, truncate the floating
                    // point portion
                    if (digits10 + specs_.precision >= std::numeric_limits<decltype(val)>::max_digits10) {
                        specs_.precision = 0;
                        spec_builder();
                        // add '.' to match old RoundSigDigits
                        buffer.push_back('.');
                        std::string_view fmt_buffer(buffer.data(), buffer.size());
                        return fmt::format_to(ctx.out(), fmt_buffer, val);
                    } else {
                        return fmt::format_to(ctx.out(), spec_builder(), val);
                    }
                } else {
                    if (val == 0.0 || val == -0.0) {
                        return fmt::format_to(ctx.out(), spec_builder(), 0.0);
                    } else {
                        // nudge up to next rounded val
                        return fmt::format_to(ctx.out(), spec_builder(), next_float(next_float(next_float(val))));
                    }
                }
            } else {
                specs_.type = 'E';
                auto str = fmt::format(spec_builder(), next_float(val));
                return fmt::format_to(ctx.out(), "{}", zero_pad_exponent(str));
            }
        } else if (specs_.type == 'T') { // matches TrimSigDigits behavior
            const auto fixed_output = should_be_fixed_output(val);

            if (fixed_output) {
                const auto magnitude = std::pow(10, specs_.precision);
                const auto adjusted = (val * magnitude) + 0.0001;
                const auto truncated = std::trunc(adjusted) / magnitude;
                specs_.type = 'F';
                return fmt::format_to(ctx.out(), spec_builder(), truncated);
            } else {
                specs_.type = 'E';
                specs_.precision += 2;

                // write the `E` formatted float to a std::string
                auto str = fmt::format(spec_builder(), val);
                str = zero_pad_exponent(str);

                // Erase last 2 numbers to truncate the value
                const auto E_itr = std::find(begin(str), end(str), 'E');
                if (E_itr != str.end()) {
                    str.erase(std::prev(E_itr, 2), E_itr);
                }

                return fmt::format_to(ctx.out(), "{}", str);
            }
        }
        return fmt::format_to(ctx.out(), spec_builder(), val);
    }
};
} // namespace fmt

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

enum class FormatSyntax
{
    Invalid = -1,
    Fortran,
    FMT,
    Printf,
    Num
};

inline constexpr bool is_fortran_syntax(const std::string_view format_str)
{
    bool within_fmt_str = false;
    for (auto const c : format_str) {
        switch (c) {
        case '{':
            within_fmt_str = true;
            break;
        case '}':
            within_fmt_str = false;
            break;
        case 'R':
        case 'T':
            if (within_fmt_str) {
                return true;
            } else {
                break;
            }
        default:
            break;
        }
    }
    return false;
}

class InputOutputFile;
template <FormatSyntax formatSyntax = FormatSyntax::Fortran, typename... Args>
void print(InputOutputFile &outputFile, std::string_view format_str, Args &&...args);

inline constexpr FormatSyntax check_syntax(const std::string_view format_str)
{
    if (is_fortran_syntax(format_str)) {
        return FormatSyntax::Fortran;
    } else {
        return FormatSyntax::FMT;
    }
}

class InputFile
{
public:
    template <typename Type> struct ReadResult
    {
        ReadResult(Type data_, bool eof_, bool good_) : data{std::move(data_)}, eof{eof_}, good{good_}
        {
        }

        // Update the current eof/good state from the incoming value
        // but only update the `data` member if the state is good
        // The idea is to keep consistency with the operator>> that was used
        // from gio
        void update(ReadResult &&other)
        {
            eof = other.eof;
            good = other.good;
            if (good) {
                data = std::move(other.data);
            }
        }

        Type data;
        bool eof;
        bool good;
    };

    void close();

    // This is different from istream::good(), which is false if EOF is true while there were no errors (happens when no EOL at end of file)
    // this operate like `operator bool(istream& is)` <=> `!is.bad() && !is.fail()`
    bool good() const noexcept;

    bool is_open() const noexcept;

    void backspace() noexcept;

    std::string error_state_to_string() const;

    // opens the file if it is not currently open and returns
    // a reference back to itself
    InputFile &ensure_open(EnergyPlusData &state, const std::string &caller, bool output_to_file = true);
    std::istream::iostate rdstate() const noexcept;

    fs::path filePath;
    void open(bool = false, bool = true);
    std::fstream::pos_type position() const noexcept;

    void rewind() noexcept
    {
        if (is) {
            is->clear(); // clear potentially failbit and badbit (seekg would only clear eofbit)
            is->seekg(0, std::ios::beg);
        }
    }

    ReadResult<std::string> readLine() noexcept;

    template <typename T> ReadResult<T> read() noexcept
    {
        if (is) {
            T result;
            *is >> result;
            // Use operator bool, see ReadResult::good() docstring
            return ReadResult<T>{result, is->eof(), bool(is)};
        } else {
            return ReadResult<T>{T{}, true, false};
        }
    }

    std::string readFile();

    nlohmann::json readJSON();

    explicit InputFile(fs::path FilePath);

private:
    std::uintmax_t file_size{};
    std::unique_ptr<std::istream> is;
    friend class IOFiles;
};

class InputOutputFile
{
public:
    fs::path filePath;
    bool defaultToStdOut = false;

    void close();
    void del();
    bool good() const;

    // opens the file if it is not currently open and returns
    // a reference back to itself
    InputOutputFile &ensure_open(EnergyPlusData &state, const std::string &caller, bool output_to_file = true);

    void open(const bool forAppend = false, bool output_to_file = true);
    std::fstream::pos_type position() const noexcept;
    std::vector<std::string> getLines();
    void open_as_stringstream();
    std::string get_output();
    void flush();
    explicit InputOutputFile(fs::path FilePath, const bool DefaultToStdOut = false);

private:
    std::unique_ptr<std::iostream> os;
    bool print_to_dev_null = false;
    template <FormatSyntax, typename... Args> friend void print(InputOutputFile &outputFile, std::string_view format_str, Args &&...args);
    friend class IOFiles;
};

template <typename FileType> struct IOFilePath
{
    fs::path filePath;
    FileType open(EnergyPlusData &state, const std::string &caller, bool output_to_file = true)
    {
        FileType file{filePath};
        file.ensure_open(state, caller, output_to_file);
        return file;
    }
    FileType try_open(bool output_to_file = true)
    {
        FileType file{filePath};
        file.open(false, output_to_file);
        return file;
    }
};

using InputOutputFilePath = IOFilePath<InputOutputFile>;
using InputFilePath = IOFilePath<InputFile>;

struct JsonOutputFilePaths
{
    fs::path outputJsonFilePath;
    fs::path outputTSHvacJsonFilePath;
    fs::path outputTSZoneJsonFilePath;
    fs::path outputTSJsonFilePath;
    fs::path outputYRJsonFilePath;
    fs::path outputMNJsonFilePath;
    fs::path outputDYJsonFilePath;
    fs::path outputHRJsonFilePath;
    fs::path outputSMJsonFilePath;
    fs::path outputCborFilePath;
    fs::path outputTSHvacCborFilePath;
    fs::path outputTSZoneCborFilePath;
    fs::path outputTSCborFilePath;
    fs::path outputYRCborFilePath;
    fs::path outputMNCborFilePath;
    fs::path outputDYCborFilePath;
    fs::path outputHRCborFilePath;
    fs::path outputSMCborFilePath;
    fs::path outputMsgPackFilePath;
    fs::path outputTSHvacMsgPackFilePath;
    fs::path outputTSZoneMsgPackFilePath;
    fs::path outputTSMsgPackFilePath;
    fs::path outputYRMsgPackFilePath;
    fs::path outputMNMsgPackFilePath;
    fs::path outputDYMsgPackFilePath;
    fs::path outputHRMsgPackFilePath;
    fs::path outputSMMsgPackFilePath;
};

class IOFiles
{
public:
    struct OutputControl
    {
        OutputControl() = default;

        void getInput(EnergyPlusData &state);
        bool writeTabular(EnergyPlusData &state);

        bool csv = false;
        bool mtr = true;
        bool eso = true;
        bool eio = true;
        bool audit = true;
        bool spsz = true;
        bool zsz = true;
        bool ssz = true;
        bool psz = true;
        bool dxf = true;
        bool bnd = true;
        bool rdd = true;
        bool mdd = true;
        bool mtd = true;
        bool end = true;
        bool shd = true;
        bool dfs = true;
        bool delightin = true;
        bool delighteldmp = true;
        bool delightdfdmp = true;
        bool edd = true;
        bool dbg = true;
        bool perflog = true;
        bool sln = true;
        bool sci = true;
        bool wrl = true;
        bool screen = true;
        bool tarcog = true;
        bool extshd = true;
        bool json = true;
        bool tabular = true;
        bool sqlite = true;
    };

    OutputControl outputControl;

    InputOutputFile audit{"eplusout.audit"};
    InputOutputFile eio{"eplusout.eio"};
    InputOutputFile eso{"eplusout.eso"}; // (hourly data only)

    InputOutputFile zsz{""};
    fs::path outputZszCsvFilePath{"epluszsz.csv"};
    fs::path outputZszTabFilePath{"epluszsz.tab"};
    fs::path outputZszTxtFilePath{"epluszsz.txt"};

    InputOutputFile spsz{""};
    fs::path outputSpszCsvFilePath{"eplusspsz.csv"};
    fs::path outputSpszTabFilePath{"eplusspsz.tab"};
    fs::path outputSpszTxtFilePath{"eplusspsz.txt"};

    InputOutputFile ssz{""};
    fs::path outputSszCsvFilePath{"eplusssz.csv"};
    fs::path outputSszTabFilePath{"eplusssz.tab"};
    fs::path outputSszTxtFilePath{"eplusssz.txt"};

    InputOutputFile psz{""};
    fs::path outputPszCsvFilePath{"epluspsz.csv"};
    fs::path outputPszTabFilePath{"epluspsz.tab"};
    fs::path outputPszTxtFilePath{"epluspsz.txt"};

    InputOutputFile map{""};
    fs::path outputMapCsvFilePath{"eplusmap.csv"};
    fs::path outputMapTabFilePath{"eplusmap.tab"};
    fs::path outputMapTxtFilePath{"eplusmap.txt"};

    InputOutputFile mtr{"eplusout.mtr"};
    InputOutputFile bnd{"eplusout.bnd"};
    InputOutputFile rdd{"eplusout.rdd"};
    InputOutputFile mdd{"eplusout.mdd"};

    InputOutputFile debug{"eplusout.dbg"};

    InputOutputFile dfs{"eplusout.dfs"};

    InputOutputFilePath sln{"eplusout.sln"};
    InputOutputFilePath dxf{"eplusout.dxf"};
    InputOutputFilePath sci{"eplusout.sci"};
    InputOutputFilePath wrl{"eplusout.wrl"};

    InputOutputFilePath delightIn{"eplusout.delightin"};

    InputOutputFile mtd{"eplusout.mtd"};
    InputOutputFile edd{"eplusout.edd", true}; // write to stdout if no file never opened
    InputOutputFile shade{"eplusshading.csv"};

    InputOutputFile csv{"eplusout.csv"};
    InputOutputFile mtr_csv{"eplusmtr.csv"};

    InputOutputFilePath screenCsv{"eplusscreen.csv"};
    InputOutputFilePath endFile{"eplusout.end"};

    InputFilePath iniFile{"EnergyPlus.ini"};

    InputFilePath outputDelightEldmpFilePath{"eplusout.delighteldmp"};
    InputFilePath outputDelightDfdmpFilePath{"eplusout.delightdfdmp"};

    // for transient uses of weather files
    // also, keeper of the currently set input weather file name
    InputFilePath inputWeatherFilePath{""};

    // for the persistent weather simulation, using the EPW
    // uses the file name set in `inputWeatherFilePath`
    InputFile inputWeatherFile{""};

    InputFilePath TempFullFilePath{""};
    InputFilePath inStatFilePath{""};

    fs::path outputErrFilePath{"eplusout.err"};
    std::unique_ptr<std::ostream> err_stream;

    JsonOutputFilePaths json; // Internal streams used for json outputs

    void flushAll(); // For RunningEnergyPlusViaAPI only
};

class SharedFileHandle
{
    std::shared_ptr<InputOutputFile> file;
    InputOutputFile *ptr()
    {
        if (!file) {
            file = std::make_shared<InputOutputFile>("");
        }

        return file.get();
    }

public:
    InputOutputFile &operator*()
    {
        return *ptr();
    }

    InputOutputFile *operator->()
    {
        return ptr();
    }
};

template <typename... Args> void vprint(std::ostream &os, std::string_view format_str, const Args &...args)
{
    //    assert(os.good());
    auto buffer = fmt::memory_buffer();
    try {
        fmt::format_to(std::back_inserter(buffer), format_str, args...);
    } catch (const fmt::format_error &) {
        throw EnergyPlus::FatalError(fmt::format("Error with format, '{}', passed {} args", format_str, sizeof...(Args)));
    }
    os.write(buffer.data(), buffer.size());
}

template <typename... Args> std::string vprint(std::string_view format_str, const Args &...args)
{
    auto buffer = fmt::memory_buffer();
    try {
        fmt::format_to(std::back_inserter(buffer), format_str, args...);
    } catch (const fmt::format_error &) {
        throw EnergyPlus::FatalError(fmt::format("Error with format, '{}', passed {} args", format_str, sizeof...(Args)));
    }
    return fmt::to_string(buffer);
}

// Uses lib {fmt} (which has been accepted for C++20)
// Formatting syntax guide is here: https://fmt.dev/latest/syntax.html
// The syntax is similar to printf, but uses {} to indicate parameters to be formatted
// you must escape any {} that you want with {}, like `{{}}`
//
// Defines a custom formatting type 'R' (round_ which chooses between `E` and `G` depending
// on the value being printed.
// This is necessary for parity with the old "RoundSigDigits" utility function
//
// Defines a custom formatting type 'T' that that truncates the value
// to match the behavior of TrimSigDigits utility function
//

namespace {
    template <typename... Args> void print_fortran_syntax(std::ostream &os, std::string_view format_str, const Args &...args)
    {
        EnergyPlus::vprint<std::conditional_t<std::is_same_v<double, Args>, DoubleWrapper, Args>...>(os, format_str, args...);
    }

    template <typename... Args> std::string format_fortran_syntax(std::string_view format_str, const Args &...args)
    {
        return EnergyPlus::vprint<std::conditional_t<std::is_same_v<double, Args>, DoubleWrapper, Args>...>(format_str, args...);
    }
} // namespace

template <FormatSyntax formatSyntax = FormatSyntax::Fortran, typename... Args>
void print(std::ostream &os, std::string_view format_str, Args &&...args)
{
    if constexpr (formatSyntax == FormatSyntax::Fortran) {
        print_fortran_syntax(os, format_str, args...);
    } else if constexpr (formatSyntax == FormatSyntax::FMT) {
        fmt::print(os, format_str, std::forward<Args>(args)...);
    } else {
        static_assert(!(formatSyntax == FormatSyntax::Fortran || formatSyntax == FormatSyntax::FMT), "Invalid FormatSyntax selection");
    }
}

template <FormatSyntax formatSyntax, typename... Args> void print(InputOutputFile &outputFile, std::string_view format_str, Args &&...args)
{
    auto *outputStream = [&]() -> std::ostream * {
        if (outputFile.os) {
            return outputFile.os.get();
        } else {
            if (outputFile.defaultToStdOut) {
                return &std::cout;
            } else {
                assert(outputFile.os);
                return nullptr;
            }
        }
    }();
    if constexpr (formatSyntax == FormatSyntax::Fortran) {
        print_fortran_syntax(*outputStream, format_str, args...);
    } else if constexpr (formatSyntax == FormatSyntax::FMT) {
        fmt::print(*outputStream, format_str, std::forward<Args>(args)...);
    } else {
        static_assert(!(formatSyntax == FormatSyntax::Fortran || formatSyntax == FormatSyntax::FMT), "Invalid FormatSyntax selection");
    }
}

template <FormatSyntax formatSyntax = FormatSyntax::Fortran, typename... Args> std::string format(std::string_view format_str, Args &&...args)
{
    if constexpr (formatSyntax == FormatSyntax::Fortran) {
        return format_fortran_syntax(format_str, args...);
    } else if constexpr (formatSyntax == FormatSyntax::FMT) {
        return fmt::format(format_str, std::forward<Args>(args)...);
    } else if constexpr (formatSyntax == FormatSyntax::Printf) {
        return fmt::sprintf(format_str, std::forward<Args>(args)...);
    }
}

} // namespace EnergyPlus

// extern template the most commonly used format function calls
// to save on compilation time. They will be explicitly instantiated
// in IOFiles.cc
extern template std::string EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, int>(std::string_view, int &&);
extern template std::string EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, const char *const &>(std::string_view, const char *const &);
extern template std::string EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, int &, std::string &>(std::string_view, int &, std::string &);
extern template std::string EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, std::string &, std::string &, std::string &, double &>(
    std::string_view, std::string &, std::string &, std::string &, double &);
extern template std::string EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, const std::string_view &>(std::string_view,
                                                                                                            const std::string_view &);
extern template std::string EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, const std::string_view &, std::string &>(std::string_view,
                                                                                                                           const std::string_view &,
                                                                                                                           std::string &);
extern template std::string
EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, std::string &, double &, double &>(std::string_view, std::string &, double &, double &);
extern template std::string
EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, std::string &, std::string &, int &>(std::string_view, std::string &, std::string &, int &);
extern template std::string
EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, double &, double &, double &>(std::string_view, double &, double &, double &);
extern template std::string EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, double &, std::string &>(std::string_view, double &, std::string &);
extern template std::string EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, std::string &>(std::string_view, std::string &);
extern template std::string EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, const int &, int &>(std::string_view, const int &, int &);
extern template std::string EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, double>(std::string_view, double &&);
extern template std::string EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, int &, int &>(std::string_view, int &, int &);
extern template std::string EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, const double &>(std::string_view, const double &);
extern template std::string EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, std::string &, int &>(std::string_view, std::string &, int &);
extern template std::string EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, std::string &, std::string &, double &>(std::string_view,
                                                                                                                          std::string &,
                                                                                                                          std::string &,
                                                                                                                          double &);
extern template std::string EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, std::string &, double &, std::string &, double &>(
    std::string_view, std::string &, double &, std::string &, double &);
extern template std::string EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, const int &>(std::string_view, const int &);
extern template std::string EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, int &, const std::string &, std::string &>(std::string_view,
                                                                                                                             int &,
                                                                                                                             const std::string &,
                                                                                                                             std::string &);
extern template std::string
EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, int &, int &, const std::string &>(std::string_view, int &, int &, const std::string &);
extern template std::string
EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, int &, int &, std::string_view &>(std::string_view, int &, int &, std::string_view &);
extern template std::string EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, int &, std::string_view &, std::string &>(std::string_view,
                                                                                                                            int &,
                                                                                                                            std::string_view &,
                                                                                                                            std::string &);
extern template std::string EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, double &, double &>(std::string_view, double &, double &);
extern template std::string EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, int &>(std::string_view, int &);
extern template std::string EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, std::string &, double &>(std::string_view, std::string &, double &);
extern template std::string EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, double &>(std::string_view, double &);

#endif
