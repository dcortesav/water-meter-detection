# Sistema de Lectura Automática de Contadores de Agua (IoT + Visión Artificial)

Este proyecto implementa un sistema prototipo para la digitalización de lecturas de contadores de agua mecánicos analógicos. Utiliza un dispositivo IoT (ESP32-CAM) para la captura de imágenes en campo y un sistema de procesamiento basado en Inteligencia Artificial (YOLOv11) para extraer los dígitos y exportarlos a un formato estructurado (CSV/Excel).

## 📋 Tabla de Contenidos

1.  [Descripción del Flujo de Trabajo](#-descripción-del-flujo-de-trabajo)
2.  [Estructura del Proyecto](#-estructura-del-proyecto)
3.  [Requisitos de Hardware y Software](#-requisitos)
4.  [Instalación y Configuración](#️-instalación-y-configuración)
5.  [Instrucciones de Uso](#-instrucciones-de-uso)
6.  [Solución de Problemas Comunes](#-solución-de-problemas-comunes)

-----

## 🔄 Descripción del Flujo de Trabajo

El sistema opera en una modalidad de **Registro y Post-procesamiento (Batch Processing)**. El ciclo de vida del dato es el siguiente:

1.  **Captura (Edge):** La **ESP32-CAM** se despierta automáticamente. Inicializa el sensor OV2640 y ajusta parámetros de exposición y balance de blancos.
2.  **Transmisión:** La imagen capturada se transmite a un servidor por medio de una red WiFi que es compartida por el servidor y por la ESP32-CAM.
3.  **Preprocesamiento (Backend):** El script de Python toma la imagen, aplica recortes y filtros (escala de grises) para facilitar la lectura.
4.  **Inferencia (IA):** La imagen procesada pasa por el modelo **YOLOv11** previamente entrenado. El modelo detecta las cajas delimitadoras de los números y sus clases (dígitos 0-9).
5.  **Lógica de Negocio:** El script ordena los dígitos detectados de izquierda a derecha (según su coordenada X) para reconstruir la cifra completa del contador.
6.  **Persistencia:** El resultado final (Nombre de archivo + Lectura numérica) se escribe en una nueva fila del archivo `medidas_contador.csv`.

-----

## 📂 Estructura del Proyecto

El repositorio funciona como un *Monorepo*, conteniendo tanto el firmware como el software de análisis:

```text
PROYECTO_CONTADOR/
│
├── client_esp32/              # Firmware C++ para el dispositivo IoT
│   ├── src/main.cpp           # Código principal de captura y transmisión
│   ├── platformio.ini         # Configuración de compilación y hardware
│   └── ...
│
├── backend_python/            # Software de procesamiento e IA
│   ├── notebooks/             # Contiene el notebook con el que se entrenaron los modelos YOLO en google Colab
│   ├── src/
│   │   ├── preprocessing.py   # Funciones de preprocesamiento de imagen
│   │   └── main.py            # Script que genera el servidor y realiza el proceso de inferencia y registro del dato medido
│   ├── trained_models/        # Modelos YOLO (.pt)
│   ├── requirements.txt       # Dependencias de Python
│   └── medidas_contador.csv   # [Salida] Archivo Excel/CSV generado
│
└── README.md                  # Documentación del proyecto
```

-----

## 🛠 Requisitos

### Hardware

  * **ESP32-CAM:** Modelo AI-Thinker (con sensor OV2640).
  * **Fuente de Alimentación:** Cargador USB de 5V/2A (Conexión directa a pines 5V/GND recomendada para estabilidad), o bateria.
  * **Conversor FTDI o Base MB:** Solo necesario para cargar el código la primera vez.

### Software

  * **VS Code:** Editor de código principal.
  * **PlatformIO (Extensión VS Code):** Para compilar y subir código a la ESP32.
  * **Python 3.10+:** Para correr el script de análisis.

-----

## ⚙️ Instalación y Configuración

### 1\. Configurar el Firmware (ESP32)

1.  Abre la carpeta `client_esp32` con VS Code.
2.  Asegúrate de tener instalada la extensión **PlatformIO**.
3.  Conecta la ESP32 al PC.
4.  Haz clic en el botón de **Upload (Flecha Derecha ➡️)** en la barra inferior de PlatformIO.
5.  Una vez cargado, desconecta la ESP32 del PC.

### 2\. Configurar el Entorno Python

1.  Abre una terminal en la carpeta `backend_python`.
2.  Crea un entorno virtual (recomendado):
    ```bash
    python -m venv .venv
    ```
3.  Activa el entorno:
      * Windows: `.venv\Scripts\activate`
      * Mac/Linux: `source .venv/bin/activate`
4.  Instala las librerías necesarias:
    ```bash
    pip install -r requirements.txt
    ```

-----

## 🚀 Instrucciones de Uso

### Paso 1: Recolección de Datos (En Campo)

1.  Conecta la ESP32 a una fuente de energía (Batería o Cargador USB).
2.  Espera. El LED rojo trasero parpadeará brevemente cada vez que tome y guarde una foto (por defecto cada 15 segundos).
3.  Apunta la cámara al contador. Asegúrate de que la imagen esté enfocada y bien iluminada.

### Paso 2: Procesamiento de Datos (En Ordenador)

1. Recibir la primera imágen enviada por la ESP32-CAM luego de que esta haya sido instalada en campo y ajustar los parámetros del archivo ubicado en la ruta backend_python/src/preprocessing.main, de tal forma que en la imágen preprocesada solo se observe la parte del contador que da la medida.
2. En VS Code, abre la terminal dentro de `backend_python` y ejecuta:
    ```bash
    python src/main.py
    ```
3. El script creará un servidor que estará atento a la comunicación de la ESP32-CAM. Cuando el servidor detecte la llegada de un dato, ejecutará el script de inferencia del modelo de IA. Al finalizar, abre el archivo `medidas_contador.csv` para ver las lecturas digitalizadas.

-----

## ❓ Solución de Problemas Comunes

| Problema | Causa Probable | Solución |
| :--- | :--- | :--- |
| **Fotos con colores raros (verde/rosa)** | Fallo de alimentación o sensor saturado. | Asegurar alimentación robusta de 5V. El código actual incluye un "flush" (limpieza) previo para mitigar esto. |
| **El modelo no detecta números** | Iluminación pobre o reflejos. | Mejorar la iluminación externa sobre el contador o reentrenar el modelo con imágenes de la nueva ubicación. |