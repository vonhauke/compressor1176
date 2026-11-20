# Pesquisa — compressor FET, modelo 1176LN revisão E

Data: 20/11/2026. Etapa 1 do projeto (antes de qualquer código).

## Como ler este documento

Cada seção separa duas coisas:

- **Fato documentado** — está escrito numa fonte, com link. Quando a fonte é um
  fórum ou um texto secundário, digo isso.
- **Suposição minha** — conclusão que tirei juntando fatos, ou hipótese que
  ainda precisa ser testada. Toda suposição tem um jeito de ser verificada no
  plano.

Quando duas fontes divergem, as duas aparecem lado a lado na seção
"Divergências entre fontes" (no fim).

Nenhum código de emulação existente foi aberto ou copiado. As emulações de
código aberto foram consultadas só para saber que existem e qual é a licença.

---

## 1. Qual revisão modelar

### Fato documentado

| Revisão | O que mudou | Fonte |
|---|---|---|
| A (1967) | Projeto original, painel prateado com faixa azul | [Mix][mix-rev] |
| AB | Valores de resistores do pré-amplificador mudados; capacitores de bypass em volta do resistor que alimenta o FET | [Mix][mix-rev] |
| B | Pré-amplificador passa a ser todo bipolar, com uma derivação do emissor do primeiro transistor realimentando o estágio do FET | [Mix][mix-rev] |
| C (1970) | Primeiro painel preto e sufixo LN. Circuito de Brad Plunkett que reduz a tensão dreno-fonte do FET para mantê-lo na região linear, montado como módulo encapsulado sobre a placa B | [Mix][mix-rev], [manual UA p.31][ua-31] |
| D | Nenhuma mudança de circuito; placa redesenhada já com o circuito LN e o trimpot de "Q bias" | [Mix][mix-rev] |
| E | Transformador de força comutável 110/220 V. O manual da UA acrescenta um resistor de 10 MΩ em paralelo com a chave de ratio para evitar estalos na troca | [Mix][mix-rev], [manual UA p.32][ua-32] |
| F | Saída passa de classe A para push-pull (classe AB) | [Mix][mix-rev], [manual UA p.32][ua-32] |
| G | Transformador de entrada substituído por entrada diferencial com amplificador operacional | [Mix][mix-rev], [manual UA p.30][ua-30] |
| H | Painel volta a ser prateado | [Mix][mix-rev] |

- A reedição da Universal Audio declara ter sido baseada nas versões D/E
  ([manual da reedição 1176LN][ua-reissue]; [webzine UA, maio 2003][ua-webzine-may]).
- Segundo um comparativo de plugins, a maioria das emulações comerciais se
  baseia na Rev E ([SonicScoop][sonicscoop]) — fonte secundária.

### Suposição minha

- **Mantenho a Rev E.** Pelo que as fontes dizem, o caminho de áudio da E é o
  mesmo da D. As diferenças são de fonte de alimentação (110/220 V) e o
  resistor anti-estalo de 10 MΩ na chave de ratio, que só atua no instante da
  troca de ratio. Na prática estamos modelando o circuito D/E, que é também o
  que a reedição e a maioria das emulações tomam como base. Não há motivo para
  trocar de revisão.
- A lista de materiais do clone MNATS Rev D v2.2 já inclui um resistor de
  10 MΩ (R45) na placa de ratio ([BOM MNATS][mnats-bom]). É provável que esse
  clone já incorpore a mudança da Rev E. Vou confirmar no esquema.

---

## 2. Topologia geral (o caminho do sinal)

### Fato documentado

Fonte principal: seção técnica do manual da Universal Audio
([p.29][ua-29], [p.30][ua-30], [p.31][ua-31], [p.32][ua-32], [p.33][ua-33]) e
manual da reedição ([ua-reissue]).

1. **Entrada**: atenuador passivo ajustável (o botão Input) seguido de
   transformador. Serve para baixar o nível antes do FET. O mesmo botão define
   quanto o aparelho comprime, porque o limiar (threshold) é fixo.
2. **Estágio de redução de ganho**: o FET (Q1) funciona como resistência
   variável e forma a metade de baixo de um divisor de tensão com R5. Quanto
   maior a tensão no gate, menor a resistência do FET e menor o ganho.
