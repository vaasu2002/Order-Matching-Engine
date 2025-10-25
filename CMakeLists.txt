cmake_minimum_required(VERSION 3.16)    # 3.16+ for FetchContent convenience; you had 3.28.3 which is fine
project(OrderMatchingEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)  # use -std=c++20, not -std=gnu++20

# --- sources ---
set(SOURCES
        Scheduler/Worker/Task.h
        Scheduler/Worker/Worker.cpp
        Scheduler/Worker/Worker.h
        Scheduler/Scheduler.h
        Scheduler/Scheduler.cpp
        main.cpp
        OrderBook/Order/Types.h
        OrderBook/Order/Order.h
        OrderBook/Order/Validation.h
        OrderBook/Order/Validation.cpp
        OrderBook/PriceLevel/PriceLevel.cpp
        OrderBook/PriceLevel/PriceLevel.h
        OrderBook/OrderTracker/OrderTracker.cpp
        OrderBook/OrderTracker/OrderTracker.h
        OrderBook/OrderBook.cpp
        OrderBook/OrderBook.h
        OrderBook/OrderBook_Registry.cpp
        Scheduler/OrderBookScheduler.cpp
        Scheduler/OrderBookScheduler.h
        Application.cpp
        Application.h
        Config/ConfigReader.cpp
        Config/ConfigReader.h
        Scheduler/OrderInjectorScheduler.cpp
        Scheduler/OrderInjectorScheduler.h
)

add_executable(OrderMatchingEngine ${SOURCES})

# --- TinyXML2 resolution & linking (robust) ---
# Try config-mode first (preferred)
find_package(TinyXML2 QUIET)

# If find_package created an imported target, use it immediately.
if(TinyXML2_FOUND AND TARGET TinyXML2::TinyXML2)
    message(STATUS "Using imported target TinyXML2::TinyXML2")
    target_link_libraries(OrderMatchingEngine PRIVATE TinyXML2::TinyXML2)
else()
    # Try common lowercase target (from tinyxml2's own CMake)
    if(TARGET tinyxml2)
        message(STATUS "Using target tinyxml2")
        target_link_libraries(OrderMatchingEngine PRIVATE tinyxml2)
    else()
        # Try pkg-config
        find_package(PkgConfig QUIET)
        if(PKG_CONFIG_FOUND)
            pkg_check_modules(PKG_TINYXML2 QUIET tinyxml2)
            if(PKG_TINYXML2_FOUND)
                message(STATUS "Using tinyxml2 via pkg-config")
                target_include_directories(OrderMatchingEngine PRIVATE ${PKG_TINYXML2_INCLUDE_DIRS})
                target_link_libraries(OrderMatchingEngine PRIVATE ${PKG_TINYXML2_LIBRARIES})
            endif()
        endif()
    endif()
endif()

# If still not linked, try manual find_library/find_path (works on many Linux installs)
if(NOT TARGET tinyxml2 AND NOT (TARGET TinyXML2::TinyXML2) AND NOT (DEFINED PKG_TINYXML2_FOUND AND PKG_TINYXML2_FOUND))
    find_library(TINYXML2_LIB NAMES tinyxml2 PATHS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib /opt/homebrew/opt/tinyxml2/lib)
    find_path(TINYXML2_INCLUDE tinyxml2.h PATHS /usr/include /usr/local/include /opt/homebrew/opt/tinyxml2/include)
    if(TINYXML2_LIB AND TINYXML2_INCLUDE)
        message(STATUS "Linking tinyxml2 manually: ${TINYXML2_LIB}, includes: ${TINYXML2_INCLUDE}")
        target_include_directories(OrderMatchingEngine PRIVATE ${TINYXML2_INCLUDE})
        target_link_libraries(OrderMatchingEngine PRIVATE ${TINYXML2_LIB})
    endif()
endif()

# Final fallback: FetchContent to download and build tinyxml2
if(NOT (TARGET tinyxml2) AND NOT (TARGET TinyXML2::TinyXML2) AND NOT (TINYXML2_LIB AND TINYXML2_INCLUDE) AND NOT (DEFINED PKG_TINYXML2_FOUND AND PKG_TINYXML2_FOUND))
    message(STATUS "TinyXML2 not found on system — using FetchContent to build tinyxml2.")
    include(FetchContent)
    FetchContent_Declare(
            tinyxml2
            GIT_REPOSITORY https://github.com/leethomason/tinyxml2.git
            GIT_TAG        master
    )
    FetchContent_MakeAvailable(tinyxml2)

    if(TARGET tinyxml2)
        target_link_libraries(OrderMatchingEngine PRIVATE tinyxml2)
    elseif(TARGET TinyXML2::TinyXML2)
        target_link_libraries(OrderMatchingEngine PRIVATE TinyXML2::TinyXML2)
    else()
        message(FATAL_ERROR "tinyxml2 downloaded but no known CMake target found.")
    endif()
endif()