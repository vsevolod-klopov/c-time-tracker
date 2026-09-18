cmake_minimum_required(VERSION 3.16)

project(intec_time_tracker LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --------------------------------------------------
# CURL
# --------------------------------------------------

find_package(CURL CONFIG REQUIRED)


# --------------------------------------------------
# Source files
# --------------------------------------------------

set(SRC
    apiClient.cpp
    main.cpp
)

# Windows event source
if(WIN32)

    list(APPEND SRC
        eventSourceWin.cpp
    )

endif()

# Linux event source
if(UNIX AND NOT WIN32)

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/eventSourcePosix.cpp")

        list(APPEND SRC
            eventSourcePosix.cpp
        )

    endif()

endif()


# --------------------------------------------------
# Executable
# --------------------------------------------------

add_executable(intec_time_tracker
    ${SRC}
)


# --------------------------------------------------
# Include directories
# --------------------------------------------------

target_include_directories(intec_time_tracker
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)


# --------------------------------------------------
# CURL
# --------------------------------------------------

target_link_libraries(intec_time_tracker
    PRIVATE
        CURL::libcurl
)


# --------------------------------------------------
# Linux dependencies
# --------------------------------------------------

if(UNIX AND NOT WIN32)

    # X11
    find_package(X11 REQUIRED)

    target_include_directories(intec_time_tracker
        PRIVATE
            ${X11_INCLUDE_DIR}
    )

    target_link_libraries(intec_time_tracker
        PRIVATE
            ${X11_LIBRARIES}
    )


    # X11 Screensaver extension
    find_library(XSS_LIB Xss)

    if(XSS_LIB)

        target_link_libraries(intec_time_tracker
            PRIVATE
                ${XSS_LIB}
        )

    endif()


    # Threads
    find_package(Threads REQUIRED)

    target_link_libraries(intec_time_tracker
        PRIVATE
            Threads::Threads
    )

endif()


# --------------------------------------------------
# Windows
# --------------------------------------------------

if(WIN32)

    target_compile_definitions(intec_time_tracker
        PRIVATE
            _CRT_SECURE_NO_WARNINGS
    )

endif()