3. **Pré-amplificador** depois do FET, para recuperar o nível.
4. **Potenciômetro de saída** (R23).
5. **Amplificador de linha**: par Darlington seguido de estágio classe A com
   transistor 2N3053. O transformador de saída tem enrolamentos extras que
   fazem parte da realimentação negativa do amplificador.
6. **Detector (sidechain)**: a entrada do detector é a **saída do
   pré-amplificador, antes do potenciômetro de saída**. Por isso o 1176 é um
   compressor de realimentação (feedback): ele mede o sinal depois da redução
   de ganho.

Esse arranjo é confirmado por textos independentes ([Sound On Sound][sos-ua];
[Mix][mix-101102]; [blog UA][ua-blog-history]).

### Suposição minha

- O seu documento diz "o detector lê o sinal de saída do estágio de ganho".
  Isso confere com as fontes, com um detalhe que importa: o ponto de leitura é
  a saída do **pré-amplificador**, antes do botão Output e antes do
  amplificador de linha e do transformador de saída. Consequências: o botão
  Output não muda a compressão, e a saturação do transformador de saída não
  entra no laço do detector.

---

## 3. O elemento de ganho FET e o circuito LN

### Fato documentado

- R5 = 27 kΩ; Q1 = 2N5457 (FET de junção canal N) ([BOM MNATS][mnats-bom]).
- O FET é não linear e distorce; o circuito LN existe para mantê-lo o mais
  perto possível da região linear ([manual UA p.31][ua-31]).
- Pela história da Mix, o circuito LN faz isso **reduzindo a tensão
  dreno-fonte** do FET ([Mix][mix-rev]).
- Técnica clássica e bem documentada para linearizar um FET usado como
  resistor: devolver ao gate cerca de metade da tensão do dreno. Isso cancela
  o termo quadrático da corrente do FET e deixa a resistência quase linear
  ([nota de aplicação Vishay/Siliconix AN105][vishay-an105];
  [patente US 7496342][patent-7496342]; [patente US 5894522][patent-5894522]).
- Valores do circuito D (BOM MNATS): R6 e R7 = 2,2 MΩ; R4 = 270 Ω; C1 = 1 µF
  ([BOM MNATS][mnats-bom]).
- Parâmetros medidos de um 2N5457 real numa tese que modelou a Rev D
  ([Antão, 2014][antao]): tensão de corte ≈ −1,1 V, IDSS ≈ 2,2 mA. No
  protótipo, a tensão gate-fonte em repouso era −1,22 V.
- Um participante de fórum estima que a faixa útil de controle do gate é de
  só ~0,2 V e que o divisor R6/R7 pode dobrá-la ([KVR][kvr-env]) — fórum.

### Suposição minha

- Acho que R6/R7 são o divisor "metade do dreno para o gate". Preciso
  confirmar no esquema ([MNATS Rev D][mnats-revd-sch]) antes da etapa (b).
- A distorção que muda com a quantidade de redução de ganho vem
  principalmente daqui: com mais compressão, a resistência do FET cai, a
  tensão sobre ele muda e a parte não cancelada do termo quadrático muda
  junto. Um modelo físico do FET (equação de Shichman–Hodges ou equivalente)
  reproduz isso sem curva inventada.
- FETs 2N5457 variam muito de uma peça para outra. Por isso o aparelho tem o
  ajuste de "Q bias" (seção 8). No modelo, vou usar os parâmetros medidos por
  Antão como ponto de partida e calibrar o Q bias pelo mesmo procedimento do
  manual.

---

## 4. Pré-amplificador e amplificador de linha

### Fato documentado

- Transistores (BOM MNATS): Q2, Q3, Q4 = 2N3391 (pré); Q5 = 2N3707 e Q6 =
  2N3053 (linha, Q6 com dissipador) ([BOM MNATS][mnats-bom]).
- O pré-amplificador tem três estágios a transistor ([Antão][antao]).
- Saída: Darlington + classe A com 2N3053 ([manual UA p.32][ua-32]).
  Praticamente o mesmo amplificador do pré UA 1108 ([manual UA p.32][ua-32];
  [Mix][mix-101102]).
- O transformador de saída é parte da realimentação do estágio de saída
  ([manual UA p.32][ua-32]). Como ele fica dentro da realimentação negativa,
  as não linearidades dele são em boa parte corrigidas ([Sound On
  Sound][sos-ua]).
- Transformador original citado: UA-5002A ([Mix][mix-rev]). Réplicas usadas em
  clones: Hairball 5002 e Cinemag CM96731; entrada Altran C-3837-1
  ([BOM MNATS][mnats-bom]).
