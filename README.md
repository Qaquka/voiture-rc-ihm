# IHM Voiture RC (ESP32 + ESP32-CAM)

Interface web **HTML/CSS/JavaScript vanilla** pour piloter une voiture RC via **HTTP**.

## Fonctionnalités

- Pilotage voiture (avant/arrière/gauche/droite/stop) en HTTP.
- Flux caméra MJPEG ESP32-CAM.
- Vues:
  - **Contrôles + Caméra**
  - **Caméra seule** (mobile / plein écran compatible iOS Safari)
  - **Aide** (guide rapide + dépannage)
- Télémétrie (optionnelle): vitesse + batterie par polling HTTP.
- Contrôle clavier (flèches, ZQSD, espace) + boutons tactiles.

## Structure du repo

- `index.html` : interface principale complète (styles + scripts inline).
- `eps32/` : firmware ESP32 (commande moteur / endpoints HTTP).
- `Esp32_cam/` : firmware ESP32-CAM (stream vidéo).
- `iut_univ_blanc.png` : logo affiché dans l’en-tête.

## Endpoints attendus

### Commandes voiture

- `GET /cmd?c=F` : forward
- `GET /cmd?c=B` : backward
- `GET /cmd?c=L` : left
- `GET /cmd?c=R` : right
- `GET /cmd?c=S` : stop

### Télémétrie (optionnelle)

- `GET /telemetry` → JSON (ex: `{ "speed": 12.3, "battery": 76 }`)
- fallback:
  - `GET /speed`
  - `GET /battery`

## Utilisation

1. Connecter le téléphone/PC au réseau de l’ESP32 (AP ou LAN).
2. Ouvrir `index.html` dans un navigateur.
3. Dans l’interface:
   - renseigner l’URL du flux MJPEG (ex: `http://192.168.4.1:81/stream`)
   - renseigner la base HTTP voiture (ex: `http://192.168.4.1`)
4. Cliquer **Test HTTP** puis piloter.

## Paramètres URL utiles

Exemples de query params supportés:

- `?view=full|cam|help`
- `?stream=http://.../stream`
- `?http=http://...`
- `?logo=...`
- `?team=...`
- `?demo=1`

## Contraintes projet

- Communication **HTTP uniquement**.
- Pas de framework front.
- Compatible mobile/desktop + Safari iOS (fullscreen inclus).
- Code léger pour environnement embarqué.


## Configuration Wi-Fi (recommandée)

Pour éviter de versionner des identifiants en clair:

1. Copier:
   - `eps32/include/secrets.h.example` → `eps32/include/secrets.h`
   - `Esp32_cam/include/secrets.h.example` → `Esp32_cam/include/secrets.h`
2. Remplir `WIFI_SSID` et `WIFI_PASSWORD`.
3. Les fichiers `secrets.h` sont ignorés par Git via `.gitignore`.

## Développement local

Lancer un serveur statique depuis la racine:

```bash
python3 -m http.server 4173 --bind 0.0.0.0
```

Puis ouvrir: `http://127.0.0.1:4173/index.html`

