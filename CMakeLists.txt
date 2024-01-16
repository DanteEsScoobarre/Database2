cmake_minimum_required(VERSION 3.27)
project(Database2)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(
        sfml
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 2.6.1
)
FetchContent_MakeAvailable(sfml)

add_executable(Database2 Database/main.cpp
        Database/Database.cpp
        Database/Database.h
        Database/Prerequestion.h
        Database/FileOps.cpp
        Database/FileOps.h
        Database/CLI.cpp
        Database/CLI.h
        Database/Expression.h
        Database/Parser.cpp
        Database/Parser.h)
target_link_libraries(
        Database2
        sfml-graphics
        sfml-window
        sfml-system
)
IF (WIN32)
    add_custom_command(TARGET Database2 POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:Database2> $<TARGET_FILE_DIR:Database2>
            COMMAND_EXPAND_LISTS
    )
    ENDIF ()