- Ganho medido do amplificador de linha no protótipo de Antão: 18,33 V/V
  (25,26 dB). A tese **não modelou** o amplificador de linha nem o
  transformador de saída; deixou o transformador como trabalho futuro
  ([Antão][antao]).
- Especificações da reedição: saída de até +22 dBm com no máximo 0,5% de THD;
  resposta ±1 dB de 20 Hz a 20 kHz; ganho total 45 dB ±1 dB; impedância de
  entrada e carga de saída de 600 Ω ([manual da reedição][ua-reissue]).

### Suposição minha

- **Não encontrei dados publicados do núcleo do transformador de saída**
  (indutância, material, curva de saturação). Esta é a maior lacuna da
  pesquisa. No plano proponho estimar os parâmetros a partir da resposta em
  frequência especificada e de dados de réplicas, se eu conseguir algum. O
  valor desses parâmetros será, assumidamente, uma estimativa.
- Modelos de transformador com histerese (Jiles–Atherton) existem e já foram
  integrados a simulação de circuitos de áudio ([Holters & Zölzer,
  DAFx-16][holters-ja]). Sem dados do núcleo, os parâmetros desse modelo
  seriam chutados.

---

## 5. Detector, ratio, ataque e release

### Fato documentado

Funcionamento ([manual UA p.32–33][ua-32]; [p.33][ua-33]):

- A entrada do detector vem da saída do pré-amplificador, logo antes de R23.
- Os botões de ratio definem quanto sinal vai para o detector.
- Q7 (inversor de fase) → Q8 (seguidor de emissor) → diodo CR3; Q9/Q10 (outro
  par inversor/seguidor) → diodo CR2. Os sinais em CR2 e CR3 estão em fase
  oposta, então os dois diodos formam um retificador de onda completa.
- C22 filtra a saída do retificador e vira uma tensão contínua.
- Os botões de ratio também aplicam uma polarização (bias) aos diodos, que
  ajusta o limiar para cada ratio.
- R55 controla o ataque (velocidade de carga de C22); R56 controla o release
  (velocidade de descarga). A tensão resultante vai para o gate do FET.

Valores (BOM MNATS, Rev D) ([mnats-bom]):

| Peça | Valor | Função |
|---|---|---|
| C22 | 0,22 µF | capacitor de tempo |
| R55 | potenciômetro 25 kΩ | ataque |
| R56 | potenciômetro 5 MΩ | release |
| R57 | 270 kΩ | release mínimo (em série) |
| C27 | 0,022 µF (placa de ratio) | — |
| Placa de ratio | R19 68 kΩ, R78 56 kΩ, R63 1,5 kΩ, R45 10 MΩ, R54/R61 470 Ω, R65 3,9 MΩ | divisores de ratio e bias |
| CR2, CR3 | FDH333 (diodo de baixa fuga; "não substituir") | retificador |
| Q7–Q10 | 2N3707, hFE > 250 | amplificador do detector |

- Na tese de Antão, os botões de ratio formam dois divisores de tensão
  ([Antão][antao]).
- Diodos FDH333 medidos: corrente de saturação ≈ 2,2–2,8 × 10⁻¹⁰ A, fator de
  idealidade ≈ 1,68–1,71 ([Antão][antao]).

Tempos especificados ([manual da reedição][ua-reissue]):

- Ataque: 20 a 800 µs. Release: 50 ms a 1,1 s.
- Ataque mais rápido com o botão todo no sentido horário; release mais rápido
  também todo no sentido horário. Ataque todo anti-horário desliga a
  compressão.

Conta simples de RC (fóruns):

- Release: 0,22 µF × (270 kΩ a 5,27 MΩ) ≈ 59 ms a 1,16 s. Bate com a
  especificação ([KVR][kvr-env]; [groupdiy][gdiy-62264]).
- Ataque: a mesma conta dá algo como 0,05–0,1 ms a 5,5 ms, ou seja, **5 a 7
  vezes mais lento que o especificado**. Nenhum dos dois fóruns chega a uma
  explicação ([KVR][kvr-env]; [groupdiy][gdiy-62264]).

Dependência do programa:

- O 1176 comprime na ratio escolhida durante o transiente, mas a ratio sobe um
  pouco depois dele; quanto sobe depende do material ([Will Shanks, webzine
  UA][ua-webzine-apr], reproduzido em [Sweetwater][sw-allbutton]).
