set(public_headers
        btwxt.h
        grid-axis.h
        grid-point-data.h
        logging.h
        regular-grid-interpolator.h
        )

add_library(${PROJECT_NAME}_interface INTERFACE ${public_headers})
target_include_directories(${PROJECT_NAME}_interface INTERFACE "${PROJECT_SOURCE_DIR}/include")