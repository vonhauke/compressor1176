#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

// Identificadores estáveis dos parâmetros. NÃO renomear depois de publicado:
// o Ableton grava automação e estado por esses IDs.
namespace breu::param
{
    inline constexpr const char* input      = "input";
    inline constexpr const char* output     = "output";
    inline constexpr const char* attack     = "attack";
    inline constexpr const char* attackOff  = "attack_off";
    inline constexpr const char* release    = "release";
    inline constexpr const char* ratio      = "ratio";
    inline constexpr const char* meter      = "meter";
    inline constexpr const char* bypass     = "bypass";
    inline constexpr const char* quality    = "quality";

    // Versão dos parâmetros (juce::ParameterID). Só aumenta se um parâmetro
    // novo for acrescentado.
    inline constexpr int version = 1;

    enum class Ratio   { r4 = 0, r8, r12, r20, all, off };
    enum class Meter   { gr = 0, plus4, plus8, off };
    enum class Quality { normal = 0, high };

    juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
}