- Joelho mais suave nas ratios baixas e mais duro nas altas; o limiar sobe com
  a ratio ([Moore, 2012][moore-arp]; [Moore, tese][moore-thesis]).

Ratios medidas, comparadas com as nominais:

| Nominal | 4:1 | 8:1 | 12:1 | 20:1 | Fonte |
|---|---|---|---|---|---|
| Protótipo Rev D (medido) | 5,5 | 9,4 | 15,6 | 21,2 | [Antão][antao] |
| Simulação de Antão | 5,2 | 9,7 | 16,3 | 19,2 | [Antão][antao] |
| Clone Rev D de um usuário | 5,5 | 9 | 14 | 24 | [groupdiy][gdiy-revd-69] (fórum) |

### Suposição minha

- **Por que o ataque real é mais rápido que o RC.** Minha hipótese: o
  amplificador do detector entrega uma tensão muito maior que o limiar dos
  diodos quando chega um transiente forte; C22 só precisa subir uma fração
  pequena da tensão de pico para fechar o ganho. O tempo "até a compressão
  pegar" fica então bem menor que R×C. O laço de realimentação acelera ainda
  mais. Isso vai ser testado na simulação SPICE da etapa (b), medindo o tempo
  de ataque do mesmo jeito que se mede num aparelho (degrau de tom).
- **De onde vem o release dependente do programa.** Não achei explicação de
  circuito numa fonte confiável. Hipótese: (1) a relação entre a tensão de
  gate e o ganho em dB é muito curva, então uma descarga exponencial de C22
  vira um release não exponencial em dB, mais rápido na parte de muita
  compressão e mais lento perto de zero; (2) o laço de realimentação muda a
  carga sobre C22 conforme o nível de saída. Com o circuito modelado, esse
  comportamento deve aparecer sozinho, sem curva escrita à mão. Vou conferir
  com os ensaios de rajada de tom de Moore (seção 7).
- **Ratio vem dos divisores.** A ratio não é um número escolhido: sai do
  quanto de sinal chega ao detector e da polarização dos diodos, multiplicados
  pelo ganho do laço. Por isso as ratios medidas não batem com as nominais. O
  modelo deve reproduzir as medidas, não os rótulos do painel.

---

## 6. Modo "todos os botões apertados"

### Fato documentado

- O efeito é obtido apertando só 4:1 e 20:1 juntos ([webzine UA, abril
  2003][ua-webzine-apr]; [Moore, tese][moore-thesis]).
- A ratio vai para algum ponto entre 12:1 e 20:1, os pontos de polarização
  mudam no circuito inteiro e, com isso, mudam também ataque e release. A
  distorção aumenta muito por causa de um atraso no ataque dos transientes
  iniciais ([manual UA p.23][ua-23]).
- O medidor de GR "se comporta de forma estranha" nesse modo, e isso faz parte
  do aparelho ([manual do plugin UA][ua-plugin-manual]). Moore diz que a agulha
  indo para o vermelho é só visual, sem efeito no áudio ([Moore,
  tese][moore-thesis]).
- Medição de intermodulação (SMPTE 60 Hz + 7 kHz) com ataque e release no
  máximo: 18,53% no 1176 Rev D original contra 31,73% na reedição, enquanto o
  mesmo ajuste em 4:1 dá ≈ 2,1–2,3% ([Moore, tese, Tabela 6-4][moore-thesis]).
- Num microfone de ambiente de bateria, o modo reduziu a faixa dinâmica de
  19,57 dB para 6,54 dB no original ([Moore, tese, Tabela 6-14][moore-thesis]).

### Suposição minha

- Com 4 e 20 apertados ao mesmo tempo, as duas derivações dos divisores de
  ratio ficam ligadas em paralelo. Isso muda, numa combinação que não foi
  projetada, tanto o nível que chega ao detector quanto a polarização dos
  diodos. O resultado é um limiar mais baixo, polarização deslocada e o
  atraso de ataque que deixa passar o pico inicial (o overshoot). Para
  modelar isso **sem improvisar**, preciso da fiação exata dos contatos da
  botoeira com dois botões apertados. Ela está nos documentos de fiação dos
  clones ([MNATS/Hairball][mnats-wiring]) e no esquema D. Vou transcrever na
  etapa (c) e conferir na simulação SPICE se o resultado fica dentro de
  "12:1 a 20:1" com overshoot no ataque.

