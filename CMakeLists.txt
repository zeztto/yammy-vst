cmake_minimum_required(VERSION 3.15)

project(YAMMY VERSION 0.0.1)

# JUCE setup
add_subdirectory(JUCE)

juce_add_plugin(YAMMY
    COMPANY_NAME "h4ppy Labs"
    BUNDLE_ID "com.h4ppyLabs.YAMMY"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE "H4py"
    PLUGIN_CODE "Yamy"
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "YAMMY"
)

target_sources(YAMMY
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/DSP/PitchShifter.cpp
        Source/DSP/PitchShifter.h
        Source/UI/StyleSheet.h
)

target_compile_features(YAMMY PUBLIC cxx_std_17)

juce_generate_juce_header(YAMMY)

target_compile_definitions(YAMMY
    PUBLIC
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(YAMMY
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
