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


## SSC (SAM Simulation Core)

The SSC library is brought in as a git subtree from https://github.com/NREL/ssc. Generally we bring in a release tag instead of their `develop` branch. Then I have commented out many parts of SSC in the CMakeLists.txt and a few other files that are not used in EnergyPlus to minimize the size of the binary.

The first step to updating SSC is to add that repo as a remote in your local clone of EnergyPlus.

```
git remote add ssc git@github.com:NREL/ssc.git
git fetch ssc
```

Currently the most recent release of SSC merged into EnergyPlus is `2020.11.29.r0.ssc.250`. You can see the changes made to the base SSC library that we'll need to update after the merge by doing

```
git fetch ssc 2020.11.29.r0.ssc.250
git diff 2020.11.29.r0.ssc.250 HEAD:/third_party/ssc
```

The primary changes within `third_party/ssc` are as follows:

- `CMakeLists.txt` comment out libraries that aren't used (nlopt, lpsolve, solarpilot tcs)
- `shared/CMakeLists.` comment out header files that aren't used in the compute modules you're using. My approach was to comment them all out and look at the headers imported in `cmod_pvwatts5.cpp` and follow those imports backwards adding them back in as needed. It won't compile unless you get this right.
-  `ssc/CMakeLists.txt` Comment out all the cmod files you aren't using. Comment out unused dependencies.
-  `ssc/ssc_equations.h` Comment out equations for compute modules you aren't compiling anymore. If there aren't any left, add an empty item to the list `{}` so you avoid a compiler error. 

Updating SSC will likely create some merge conflicts here that will need to be resolved. 

To update SSC to a new release

```
git fetch ssc
git subtree pull --prefix=third_party/ssc --squash ssc new_release_tag
```
