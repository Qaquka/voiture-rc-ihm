# 🚗 Voiture RC – IHM Web (ESP32)

Interface Web permettant de piloter une **voiture RC en Wi-Fi** à l’aide de modules **ESP32**, avec affichage temps réel des informations de conduite et **retour vidéo via une ESP32-CAM**.

Le projet est utilisable :
- sur **PC** (clavier / manette)
- sur **smartphone** (vue caméra seule, manette)

---

## 📌 Objectifs du projet

- Piloter une voiture RC à distance via le **Wi-Fi (2,4 GHz)**
- Développer une **IHM Web responsive** (PC & mobile)
- Assurer une communication fiable entre l’IHM et les ESP32
- Intégrer un **flux vidéo en temps réel**
- Centraliser commandes et retours d’état (vitesse, batterie, latence)

---

## 🧩 Architecture du système

```

Navigateur Web (PC / Smartphone)
│
│ HTTP / WebSocket
▼
ESP32 principal (commande moteur)
│
│ Signaux moteurs / capteurs
▼
Voiture RC
│
└── ESP32-CAM → Flux vidéo MJPEG

````

---

## 🖥️ Interface Web (IHM)

L’IHM est développée en **HTML / CSS / JavaScript** et hébergée via **GitHub Pages**.

### Fonctionnalités principales
- 🎮 Commandes de déplacement :
  - Clavier (ZQSD / flèches)
  - Manette (Gamepad API)
- 📷 Affichage du flux caméra
- ⚡ Affichage dynamique :
  - Vitesse
  - Niveau de batterie
  - Ping / latence
- 📱 Mode mobile :
  - Vue caméra seule
  - Interface adaptée à la manette
- 🌙 Thème sombre optimisé

---

## 🔧 Matériel utilisé

- ESP32 DevKit (contrôle moteur)
- ESP32-CAM (AI Thinker – OV2640)
- Châssis voiture RC
- Pont en H / driver moteur
- Batterie embarquée
- PC ou smartphone avec navigateur moderne

---

## 🧠 Technologies utilisées

### Embarqué
- ESP32
- Wi-Fi 2,4 GHz
- Serveur HTTP

### Caméra
- ESP32-CAM
- Streaming MJPEG

### Front-end
- HTML5
- CSS3
- JavaScript
- WebSocket / HTTP
- Gamepad API

### Outils
- Visual Studio Code
- Arduino IDE / PlatformIO
- GitHub Pages

---

## 🚀 Installation

### 1️⃣ Cloner le dépôt
```bash
git clone https://github.com/Qaquka/voiture-rc-ihm.git
````

---

### 2️⃣ Héberger l’IHM avec GitHub Pages

1. Aller dans **Settings**
2. Onglet **Pages**
3. Sélectionner la branche principale
4. Déployer le site

---

### 3️⃣ Configuration ESP32

Dans le code ESP32 :

* Modifier le **SSID** et le **mot de passe Wi-Fi**
* Lire l’adresse IP via le moniteur série
* Renseigner cette IP dans l’IHM si nécessaire

---

### 4️⃣ Configuration ESP32-CAM

* Flasher l’ESP32-CAM avec un sketch de streaming MJPEG
* Noter l’URL du flux vidéo
  Exemple :

  ```
  http://IP_CAM:81/stream
  ```
* Intégrer l’URL dans l’IHM

---

## ▶️ Utilisation

1. Connecter le PC ou le smartphone au même réseau Wi-Fi que les ESP32
2. Ouvrir l’IHM dans le navigateur
3. Allumer la voiture RC
4. Piloter via clavier ou manette
5. Visualiser le flux caméra en temps réel

---

## 🧪 Tests réalisés

* Test de latence Wi-Fi
* Compatibilité navigateurs PC / mobile
* Stabilité du flux vidéo
* Réactivité des commandes
* Gestion des pertes de connexion

---

## ⚠️ Limitations connues

* Le flux vidéo dépend fortement de la qualité du Wi-Fi
* Le streaming caméra fonctionne en **HTTP** (limitation ESP32)
* Performances variables selon la charge réseau

---

## 🛣️ Améliorations prévues

* Enregistrement vidéo et captures via carte SD
* Mode point d’accès Wi-Fi dédié
* Historique des données (vitesse, batterie)
* Amélioration de la sécurité réseau
* Assistance à la conduite

---

## 👨‍🎓 Contexte pédagogique

Projet réalisé dans le cadre du **BUT GEII – Parcours AII (Automatisme & Informatique Industrielle)**.

Compétences mobilisées :

* Systèmes embarqués
* Réseaux
* Développement web
* Communication temps réel
* Automatisme

---

## 📄 Licence

Projet open-source à usage pédagogique et personnel.

---

## ✨ Auteur

**Axel, Cyprien et Thomas**
Étudiant BUT GEII – AII
Projet Voiture RC – ESP32

