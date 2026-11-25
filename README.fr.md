# STACH

[Русский](README.md) · [English](README.en.md) · [简体中文](README.zh-CN.md) · [繁體中文](README.zh-TW.md) · [Español](README.es.md) · [हिन्दी](README.hi.md) · [العربية](README.ar.md) · **Français** · [বাংলা](README.bn.md) · [Português](README.pt.md) · [Bahasa Indonesia](README.id.md) · [اردو](README.ur.md) · [Deutsch](README.de.md) · [日本語](README.ja.md) · [한국어](README.ko.md) · [Türkçe](README.tr.md) · [Tiếng Việt](README.vi.md) · [Italiano](README.it.md) · [فارسی](README.fa.md) · [Polski](README.pl.md) · [Українська](README.uk.md)

STACH transforme un **Waveshare ESP32-S3-Touch-AMOLED-1.75** en un compagnon de poche. Ses yeux suivent votre doigt et réagissent à l'inclinaison. Faites glisser votre doigt vers la droite pour jouer à des jeux ou utilisez votre téléphone pour modifier son expression, sa luminosité, son comportement et son nom d'affichage. La carte héberge son propre réseau Wi-Fi et son propre panneau de commande.

![STACH eye animation](preview/stach-smart-eyes.gif)

## Installer sur un Waveshare 1.75

Ces instructions ont été testées sous Windows avec l'environnement `stach-eyes-175` PlatformIO. Il construit l'application complète de `src/main.cpp` pour l'écran rond 466 × 466 CO5300, 16 Mo de flash et 8 Mo de PSRAM. Vérifiez le modèle sur la carte ou son emballage avant de flasher. Les révisions 1.43, 1.43B et 1.75C ont un matériel différent.

### 1. Préparez votre ordinateur

