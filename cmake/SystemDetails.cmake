macro(UNIX_SYSTEM_NICKNAME)
    # OS_RELEASE is the result of `uname -r` which is unhelpful (eg '5.4.0-42-generic')
    find_program(LSB_RELEASE lsb_release)
    # -rs outputs only 16.04, or 18.04
    execute_process(COMMAND ${LSB_RELEASE} -rs OUTPUT_VARIABLE LSB_RELEASE_VERSION_SHORT OUTPUT_STRIP_TRAILING_WHITESPACE)
    # -is outputs "Ubuntu" or "Fedora"
    execute_process(COMMAND ${LSB_RELEASE} -is OUTPUT_VARIABLE LSB_RELEASE_ID_SHORT OUTPUT_STRIP_TRAILING_WHITESPACE)
    # eg Ubuntu18.04
    set(SYSTEM_NICKNAME "${LSB_RELEASE_ID_SHORT}${LSB_RELEASE_VERSION_SHORT}")
endmacro()

macro(MAC_SYSTEM_NICKNAME)
    # Looking at cmake source code OS_RELEASE is already set to the output of `sw_vers -productVersion` which is what we want
    cmake_host_system_information(RESULT OSX_VERSION QUERY OS_RELEASE)
    message("-- OS_RELEASE variable is set to: " ${OSX_VERSION})
    if(NOT CMAKE_OSX_DEPLOYMENT_TARGET STREQUAL "")
        message("Using CMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}")
        set(OSX_VERSION "${CMAKE_OSX_DEPLOYMENT_TARGET}")
    endif()
    # The output is like 10.12.6 or 10.13, let's strip the end component if any
    string(REGEX REPLACE "^([0-9]+\\.[0-9]+)\\.?.*" "\\1" OSX_VERSION_MAJOR_MINOR ${OSX_VERSION})
    # eg macOS10.13
    set(SYSTEM_NICKNAME "macOS${OSX_VERSION_MAJOR_MINOR}")
endmacro()

macro(WINDOWS_SYSTEM_NICKNAME)
    set(SYSTEM_NICKNAME "")
endmacro()

macro(SET_SYSTEM_NICKNAME)
    set(SYSTEM_NICKNAME "Windows")
    if(APPLE)
        MAC_SYSTEM_NICKNAME()
    elseif(UNIX)
        UNIX_SYSTEM_NICKNAME()
    endif()
endmacro()
