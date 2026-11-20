#include "PluginProcessor.h"

namespace breu
{
namespace
{
    constexpr int stateVersion = 1;
    constexpr double switchFadeSeconds = 0.020;
    constexpr double bypassFadeSeconds = 0.010;
}

BreuAudioProcessor::BreuAudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "BreuState", param::createLayout())
{
    bypassParam  = dynamic_cast<juce::AudioParameterBool*>   (apvts.getParameter (param::bypass));
    qualityParam = dynamic_cast<juce::AudioParameterChoice*> (apvts.getParameter (param::quality));
    jassert (bypassParam != nullptr && qualityParam != nullptr);
}

bool BreuAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& in  = layouts.getMainInputChannelSet();
    const auto& out = layouts.getMainOutputChannelSet();

    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;

    return in == out;
}

void BreuAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    const int numCh = juce::jmax (1, getTotalNumOutputChannels());
    maxBlock = juce::jmax (1, samplesPerBlock);

    paths[(size_t) param::Quality::normal].prepare (numCh, sampleRate, maxBlock, minRateNormal);
    paths[(size_t) param::Quality::high]  .prepare (numCh, sampleRate, maxBlock, minRateHigh);

    // Mesma latência nos dois modos: o caminho de menor latência ganha atraso extra.
    latency = juce::jmax (paths[0].getOversamplingLatency(), paths[1].getOversamplingLatency());
    for (auto& p : paths)
        p.setPadding (latency - p.getOversamplingLatency());

    dryDelay.prepare (numCh, latency);
    dryDelay.setDelay (latency);
    dryDelay.reset();

    scratchA.setSize (numCh, maxBlock, false, true, true);
    scratchB.setSize (numCh, maxBlock, false, true, true);
    dryBuffer.setSize (numCh, maxBlock, false, true, true);

    activePath = targetPath = qualityParam->getIndex();
    switchState = SwitchState::idle;
    fadeLength = juce::jmax (1, (int) std::lround (switchFadeSeconds * sampleRate));

    bypassMix.reset (sampleRate, bypassFadeSeconds);
    bypassMix.setCurrentAndTargetValue (bypassParam->get() ? 1.0f : 0.0f);

    setLatencySamples (latency);
}

void BreuAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int numCh = juce::jmin (buffer.getNumChannels(), scratchA.getNumChannels());

    for (int start = 0; start < buffer.getNumSamples(); start += maxBlock)
    {
        const int n = juce::jmin (maxBlock, buffer.getNumSamples() - start);
        std::array<float*, 2> ptrs {};
        for (int ch = 0; ch < numCh; ++ch)
            ptrs[(size_t) ch] = buffer.getWritePointer (ch, start);
        processChunk (ptrs.data(), numCh, n, false);
    }
}

void BreuAudioProcessor::processBlockBypassed (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    // Chamado por hosts que não usam o parâmetro de bypass: saída seca, mas com
    // o mesmo atraso do processamento, para não deslocar o áudio no tempo.
    juce::ScopedNoDenormals noDenormals;
    const int numCh = juce::jmin (buffer.getNumChannels(), scratchA.getNumChannels());

    for (int start = 0; start < buffer.getNumSamples(); start += maxBlock)
    {
        const int n = juce::jmin (maxBlock, buffer.getNumSamples() - start);
        std::array<float*, 2> ptrs {};
        for (int ch = 0; ch < numCh; ++ch)
            ptrs[(size_t) ch] = buffer.getWritePointer (ch, start);
        processChunk (ptrs.data(), numCh, n, true);
    }
}

void BreuAudioProcessor::processChunk (float* const* channels, int numCh, int n, bool forceBypass) noexcept
{
    // 1. Cópia seca, atrasada pela latência total.
    for (int ch = 0; ch < numCh; ++ch)
        dryBuffer.copyFrom (ch, 0, channels[ch], n);
    {
        juce::dsp::AudioBlock<float> dry (dryBuffer.getArrayOfWritePointers(), (size_t) numCh, (size_t) n);
        dryDelay.process (dry);
    }

    // 2. Pedido de troca de modo de qualidade.
    const int requested = qualityParam->getIndex();
    if (switchState == SwitchState::idle && requested != activePath)
    {
        targetPath = requested;
        paths[(size_t) targetPath].reset();
        warmRemaining = paths[(size_t) targetPath].getTotalLatency() + 64;
        switchState = SwitchState::warming;
    }

    // 3. Caminho ativo.
    for (int ch = 0; ch < numCh; ++ch)
        scratchA.copyFrom (ch, 0, channels[ch], n);
    juce::dsp::AudioBlock<float> blockA (scratchA.getArrayOfWritePointers(), (size_t) numCh, (size_t) n);
    paths[(size_t) activePath].process (blockA);

    // 4. Caminho novo, durante a troca.
    if (switchState != SwitchState::idle)
    {
        for (int ch = 0; ch < numCh; ++ch)
            scratchB.copyFrom (ch, 0, channels[ch], n);
        juce::dsp::AudioBlock<float> blockB (scratchB.getArrayOfWritePointers(), (size_t) numCh, (size_t) n);
        paths[(size_t) targetPath].process (blockB);

        for (int i = 0; i < n; ++i)
        {
            float g = 0.0f; // peso do caminho novo
            if (switchState == SwitchState::warming)
            {
                if (--warmRemaining <= 0)
                {
                    switchState = SwitchState::fading;
                    fadePos = 0;
                }
            }
            else if (switchState == SwitchState::fading)
            {
                g = (float) fadePos / (float) fadeLength;
                if (++fadePos >= fadeLength)
                {
                    g = 1.0f;
                    switchState = SwitchState::idle;
                    activePath = targetPath;
                }
            }

            for (int ch = 0; ch < numCh; ++ch)
            {
                auto* a = scratchA.getWritePointer (ch);
                a[i] = a[i] * (1.0f - g) + scratchB.getReadPointer (ch)[i] * g;
            }

            if (switchState == SwitchState::idle)
            {
                // Terminou no meio do bloco: o resto vem só do caminho novo.
                for (int ch = 0; ch < numCh; ++ch)
                    scratchA.copyFrom (ch, i + 1, scratchB, ch, i + 1, n - i - 1);
                break;
            }
        }
    }

    // 5. Mistura com o bypass (transição curta para não estalar).
    bypassMix.setTargetValue ((forceBypass || bypassParam->get()) ? 1.0f : 0.0f);

    if (! bypassMix.isSmoothing())
    {
        const bool dryOnly = bypassMix.getCurrentValue() >= 0.5f;
        for (int ch = 0; ch < numCh; ++ch)
            juce::FloatVectorOperations::copy (channels[ch],
                                               dryOnly ? dryBuffer.getReadPointer (ch) : scratchA.getReadPointer (ch),
                                               n);
        return;
    }

    for (int i = 0; i < n; ++i)
    {
        const float d = bypassMix.getNextValue();
        for (int ch = 0; ch < numCh; ++ch)
            channels[ch][i] = scratchA.getReadPointer (ch)[i] * (1.0f - d) + dryBuffer.getReadPointer (ch)[i] * d;
    }
}

//==============================================================================
void BreuAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty ("stateVersion", stateVersion, nullptr);
    state.setProperty ("pluginVersion", JucePlugin_VersionString, nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void BreuAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessorEditor* BreuAudioProcessor::createEditor()
{
    // Interface provisória (etapa e terá o painel). Mostra todos os parâmetros.
    return new juce::GenericAudioProcessorEditor (*this);
}
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new breu::BreuAudioProcessor();
}