---

## 7. Medições publicadas do aparelho original

### Fato documentado

Fonte: [tese de doutorado de Austin Moore][moore-thesis] (Univ. Huddersfield),
que comparou um 1176 Rev D original ("blackface") com reedições da UA. A THD
inclui até 10 harmônicos.

THD sem compressão, tom de 1 kHz, três níveis de entrada (Tabela 6-1):

| Nível de entrada | Original Rev D | Reedição |
|---|---|---|
| 1 | 0,53% | 0,15% |
| 2 | 1,85% | 0,17% |
| 3 | 5,53% | 7,59% (clipping) |

THD durante a compressão (Tabelas 6-2 e 6-3):

| Condição | Original Rev D | Reedição |
|---|---|---|
| 1 kHz, comprimindo | 0,16% | 0,37% |
| 1 kHz, em release | 0,05% | 0,19% |
| 50 Hz, comprimindo | 2,49% | 3,81% |
| 50 Hz, em release | 0,12% | 1,14% |

Outros resultados de Moore:

- Harmônicos pares maiores que os ímpares em todas as entradas, exceto a
  reedição no nível 3.
- O release afeta a distorção mais que o ataque. Os harmônicos caem bem a
  partir da posição 6 do release. Com ataque mais rápido e release mais lento,
  o 3º e o 5º harmônicos ficam ~20 dB e ~17 dB abaixo do caso oposto.
- O original solta (release) mais rápido que a reedição, e com a mesma
  leitura no medidor atenua mais.
- Calibração usada: entrada 30 / saída 18 ≈ ganho unitário a +16 dBu; cada
  passo do botão de entrada ≈ 5 dB.

Especificações da reedição ([manual da reedição][ua-reissue]): THD < 0,5% de
50 Hz a 15 kHz comprimindo com release de 1,1 s; relação sinal/ruído > 81 dB.

### Suposição minha

- Os números de Moore são de **um** aparelho original, com os componentes e o
  ajuste de bias daquele exemplar. Vou usá-los como faixa de referência, não
  como alvo exato.
- Os "níveis de entrada" de Moore são posições do botão, não valores em dBu
  exatos (o próprio texto se contradiz sobre o nível 2). Para comparar, vou
  reproduzir o procedimento dele (ganho unitário a +16 dBu, passos de ~5 dB) e
  dizer claramente que é uma reconstrução.
- **Não encontrei** curvas de transferência estática nem respostas ao degrau
  de ataque e release publicadas em números para um 1176 original (só gráficos
  em artigos pagos ou figuras que não consegui ler). A comparação de tempos
  vai se apoiar na especificação, nas medidas de Moore e na simulação SPICE.

---

## 8. Calibração interna (Q bias e medidor)

### Fato documentado

- Q bias: girar R59 no sentido horário devagar até a saída cair 1,0 dB
  ([manual UA p.27][ua-27]). Antão fez: 5 dBu na saída, ajustar até 4 dBu
  ([Antão][antao]).
- Bias errado gera distorção forte durante a redução de ganho ([Moore,
  tese][moore-thesis]).
- O medidor de GR tem ajuste de zero (R71), rastreamento (R44) e anulação do
  driver (R75) ([manual UA p.27][ua-27]; [BOM MNATS][mnats-bom]).
- Há um segundo 2N5457 (Q11) na placa ([BOM MNATS][mnats-bom]).

### Suposição minha

- Q11 deve ser o FET do medidor, recebendo a mesma tensão de gate de Q1 para
  que a agulha acompanhe a redução de ganho. Isso explicaria a leitura
  "estranha" no modo todos-os-botões: a agulha mostra a tensão de gate, não a
  redução real. Confirmar no esquema.

---

## 9. Medidor VU

### Fato documentado

- Norma (ASA 1954, base da ANSI C16.5 e IEC 60268-17): com um sinal súbito e
  constante, a agulha chega a 99% do valor final em 0,3 s ± 0,03 s e
  ultrapassa o valor final entre 1% e 1,5% ([Lobdell & Allen][lobdell-allen]).
- Um VU é um retificador de onda completa seguido de um instrumento
  mecânico (massa, mola, amortecimento), o que dá um sistema de 2ª ordem
  ([Lobdell & Allen][lobdell-allen]).
- No 1176LN: chave +4 (0 VU = +4 dBm na saída), +8, GR e desligado
  ([manual da reedição][ua-reissue]).

