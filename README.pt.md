# STACH

[Русский](README.md) · [English](README.en.md) · [简体中文](README.zh-CN.md) · [繁體中文](README.zh-TW.md) · [Español](README.es.md) · [हिन्दी](README.hi.md) · [العربية](README.ar.md) · [Français](README.fr.md) · [বাংলা](README.bn.md) · **Português** · [Bahasa Indonesia](README.id.md) · [اردو](README.ur.md) · [Deutsch](README.de.md) · [日本語](README.ja.md) · [한국어](README.ko.md) · [Türkçe](README.tr.md) · [Tiếng Việt](README.vi.md) · [Italiano](README.it.md) · [فارسی](README.fa.md) · [Polski](README.pl.md) · [Українська](README.uk.md)

STACH transforma um **Waveshare ESP32-S3-Touch-AMOLED-1.75** em um companheiro de bolso. Seus olhos seguem seu dedo e respondem à inclinação. Deslize para a direita para jogar ou use seu telefone para alterar sua expressão, brilho, comportamento e nome de exibição. A placa hospeda sua própria rede Wi-Fi e painel de controle.

![STACH eye animation](preview/stach-smart-eyes.gif)

## Instale em um Waveshare 1.75

Estas instruções foram testadas no Windows com o ambiente `stach-eyes-175` PlatformIO. Ele constrói o aplicativo completo do `src/main.cpp` para a tela redonda 466 × 466 CO5300, 16 MB de flash e 8 MB de PSRAM. Verifique o modelo na placa ou na embalagem antes de piscar. As revisões 1.43, 1.43B e 1.75C possuem hardware diferente.

### 1. Prepare seu computador

