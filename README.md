# Automatic Water Meter Reading System (IoT + Computer Vision)

This project implements a prototype system for digitizing readings from analog mechanical water meters. It uses an IoT device (ESP32-CAM) to capture images in the field and an Artificial Intelligence-based processing pipeline (YOLOv11) to extract digits and export them to a structured format (CSV/Excel).

## 📋 Table of Contents

1.  [Workflow Description](#-workflow-description)
2.  [Project Structure](#-project-structure)
3.  [Hardware and Software Requirements](#-requirements)
4.  [Installation and Configuration](#️-installation-and-configuration)
5.  [Usage Instructions](#-usage-instructions)
6.  [Common Troubleshooting](#-common-troubleshooting)

-----

## 🔄 Workflow Description

The system operates in a **Logging and Post-processing (Batch Processing)** mode. The data lifecycle is as follows:

1.  **Capture (Edge):** The **ESP32-CAM** wakes up automatically. It initializes the OV2640 sensor and adjusts exposure and white balance parameters.
2.  **Transmission:** The captured image is sent to a server through a Wi-Fi network shared by both the server and the ESP32-CAM.
3.  **Preprocessing (Backend):** The Python script takes the image and applies crops and filters (grayscale) to make reading easier.
4.  **Inference (AI):** The processed image is passed through a previously trained **YOLOv11** model. The model detects bounding boxes for numbers and their classes (digits 0-9).
5.  **Business Logic:** The script sorts detected digits from left to right (based on X coordinate) to reconstruct the full meter value.
6.  **Persistence:** The final result (File name + Numeric reading) is written to a new row in `medidas_contador.csv`.

-----

## 📂 Project Structure

The repository works as a *Monorepo*, containing both firmware and analysis software:

```text
PROYECTO_CONTADOR/
│
├── client_esp32/              # C++ firmware for the IoT device
│   ├── src/main.cpp           # Main code for capture and transmission
│   ├── platformio.ini         # Build and hardware configuration
│   └── ...
│
├── backend_python/            # Processing and AI software
│   ├── notebooks/             # Notebook used to train YOLO models in Google Colab
│   ├── src/
│   │   ├── preprocessing.py   # Image preprocessing functions
│   │   └── main.py            # Script that starts the server and runs inference and logging for measured data
│   ├── trained_models/        # YOLO models (.pt)
│   ├── requirements.txt       # Python dependencies
│   └── measurements.csv   # [Output] Generated Excel/CSV file
│
└── README.md                  # Project documentation
```

-----

## 🛠 Requirements

### Hardware

  * **ESP32-CAM:** AI-Thinker model (with OV2640 sensor).
  * **Power Supply:** 5V/2A USB charger (direct connection to 5V/GND pins is recommended for stability), or battery.
  * **FTDI Converter or MB Base:** Only needed to upload code the first time.

### Software

  * **VS Code:** Main code editor.
  * **PlatformIO (VS Code Extension):** To compile and upload code to the ESP32.
  * **Python 3.10+:** To run the analysis script.

-----

## ⚙️ Installation and Configuration

### 1\. Configure the Firmware (ESP32)

1.  Open the `client_esp32` folder in VS Code.
2.  Make sure the **PlatformIO** extension is installed.
3.  Connect the ESP32 to your PC.
4.  Click the **Upload (Right Arrow ➡️)** button in the bottom PlatformIO bar.
5.  Once uploaded, disconnect the ESP32 from your PC.

### 2\. Configure the Python Environment

1.  Open a terminal in the `backend_python` folder.
2.  Create a virtual environment (recommended):
    ```bash
    python -m venv .venv
    ```
3.  Activate the environment:
      * Windows: `.venv\Scripts\activate`
      * Mac/Linux: `source .venv/bin/activate`
4.  Install required libraries:
    ```bash
    pip install -r requirements.txt
    ```

-----

## 🚀 Usage Instructions

### Step 1: Data Collection (Field)

1.  Connect the ESP32 to a power source (Battery or USB Charger).
2.  Wait. The rear red LED will briefly blink every time it captures and saves a photo.
3.  Point the camera at the meter. Make sure the image is focused and well lit.

### Step 2: Data Processing (Computer)

1. Receive the first image sent by the ESP32-CAM after it has been installed in the field, then adjust the parameters in the file at `backend_python/src/preprocessing.main` so that only the meter measurement area is visible in the preprocessed image.
2. In VS Code, open a terminal inside `backend_python` and run:
    ```bash
    python src/main.py
    ```
3. The script will create a server that listens for ESP32-CAM communication. When the server detects incoming data, it runs the AI model inference script. At the end, open `medidas_contador.csv` to see the digitized readings.

-----

## ❓ Common Troubleshooting

| Problem | Likely Cause | Solution |
| :--- | :--- | :--- |
| **Photos with strange colors (green/pink)** | Power failure or saturated sensor. | Ensure a stable 5V power supply. The current code includes a prior "flush" cleanup to mitigate this. |
| **Model does not detect numbers** | Poor lighting or reflections. | Improve external lighting on the meter or retrain the model with images from the new location. |