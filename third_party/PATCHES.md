# Third-party code changes

This file captures descriptive changes made to libraries in the third_party folder.
Occasionally, some of the third party library code we use needs small modifications to work.
This could be due to a bug in the code or a required enhancement.
These changes have a strong tendency to get overridden when the library code is updated from upstream.
This generally results in diffs or warnings that we then have to go back and slog through to see what had changed.
This file is intended to be a freeform list of the changes made, so that when a library is updated, we can quickly apply those again.
Each library should have a heading here, and the content of each section could be bullet list, a descriptive paragraph, a table, and would optimally include either SHAs or links to the PRs where the changes were made.

## Eigen

The Eigen library causes some build warnings when brought in as-is.
A couple minor changes clean these build warnings up.
This was most recently applied here: https://github.com/NREL/EnergyPlus/commit/cd8f41aeab471832824c9c7ac9f1d4acabebd8ae#diff-a6f7a8949616a88e6a39813eef96c9ca

## doj

The doj library had an issue with ordering numerics, so this was patched here: https://github.com/NREL/EnergyPlus/pull/7036

## json

As of [nlhohmann/json](https://github.com/nlohmann/json) version 3.7.3, the only changes needed are the use of the `doj` library for comparator.

cf [third_party/nlohmann/json3.12.0.patch](third_party/nlohmann/json3.12.0.patch)

## valijson

The `validation_visitor.hpp` was modified to include better error messages and use RE2 instead of std::regex.


## Kiva

Our vendored copy has `vendor/eigen` removed to avoid duplicating the shared `third_party/eigen`.
Since `libkiva` exports `vendor/eigen` as a PUBLIC include directory, `third_party/CMakeLists.txt` points the `libkiva` target at `third_party/eigen` after `add_subdirectory(kiva)`.

## libtk205

Our vendored copy has `vendor/btwxt` and `vendor/valijson` removed; its `vendor/CMakeLists.txt` skips adding them because the `btwxt` and `nlohmann_json` targets already exist by the time it is processed.
`src/CMakeLists.txt` was patched to link the `btwxt` target (getting its headers as a usage requirement) instead of including the removed `vendor/btwxt/include`. This change would be valid upstream too.

## SSC (SAM Simulation Core)

SSC (https://github.com/NREL/ssc) was brought in as a git subtree, last pulled at upstream `5e3099203a` (2021-03). It has since been trimmed down to what EnergyPlus actually uses: the `pvwattsv5_1ts` compute module (`PVWatts.cc`) and `lib_battery` (`ElectricPowerServiceManager`).

- Only the `ssc` and `shared` directories are kept, and only the files that are compiled or included (determined with `g++ -MM` on the compile commands). `ssc/vbapi.*` is kept because it is compiled on MSVC.
- `ssc/common.{h,cpp}`: removed `shading_factor_calculator::fbeam_shade_db` (only used by `pvsamv1`), which pulled the 3 MB shading database (`lib_pv_shade_loss_mpp`, `DB8_vmpp_impp_uint8_bin.h`) and `lib_miniz` into the link.
- `ssc/sscapi.{h,cpp}`: removed `json_to_ssc_data` / `ssc_data_to_json`, the only users of jsoncpp, which is no longer bundled.
- `shared/lib_shared_inverter.{h,cpp}`: removed the `OND_INVERTER` branches (`ond_inverter` is now an opaque type), so `lib_ondinv` and `splinter`, with its bundled Eigen 3.2.8 that clashed with `third_party/eigen` (ODR), are no longer built. The header keeps the includes and `using namespace std;` that `lib_ondinv.h` used to leak, so overload resolution in its includers is unchanged.
- `ssc/ssc_equations.h`: equations for compute modules that aren't compiled are commented out (an empty `{}` entry avoids a compiler error if none are left).
- Plus build fixes for newer compilers (GCC 13/15, Clang 17), the battery state fix (#10543) and the NaN fix for #11244: see `git log -- third_party/ssc`.

Code removals are marked with `// NOTE: EnergyPlus:` comments. Because of the trimming, `git subtree pull` would now conflict on every upstream file we deleted: to update SSC, re-vendor a release tag and re-apply the above.
