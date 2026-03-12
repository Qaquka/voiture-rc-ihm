# Voiture RC ESP32 + ESP32-CAM (BUT3 GEII)

![PlatformIO](https://img.shields.io/badge/PlatformIO-ESP32-orange)
![ESP32](https://img.shields.io/badge/ESP32-IoT-blue)
![Project](https://img.shields.io/badge/BUT3-GEII-green)

## Présentation du projet

Ce dépôt contient une interface de pilotage pour une voiture RC réalisée dans le cadre d’un projet BUT3 GEII.

Le système repose sur :

- un **ESP32** pour la commande de la voiture (moteur + direction) via HTTP ;
- un **ESP32-CAM** pour le flux vidéo MJPEG ;
- une **IHM web** (HTML/CSS/JavaScript) pour piloter et visualiser la caméra.

## Architecture

```text
Navigateur Web
      │
      │ HTTP
      ▼
    ESP32
 (commande voiture)
      │
      ├── moteur DC
      └── servomoteur

ESP32-CAM
      │
      ▼
  flux MJPEG
      │
      ▼
interface web
```

## Fonctionnalités

- Pilotage de la voiture depuis navigateur web.
- Envoi des commandes en HTTP vers l’ESP32.
- Gestion de la direction (servo) et de la traction (moteur DC).
- Affichage du flux caméra MJPEG dans l’interface.
- Contrôles clavier + boutons tactiles.

## Matériel nécessaire

- 1 × ESP32 (commande voiture)
- 1 × ESP32-CAM (vidéo)
- 1 × servomoteur (direction)
- 1 × moteur DC (avance/recul)
- 1 × driver moteur adapté
- Alimentation adaptée à la voiture et aux cartes
- Câble USB/UART pour le flash
- PC avec VS Code + PlatformIO (ou PlatformIO CLI)

## Structure du projet

```text
.
├── index.html
├── README.md
├── esp32/
│   ├── platformio.ini
│   └── src/main.cpp
└── Esp32_cam/
    ├── platformio.ini
    └── src/main.cpp
```

Fichiers principaux :

- `index.html` : interface web de pilotage.
- `esp32/src/main.cpp` : firmware de commande HTTP.
- `Esp32_cam/src/main.cpp` : firmware caméra MJPEG.

## Installation

1. Cloner le dépôt :

```bash
git clone <url-du-repo>
cd <nom-du-repo>
```

2. Installer PlatformIO (extension VS Code ou CLI).

## Flash des cartes ESP32

### 1) ESP32 commande

- Ouvrir le projet firmware ESP32 dans PlatformIO.
- Vérifier/adapter le port série dans `platformio.ini`.
- Compiler puis téléverser.

### 2) ESP32-CAM

- Ouvrir le projet firmware ESP32-CAM dans PlatformIO.
- Vérifier/adapter le port série dans `platformio.ini`.
- Compiler puis téléverser.

## Configuration WiFi

Les identifiants WiFi sont définis **directement dans le code** :

- `esp32/src/main.cpp`
- `Esp32_cam/src/main.cpp`

Modifier les constantes SSID/mot de passe dans ces fichiers avant flash.

## Lancement de l’interface web

- Ouvrir directement `index.html` dans un navigateur,
- ou lancer un serveur statique local (recommandé) :

```bash
python3 -m http.server 4173 --bind 0.0.0.0
```

Puis ouvrir :

```text
http://127.0.0.1:4173/index.html
```

## Configuration des IP dans l’IHM

Dans l’interface web :

- URL caméra MJPEG : `http://<ip-esp32-cam>/stream`
- Base HTTP commande : `http://<ip-esp32>`

Exemple :

- Caméra : `http://192.168.4.20/stream`
- Commande : `http://192.168.4.10`

## Commandes HTTP

Les commandes utilisées par l’ESP32 sont :

- `/cmd?c=A` → avancer
- `/cmd?c=R` → reculer
- `/cmd?c=G` → gauche
- `/cmd?c=D` → droite
- `/cmd?c=S` → stop moteur
- `/cmd?c=C` → centre direction
- `/cmd?c=0` à `/cmd?c=3` → niveau de vitesse

Niveaux de vitesse :

- `0` = 25 %
- `1` = 50 %
- `2` = 75 %
- `3` = 100 %

## Contrôles clavier

- `↑` : avancer
- `↓` : reculer
- `←` : gauche
- `→` : droite
- `Espace` : stop

(L’interface peut aussi mapper ZQSD selon le navigateur/appareil.)

## Flux caméra

Le flux vidéo est un MJPEG servi par l’ESP32-CAM :

- `GET /stream`

L’interface web lit ce flux et l’affiche en direct.

## URLs des firmwares

### ESP32 (commande)

- `GET /` (si implémenté dans le firmware)
- `GET /cmd?c=<commande>`

### ESP32-CAM

- `GET /stream`
- `GET /` (page locale caméra, selon firmware)
