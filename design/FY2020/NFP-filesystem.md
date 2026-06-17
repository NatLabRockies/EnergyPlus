Use C++17 std::filesystem library
==================================

**Julien Marrec, EffiBEM**

 - Original Date: 2020-12-07
 - Revision Date

## Justification for New Feature ##

[#8376](https://github.com/NatLabRockies/EnergyPlus/issues/8376) identified a bug with the `EnergyPlus::FileSystem::moveFile(std::string const &filePath, std::string const &destination)` function on Unix that calls the stdio [`rename`](https://www.cplusplus.com/reference/cstdio/rename/). The rename method would fail when `filePath` and `destination` aren't on the same device (eg: two different hard drives).

Looking into the `FileSystem.cc`, it is ridden with macros to conditionally call into win32 API or unix APIs.

Now that we have enabled C++17, we can leverage `std::filesystem` to simplify things greatly.

Note that there are only a handful of files that use the `FileSystem.hh` header, and by far the main user is `CommandLineInterface.cc`, which will call into the `FileSystem` functions like `getParentDirectoryPath` and use `std::string` concatenation to produce `std::string` representation of paths to be ultimately handled by a system call. Other files that use it mainly make use of `fileExists` or `removeFile` and are very easy to deal with and without much risks.

=> **Bottom line**: the proposed changes mainly affects the CLI.

**Note:** [#8376](https://github.com/NatLabRockies/EnergyPlus/issues/8376) was fixed by a minimal approach that doesn't include upgrading to `std::filesystem` (see [PR#8410](https://github.com/NatLabRockies/EnergyPlus/pull/8410)).
The `std::filesystem` was kept out of it and moved into this NFP so that it can be reviewed and approved (or not) by the development team.
If the minimal approach is taken, it will be a small defect; if the filesystem conversion approach is taken, it will be a larger defect.

## E-mail and  Conference Call Conclusions ##

insert text

## Approach ##

There are two things we can do:

1. Only modify the `FileSystem` namespace function to make use of `std::filesystem` internally, but keep providing functions that take in `std::string` arguments and return `std::string` representation of paths.
    * Minor adjustments will be needed to maintain backward compatibility. eg `getParentDirectoryPath` would call `fs::path::parent_path()` then will need to manually append a trailing slash for `std::string` concatenation to work in `CommandLineInterface` (though there are definitely some cases where that's also done in `CommandLineInterface`...).
2. Move everything to use `fs::path`, including in consumers.
    * `CommandLineInterface` would no longer use concatenation of std::string, like `std::string outputFilePrefix = std::string{outDirPathName} + std::string{prefixOutName};`, instead it would use `fs::path` and not have to deal with trailing slashes etc, it can just use the operator/ overload, eg: `fs::path outputFilePrefix = fs::path{outDirPathName} / prefixOutName;`

I have - for the most part - implemented Part 1., you can find a difference here: https://github.com/NatLabRockies/EnergyPlus/compare/8376_FileSystem_Move_minimal...jmarrec:8376_FileSystem_Move

## Testing/Validation/Data Sources ##

This will require testing the CLI on all three platforms: mac, windows, and ubuntu.


## References ##

* https://en.cppreference.com/w/cpp/filesystem/
* https://en.cppreference.com/w/cpp/filesystem/path
