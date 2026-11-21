#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

#include "Parameters.h"
#include "dsp/OversampledPath.h"

namespace breu
{
class BreuAudioProcessor final : public juce::AudioProcessor
{
public:
    BreuAudioProcessor();
    ~BreuAudioProcessor() override = default;

    //==========================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void processBlockBypassed (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;
    using AudioProcessor::processBlockBypassed;

    //==========================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override  { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorParameter* getBypassParameter() const override { return bypassParam; }

    //==========================================================================
    juce::AudioProcessorValueTreeState& getState() noexcept { return apvts; }

    // Informações para testes e medições.
    int    getReportedLatency() const noexcept { return latency; }
    int    getFactor (param::Quality q) const noexcept { return paths[(size_t) q].getFactor(); }
    double getInternalRate (param::Quality q) const noexcept { return paths[(size_t) q].getInternalRate(); }
    int    getActiveQualityIndex() const noexcept { return activePath; }
    bool   isSwitchingQuality() const noexcept { return switchState != SwitchState::idle; }

    // Taxa interna mínima de cada modo (plano, seção 4).
    static constexpr double minRateNormal = 176400.0;
    static constexpr double minRateHigh   = 352800.0;

private:
    void processChunk (float* const* channels, int numChannels, int numSamples, bool forceBypass) noexcept;

    juce::AudioProcessorValueTreeState apvts;
    juce::AudioParameterBool*   bypassParam  = nullptr;
    juce::AudioParameterChoice* qualityParam = nullptr;

    std::array<OversampledPath, 2> paths;
    IntegerDelay dryDelay;
    juce::AudioBuffer<float> scratchA, scratchB, dryBuffer;

    int latency = 0;
    int maxBlock = 0;

    // Troca de modo de qualidade sem estalo: o caminho novo "aquece" (passa
    // pela latência dele) e depois há uma transição suave.
    enum class SwitchState { idle, warming, fading };
    SwitchState switchState = SwitchState::idle;
    int activePath = 0;
    int targetPath = 0;
    int warmRemaining = 0;
    int fadeLength = 0;
    int fadePos = 0;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> bypassMix; // 0 = processado, 1 = seco

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BreuAudioProcessor)
};
}
