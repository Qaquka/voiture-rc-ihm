# Configuration Wi-Fi firmware (ESP32 / ESP32-CAM)

Pour éviter de versionner des identifiants en clair:

1. Copier:
   - `eps32/include/secrets.h.example` → `eps32/include/secrets.h`
   - `Esp32_cam/include/secrets.h.example` → `Esp32_cam/include/secrets.h`
2. Remplir `WIFI_SSID` et `WIFI_PASSWORD`.
3. Les fichiers `secrets.h` sont ignorés par Git via `.gitignore`.

Les firmwares utilisent automatiquement `secrets.h` si présent (fallback sur des valeurs par défaut sinon).