Vous avez besoin de Python 3.13 avec le lanceur `py`, Git pour Windows, un câble de données USB et une connexion Internet pour la première version. [Python pour Windows](https://www.python.org/downloads/windows/) · [Git pour Windows](https://git-scm.com/downloads/win)

Ouvrez PowerShell et clonez le projet dans un nouveau dossier. Un chemin court avec des caractères ASCII fonctionne bien avec les outils ESP32.

```powershell
cd $env:USERPROFILE
git clone https://github.com/leonidgg2323-code/stach.git stach-firmware
cd stach-firmware
py -3.13 -m venv .venv
.\.venv\Scripts\python.exe -m pip install "platformio==6.2.0"
```

Vous pouvez également choisir Code **→ Télécharger ZIP** sur GitHub et extraire l'archive. Ouvrez PowerShell dans le dossier contenant `platformio.ini` et exécutez les deux dernières commandes ci-dessus. Les commandes ci-dessous appellent directement le Python de l’environnement virtuel.

### 2. Installez les dépendances et construisez

```powershell
.\.venv\Scripts\python.exe -m platformio pkg install -e stach-eyes-175
.\.venv\Scripts\python.exe -m platformio run -e stach-eyes-175
```

La première version télécharge la chaîne d'outils ESP32 et peut prendre plusieurs minutes. La version applique automatiquement le correctif de compatibilité de la bibliothèque de souris Bluetooth. Attendez `stach-eyes-175  SUCCESS` avant de télécharger.

### 3. Trouvez le port série et flashez la carte

Connectez la carte par USB et listez les ports disponibles.

```powershell
.\.venv\Scripts\python.exe -m platformio device list
```

Identifiez le port qui apparaît lorsque vous branchez la carte et disparaît lorsque vous la débranchez. Remplacez `COM3` dans les exemples par votre port. Fermez tout programme qui l'utilise, y compris le moniteur série Arduino IDE.

Le téléchargement **remplace le firmware de la carte.** Pour conserver la possibilité de restaurer le menu d'usine, enregistrez une sauvegarde flash complète en dehors du référentiel avant de télécharger. Une sauvegarde complète pour ce modèle est **16,777,216 bytes**. Utilisez un nouveau nom de fichier si vous disposez déjà d’une sauvegarde. L'image peut contenir des paramètres et des informations d'identification, alors gardez-la privée.

```powershell
.\.venv\Scripts\python.exe -m pip install "esptool>=5,<6"
.\.venv\Scripts\python.exe -m esptool --chip esp32s3 --port COM3 read-flash 0 ALL "$env:USERPROFILE\stach-before-install.flash-backup.bin"
```

Pour installer STACH, exécutez la commande upload.

```powershell
.\.venv\Scripts\python.exe -m platformio run -e stach-eyes-175 -t upload --upload-port COM3
```

La carte redémarre après la vérification de la somme de contrôle et le message `SUCCESS`. PlatformIO écrit le chargeur de démarrage, la table de partition et l'application à leurs adresses respectives. Le fichier `firmware.bin` dans le répertoire build contient uniquement l'application.

### 4. Enregistrez le compte propriétaire

1. Au premier démarrage, scannez **1/2 JOIN WI-FI** avec votre téléphone. Rejoignez le réseau `Stach-XXXXXX`. Si votre téléphone signale qu'il n'a pas d'accès à Internet, choisissez **Rester connecté**.
2. L'affichage passe à **2/2 REGISTER** lorsque le téléphone se connecte. Si le premier code est toujours affiché, appuyez une fois sur l'écran. Scannez le deuxième code avec le même téléphone.
3. Choisissez un nom d'utilisateur de 3 à 32 lettres (A à Z ou a à z), des chiffres, des traits d'union ou des traits de soulignement et un mot de passe d'au moins 12 caractères. Enregistrez vos informations de connexion. Le compte et les paramètres sont stockés sur le tableau.
4. Dans **Аккаунт владельца** (compte propriétaire), vous pouvez modifier le nom affiché dans le panneau ou vous déconnecter. Pour des visites ultérieures, rejoignez le Wi-Fi du tableau et ouvrez le [panneau de commande local STACH](http://192.168.4.1/).

Le conseil accepte un seul compte propriétaire. L'inscription nécessite le code à usage unique affiché sur l'écran physique. Les commandes de contrôle nécessitent une session propriétaire et un jeton de demande. Le panneau utilise HTTP sur le réseau Wi-Fi local de la carte. Connectez-vous à nouveau après le redémarrage du forum.

## Gestes et jeux

| Action | Résultat |
|---|---|
| Touchez les yeux ou inclinez la planche | Les yeux suivent votre doigt ou l'inclinaison. |
| Glissez vers la droite depuis les yeux | Ouvrez le menu des jeux. |
| Balayez vers la gauche dans un jeu | Revenez au menu. Balayez à nouveau vers la gauche pour revenir aux yeux. |
| Tenez l'écran oculaire pendant environ une seconde | Affichez le code QR Wi-Fi. Appuyez pour basculer entre les codes réseau et ceux du panneau. Tenez pour revenir aux yeux. |
| Glissez vers le haut depuis les yeux | Démarrez le mode souris Bluetooth. Associez-le au **Stach Mouse** sur votre ordinateur et maintenez la carte immobile pour l'étalonnage. |
| Touchez l'écran en mode souris | Appuyez pour cliquer avec le bouton gauche, appuyez deux fois pour cliquer avec le bouton droit, faites glisser pour faire défiler ou maintenez enfoncé pour quitter. |

Dans **Targets**, touchez autant de cibles que possible en 20 secondes. Dans **Sums**, répondez à cinq questions supplémentaires. Les deux jeux affichent votre score et enregistrent votre record personnel. **Dice** lance un dé à six faces et **Coin** lance une pièce de monnaie. Chaque résultat provient du générateur de nombres aléatoires matériel ESP32. Appuyez à nouveau une fois l'animation terminée pour un nouveau lancer ou retournement.

## Affichage et puissance

Le panneau du téléphone comprend la sélection d'expressions, un joystick de regard, des commandes de clignement et d'inclinaison, la luminosité des yeux et un paramètre permettant aux yeux de bouger seuls. Les paramètres d’apparence persistent lors des redémarrages. Vous pouvez activer le mode souris après le démarrage.

Sur la carte 1,75, le **AXP2101** indique le pourcentage de batterie et l'état de charge. L'indicateur affiche `USB` lorsque la carte fonctionne sur alimentation par câble et ne détecte aucune batterie. Lorsque l'économie d'énergie est activée, le taux de rafraîchissement des yeux diminue après 45 secondes d'inactivité et l'écran s'éteint après 5 minutes. À une charge signalée de 15 % ou moins, ces intervalles tombent à 10 secondes et 2 minutes. Touchez l'écran pour le réveiller. Les jeux, les codes QR, le mode souris et la charge active maintiennent l'écran éveillé.

## Dépannage et récupération

| Symptôme | Quoi essayer |
|---|---|
| Aucun nouveau port COM n'apparaît | Essayez un câble de données USB et un autre port USB, puis exécutez à nouveau `platformio device list`. |
| Le port ne peut pas être ouvert ou signale `Access denied` | Fermez le moniteur série. Reconnectez la carte et vérifiez son numéro COM. |
| La construction échoue | Vérifiez que votre dossier actuel contient `platformio.ini`, puis répétez les deux commandes à l'étape 2. |
| L'écran reste vide | Vérifiez le modèle de carte et le nom de l'environnement dans le journal de téléchargement. Pour la carte 1.75, la ligne de réussite devrait inclure `stach-eyes-175`. |
| Le téléphone ne peut pas ouvrir le panneau | Vérifiez qu'il est toujours sur `Stach-XXXXXX` et ouvrez `http://192.168.4.1/`. La première inscription nécessite le deuxième code QR. |

Pour afficher les diagnostics, ouvrez le moniteur série à 115 200 bauds.

```powershell
.\.venv\Scripts\python.exe -m platformio device monitor -p COM3 -b 115200
```

Envoyez `I` pour un rapport d'initialisation du matériel, `S` pour répertorier les appareils I²C ou `D` pour un test d'affichage de cinq secondes. Appuyez sur **Ctrl+C** pour fermer le moniteur.

Pour restaurer votre sauvegarde complète sur la même carte, fermez le moniteur et vérifiez d'abord la taille de l'image. La commande suivante restaure également les paramètres et les données de compte contenus dans la sauvegarde.

```powershell
.\.venv\Scripts\python.exe -m esptool --chip esp32s3 --port COM3 write-flash 0x0 "$env:USERPROFILE\stach-before-install.flash-backup.bin"
```

**Besoin d'aide?** Contactez `twiinkprostolox123` sur Discord ou [@def0kk (Telegram)](https://t.me/def0kk). Incluez le modèle de carte, l'environnement PlatformIO et le message d'erreur. Gardez votre mot de passe, votre code QR d'enregistrement et votre sauvegarde flash complète privés.

## Historique et développement du projet

- **Étape 9 — Puissance.** Les anciens profils 1.43 mesurent et filtrent la tension ADC sur GPIO4. L'estimation de la charge nécessite de vérifier le schéma de la révision spécifique et de l'étalonner par rapport à une tension de batterie mesurée. L'alimentation USB à elle seule ne confirme pas la charge. Le profil 1,75 lit directement l'AXP2101.
- **Stage 10 — Modes et personnalisation.** Basculez entre le visage, la souris Bluetooth, le Wi-Fi QR et le panneau QR depuis l'écran ou via `POST /api/mode`. La luminosité des yeux et l'activité du regard varient de 20 % à 100 %. Le contrôle de la souris BLE nécessite un profil matériel.
- **Étape 11 — Fiabilité et sécurité.** Le micrologiciel vérifie l'origine des requêtes HTTP, protège les commandes avec un jeton de requête, limite le débit des requêtes et récupère le point d'accès Wi-Fi. Le profil 1.75 nécessite également une connexion propriétaire. [Notes de sécurité de l'étape 11](STAGE11-SECURITY.md)
- **Étape 12 — Constructions et diagnostics.** GitHub Actions vérifie le code et les packages construits avec des sommes de contrôle. `GET /api/hardware` rapporte les résultats de l'initialisation du logiciel. La sortie d'affichage, la réponse tactile et l'appairage peuvent être testés sur la carte physique. [Construire et publier](STAGE12-RELEASE.md) · [Tests matériels](STAGE12-HARDWARE-TESTS.md) · [Diagnostic matériel](STAGE2-HARDWARE-DIAGNOSTICS.md)

Le point d'entrée du micrologiciel est `src/main.cpp`. Les en-têtes résident dans `include/` et la source du panneau de commande est `web/stach-panel.html`. Après avoir modifié le panneau, mettez à jour `preview/stach-panel.html` pour qu'il corresponde, puis exécutez `python tools/build_web_header.py`. Exécutez `python tools/check_panel.py` pour vérifier la page intégrée et le contrat API (nécessite Node.js). Sous Linux, exécutez `sh tools/test_core.sh` pour les tests de base portables. Exécutez `python tools/test_ble_patch.py` pour vérifier la compatibilité des correctifs BLE.

STACH est un projet indépendant inspiré du genre Pip-KeyPet. PlatformIO installe des bibliothèques tierces et le projet applique un correctif de compatibilité à ESP32 BLE Mouse 0.3.1. [Ressources Waveshare 1.75](https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.75) · [Référence Waveshare 1.43](https://www.waveshare.com/wiki/ESP32-S3-Touch-AMOLED-1.43)
