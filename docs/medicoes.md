# Medições

Cada etapa acrescenta uma seção. Todo número aqui vem de um teste que roda
sozinho (`BreuTests`, ou o pluginval). Onde não houver valor publicado para
comparar, isso é dito.

---

## Etapa (a) — plugin que passa o áudio

Data: 20/11/2026. Compilação: Linux (GCC 13), JUCE 9.0.6, Release. Os mesmos
testes rodam no Windows e no macOS no GitHub Actions.

### O que foi medido

Nesta etapa o "núcleo" (onde o circuito vai entrar) não faz nada. O caminho
é: sobe a taxa (oversampling) → núcleo vazio → desce a taxa. Então a saída
deveria ser a entrada atrasada, só com o efeito dos filtros de oversampling.

### Oversampling e latência

| Taxa do projeto | Fator Normal | Fator Alta | Taxa interna Normal | Taxa interna Alta | Latência |
|---|---|---|---|---|---|
| 44,1 kHz | 4x | 8x | 176,4 kHz | 352,8 kHz | 65 amostras (1,47 ms) |
| 48 kHz | 4x | 8x | 192 kHz | 384 kHz | 65 amostras (1,35 ms) |
| 88,2 kHz | 2x | 4x | 176,4 kHz | 352,8 kHz | 60 amostras (0,68 ms) |
| 96 kHz | 2x | 4x | 192 kHz | 384 kHz | 60 amostras (0,63 ms) |
| 176,4 kHz | 1x | 2x | 176,4 kHz | 352,8 kHz | 49 amostras (0,28 ms) |
| 192 kHz | 1x | 2x | 192 kHz | 384 kHz | 49 amostras (0,26 ms) |

- A latência é **a mesma nos dois modos** e é a que o plugin informa ao
  Ableton (testado).
- Um impulso entra e sai **exatamente** na amostra informada como latência, em
  todas as combinações de taxa (44,1 / 48 / 96 kHz), modo e tamanho de bloco
  (64, 512 e 1001 amostras).

**Uma correção feita durante a etapa.** Na primeira versão usei a opção do
JUCE que arredonda a latência para um número inteiro. O teste mostrou que ela
faz isso com um filtro passa-tudo, que atrasa os agudos de forma diferente
dos graves: o resíduo chegava a −10 dB em 10 kHz. Troquei por um atraso puro
dentro da taxa interna, que completa a fração de amostra sem mexer na fase. O
resíduo caiu para −60 a −87 dB (tabela abaixo).

### Resposta em frequência e resíduo

Ganho = nível de saída ÷ nível de entrada. Resíduo = o que sobra quando se
subtrai da saída a entrada atrasada; quanto mais negativo, mais a saída é
idêntica à entrada.

| Freq. | 48 kHz Normal: ganho | resíduo | 48 kHz Alta: ganho | resíduo | 44,1 kHz Normal: ganho | resíduo |
|---|---|---|---|---|---|---|
| 20 Hz | −0,002 dB | −74,5 dB | −0,008 dB | −60,4 dB | −0,002 dB | −74,5 dB |
| 1 kHz | −0,001 dB | −85,6 dB | −0,007 dB | −62,0 dB | −0,001 dB | −85,2 dB |
| 10 kHz | −0,002 dB | −74,2 dB | −0,002 dB | −72,2 dB | −0,001 dB | −79,5 dB |
| 15 kHz | −0,003 dB | −69,6 dB | −0,005 dB | −64,4 dB | −0,004 dB | −67,2 dB |
| 18 kHz | −0,002 dB | −73,2 dB | −0,006 dB | −63,0 dB | −0,000 dB | −87,5 dB |
| 20 kHz | −0,001 dB | −77,9 dB | −0,006 dB | −62,9 dB | −0,083 dB | −40,5 dB |

- Critério: ganho dentro de ±0,1 dB de 20 Hz a 18 kHz e resíduo abaixo de
  −50 dB até 15 kHz. **Passou** em todas as taxas e modos testados.
- O resíduo de −60 dB no modo Alta é a ondulação dos filtros (variação de
  ±0,008 dB no ganho), não um erro de tempo. É inaudível.
- A 44,1 kHz, 20 kHz fica perto do limite de Nyquist (22,05 kHz), onde os
  filtros começam a cortar: −0,08 dB.

### Outros testes

| Teste | Resultado |
|---|---|
| Bypass | saída **bit a bit idêntica** à entrada atrasada |
| Troca Normal → Alta durante a reprodução | sem salto: desvio máximo −62 dB em relação à entrada atrasada |
| Salvar e reabrir o estado | todos os 9 parâmetros voltam com o mesmo valor |
| Faixa mono | processa com ganho unitário |
| pluginval, nível 5 | passou (Linux) |
| pluginval, nível 10 | passou (Linux) |

### O que não foi verificado nesta etapa

- Carregar no Ableton (precisa do binário do macOS gerado no GitHub Actions e
  do seu teste no MacBook).
- pluginval no Windows e no macOS: roda no GitHub Actions; o resultado fica
  registrado na execução.
- Uso de CPU: só faz sentido medir quando houver circuito (etapa b).