---

## 10. Trabalhos anteriores de modelagem

### Fato documentado

- **[Antão, 2014][antao]** — tese que aplicou o método nodal DK (espaço de
  estados não linear) ao 1176LN Rev D, com protótipo físico construído na
  placa MNATS v2.2. Circuito com 50 nós e 10 dispositivos não lineares (7
  transistores bipolares, 1 FET, 2 diodos). Taxa de 48 kHz; o texto não menciona oversampling.
  Newton com média de 2,4 a 2,9 iterações por amostra, mas transientes de
  bateria passaram de 50 iterações em algumas amostras. Baixo e voz sem
  diferença audível para o protótipo; bateria com diferenças pequenas. O
  amplificador de linha virou só um ganho fixo, e o transformador de saída
  ficou de fora.
- **[Gerat, Eichas & Zölzer, AES 2017][gerat]** — modelo "caixa cinza"
  (blocos paramétricos ajustados por medições de entrada e saída) de um
  1176LN. Artigo pago; li só o resumo.
- **[Holters & Zölzer, DAFx-11][holters-dk11]** — método DK com potenciômetros
  que mudam em tempo real (recalcular a matriz sem refazer tudo).
  **[Yeh, 2010][yeh]** — base do método DK. **[Holters & Zölzer,
  2015][holters-2015]** — generalização.
- **Wave digital filters (WDF):** [chowdsp_wdf][chowdsp-wdf] (BSD-3,
  C++14); [RT-WDF][rt-wdf]; circuitos com vários elementos não lineares
  exigem adaptadores R-type e iteração ([Werner et al.][werner-grouped];
  [Bernardini et al.][bernardini]).
- **Redes neurais:** [Steinmetz & Reiss][steinmetz] e trabalhos recentes
  modelam compressores (sobretudo o LA-2A) a partir de gravações do aparelho.
- **Código aberto com modo FET:** [Multi-Comp][multicomp] (GPL-2.0);
  framework DK [dkmethod][dkmethod] (GPL-3). **Não abri o código.** As duas
  licenças são incompatíveis com fechar o código depois; não vamos depender
  delas.

### Suposição minha

- O trabalho de Antão é a evidência mais forte de que modelar o circuito
  inteiro é possível, e também mostra os riscos: iterações explodindo em
  transientes e o estágio de saída não modelado. O plano parte disso.

---

## 11. Licenças das ferramentas

### Fato documentado

- JUCE tem licença dupla: AGPLv3 ou licença JUCE. O plano gratuito "Starter"
  vale até US$ 20 mil de receita em 12 meses e não exige mais a tela "Made with
  JUCE" ([fórum JUCE][juce-license]).
- chowdsp_wdf: BSD-3 ([GitHub][chowdsp-wdf]).
- pluginval: GPL-3, usado só como ferramenta de teste; nível de rigor 5 é o
  mínimo recomendado para compatibilidade com hosts ([README][pluginval]).

---

## 12. O que não consegui encontrar

1. Esquema original da UREI legível. Há reproduções de clones (MNATS) com
   tensões anotadas: [Rev D][mnats-revd-sch]. Vou trabalhar a partir delas.
2. Dados do núcleo do transformador de saída UA-5002A.
3. Curva (taper) dos potenciômetros de ataque e release. Moore diz que o
   release muda quase tudo na primeira metade do giro ([tese][moore-thesis]),
   o que sugere curva logarítmica, mas não há valor publicado.
4. Respostas ao degrau de ataque e release em números de um aparelho real.
5. Curvas de transferência estática do original, além das ratios medidas por
   Antão.
6. A fiação exata da botoeira para combinações de botões (preciso do
   documento de fiação dos clones).

---

## Divergências entre fontes

