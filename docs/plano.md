# Plano — compressor FET (1176LN Rev E)

Data: 20/11/2026. Etapa 2. **Nada de código antes da sua aprovação.**

Base: [`docs/pesquisa.md`](pesquisa.md). Quando falo "fato" ou "suposição",
refiro-me às seções de lá.

---

## 0. Resumo em cinco linhas

1. Vou modelar o circuito D/E estágio por estágio, com equações físicas de
   cada transistor, FET e diodo, resolvidas amostra a amostra.
2. Para saber se o modelo está certo, vou montar **duas réguas**: uma
   simulação SPICE do circuito inteiro (lenta, mas é o "circuito ideal") e os
   números publicados de aparelhos reais.
3. O plugin terá dois modos: **Normal** (leve) e **Alta qualidade** (mais
   pesado e mais preciso no laço de realimentação e no aliasing).
4. Cinco etapas (a–e), cada uma num branch com tag. A partir da (c), cada
   etapa tem arquivos WAV para você ouvir.
5. Há simplificações. Cada uma está listada na seção 9 com o custo em
   fidelidade.

## Decisões tomadas

| # | Decisão | Data |
|---|---|---|
| D1 | **B — espaço de estados por estágios + régua SPICE.** O circuito inteiro (A) fica como referência offline | 20/11/2026 |
| D2 | **D2a** — transformador de saída com núcleo que satura, sem histerese | 20/11/2026 |
| D3 | Modos **Normal** e **Alta qualidade**, com a mesma latência nos dois | 20/11/2026 |
| D4 | **0 dBFS = +22 dBu** (−18 dBFS = +4 dBu = 0 VU) | 20/11/2026 |
| D5 | **GitHub Actions** (runner Windows) para compilar e rodar o pluginval; o PC do Pedro para testar no Ableton | 20/11/2026 |
| D6 | Nome provisório **"Breu"** (pode mudar até a etapa e) | 20/11/2026 |
| D7 | Sem ruído de fundo na v1 | 20/11/2026 |
| D8 | Voz, bateria e baixo: gravações do Pedro; sintetizador e ruído rosa gerados | 20/11/2026 |
| D9 | **macOS entra na v1**: VST3 + Standalone para Windows (x64) e macOS (Apple Silicon), compilados no GitHub Actions. No Mac o plugin usa assinatura ad-hoc (sem conta paga da Apple); a primeira abertura exige liberar a quarentena. Testes no Ableton: no MacBook | 20/11/2026 |

## Decisões propostas (histórico)

| # | Decisão | Minha recomendação |
|---|---|---|
| D1 | Método de modelagem (seção 2) | **B — espaço de estados por estágios + régua SPICE** |
| D2 | Modelo do transformador de saída (seção 3.5) | Núcleo que satura, **sem histerese**, na v1 |
| D3 | Modos de qualidade (seção 4) | Normal + Alta qualidade, com a mesma latência nos dois |
| D4 | Calibração de nível digital ↔ analógico (seção 5) | 0 dBFS = +22 dBu (então −18 dBFS = +4 dBu = 0 VU) |
| D5 | Onde compilar e testar no Windows (seção 11) | GitHub Actions num runner Windows **ou** ligar o seu PC a esta conversa |
| D6 | Nome do plugin no painel | Você escolhe. Sugestões neutras: "Breu", "Nanquim", "Carvão" |
| D7 | Ruído de fundo (chiado) do aparelho | Fora da v1 (desligado) |
| D8 | Material de teste de voz, bateria e baixo (seção 8) | Gravações suas; sintetizador e ruído rosa eu gero |

---

## 1. Revisão alvo

**Rev E, como você pediu.** A pesquisa indica que o caminho de áudio da E é o
mesmo da D (pesquisa §1). As únicas diferenças são a fonte 110/220 V e o
resistor anti-estalo de 10 MΩ na chave de ratio. O modelo vai incluir esse
resistor. Não vejo motivo para outra revisão.

---

## 2. Método de modelagem — opções

### Em linguagem simples

Um circuito é um conjunto de equações: cada resistor, capacitor, transistor e
diodo impõe uma relação entre tensões e correntes. "Modelar o circuito" é
resolver essas equações a cada amostra de áudio. As opções diferem em **como**
resolver e **quanto** do circuito resolver junto.

