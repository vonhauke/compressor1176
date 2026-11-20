#include "Parameters.h"

namespace breu::param
{
namespace
{
    juce::ParameterID id (const char* name) { return { name, version }; }

    // Posição de botão giratório, 0 a 10, como nas marcações de um painel.
    // A relação posição -> valor elétrico (atenuação, resistência) é feita no
    // modelo do circuito, a partir das etapas (b) e (c).
    std::unique_ptr<juce::AudioParameterFloat> knob (const char* name,
                                                     const juce::String& label,
                                                     float minPos, float maxPos,
                                                     float defaultPos)
    {
        auto attrs = juce::AudioParameterFloatAttributes()
                         .withStringFromValueFunction ([] (float v, int) { return juce::String (v, 1); });
        return std::make_unique<juce::AudioParameterFloat> (id (name), label,
                                                            juce::NormalisableRange<float> (minPos, maxPos, 0.0f),
                                                            defaultPos, attrs);
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (knob (input,  "Input",  0.0f, 10.0f, 3.0f));
    layout.add (knob (output, "Output", 0.0f, 10.0f, 5.0f));

    // Ataque e release: 1 a 7, sentido horário = MAIS RÁPIDO (como no original).
    layout.add (knob (attack,  "Attack",  1.0f, 7.0f, 3.0f));
    layout.add (knob (release, "Release", 1.0f, 7.0f, 5.0f));

    // No original, o fim anti-horário do botão de ataque é uma chave que
    // desliga a redução de ganho.
    layout.add (std::make_unique<juce::AudioParameterBool> (id (attackOff), "Attack Off", false));

    layout.add (std::make_unique<juce::AudioParameterChoice> (
        id (ratio), "Ratio", juce::StringArray { "4:1", "8:1", "12:1", "20:1", "All", "Off" }, 0));

    layout.add (std::make_unique<juce::AudioParameterChoice> (
        id (meter), "Meter", juce::StringArray { "GR", "+4", "+8", "Off" }, 0));

    layout.add (std::make_unique<juce::AudioParameterBool> (id (bypass), "Bypass", false));

    layout.add (std::make_unique<juce::AudioParameterChoice> (
        id (quality), "Quality", juce::StringArray { "Normal", "Alta" }, 0));

    return layout;
}
}
