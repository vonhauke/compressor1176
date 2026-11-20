# Breu

Plugin compressor (VST3 + Standalone, Windows e macOS) que modela o circuito
de um compressor FET clássico de 1967, revisão "painel preto" LN Rev E.

Estado atual: **etapa (a) — o plugin deixa o áudio passar** (oversampling,
latência, parâmetros, bypass). Ainda não comprime.

## Documentos

- [`docs/pesquisa.md`](docs/pesquisa.md) — fontes, fatos documentados e suposições
- [`docs/plano.md`](docs/plano.md) — decisões de DSP, simplificações e etapas
- [`docs/medicoes.md`](docs/medicoes.md) — resultados medidos, etapa por etapa
- [`docs/instalar.md`](docs/instalar.md) — como baixar e instalar a versão de teste
- [`docs/ambiente.md`](docs/ambiente.md) — ferramentas no PC Windows

## Compilar

O GitHub Actions compila para Windows e macOS a cada envio e roda os testes e
o pluginval. Para compilar localmente (CMake 3.22+ e um compilador C++20):

```
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

O JUCE 9.0.6 é baixado automaticamente na primeira configuração.

## Organização

| Pasta | Conteúdo |
|---|---|
| `src/` | processador do plugin e parâmetros |
| `src/dsp/` | oversampling; nas próximas etapas, o modelo do circuito |
| `tests/` | testes automáticos com medições |
| `scripts/` | utilitários (verificação do ambiente Windows) |
| `docs/` | pesquisa, plano, medições, instalação |