- **Espaço de estados não linear (método "nodal DK")**: escreve o circuito como
  matrizes e, a cada amostra, resolve as partes não lineares (transistores,
  diodos) por tentativa e correção (método de Newton). É o que a tese de Antão
  fez com o 1176 inteiro.
- **Wave digital filters (WDF)**: reorganiza o mesmo circuito em "ondas" que
  viajam entre os componentes. Muito eficiente quando há **uma** peça não
  linear; com vários transistores interligados, volta a precisar de Newton.
- **Caixa cinza calibrada por medições**: blocos genéricos (filtro → curva de
  saturação → compressor → filtro) com parâmetros ajustados até a saída bater
  com medições do aparelho.
- **Rede neural**: aprende o aparelho a partir de horas de gravação dele.

### Comparação

Custo de CPU: **estimativa minha**, por conta de operações por amostra, para
uma instância estéreo a 48 kHz com oversampling 4x, em % de um núcleo de um
processador de desktop atual. Os números reais serão medidos na etapa (b).

| | Método | CPU estimada | Fidelidade ao circuito | Riscos |
|---|---|---|---|---|
| A | DK do circuito inteiro (≈18–20 variáveis não lineares resolvidas juntas) | 60–200% de um núcleo | Máxima: todas as interações entre estágios | Pesado demais para várias faixas; Newton pode estourar em transientes (Antão viu >50 iterações) |
| **B** | **DK por estágios** (entrada, FET + pré, detector, linha + transformador), ligados nos pontos onde um estágio de baixa impedância alimenta outro de alta | 15–40%; 8–20% se os transistores do detector puderem ser linearizados (ver 3.4) | Alta. Perde só a interação entre estágios nos pontos de corte, que eu escolho onde ela é pequena e **meço** contra o SPICE | Corte no laço de realimentação introduz um atraso mínimo (seção 4) |
| C | WDF por estágios | Parecida com B | Igual a B | Estágios com 3–4 transistores interligados exigem adaptadores R-type + Newton; mais trabalho para o mesmo resultado |
| D | Caixa cinza calibrada | < 2% | Média. Acerta curva estática e tempos médios; o modo todos-os-botões, a distorção que muda com a compressão e o release dependente do programa teriam de ser **desenhados à mão** | Sem aparelho real para medir, eu calibraria contra o meu próprio SPICE. E você pediu para não inventar curvas |
| E | Rede neural | 5–30% | Depende dos dados | Exige horas de gravações de um 1176 real em vários ajustes. Inviável agora |

### Recomendação: **B**

Motivos:

1. É modelagem de circuito de verdade: o comportamento sai das equações dos
   componentes, não de curvas escritas à mão. Release dependente do programa,
   ratio que muda depois do transiente, distorção que cresce com a compressão
   e o modo todos-os-botões devem aparecer sozinhos (pesquisa §5–6).
2. Fica numa faixa de CPU usável em várias faixas do Ableton. A opção A
   vira o **modo de referência offline**: eu rodo para comparar, mas não vai no
   plugin.
3. Cada estágio pode ser testado isoladamente contra o SPICE, o que facilita
   achar erro.

Custo em fidelidade de B contra A: a interação entre estágios nos pontos de
corte. Critério de aceite: **erro menor que 0,1 dB na resposta em frequência e
na curva estática** em relação ao SPICE do circuito inteiro. Se um corte
falhar, junto os dois estágios num só bloco (mais CPU, menos erro).

### A régua SPICE

Vou transcrever o esquema da Rev D/E ([MNATS][mnats-revd-sch], conferido com o
BOM e o manual) para um arquivo de simulação do **ngspice** (software livre de
simulação de circuitos), com modelos SPICE de 2N5457, 2N3391, 2N3707, 2N3053 e
FDH333. Isso dá duas camadas de verificação, sempre reportadas separadas:

- **modelo do plugin × circuito simulado**: mede se eu implementei bem;
- **circuito simulado × aparelho real** (números publicados): mede se o
  circuito transcrito e os modelos de componente representam o aparelho.

---

## 3. Modelo por estágio

A divisão abaixo é provisória; confirmo depois de transcrever o esquema.

### 3.1 Entrada (atenuador + transformador de entrada)