| Assunto | Fonte A | Fonte B | Minha leitura |
|---|---|---|---|
| O que muda na Rev E | Só a alimentação 110/220 V ([Mix][mix-rev]) | 220 V **e** resistor de 10 MΩ na chave de ratio ([manual UA][ua-32]) | Não se contradizem; o manual é mais detalhado. Nenhuma das duas mudanças altera o áudio em regime |
| Faixa de ataque | 20–800 µs ([manual da reedição][ua-reissue]) | 200–800 µs no texto de [Moore][moore-thesis] | Provável erro de digitação em Moore; fico com 20–800 µs |
| Ataque calculado por RC | ~0,1 a 5,6 ms ([KVR][kvr-env]) | ~0,05 a 5,5 ms ([groupdiy][gdiy-62264]) | As duas contas ignoram o laço e o nível do detector; ver hipótese na seção 5 |
| "Ataque" medido por Moore | 20–25 ms em programa real (Tabela 6-17) | 20–800 µs na especificação | Medem coisas diferentes: Moore mediu o envelope da caixa de bateria, não a constante de tempo do circuito |
| Ratio real vs. rótulo | 4/8/12/20 (painel) | 5,5/9,4/15,6/21,2 ([Antão][antao]); 5,5/9/14/24 (fórum) | O alvo é a medida, não o rótulo |
| Ratio no modo todos-os-botões | Entre 12:1 e 20:1 ([manual UA][ua-23]) | "basicamente 7.500:1" ([groupdiy][gdiy-7500], fórum, sem fonte) | Fico com o manual; o valor do fórum não tem medição |
| Todos os botões = 4+20? | Sim ([webzine UA][ua-webzine-apr]; [Moore][moore-thesis]) | Um usuário diz que é eletricamente idêntico a 4+20 ([groupdiy][gdiy-62264]); Gyraf diz que no clone dele precisa de chave separada ([Gyraf][gyraf]) | Concordam para o original; o caso Gyraf é específico da chave rotativa dele |
| Medidor no modo todos-os-botões | Comportamento estranho faz parte do aparelho ([manual plugin UA][ua-plugin-manual]) | A agulha no vermelho é só visual ([Moore][moore-thesis]) | Compatíveis: o medidor lê mal, o áudio não depende dele |

---

## Fontes

[mix-rev]: https://www.mixonline.com/recording/1176-revision-history-372048
[mix-101102]: https://www.mixonline.com/?p=101102
[ua-23]: https://www.manualsdir.com/manuals/293279/universal-audio-1176ae.html?page=23
[ua-27]: https://www.manualsdir.com/manuals/293279/universal-audio-1176ae.html?page=27
[ua-29]: https://www.manualsdir.com/manuals/293279/universal-audio-1176ae.html?page=29
[ua-30]: https://www.manualsdir.com/manuals/293279/universal-audio-1176ae.html?page=30
[ua-31]: https://www.manualsdir.com/manuals/293279/universal-audio-1176ae.html?page=31
[ua-32]: https://www.manualsdir.com/manuals/293279/universal-audio-1176ae.html?page=32
[ua-33]: https://www.manualsdir.com/manuals/293279/universal-audio-1176ae.html?page=33
[ua-reissue]: https://c3.zzounds.com/media/1176LN_manual-1a90c9dff91ec30316551923e877c4d0.pdf
[ua-webzine-apr]: https://uaudio.com/webzine/2003/april/text/content4.html
[ua-webzine-may]: https://www.uaudio.com/webzine/2003/may/text/content4.html
[ua-plugin-manual]: https://help.uaudio.com/hc/en-us/articles/34530260482324-1176-Classic-FET-Compressor-Manual
[ua-blog-history]: https://www.uaudio.com/blog/analog-obsession-1176-history
[sos-ua]: https://www.soundonsound.com/reviews/universal-appeal
[sonicscoop]: https://sonicscoop.com/?p=82073
[sw-allbutton]: https://www.sweetwater.com/insync/all-button-mode/
[mnats-bom]: https://dsl-man.de/download/attachments/128057461/1176LN_REVD_V2.2_DOC.pdf?api=v2
[mnats-revd-sch]: http://mnats.net/files/1176REVD_VOLTS.pdf
[mnats-wiring]: http://mnats.net/1176_reva-d_hairball_wiring_power.html
[antao]: https://ria.ua.pt/bitstream/10773/12791/1/tese.pdf
[moore-thesis]: https://eprints.hud.ac.uk/id/eprint/34118/1/FINAL%20THESIS%20-%20MOORE.pdf
[moore-arp]: https://eprints.hud.ac.uk/id/eprint/27391/
[gerat]: https://aes2.org/publications/elibrary-page/?id=19249
[holters-dk11]: https://dafx.de/paper-archive/details/hR2kI9NQ8b1kjRgzxNk0pw
[holters-2015]: https://new.eurasip.org/Proceedings/Eusipco/Eusipco2015/papers/1570103545.pdf
[holters-ja]: https://www.hsu-hh.de/ant/wp-content/uploads/sites/699/2017/10/Holters_jamodel_DAFx16.pdf
[yeh]: https://ccrma.stanford.edu/~dtyeh/papers/yeh10_taslp.pdf
[chowdsp-wdf]: https://github.com/Chowdhury-DSP/chowdsp_wdf
[rt-wdf]: https://www.dafx.de/paper-archive/2016/dafxpapers/40-DAFx-16_paper_35-PN.pdf
[werner-grouped]: https://core.ac.uk/download/96664744.pdf
[bernardini]: https://re.public.polimi.it/bitstream/11311/1156955/1/WD3Terminal_Bernardini_RG.pdf
[steinmetz]: https://arxiv.org/pdf/2102.06200.pdf
[multicomp]: https://dusk-audio.github.io/plugins/multi-comp
[dkmethod]: https://github.com/joaorossi/dkmethod
[vishay-an105]: https://www.vishay.com/docs/70598/70598.pdf
[patent-7496342]: https://image-ppubs.uspto.gov/dirsearch-public/print/downloadPdf/7496342
[patent-5894522]: https://image-ppubs.uspto.gov/dirsearch-public/print/downloadPdf/5894522
[kvr-env]: https://www.kvraudio.com/forum/viewtopic.php?p=6432329
[gdiy-62264]: https://www.groupdiy.com/threads/some-more-1176-qs-ideas-attack-confusion-plus-ratios-nuke-content.62264/
[gdiy-revd-69]: https://groupdiy.com/threads/build-1176ln-rev-d-diy.20058/page-69
[gdiy-7500]: https://groupdiy.com/threads/all-things-g1176-the-new-repost-thread.646/page-122
[gyraf]: https://gyraf.dk/gy_pd/1176/slam.htm
[lobdell-allen]: https://jontalle.web.engr.illinois.edu/Public/\\/LobdellAllen06-final.pdf
[juce-license]: https://forum.juce.com/t/amendments-to-the-juce-end-user-licence-agreement-for-juce-8/61265
[pluginval]: https://github.com/Tracktion/pluginval/blob/master/README.md