Você precisa do Python 3.13 com o inicializador `py`, Git para Windows, um cabo de dados USB e uma conexão com a Internet para a primeira compilação. [Python para Windows](https://www.python.org/downloads/windows/) · [Git para Windows](https://git-scm.com/downloads/win)

Abra o PowerShell e clone o projeto em uma nova pasta. Um caminho curto com caracteres ASCII funciona bem com as ferramentas ESP32.

```powershell
cd $env:USERPROFILE
git clone https://github.com/leonidgg2323-code/stach.git stach-firmware
cd stach-firmware
py -3.13 -m venv .venv
.\.venv\Scripts\python.exe -m pip install "platformio==6.2.0"
```

Alternativamente, escolha Código **→ Baixe ZIP** no GitHub e extraia o arquivo. Abra o PowerShell na pasta que contém `platformio.ini` e execute os dois últimos comandos acima. Os comandos abaixo chamam diretamente o Python do ambiente virtual.

### 2. Instale as dependências e construa

```powershell
.\.venv\Scripts\python.exe -m platformio pkg install -e stach-eyes-175
.\.venv\Scripts\python.exe -m platformio run -e stach-eyes-175
```

A primeira compilação baixa o conjunto de ferramentas ESP32 e pode levar vários minutos. A compilação aplica o patch de compatibilidade da biblioteca de mouse Bluetooth automaticamente. Aguarde `stach-eyes-175  SUCCESS` antes de fazer upload.

### 3. Encontre a porta serial e atualize a placa

Conecte a placa via USB e liste as portas disponíveis.

```powershell
.\.venv\Scripts\python.exe -m platformio device list
```

Identifique a porta que aparece quando você conecta a placa e desaparece quando você a desconecta. Substitua `COM3` nos exemplos pela sua porta. Feche qualquer programa que o utilize, incluindo o monitor serial Arduino IDE.

O upload de **substitui o firmware na placa.** Para manter a opção de restaurar o menu de fábrica, salve um backup flash completo fora do repositório antes de fazer o upload. Um backup completo para este modelo é **16,777,216 bytes**. Use um nome de arquivo novo se você já tiver um backup. A imagem pode conter configurações e credenciais, portanto, mantenha-a privada.

```powershell
.\.venv\Scripts\python.exe -m pip install "esptool>=5,<6"
.\.venv\Scripts\python.exe -m esptool --chip esp32s3 --port COM3 read-flash 0 ALL "$env:USERPROFILE\stach-before-install.flash-backup.bin"
```

Para instalar o STACH, execute o comando upload.

```powershell
.\.venv\Scripts\python.exe -m platformio run -e stach-eyes-175 -t upload --upload-port COM3
```

A placa reinicia após a verificação da soma de verificação e a mensagem `SUCCESS`. PlatformIO grava o bootloader, a tabela de partição e o aplicativo em seus respectivos endereços. O arquivo `firmware.bin` no diretório de construção contém apenas o aplicativo.

### 4. Registre a conta do proprietário

1. Na primeira inicialização, digitalize **1/2 JOIN WI-FI** com seu telefone. Junte-se à rede `Stach-XXXXXX`. Se o seu telefone informar que não tem acesso à Internet, escolha **Fique conectado**.
2. O display muda para **2/2 REGISTER** quando o telefone é conectado. Se o primeiro código ainda estiver sendo exibido, toque uma vez na tela. Digitalize o segundo código com o mesmo telefone.
3. Escolha um nome de usuário de 3 a 32 letras (A–Z ou a–z), dígitos, hifens ou sublinhados e uma senha de pelo menos 12 caracteres. Salve seus dados de login. A conta e as configurações são armazenadas no quadro.
4. Em **Аккаунт владельца** (conta de proprietário), você pode alterar o nome mostrado no painel ou sair. Para visitas posteriores, conecte-se ao Wi-Fi da placa e abra o [painel de controle STACH local](http://192.168.4.1/).

O conselho aceita uma conta de proprietário. O registro requer o código único mostrado no display físico. Os comandos de controle requerem uma sessão do proprietário e um token de solicitação. O painel usa HTTP na rede Wi-Fi local da placa. Faça login novamente depois que o quadro for reiniciado.

## Gestos e jogos

| Ação | Resultado |
|---|---|
| Toque nos olhos ou incline o tabuleiro | Os olhos seguem o seu dedo ou a inclinação. |
| Deslize para a direita a partir dos olhos | Abra o menu de jogos. |
| Deslize para a esquerda em um jogo | Retorne ao cardápio. Deslize para a esquerda novamente para retornar aos olhos. |
| Segure a tela ocular por cerca de um segundo | Mostre o código QR do Wi-Fi. Toque para alternar entre os códigos de rede e de painel. Segure para retornar aos olhos. |
| Deslize para cima a partir dos olhos | Inicie o modo de mouse Bluetooth. Emparelhe com **Stach Mouse** em seu computador e mantenha a placa imóvel para calibração. |
| Toque na tela no modo mouse | Toque para clicar com o botão esquerdo, toque duas vezes para clicar com o botão direito, deslize para rolar ou segure para sair. |

Em **Targets**, acerte o máximo de alvos que puder em 20 segundos. Em **Sums**, responda cinco questões adicionais. Ambos os jogos mostram sua pontuação e salvam seu recorde pessoal. **Dice** lança um dado de seis faces e **Coin** lança uma moeda. Cada resultado vem do gerador de números aleatórios de hardware ESP32. Toque novamente após a animação terminar para um novo lançamento ou virada.

## Exibição e potência

O painel do telefone inclui seleção de expressão, joystick de olhar, controles de piscar e inclinar, brilho dos olhos e uma configuração de quanto os olhos se movem por conta própria. As configurações de aparência persistem durante as reinicializações. Você pode ativar o modo mouse após a inicialização.

Na placa 1.75, o **AXP2101** informa a porcentagem da bateria e o status de carregamento. O indicador mostra `USB` quando a placa funciona com alimentação por cabo e não detecta bateria. Com a economia de energia ativada, a taxa de atualização dos olhos cai após 45 segundos de inatividade e a tela desliga após 5 minutos. Com uma carga relatada de 15% ou menos, esses intervalos caem para 10 segundos e 2 minutos. Toque na tela para ativá-la. Jogos, códigos QR, modo mouse e carregamento ativo mantêm a tela ativa.

## Solução de problemas e recuperação

| Sintoma | O que tentar |
|---|---|
| Nenhuma nova porta COM aparece | Experimente um cabo de dados USB e outra porta USB e execute `platformio device list` novamente. |
| A porta não pode ser aberta ou reporta `Access denied` | Feche o monitor serial. Reconecte a placa e verifique seu número COM. |
| A compilação falha | Verifique se sua pasta atual contém `platformio.ini` e repita os dois comandos na etapa 2. |
| A tela fica em branco | Verifique o modelo da placa e o nome do ambiente no log de upload. Para a placa 1,75, a linha de sucesso deve incluir `stach-eyes-175`. |
| O telefone não consegue abrir o painel | Verifique se ainda está em `Stach-XXXXXX` e abra `http://192.168.4.1/`. O primeiro registro requer o segundo código QR. |

Para visualizar o diagnóstico, abra o monitor serial a 115200 baud.

```powershell
.\.venv\Scripts\python.exe -m platformio device monitor -p COM3 -b 115200
```

Envie `I` para um relatório de inicialização de hardware, `S` para listar dispositivos I²C ou `D` para um teste de exibição de cinco segundos. Pressione **Ctrl+C** para fechar o monitor.

Para restaurar seu backup completo para a mesma placa, feche o monitor e verifique primeiro o tamanho da imagem. O comando a seguir também restaura as configurações e os dados da conta contidos no backup.

```powershell
.\.venv\Scripts\python.exe -m esptool --chip esp32s3 --port COM3 write-flash 0x0 "$env:USERPROFILE\stach-before-install.flash-backup.bin"
```

**Precisa de ajuda?** Contate `twiinkprostolox123` no Discord ou [@def0kk (Telegram)](https://t.me/def0kk). Inclua o modelo da placa, o ambiente PlatformIO e a mensagem de erro. Mantenha sua senha, código QR de registro e backup completo em flash privados.

## História e desenvolvimento do projeto

- **Estágio 9 – Potência.** Os perfis 1.43 mais antigos medem e filtram a tensão ADC em GPIO4. A estimativa da carga requer a verificação do esquema para a revisão específica e a calibração em relação à tensão medida da bateria. A alimentação USB por si só não confirma o carregamento. O perfil 1.75 lê o AXP2101 diretamente.
- **Estágio 10 — Modos e personalização.** Alternar entre rosto, mouse Bluetooth, QR Wi-Fi e QR do painel a partir do display ou via `POST /api/mode`. O brilho dos olhos e a atividade do olhar variam de 20% a 100%. O controle do mouse BLE requer um perfil de hardware.
- **Estágio 11 — Confiabilidade e segurança.** O firmware verifica as origens da solicitação HTTP, protege os comandos com um token de solicitação, limita a taxa das solicitações e recupera o ponto de acesso Wi-Fi. O perfil 1.75 também requer login de proprietário. [Notas de segurança do estágio 11](STAGE11-SECURITY.md)
- **Estágio 12 — Construções e diagnósticos.** GitHub Actions verifica o código e os pacotes compilados com somas de verificação. `GET /api/hardware` relata resultados de inicialização de software. A saída do display, a resposta ao toque e o emparelhamento podem ser testados na placa física. [Construir e lançar](STAGE12-RELEASE.md) · [Testes de hardware](STAGE12-HARDWARE-TESTS.md) · [Diagnóstico de hardware](STAGE2-HARDWARE-DIAGNOSTICS.md)

O ponto de entrada do firmware é `src/main.cpp`. Os cabeçalhos residem em `include/` e a fonte do painel de controle é `web/stach-panel.html`. Após editar o painel, atualize `preview/stach-panel.html` para corresponder e execute `python tools/build_web_header.py`. Execute `python tools/check_panel.py` para verificar a página incorporada e o contrato de API (requer Node.js). No Linux, execute `sh tools/test_core.sh` para os testes principais portáteis. Execute `python tools/test_ble_patch.py` para verificar a compatibilidade do patch BLE.

STACH é um projeto independente inspirado no gênero Pip-KeyPet. PlatformIO instala bibliotecas de terceiros e o projeto aplica um patch de compatibilidade a ESP32 BLE Mouse 0.3.1. [Recursos do Waveshare 1.75](https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.75) · [Referência do Waveshare 1.43](https://www.waveshare.com/wiki/ESP32-S3-Touch-AMOLED-1.43)