- Atenuador T de 600 Ω com a curva do botão Input.
- Transformador de entrada **linear**: indutância de magnetização, dispersão,
  resistência dos enrolamentos e capacitância. Dá o rolloff de graves e
  agudos.
- Simplificação: sem saturação do transformador de entrada. Custo: quase nulo,
  porque ele vem depois do atenuador e trabalha em nível baixo.

### 3.2 FET + circuito LN + pré-amplificador (um bloco só)

- FET 2N5457 com modelo físico (Shichman–Hodges, região triodo e saturação,
  simétrico para tensão dreno-fonte negativa, junções do gate como diodos).
  Ponto de partida: parâmetros medidos por Antão (pesquisa §3).
- Rede LN (provavelmente R6/R7) que leva parte da tensão do dreno ao gate.
- Pré-amplificador Q2–Q4 com modelo de Ebers–Moll e efeito Early (parâmetros
  de Antão).
- FET e pré ficam no mesmo bloco porque a Rev B em diante tem uma
  realimentação do emissor do primeiro transistor para o estágio do FET
  (pesquisa §1). Cortar ali daria erro.

### 3.3 Laço de realimentação (pré → detector → gate)

Ver seção 4.

### 3.4 Detector (ratio, retificador, ataque e release)

- Rede de ratio transcrita do esquema e da fiação da botoeira, **incluindo a
  combinação 4+20** para o modo todos-os-botões.
- Q7–Q10 (2N3707), diodos FDH333 (parâmetros de Antão), C22, R55 (ataque),
  R56 + R57 (release), C27, trimpot de Q bias R59.
- Variante leve, a testar: transistores do detector como amplificadores
  lineares e só os diodos não lineares. Só entra no modo Normal se o erro
  contra o detector completo ficar abaixo de 0,2 dB em redução de ganho e 5%
  nos tempos, **inclusive no modo todos-os-botões**. Se não passar, fica o
  detector completo.
- Q bias calibrado pelo procedimento do manual: girar até a saída cair 1 dB
  (pesquisa §8). Faço isso uma vez, no modelo, e fixo o valor.

### 3.5 Amplificador de linha + transformador de saída

- Q5 (2N3707) + Q6 (2N3053) classe A com modelo físico, incluindo o ponto de
  polarização e a saturação de saída (alvo: +22 dBm com ≤ 0,5% de THD).
- Transformador de saída com os enrolamentos de realimentação.
- **Decisão D2**:
  - **D2a (recomendada)**: núcleo com indutância que diminui quando o fluxo
    cresce (satura), sem histerese. Barato, estável, e dá a distorção de grave
    em nível alto.
  - **D2b**: modelo Jiles–Atherton com histerese ([Holters & Zölzer][holters-ja]).
    Mais fiel em teoria, porém mais CPU (estimo +5 a 15% sobre o total) e com
    **parâmetros chutados**, porque não há dados do núcleo (pesquisa §4).
  - Custo de D2a: perde efeitos de memória do núcleo (assimetria de harmônicos
    em grave alto). Como o transformador está dentro da realimentação
    negativa, a maior parte dessa distorção é corrigida pelo próprio circuito.
    Minha estimativa é que a diferença seja pequena, mas não consigo medir sem
    um aparelho.
- Parâmetros do transformador: estimados para cumprir a resposta especificada
  (±1 dB, 20 Hz–20 kHz) e o ganho do estágio medido por Antão (25,26 dB).
  **Esta é a parte com mais incerteza do modelo inteiro.**

### 3.6 Medidor

- Circuito do medidor (provavelmente o segundo FET Q11, pesquisa §8), para que
  a leitura de GR erre do mesmo jeito que o original no modo todos-os-botões.
- Agulha: sistema massa-mola-amortecedor de 2ª ordem ajustado para chegar a
  99% em 300 ms ± 30 ms com 1–1,5% de ultrapassagem (pesquisa §9).
- Chave: GR / +4 / +8 / desligado. O original tem duas posições de nível de
  saída (+4 e +8); mantenho as duas.

---

## 4. Laço de realimentação, oversampling e latência

**O problema em palavras simples.** No 1176, a saída do pré alimenta o
detector, e o detector controla o FET que está antes do pré. É um círculo. Num
computador, cada estágio é calculado em sequência; para fechar o círculo,
alguém precisa usar o valor "da amostra anterior". Isso é um atraso de uma
amostra dentro do laço. Com ataque de 20 µs, esse atraso importa.

