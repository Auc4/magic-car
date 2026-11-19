# WILS 2026 - ESP32 Wireless Car Firmware

Firmware desarrollado para el microcontrolador **ESP32**, diseñado para el control y teleoperación inalámbrica de un vehículo robótico en el marco de la **Competencia Nacional Wireless Car (WILS 2026)**.

## Descripción Corta
Este proyecto gestiona la comunicación en red local (a través de un router/AP) para recibir comandos de movimiento desde una estación de control en la PC, procesarlos de forma segura (con control de IDs y temporizadores de seguridad/watchdog) y accionar un puente H para el movimiento de motores de tracción.

## 🛠️ Características Principales
* Conexión Wi-Fi en modo estación (STA) conectada a un AP/Router central.
* Recepción de comandos de control mediante protocolos de red (UDP/MQTT).
* Lógica de seguridad contra pérdida de comunicación (Watchdog por temporizador).
* Prevención de inversión de giro brusca para proteger el hardware del motor.

## 📄 Licencia
Este proyecto está bajo la licencia **GNU General Public License v3.0 (GPL v3)**. Consulta el archivo `LICENSE` para más detalles.