Lista legível (os mesmos links acima):

- Mix — histórico de revisões: <https://www.mixonline.com/recording/1176-revision-history-372048>
- Manual UA 1176AE, seção técnica: <https://www.manualsdir.com/manuals/293279/universal-audio-1176ae.html?page=29> (páginas 23, 27, 29–33)
- Manual da reedição UA 1176LN: <https://c3.zzounds.com/media/1176LN_manual-1a90c9dff91ec30316551923e877c4d0.pdf>
- Webzine UA (Will Shanks), abril e maio de 2003: <https://uaudio.com/webzine/2003/april/text/content4.html>
- Manual do plugin UA 1176 Classic: <https://help.uaudio.com/hc/en-us/articles/34530260482324-1176-Classic-FET-Compressor-Manual>
- Sound On Sound, review da reedição: <https://www.soundonsound.com/reviews/universal-appeal>
- BOM do clone MNATS Rev D v2.2: <https://dsl-man.de/download/attachments/128057461/1176LN_REVD_V2.2_DOC.pdf?api=v2>
- Esquema MNATS Rev D com tensões: <http://mnats.net/files/1176REVD_VOLTS.pdf>
- Antão (2014), tese DK do 1176LN: <https://ria.ua.pt/bitstream/10773/12791/1/tese.pdf>
- Moore, tese de doutorado: <https://eprints.hud.ac.uk/id/eprint/34118/1/FINAL%20THESIS%20-%20MOORE.pdf>
- Moore (2012), "All Buttons In": <https://eprints.hud.ac.uk/id/eprint/27391/>
- Gerat, Eichas & Zölzer (AES 2017): <https://aes2.org/publications/elibrary-page/?id=19249>
- Holters & Zölzer (DAFx-11, DAFx-16), Yeh (2010), Werner et al., Bernardini et al.: links na tabela acima
- Lobdell & Allen, VU meter: <https://jontalle.web.engr.illinois.edu/Public/\\/LobdellAllen06-final.pdf>
- Vishay AN105 (FET como resistor): <https://www.vishay.com/docs/70598/70598.pdf>
- Fóruns (secundários): KVR <https://www.kvraudio.com/forum/viewtopic.php?p=6432329>, groupdiy <https://www.groupdiy.com/threads/some-more-1176-qs-ideas-attack-confusion-plus-ratios-nuke-content.62264/>
