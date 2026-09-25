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
# git clone ではなくアーカイブで取得する(Windows でフォルダが深いと git の "Filename too long" で失敗するため)
FetchContent_Declare(
    raylib
    URL      https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
    URL_HASH SHA256=2b3ee1e2120c7a0796b33062c7e9a694dd8a8caa56a96319ac8c8ecf54a90d0b
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
# Web ビルド時は raylib 側が EMSCRIPTEN を検出して PLATFORM=Web(PLATFORM_WEB 定義)になる
FetchContent_MakeAvailable(raylib)

# ------------------------------------------------------------
# 共通のコンパイル設定
# ------------------------------------------------------------
function(kansei_set_compile_options target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /utf-8 /W4)  # 日本語文字列を UTF-8 として扱う
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra)
    endif()
endfunction()

# ------------------------------------------------------------
# core 層 + game 層(ルールだけ)
# ------------------------------------------------------------
# raylib をリンクしないライブラリにすることで、game/ や core/ から
# #include "raylib.h" するとコンパイルエラーになる(層の依存方向をビルドで強制する)。
# .cpp を追加したら CMake の再構成時に自動で拾われる。
file(GLOB_RECURSE KANSEI_GAME_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/core/*.cpp" "${CMAKE_SOURCE_DIR}/src/core/*.h"
    "${CMAKE_SOURCE_DIR}/src/game/*.cpp" "${CMAKE_SOURCE_DIR}/src/game/*.h")
add_library(kansei_game STATIC ${KANSEI_GAME_SOURCES})
target_include_directories(kansei_game PUBLIC "${CMAKE_SOURCE_DIR}/src")  # "game/..." のように src からのパスで include する
kansei_set_compile_options(kansei_game)

# ------------------------------------------------------------
# 盤面ロジックのテスト(ネイティブのみ。raylib なしで動く)
# ------------------------------------------------------------
if(NOT EMSCRIPTEN)
    add_executable(KanseiTests tests/BoardTest.cpp)
    target_link_libraries(KanseiTests PRIVATE kansei_game)
    kansei_set_compile_options(KanseiTests)
    enable_testing()
    add_test(NAME BoardTest COMMAND KanseiTests)
endif()

# ------------------------------------------------------------
# ゲーム本体(infrastructure 層 + 起動処理)
# ------------------------------------------------------------
file(GLOB_RECURSE KANSEI_APP_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.cpp" "${CMAKE_SOURCE_DIR}/src/*.h")
list(FILTER KANSEI_APP_SOURCES EXCLUDE REGEX "/src/(core|game)/")
add_executable(${PROJECT_NAME} ${KANSEI_APP_SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE kansei_game raylib)
kansei_set_compile_options(${PROJECT_NAME})

if(MSVC)
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
    # VS のデバッガから起動したときもカレントディレクトリをプロジェクト直下にし、resources/ を読めるようにする
    set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
    # vcpkg の MSBuild 統合(vcpkg integrate install)が入った PC で余計な後処理が走らないようにする
    set_target_properties(${PROJECT_NAME} PROPERTIES VS_GLOBAL_VcpkgEnabled false)
    set_target_properties(kansei_game KanseiTests PROPERTIES VS_GLOBAL_VcpkgEnabled false)
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

# ------------------------------------------------------------
# Web (Emscripten) 専用の設定
# ------------------------------------------------------------
if(EMSCRIPTEN)
    # 出力を <build>/web/index.{html,js,wasm,data} にする。
    # このフォルダの中身をそのまま zip すれば itch.io に提出できる。
    set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "index"
        SUFFIX ".html"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/web"
    )

    target_link_options(${PROJECT_NAME} PRIVATE
        # 【Web の落とし穴: ファイルアクセス】
        # ブラウザの wasm は PC のファイルを直接読めない。resources/ を仮想ファイルシステムに
        # 詰めた index.data を生成し、起動時にダウンロードさせる(LoadTexture("resources/...") がそのまま動く)。
        "--preload-file=${CMAKE_SOURCE_DIR}/resources@resources"
        "--shell-file=${CMAKE_SOURCE_DIR}/web/shell.html"
        -sALLOW_MEMORY_GROWTH=1     # 画像・音を読み込むとメモリが足りなくなるので自動拡張を許可
        -sSTACK_SIZE=1048576        # 既定の 64KB だとフォントアトラス生成などで溢れることがある
        # 【Web の落とし穴: スレッド】
        # std::thread / pthread は既定では使えない(使うには COOP/COEP ヘッダが必要で itch.io では面倒)。
        # 重い処理はフレームをまたいで少しずつ進める設計にする。
    )
    target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release,MinSizeRel>:-Os>)
    target_link_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release,MinSizeRel>:-Os>)

    # resources/ にファイルを追加・変更したら index.data を作り直すよう、リンクの依存に入れておく
    file(GLOB_RECURSE RESOURCE_FILES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/resources/*")
    set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY LINK_DEPENDS
        ${RESOURCE_FILES} "${CMAKE_SOURCE_DIR}/web/shell.html")

    # cmake --build <build> --target package_web で itch.io 提出用の zip を作る
    add_custom_target(package_web
        COMMAND ${CMAKE_COMMAND} -E tar cf "${CMAKE_BINARY_DIR}/${PROJECT_NAME}-web.zip" --format=zip
                index.html index.js index.wasm index.data
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/web"
        DEPENDS ${PROJECT_NAME}
        COMMENT "Creating ${PROJECT_NAME}-web.zip for itch.io"
    )
endif()