**Modo Normal**

- Oversampling adaptativo: a taxa interna fica sempre ≥ 176,4 kHz (4x em
  44,1/48 kHz, 2x em 88,2/96 kHz, 1x em 176,4/192 kHz).
- O laço é fechado com uma amostra de atraso na taxa interna (≈ 5,2 µs a
  192 kHz). Erro medido contra o SPICE no ataque mais rápido e no overshoot.

**Modo Alta qualidade**

- Taxa interna ≥ 352,8 kHz (8x em 44,1/48 kHz).
- O laço é resolvido **sem atraso**: dentro de cada amostra, repito
  pré → detector → FET até convergir.
- CPU estimada: 2,5 a 4 vezes a do Normal.

**Filtros de oversampling e latência**

- Filtros de fase linear (FIR de meia banda do JUCE), para não alterar a fase
  do sinal limpo. A latência é informada ao Ableton, que compensa.
- **Mesma latência nos dois modos** (o modo de menor latência ganha um atraso
  extra). Trocar de modo no meio da música não muda o alinhamento das faixas.
- Valor esperado: na ordem de 1 ms a 48 kHz. Medido e documentado na etapa (d).

---

## 5. Calibração de nível digital ↔ analógico (decisão D4)

Um plugin recebe números (dBFS); o 1176 recebe volts (dBu). É preciso decidir
quanto vale 0 dBFS em volts, porque isso define o quanto uma faixa típica
"empurra" o aparelho virtual.

- **Proposta**: 0 dBFS = +22 dBu. Assim, −18 dBFS = +4 dBu = 0 VU no modo +4,
  e o máximo digital coincide com a saída máxima especificada (+22 dBm).
- Os botões Input e Output funcionam como no original: com Input baixo, pouca
  compressão; com Input alto, mais compressão e mais distorção. Não há
  threshold.

---

## 6. Controles e parâmetros (automatizáveis no Ableton)

Nomes em inglês, como no painel, para bater com o que aparece na automação.

| Parâmetro | Valores | Observação |
|---|---|---|
| Input | escala do botão original | curva do atenuador real |
| Output | escala do botão original | potenciômetro de 250 kΩ (curva de áudio, BOM) |
| Attack | 1–7, horário = mais rápido; posição **OFF** no fim anti-horário | OFF desliga a redução de ganho, como no original |
| Release | 1–7, horário = mais rápido | curva do potenciômetro: **suposição** (provavelmente log; pesquisa §12) |
| Ratio | 4, 8, 12, 20, All, Off | Off = todos soltos (passa pelo circuito sem comprimir) |
| Meter | GR, +4, +8, Off | não afeta o áudio |
| Bypass | on/off | bypass real com o mesmo atraso do processamento, para não deslocar o áudio |
| Quality | Normal, Alta | latência igual nos dois |

- Girar um potenciômetro muda as matrizes do circuito. Uso a técnica de
  recálculo eficiente de [Holters & Zölzer][holters-dk11], com suavização para
  não gerar estalo.
- Estéreo: dois canais independentes (dois aparelhos idênticos, controles
  ligados). O 1176 é mono; o acoplamento estéreo do original fica fora da v1
  junto com o sidechain. Custo: em material com imagem desequilibrada, a
  imagem estéreo pode se mexer quando só um lado comprime.

---

## 7. Interface, estado e presets

- Painel preto em estilo de rack, desenhado em vetor (sem imagens em bitmap),
  para ficar nítido em qualquer DPI e redimensionável com proporção fixa.
- VU de agulha com a balística da seção 3.6; botões de ratio iluminados.
- Sem marca e sem nome de modelo; só o nome escolhido em D6.
- Estado salvo no projeto do Ableton pelo mecanismo padrão do VST3 (todos os
  parâmetros e o modo de qualidade).
- Alguns presets de fábrica (voz, baixo, bateria, todos-os-botões). O Ableton
  também salva presets do próprio VST3.

---

## 8. Etapas

Cada etapa: branch `etapa-x`, commits pequenos, tag no fim, entrada em
`docs/medicoes.md`. Eu paro ao fim de cada etapa e espero o seu retorno.

