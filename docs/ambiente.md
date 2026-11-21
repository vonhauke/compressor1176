# Ambiente de desenvolvimento — PC Windows

Verificado em 20/11/2026 com `scripts/verificar-ambiente.ps1` e consulta à
lista de programas instalados (registro do Windows). Nada foi instalado.

## Máquina

- Windows 11 Home Single Language, build 26300, x64
- CPU: Intel Core i7-1355U (notebook, baixo consumo: 2 núcleos de desempenho +
  8 de eficiência, 12 threads)
- PowerShell 5.1

## Ferramentas

| Item | Situação |
|---|---|
| winget | OK (v1.29.380) |
| Git | OK (2.55.0) |
| Python 3 | OK (3.13.15) |
| VS Code | instalado (não é necessário para compilar) |
| Visual Studio Build Tools 2022 + C++ | **não instalado** (nenhuma instalação do Visual Studio encontrada) |
| Windows SDK | **não instalado** |
| CMake ≥ 3.22 | **não instalado** |
| Ninja | não instalado (opcional) |
| pluginval | não instalado |
| Pasta `C:\Program Files\Common Files\VST3` | **não existe** (nenhum plugin VST3 instalado até agora) |
| Ableton Live | **não encontrado** (nem nas pastas padrão nem na lista de programas instalados) |

## Consequências para o plano

- Com a decisão D5 (GitHub Actions), o PC **não precisa** de compilador para
  receber o plugin: o VST3 é compilado e validado (pluginval) num servidor
  Windows e baixado pronto.
- Para testar no Ableton, o Ableton precisa estar instalado neste PC, ou o
  teste é feito em outra máquina. A versão Standalone do plugin roda sem DAW.
- O processador é de notebook de baixo consumo. As estimativas de CPU do
  plano (seção 2) eram para desktop; neste PC o uso por instância deve ser
  maior. Vou medir aqui também na etapa (b) e usar este PC como referência de
  "máquina modesta".
- Instalação local opcional (só se você quiser compilar no próprio PC):
  Visual Studio Build Tools 2022 com a carga "Desenvolvimento para desktop
  com C++" (inclui o Windows SDK; alguns GB de download) e CMake. Não instalo
  nada sem você autorizar.
