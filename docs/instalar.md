# Instalar o Breu (versão de teste)

O plugin é compilado automaticamente no GitHub a cada envio. Para baixar:

1. Abra <https://github.com/vonhauke/compressor1176/actions>.
2. Clique na execução mais recente com o ícone verde (✓) no branch da etapa.
3. Lá embaixo, em **Artifacts**, baixe `Breu-macOS` ou `Breu-Windows` e
   descompacte.

## macOS (Ableton no MacBook)

O plugin é assinado só "ad-hoc" (sem conta paga de desenvolvedor da Apple).
O macOS bloqueia arquivos assim quando vêm da internet. No Terminal, dentro da
pasta descompactada:

```bash
mkdir -p ~/Library/Audio/Plug-Ins/VST3
cp -R Breu.vst3 ~/Library/Audio/Plug-Ins/VST3/
xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/VST3/Breu.vst3
xattr -dr com.apple.quarantine Breu.app
```

No Ableton: **Settings → Plug-Ins**, ligue **Use VST3 Plug-in System
Folders** e clique em **Rescan**. O Breu aparece em Plug-Ins → vonhauke.

`Breu.app` é a versão Standalone: abre sozinha, sem o Ableton, e usa a placa
de som escolhida nas opções dela.

## Windows

Copie a pasta `Breu.vst3` para `C:\Program Files\Common Files\VST3\` (crie a
pasta se não existir; pede permissão de administrador). `Breu.exe` é a versão
Standalone.

## O que esperar nesta etapa (a)

Nada de compressão ainda. O plugin deixa o áudio passar pelo oversampling e
informa a latência ao Ableton. Os botões aparecem numa tela provisória, mas
ainda não mudam o som (exceto Bypass e Quality, que não alteram o som de forma
audível).