| Etapa | Entrega | Critério para fechar |
|---|---|---|
| **(a)** Plugin que passa áudio | CMake + JUCE (versão fixada, via FetchContent), VST3 + Standalone, parâmetros declarados, oversampling ligado, latência informada | Saída idêntica à entrada (descontada a latência e o filtro); pluginval nível 5 sem falhas; abre no Ableton |
| **(b)** FET + pré + detector em realimentação, sem interface | Esquema transcrito para ngspice; motor DK; blocos 3.2 e 3.4 com ratio 4:1 e tempos fixos; amplificador de linha ainda linear | Curva estática e resposta em frequência a < 0,1 dB do SPICE; CPU medida e comparada com a estimativa da seção 2 |
| **(c)** Tempos, ratios e todos-os-botões | Potenciômetros de ataque/release, rede de ratio completa, botoeira com 4+20, Q bias calibrado | Ratios comparadas com as medidas de Antão; tempos comparados com a especificação e o SPICE; modo All entre 12:1 e 20:1 com overshoot de ataque; **primeiro ciclo de escuta** |
| **(d)** Estágios não lineares e transformadores | Transformador de entrada, amplificador classe A, transformador de saída, modo Alta qualidade | THD por nível e por redução de ganho comparada com Moore; resposta ±1 dB 20 Hz–20 kHz; aliasing medido; ciclo de escuta |
| **(e)** Interface | Painel, VU, redimensionamento, presets | pluginval nível 10; teste de salvar/reabrir projeto no Ableton; ciclo de escuta final |

### Ciclo de escuta (a partir da etapa c)

- Arquivos: voz, bateria, baixo, sintetizador, ruído rosa. Sintetizador e
  ruído rosa eu gero. **Voz, bateria e baixo**: o ideal são gravações suas
  (secas, sem compressão). Daqui eu não consigo baixar arquivos de bancos de
  áudio da internet.
