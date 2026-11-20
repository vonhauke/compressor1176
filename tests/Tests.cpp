// Testes automáticos da etapa (a): o plugin só deixa o áudio passar, com
// oversampling ligado. Saída: tabela de medições + código de saída 0 (tudo
// passou) ou 1 (alguma falha).

#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"

#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <string>
#include <vector>

using breu::BreuAudioProcessor;
namespace param = breu::param;

namespace
{
int failures = 0;

std::string fmt (const char* f, ...)
{
    char buf[512];
    va_list args;
    va_start (args, f);
    std::vsnprintf (buf, sizeof (buf), f, args);
    va_end (args);
    return buf;
}

void check (bool ok, const std::string& what)
{
    std::printf ("  [%s] %s\n", ok ? "ok  " : "FALHA", what.c_str());
    if (! ok)
        ++failures;
}

void setParam (BreuAudioProcessor& p, const char* id, float plainValue)
{
    auto* prm = p.getState().getParameter (id);
    prm->setValueNotifyingHost (prm->convertTo0to1 (plainValue));
}

// Processa um sinal inteiro (canal único replicado em todos os canais) em blocos.
std::vector<float> run (BreuAudioProcessor& p, const std::vector<float>& in, int block, int numCh = 2)
{
    std::vector<float> out (in.size());
    juce::AudioBuffer<float> buf (numCh, block);
    juce::MidiBuffer midi;

    for (size_t start = 0; start < in.size(); start += (size_t) block)
    {
        const int n = (int) std::min<size_t> ((size_t) block, in.size() - start);
        buf.setSize (numCh, n, false, false, true);
        for (int ch = 0; ch < numCh; ++ch)
            for (int i = 0; i < n; ++i)
                buf.setSample (ch, i, in[start + (size_t) i]);
        p.processBlock (buf, midi);
        for (int i = 0; i < n; ++i)
            out[start + (size_t) i] = buf.getSample (0, i);
    }
    return out;
}

std::vector<float> sine (double f, double sr, int len, float amp = 0.5f)
{
    std::vector<float> s ((size_t) len);
    for (int i = 0; i < len; ++i)
        s[(size_t) i] = amp * (float) std::sin (2.0 * juce::MathConstants<double>::pi * f * i / sr);
    return s;
}

double rms (const std::vector<float>& x, size_t from, size_t to)
{
    double acc = 0.0;
    for (size_t i = from; i < to; ++i)
        acc += (double) x[i] * x[i];
    return std::sqrt (acc / (double) (to - from));
}

double db (double v) { return 20.0 * std::log10 (std::max (v, 1e-12)); }

std::unique_ptr<BreuAudioProcessor> makeProcessor (double sr, int block, param::Quality q, int numCh = 2)
{
    auto p = std::make_unique<BreuAudioProcessor>();
    const auto set = numCh == 1 ? juce::AudioChannelSet::mono() : juce::AudioChannelSet::stereo();
    BreuAudioProcessor::BusesLayout layout;
    layout.inputBuses.add (set);
    layout.outputBuses.add (set);
    p->setBusesLayout (layout);
    setParam (*p, param::quality, (float) q);
    p->setRateAndBufferSizeDetails (sr, block);
    p->prepareToPlay (sr, block);
    return p;
}

//==============================================================================
void testLatencyAndFactors()
{
    std::printf ("\n== Latência e fator de oversampling ==\n");
    std::printf ("  %-9s %-8s %-8s %-12s %-12s %s\n", "taxa", "fator N", "fator A", "interna N", "interna A", "latência (amostras / ms)");

    for (double sr : { 44100.0, 48000.0, 88200.0, 96000.0, 176400.0, 192000.0 })
    {
        auto pn = makeProcessor (sr, 512, param::Quality::normal);
        auto ph = makeProcessor (sr, 512, param::Quality::high);

        std::printf ("  %-9.0f %-8d %-8d %-12.0f %-12.0f %d / %.3f\n", sr,
                     pn->getFactor (param::Quality::normal), pn->getFactor (param::Quality::high),
                     pn->getInternalRate (param::Quality::normal), pn->getInternalRate (param::Quality::high),
                     pn->getReportedLatency(), 1000.0 * pn->getReportedLatency() / sr);

        check (pn->getReportedLatency() == ph->getReportedLatency(),
               fmt ("mesma latência nos dois modos a %.0f Hz", sr));
        check (pn->getLatencySamples() == pn->getReportedLatency(),
               "latência informada ao host = latência real");
        check (pn->getInternalRate (param::Quality::normal) >= BreuAudioProcessor::minRateNormal * 0.999,
               "taxa interna Normal >= 176,4 kHz");
        check (pn->getInternalRate (param::Quality::high) >= BreuAudioProcessor::minRateHigh * 0.999,
               "taxa interna Alta >= 352,8 kHz");
    }
}

void testImpulseAlignment()
{
    std::printf ("\n== Impulso: o pico sai exatamente na latência informada ==\n");
    for (double sr : { 44100.0, 48000.0, 96000.0 })
        for (auto q : { param::Quality::normal, param::Quality::high })
            for (int block : { 64, 512, 1001 })
            {
                auto p = makeProcessor (sr, block, q);
                std::vector<float> in (8192, 0.0f);
                in[100] = 1.0f;
                auto out = run (*p, in, block);

                size_t peak = 0;
                for (size_t i = 0; i < out.size(); ++i)
                    if (std::abs (out[i]) > std::abs (out[peak]))
                        peak = i;

                const int measured = (int) peak - 100;
                check (measured == p->getReportedLatency(),
                       fmt ("%.0f Hz, %s, bloco %d: pico em %d, latência %d",
                                                sr, q == param::Quality::normal ? "Normal" : "Alta",
                                                block, measured, p->getReportedLatency()));
            }
}

void testFrequencyResponse()
{
    std::printf ("\n== Resposta em frequência e resíduo (saída - entrada atrasada) ==\n");
    for (double sr : { 44100.0, 48000.0, 96000.0 })
        for (auto q : { param::Quality::normal, param::Quality::high })
        {
            auto p = makeProcessor (sr, 512, q);
            const int lat = p->getReportedLatency();
            std::printf ("  %.0f Hz, %s\n", sr, q == param::Quality::normal ? "Normal" : "Alta");
            std::printf ("    %-10s %-14s %s\n", "freq (Hz)", "ganho (dB)", "resíduo (dB rel. sinal)");

            for (double f : { 20.0, 100.0, 1000.0, 5000.0, 10000.0, 15000.0, 18000.0, 20000.0 })
            {
                if (f >= sr * 0.5)
                    continue;
                p->reset();
                p->prepareToPlay (sr, 512);
                const int len = (int) sr; // 1 s
                auto in = sine (f, sr, len);
                auto out = run (*p, in, 512);

                const size_t from = (size_t) (sr * 0.25), to = (size_t) len - 1;
                std::vector<float> residual (out.size(), 0.0f);
                for (size_t i = from; i < to; ++i)
                    residual[i] = out[i] - in[i - (size_t) lat];

                const double gainDb = db (rms (out, from, to) / rms (in, from - (size_t) lat, to - (size_t) lat));
                const double resDb  = db (rms (residual, from, to) / rms (in, from, to));
                std::printf ("    %-10.0f %-14.4f %.1f\n", f, gainDb, resDb);

                if (f <= 18000.0)
                    check (std::abs (gainDb) < 0.1,
                           fmt ("ganho dentro de ±0,1 dB a %.0f Hz", f));
                if (f <= 15000.0)
                    check (resDb < -50.0,
                           fmt ("resíduo abaixo de -50 dB a %.0f Hz", f));
            }
        }
}

void testBypassIsExact()
{
    std::printf ("\n== Bypass: saída idêntica à entrada atrasada ==\n");
    auto p = makeProcessor (48000.0, 256, param::Quality::normal);
    setParam (*p, param::bypass, 1.0f);
    auto in = sine (440.0, 48000.0, 48000);
    auto out = run (*p, in, 256);
    const int lat = p->getReportedLatency();

    double maxErr = 0.0;
    for (size_t i = 4800; i < out.size(); ++i) // depois da transição de 10 ms
        maxErr = std::max (maxErr, (double) std::abs (out[i] - in[i - (size_t) lat]));
    std::printf ("  erro máximo: %g\n", maxErr);
    check (! (maxErr > 0.0), "bypass é bit a bit idêntico (descontado o atraso)");
}

void testQualitySwitchIsSmooth()
{
    std::printf ("\n== Troca Normal -> Alta durante a reprodução ==\n");
    const double sr = 48000.0;
    auto p = makeProcessor (sr, 256, param::Quality::normal);
    auto in = sine (1000.0, sr, (int) sr * 2);
    const int lat = p->getReportedLatency();

    std::vector<float> out (in.size());
    juce::AudioBuffer<float> buf (2, 256);
    juce::MidiBuffer midi;
    for (size_t start = 0; start < in.size(); start += 256)
    {
        if (start >= 24000 && p->getActiveQualityIndex() == (int) param::Quality::normal && ! p->isSwitchingQuality())
            setParam (*p, param::quality, (float) param::Quality::high);
        for (int ch = 0; ch < 2; ++ch)
            for (int i = 0; i < 256; ++i)
                buf.setSample (ch, i, in[start + (size_t) i]);
        p->processBlock (buf, midi);
        for (int i = 0; i < 256; ++i)
            out[start + (size_t) i] = buf.getSample (0, i);
    }

    double maxErr = 0.0;
    for (size_t i = 4800; i < out.size(); ++i)
        maxErr = std::max (maxErr, (double) std::abs (out[i] - in[i - (size_t) lat]));
    std::printf ("  desvio máximo em relação à entrada atrasada: %g (%.1f dB)\n", maxErr, db (maxErr / 0.5));
    check (p->getActiveQualityIndex() == (int) param::Quality::high, "modo Alta ativo depois da troca");
    check (db (maxErr / 0.5) < -60.0, "troca sem salto audível (desvio < -60 dB)");
}

void testStateRoundTrip()
{
    std::printf ("\n== Salvar e restaurar o estado (projeto do Ableton) ==\n");
    auto a = makeProcessor (48000.0, 512, param::Quality::normal);
    setParam (*a, param::input, 7.5f);
    setParam (*a, param::output, 2.0f);
    setParam (*a, param::attack, 6.0f);
    setParam (*a, param::release, 1.5f);
    setParam (*a, param::attackOff, 1.0f);
    setParam (*a, param::ratio, (float) param::Ratio::all);
    setParam (*a, param::meter, (float) param::Meter::plus8);
    setParam (*a, param::quality, (float) param::Quality::high);

    juce::MemoryBlock blob;
    a->getStateInformation (blob);

    auto b = std::make_unique<BreuAudioProcessor>();
    b->setStateInformation (blob.getData(), (int) blob.getSize());

    for (auto* prm : a->getParameters())
    {
        auto* withId = dynamic_cast<juce::AudioProcessorParameterWithID*> (prm);
        auto* other = b->getState().getParameter (withId->paramID);
        check (other != nullptr && std::abs (other->getValue() - prm->getValue()) < 1e-6f,
               "parâmetro " + withId->paramID.toStdString() + " = " + prm->getCurrentValueAsText().toStdString());
    }
}

void testMono()
{
    std::printf ("\n== Faixa mono ==\n");
    auto p = makeProcessor (48000.0, 512, param::Quality::normal, 1);
    auto in = sine (1000.0, 48000.0, 24000);
    auto out = run (*p, in, 512, 1);
    const int lat = p->getReportedLatency();
    const double g = db (rms (out, 12000, 23999) / rms (in, 12000 - (size_t) lat, 23999 - (size_t) lat));
    check (p->getTotalNumOutputChannels() == 1 && std::abs (g) < 0.1, "mono processa com ganho unitário");
}

void testParameterList()
{
    std::printf ("\n== Parâmetros automatizáveis ==\n");
    BreuAudioProcessor p;
    for (auto* prm : p.getParameters())
    {
        auto* withId = dynamic_cast<juce::AudioProcessorParameterWithID*> (prm);
        std::printf ("  %-11s %-11s padrão: %s\n", withId->paramID.toRawUTF8(),
                     prm->getName (32).toRawUTF8(), prm->getCurrentValueAsText().toRawUTF8());
    }
    check (p.getBypassParameter() != nullptr, "parâmetro de bypass informado ao host");
}
}

int main()
{
    juce::ScopedJuceInitialiser_GUI juceInit;

    testParameterList();
    testLatencyAndFactors();
    testImpulseAlignment();
    testFrequencyResponse();
    testBypassIsExact();
    testQualitySwitchIsSmooth();
    testStateRoundTrip();
    testMono();

    std::printf ("\n%s (%d falha%s)\n", failures == 0 ? "TUDO PASSOU" : "HOUVE FALHAS",
                 failures, failures == 1 ? "" : "s");
    return failures == 0 ? 0 : 1;
}
