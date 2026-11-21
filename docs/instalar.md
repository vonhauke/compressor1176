# Instalar o Breu (versão de teste)

O plugin é compilado automaticamente no GitHub a cada envio. Para baixar:

1. Abra <https://github.com/vonhauke/compressor1176/actions>.
2. Clique na execução mais recente com o ícone verde (✓) no branch da etapa.
3. Lá embaixo, em **Artifacts**, baixe `Breu-macOS` ou `Breu-Windows` e
   descompacte.

## macOS (Ableton no MacBook)

O pacote `Breu-macOS` traz dois arquivos compactados: `Breu-vst3.zip` (o
plugin) e `Breu-app.zip` (a versão Standalone). Funciona em Mac Apple Silicon
e Intel, e também com o Ableton aberto pelo Rosetta.

O plugin é assinado só "ad-hoc" (sem conta paga de desenvolvedor da Apple).
No Terminal, troque `PASTA` pelo caminho onde você descompactou o
`Breu-macOS` (entre aspas, se tiver espaços):

```bash
cd "PASTA"
mkdir -p ~/Library/Audio/Plug-Ins/VST3
rm -rf ~/Library/Audio/Plug-Ins/VST3/Breu.vst3
ditto -x -k Breu-vst3.zip ~/Library/Audio/Plug-Ins/VST3/
xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/VST3/Breu.vst3
codesign --force --deep --sign - ~/Library/Audio/Plug-Ins/VST3/Breu.vst3
```

No Ableton: **Settings → Plug-Ins**, ligue **Use VST3 Plug-in System
Folders** e clique em **Rescan**. O Breu aparece em Plug-Ins → vonhauke.

## Windows

Copie a pasta `Breu.vst3` para `C:\Program Files\Common Files\VST3\` (crie a
pasta se não existir; pede permissão de administrador). `Breu.exe` é a versão
Standalone.

## O que esperar nesta etapa (a)

Nada de compressão ainda. O plugin deixa o áudio passar pelo oversampling e
informa a latência ao Ableton. Os botões aparecem numa tela provisória, mas
ainda não mudam o som (exceto Bypass e Quality, que não alteram o som de forma
audível).
