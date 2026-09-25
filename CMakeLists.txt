cmake_minimum_required(VERSION 3.24)
project(KanseiPalette LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ------------------------------------------------------------
# raylib を GitHub から自動取得(初回 configure 時のみダウンロード)
# ------------------------------------------------------------
include(FetchContent)
set(RAYLIB_VERSION 6.0)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)  # raylib 付属サンプルはビルドしない
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        ${RAYLIB_VERSION}
    GIT_SHALLOW    TRUE
)
# Web ビルド時は raylib 側が EMSCRIPTEN を検出して PLATFORM=Web(PLATFORM_WEB 定義)になる
FetchContent_MakeAvailable(raylib)

# ------------------------------------------------------------
# ゲーム本体
# ------------------------------------------------------------
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /utf-8 /W4)  # 日本語文字列を UTF-8 として扱う
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
    # VS のデバッガから起動したときもカレントディレクトリをプロジェクト直下にし、resources/ を読めるようにする
    set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
endif()

if(APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework IOKit" "-framework Cocoa" "-framework OpenGL")
endif()

# ------------------------------------------------------------
# リソース
# ------------------------------------------------------------
# ネイティブ: ビルド後に resources/ を実行ファイルの隣へコピーする
if(NOT EMSCRIPTEN)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_SOURCE_DIR}/resources" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources"
    )
endif()