- Para cada arquivo: versão limpa, versão processada em ajustes definidos e
  uma folha dizendo **o que ouvir** (por exemplo: "na caixa, com ataque 7 e
  todos-os-botões, o estalo inicial deve passar antes de a compressão pegar").
- Se você mandar áudio de um 1176 real ou de uma emulação conhecida, com os
  ajustes anotados, faço a comparação: espectro, redução de ganho ao longo do
  tempo e harmônicos.

---

## 9. Simplificações e o custo de cada uma

| Simplificação | Custo em fidelidade | Como reduzo o risco |
|---|---|---|
| Circuito dividido em estágios (método B) | Interação entre estágios nos pontos de corte | Corte só em pontos de baixa → alta impedância; erro medido contra SPICE (< 0,1 dB) |
| Atraso de 1 amostra no laço (modo Normal) | Ataque mais rápido e overshoot levemente diferentes | Modo Alta qualidade sem atraso; diferença medida e reportada |
| Transformador de entrada linear | Desprezível (trabalha em nível baixo) | — |
| Transformador de saída sem histerese (D2a) | Perde efeitos de memória do núcleo em grave alto | Fica dentro da realimentação; D2b disponível se você quiser |
| Parâmetros do transformador estimados | Resposta e saturação podem diferir do original | Ajuste à especificação; é a maior incerteza e vai estar destacada nas medições |
| Componentes com valor nominal e um exemplar medido de cada transistor | O seu "1176 virtual" é um exemplar específico, não a média | Q bias calibrado como no aparelho; tolerância de componentes pode virar opção futura |
| Fonte de alimentação ideal | Sem queda de tensão em picos longos | A fonte do clone é regulada; efeito esperado pequeno |
| Sem ruído de fundo (D7) | Original tem SNR > 81 dB; o chiado some | Pode virar opção futura |
| Dois canais idênticos, sem acoplamento estéreo | Imagem estéreo pode se mexer | Acoplamento fica para a versão com sidechain |
| Curva dos potenciômetros de ataque e release | Os números no botão podem não corresponder exatamente | Suposição documentada; ajusto se você trouxer medidas ou áudio de referência |

---

## 10. Medições (`docs/medicoes.md`)

Para cada item: como medi, valor do plugin, valor do SPICE, valor publicado,
desvio. Quando não houver valor publicado, digo isso.

| Medição | Método | Comparar com |
|---|---|---|
| Curva de transferência estática, por ratio | Tom de 1 kHz em degraus de 1 dB, regime permanente | SPICE (< 0,5 dB); ratios medidas por Antão (5,5 / 9,4 / 15,6 / 21,2) |
| Resposta ao degrau: ataque | Tom com salto de nível; tempo até 63% e 90% da redução final, nas posições 1, 4 e 7 | Especificação 20–800 µs; SPICE (±10%) |
| Resposta ao degrau: release | Saída de 10 dB de redução; tempo até 63% e 90% da recuperação | Especificação 50 ms–1,1 s; SPICE (±10%) |
| Dependência do programa | Rajadas de tom como as de Moore (transiente de 10 ms, trechos ±8 dB) | Comportamento descrito por Moore (qualitativo) |
| THD por nível, sem compressão | 1 kHz e 50 Hz, três níveis no procedimento de Moore | Moore, Tabela 6-1 (um exemplar; serve de faixa) |
| THD durante compressão e release | Idem, com 10 dB de redução | Moore, Tabelas 6-2 e 6-3; especificação < 0,5% |
| Perfil de harmônicos | Pares × ímpares | Moore: pares maiores que ímpares |
| Intermodulação SMPTE | 60 Hz + 7 kHz, 1:1 | Moore, Tabela 6-4 (inclui modo All) |
| Resposta em frequência | Varredura senoidal em nível baixo, sem e com 10 dB de redução | Especificação ±1 dB 20 Hz–20 kHz |
| Aliasing | Varredura de 1 a 20 kHz em nível alto; energia fora dos harmônicos | Alvo: < −90 dB em relação à fundamental no Normal, < −110 dB no Alta |
| CPU e latência | Por modo e taxa de amostragem | Estimativas da seção 2 |
| pluginval | Nível 5 na etapa (a), nível 10 na (e) | Sem falhas |

**O que não vou poder afirmar**: que o plugin "soa igual" a um 1176. Vou
afirmar o que foi medido contra o SPICE e contra os números publicados. O
resto depende da sua escuta e, se possível, de áudio de referência.

---

## 11. Ambiente Windows (decisão D5)

O computador ligado a esta conversa é o Mac, não o PC. Por isso **ainda não
verifiquei o que está instalado no Windows**. O script
`scripts/verificar-ambiente.ps1` faz essa verificação sem instalar nada:
Visual Studio Build Tools 2022 (compilador C++ e Windows SDK), CMake (o JUCE 8
exige 3.22 ou mais), Git, Ninja, Python, pluginval, a pasta de VST3 e o
Ableton.

Opções para compilar e testar no Windows:

1. **GitHub Actions** (recomendada para o dia a dia): a cada etapa, um
   servidor Windows na nuvem compila o VST3 e roda o pluginval. Você baixa o
   arquivo pronto. Precisa de um repositório no GitHub.
2. **Ligar o PC a esta conversa**: abrir a conversa no app Claude no PC e
   adicionar a pasta do projeto. Aí eu compilo e rodo o pluginval direto na
   sua máquina.
3. **Você compila** seguindo um passo a passo.

O núcleo de DSP não depende do Windows. As medições e a simulação SPICE eu
rodo aqui, num Linux, e o resultado vale para qualquer sistema.

---

## 12. Riscos

1. **Esquema transcrito com erro.** Mitigação: conferir as tensões DC da
   simulação com as anotadas no esquema MNATS e com as medidas por Antão.
2. **Newton não converge em transientes fortes** (aconteceu com Antão).
   Mitigação: blocos menores, ponto de partida pela amostra anterior, limite de
   passo e alternativa de solução por homotopia nos casos difíceis; contador
   de falhas registrado nos testes.
3. **CPU acima do estimado.** Mitigação: tabelas para as funções exponenciais,
   os dois canais calculados juntos com instruções SIMD, variante leve do
   detector (3.4).
4. **Transformador de saída** sem dados. Mitigação: deixar explícito nas
   medições; ajustar se aparecer áudio de referência.

[mnats-revd-sch]: http://mnats.net/files/1176REVD_VOLTS.pdf
[holters-ja]: https://www.hsu-hh.de/ant/wp-content/uploads/sites/699/2017/10/Holters_jamodel_DAFx16.pdf
[holters-dk11]: https://dafx.de/paper-archive/details/hR2kI9NQ8b1kjRgzxNk0pw
