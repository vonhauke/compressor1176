#include "OversampledPath.h"

#include <cmath>

namespace breu
{
//==============================================================================
void IntegerDelay::prepare (int numChannels, int maxDelaySamples)
{
    size = juce::jmax (1, maxDelaySamples + 1);
    lines.assign ((size_t) numChannels, std::vector<float> ((size_t) size, 0.0f));
    writePos.assign ((size_t) numChannels, 0);
    delay = juce::jlimit (0, size - 1, delay);
}

void IntegerDelay::setDelay (int samples)
{
    jassert (samples >= 0 && samples < size);
    delay = juce::jlimit (0, size - 1, samples);
}

void IntegerDelay::reset()
{
    for (auto& l : lines)
        std::fill (l.begin(), l.end(), 0.0f);
    std::fill (writePos.begin(), writePos.end(), 0);
}

void IntegerDelay::process (juce::dsp::AudioBlock<float>& block) noexcept
{
    if (delay == 0)
        return;

    const auto numCh = juce::jmin ((size_t) lines.size(), block.getNumChannels());

    for (size_t ch = 0; ch < numCh; ++ch)
    {
        auto* data = block.getChannelPointer (ch);
        auto& line = lines[ch];
        int w = writePos[ch];

        for (size_t i = 0; i < block.getNumSamples(); ++i)
        {
            line[(size_t) w] = data[i];
            int r = w - delay;
            if (r < 0)
                r += size;
            data[i] = line[(size_t) r];
            if (++w == size)
                w = 0;
        }

        writePos[ch] = w;
    }
}

//==============================================================================
int OversampledPath::chooseFactorLog2 (double baseRate, double minInternalRate) noexcept
{
    int f = 0;
    // Tolerância pequena para que 44,1 kHz x 4 = 176,4 kHz conte como suficiente.
    while (f < 4 && baseRate * (double) (1 << f) < minInternalRate * 0.999)
        ++f;
    return f;
}

void OversampledPath::prepare (int numChannels, double baseRate, int maxBlockSize, double minInternalRate)
{
    factorLog2 = chooseFactorLog2 (baseRate, minInternalRate);
    internalRate = baseRate * (double) (1 << factorLog2);

    oversampler = std::make_unique<juce::dsp::Oversampling<float>> (
        (size_t) numChannels, (size_t) factorLog2,
        juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple,
        true,    // máxima qualidade (filtros mais íngremes)
        false);  // sem o atraso fracionário do JUCE (ver abaixo)

    oversampler->initProcessing ((size_t) maxBlockSize);

    // A latência dos filtros de meia banda é uma fração da amostra na taxa base
    // (múltiplo de 1/fator). O JUCE arredonda com um filtro passa-tudo, que tem
    // fase não linear nos agudos. Em vez disso, completamos a fração com um
    // atraso inteiro NA TAXA INTERNA: é um atraso puro, fase linear exata.
    const double frac = (double) oversampler->getLatencyInSamples();
    osLatency = (int) std::ceil (frac - 1.0e-9);
    const double extra = ((double) osLatency - frac) * (double) (1 << factorLog2);
    alignSamplesInternal = (int) std::lround (extra);
    jassert (std::abs (extra - (double) alignSamplesInternal) < 1.0e-6);

    internalAlign.prepare (numChannels, 64);
    internalAlign.setDelay (alignSamplesInternal);

    // Folga generosa para o padding que iguala os modos.
    padding.prepare (numChannels, 4096);
    padding.setDelay (0);
    reset();
}

void OversampledPath::reset()
{
    if (oversampler != nullptr)
        oversampler->reset();
    padding.reset();
    internalAlign.reset();
}

void OversampledPath::process (juce::dsp::AudioBlock<float>& block) noexcept
{
    jassert (oversampler != nullptr);

    auto up = oversampler->processSamplesUp (block);

    if (core)
        core (up, internalRate);

    internalAlign.process (up);

    oversampler->processSamplesDown (block);
    padding.process (block);
}
}
