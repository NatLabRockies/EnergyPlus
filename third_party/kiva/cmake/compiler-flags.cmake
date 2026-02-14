add_library(kiva_common_interface INTERFACE)

  #================#
  # Compiler flags #
  #================#

target_compile_options(kiva_common_interface INTERFACE
$<$<CXX_COMPILER_ID:MSVC>:
  /W4 # Warning level (default is W3)
  $<$<BOOL:${KIVA_WERROR}>:
    /WX # Turn warnings into errors
  >
>
$<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:
  -Wall
  -Wextra
  -Wpedantic
  $<$<BOOL:${KIVA_WERROR}>:
    -Werror # Turn warnings into errors
  >
>

)
