project(GLAD LANGUAGES C)

add_library(glad src/glad.c)

target_include_directories(glad PUBLIC include)
