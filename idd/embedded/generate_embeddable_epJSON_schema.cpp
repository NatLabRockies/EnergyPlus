#include <cstdint>  // std::uint8_t
#include <fstream>  // std::ifstream
#include <vector>   // std::vector
#ifndef __cppcheck__
#  if __has_include(<filesystem>)
#    include <filesystem>
namespace fs = std::filesystem;
#  elif __has_include(<experimental/filesystem>)
#    include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#  else
// cppcheck-suppress preprocessorErrorDirective
#    error "no filesystem support"
#  endif
#endif

#include <fmt/format.h>       // fmt::print
#include <fmt/os.h>           // fmt::output_file, fmt::file
#include <nlohmann/json.hpp>  // json

using json = nlohmann::json;

static constexpr auto header = R"cpp(
// EnergyPlus, Copyright (c) 1996-2023, The Board of Trustees of the University of Illinois,
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

#include <embedded/EmbeddedEpJSONSchema.hh>

#include <array>

namespace EnergyPlus {

namespace EmbeddedEpJSONSchema {

    // clang-format off
)cpp";

static constexpr auto footer = R"cpp(
    // clang-format on

    const gsl::span<const std::uint8_t> embeddedEpJSONSchema()
    {
        return gsl::span<const std::uint8_t>(embeddedSchema);
    }

    const std::string_view embeddedEpJSONSchemaView()
    {
        static const std::string str(embeddedSchema.begin(), embeddedSchema.end());
        return str;
    }

} // namespace EmbeddedEpJSONSchema

} // namespace EnergyPlus
)cpp";

int main(int argc, char const* argv[]) {
  if (argc != 3) {
    fmt::print(stderr, "usage: ./generate_embeddable_schema path/to/Energy+.schema.epJSON path/to/EmbeddedEpJSONSchema.cc\n");
    return 1;
  }

  fmt::print(stderr, "Generating the **embedded** epJSON schema\n");

  std::ifstream schema_stream(argv[1], std::ifstream::in);
  if (!schema_stream.is_open()) {
    fmt::print("schema file path {} not found\n", argv[1]);
    return 1;
  }
  auto const input_json = json::parse(schema_stream);
  std::vector<std::uint8_t> const v_cbor = json::to_cbor(input_json);

  const fs::path outFilePath(argv[2]);
  const auto outFileDir = outFilePath.parent_path();
  if (!fs::is_directory(outFileDir)) {
    fmt::print(stderr, "Output Directory does not exist: {}\n", outFileDir.generic_string());
    fs::create_directory(outFileDir);
  }

  auto outfile = fmt::output_file(argv[2], fmt::file::WRONLY | fmt::file::CREATE | fmt::file::TRUNC);
  outfile.print("{}", header);

  outfile.print("    static constexpr std::array< std::uint8_t, {} > embeddedSchema = {{{{\n", v_cbor.size());

  for (size_t i = 0; i < v_cbor.size(); ++i) {
    outfile.print("{:#04x},", v_cbor[i]);  // Format the std::uint8_t as hex
    if (i % 40 == 0 && i != 0) {
      outfile.print("\n");
    }
  }
  outfile.print("}}}};\n");
  outfile.print("{}", footer);

  return 0;
}
