cmake_minimum_required(VERSION 3.22)

project(Breu VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(APPLE)
    # Plugin de Apple Silicon; macOS 11 é a primeira versão com suporte a arm64.
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Versão mínima do macOS")
endif()

# ---------------------------------------------------------------------------
# JUCE, versão fixada. Para compilar sem baixar de novo, passe
#   -DFETCHCONTENT_SOURCE_DIR_JUCE=<pasta com o JUCE 9.0.6>
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        9.0.6
    GIT_SHALLOW    TRUE)
FetchContent_MakeAvailable(JUCE)

# ---------------------------------------------------------------------------
juce_add_plugin(Breu
    COMPANY_NAME             "vonhauke"
    BUNDLE_ID                "com.vonhauke.breu"
    PLUGIN_MANUFACTURER_CODE Vonh
    PLUGIN_CODE              Breu
    FORMATS                  VST3 Standalone
    PRODUCT_NAME             "Breu"
    VST3_CATEGORIES          Fx Dynamics
    IS_SYNTH                 FALSE
    NEEDS_MIDI_INPUT         FALSE
    NEEDS_MIDI_OUTPUT        FALSE
    NEEDS_WEB_BROWSER        FALSE
    NEEDS_CURL               FALSE
    COPY_PLUGIN_AFTER_BUILD  FALSE)

target_sources(Breu
    PRIVATE
        src/PluginProcessor.cpp
        src/Parameters.cpp
        src/dsp/OversampledPath.cpp)

target_include_directories(Breu PUBLIC src)

target_compile_definitions(Breu
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0)

target_link_libraries(Breu
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

# ---------------------------------------------------------------------------
# Testes automáticos (executável de console que usa o mesmo código do plugin)
option(BREU_BUILD_TESTS "Compila os testes" ON)
if(BREU_BUILD_TESTS)
    enable_testing()
    juce_add_console_app(BreuTests PRODUCT_NAME "BreuTests")
    # Os testes compilam os mesmos arquivos do plugin (não ligam com o alvo
    # do plugin, para não misturar duas configurações do JUCE no mesmo binário).
    target_sources(BreuTests
        PRIVATE
            tests/Tests.cpp
            src/PluginProcessor.cpp
            src/Parameters.cpp
            src/dsp/OversampledPath.cpp)
    target_include_directories(BreuTests PRIVATE src)
    target_compile_definitions(BreuTests
        PRIVATE
            JUCE_WEB_BROWSER=0
            JUCE_USE_CURL=0
            JucePlugin_Name="Breu"
            JucePlugin_VersionString="${PROJECT_VERSION}")
    target_link_libraries(BreuTests
        PRIVATE
            juce::juce_audio_utils
            juce::juce_dsp
        PUBLIC
            juce::juce_recommended_config_flags
            juce::juce_recommended_warning_flags)
    add_test(NAME BreuTests COMMAND BreuTests)
endif()
