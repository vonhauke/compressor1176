#pragma once

#include <juce_dsp/juce_dsp.h>
#include <functional>
#include <vector>

namespace breu
{
// Atraso inteiro simples, por canal (sem interpolação).
class IntegerDelay
{
public:
    void prepare (int numChannels, int maxDelaySamples);
    void setDelay (int samples);
    int  getDelay() const noexcept { return delay; }
    void reset();
    void process (juce::dsp::AudioBlock<float>& block) noexcept;

private:
    std::vector<std::vector<float>> lines;
    std::vector<int> writePos;
    int delay = 0;
    int size  = 1;
};

// Caminho com oversampling: sobe a taxa, chama o "núcleo" (o modelo do
// circuito, nas próximas etapas) e desce a taxa. Fator escolhido para que a
// taxa interna fique >= minInternalRate. Um atraso extra (padding) permite
// igualar a latência de caminhos diferentes.
class OversampledPath
{
public:
    using Core = std::function<void (juce::dsp::AudioBlock<float>& internalBlock, double internalRate)>;

    void prepare (int numChannels, double baseRate, int maxBlockSize, double minInternalRate);
    void reset();

    int    getOversamplingLatency() const noexcept { return osLatency; }
    int    getInternalAlignment() const noexcept   { return alignSamplesInternal; }
    int    getTotalLatency() const noexcept        { return osLatency + padding.getDelay(); }
    void   setPadding (int samples)                { padding.setDelay (samples); }
    int    getFactor() const noexcept              { return 1 << factorLog2; }
    double getInternalRate() const noexcept        { return internalRate; }

    void setCore (Core c) { core = std::move (c); }

    // Processa no lugar. block.getNumSamples() <= maxBlockSize.
    void process (juce::dsp::AudioBlock<float>& block) noexcept;

    static int chooseFactorLog2 (double baseRate, double minInternalRate) noexcept;

private:
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;
    IntegerDelay padding;        // na taxa base: iguala a latência entre modos
    IntegerDelay internalAlign;  // na taxa interna: torna a latência um número inteiro
    Core core;
    int factorLog2 = 0;
    int osLatency = 0;
    int alignSamplesInternal = 0;
    double internalRate = 0.0;
};
}
