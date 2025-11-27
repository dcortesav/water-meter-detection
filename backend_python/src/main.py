import preprocessing
import pandas as pd
import uvicorn
import cv2
import shutil
from fastapi import FastAPI, Request, File, UploadFile
from ultralytics import YOLO
from pathlib import Path
from datetime import datetime

# App initialization
app = FastAPI()

# Route configuration with pathlib
BASE_DIR = Path(__file__).parent
MODEL_PATH = (BASE_DIR / "../trained_models/local/best_m.pt").resolve()
CAPTURED_DIR = (BASE_DIR / "../captured_images").resolve()
CAPTURED_DIR.mkdir(parents=True, exist_ok=True)
CSV_FILE = Path(__file__).parent / "../medidas_contador.csv"

# Load Model
model = YOLO(MODEL_PATH) 

# Image processing and Inference
def process_image_yolo(img_path:Path):
    #processed_image = preprocessing.process_image(img_path)

    #results = model(processed_image, conf=0.4, project=str(CAPTURED_DIR / "YOLO"),save=True)
    results = model(img_path, conf=0.4, project=str(CAPTURED_DIR / "YOLO"),save=True)
    detected = []
    for r in results:
        boxes = r.boxes
        for box in boxes:
            cls = int(box.cls[0])
            conf = float(box.conf[0])
            x1, y1, x2, y2 = box.xyxy[0].tolist()

            print(f"--> Detectado: {cls} | Posicion_x: {x1} | Confianza: {conf:.2f} ")
            detected.append({"numero":cls, "x_pos":x1, "confianza":conf})
    
    if not detected:
        return "Error: No se detectaron numeros"
    
    detected_ordered = sorted(detected, key=lambda k: k["x_pos"])
    final_reading = "".join(str(d["numero"]) for d in detected_ordered)
    return final_reading

# Update CSV
def save_reading(reading):
    timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    df = pd.DataFrame([["1", timestamp, reading, len(reading)]], columns=["ID","Fecha", "Lectura", "# Digitos"])
    header = not CSV_FILE.exists()
    df.to_csv(CSV_FILE, mode='a', header=header, index=False)

#ESP32 workflow
@app.post("/upload")
async def upload_from_esp32(request: Request):
    data = await request.body()

    if not data or len(data)==0:
        return {"error":"No data received"}
    print(f"Recibidos {len(data)} bytes")

    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    filename = CAPTURED_DIR / f"img_{timestamp}_esp32.jpg"
    filename.write_bytes(data)
    print(f"[ESP32] Imagen guardada en: {filename.name}")

    # Model inference
    try:
        reading = process_image_yolo(filename)
        save_reading(reading)
        print(f"Lectura: {reading}")
    except Exception as e:
        print("Error: {e}")
        reading = "Error"

    return {"status": "ok", "lectura": reading, "origen": "ESP32"}

@app.post("/test-web")
async def upload_from_web(file:UploadFile=File(...)):
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    filename = CAPTURED_DIR / f"img_{timestamp}_web.jpg"

    with open(filename ,"wb") as buffer:
        shutil.copyfileobj(file.file, buffer)
    
    print(f"[WEB] Imagen guardada en: {filename.name}")

    # Model inference
    try:
        reading = process_image_yolo(filename)
        save_reading(reading)
        print(f"Lectura: {reading}")
    except Exception as e:
        print("Error: {e}")
        reading = "Error"
    
    return {"status": "ok", "lectura": reading, "origen": "WEB TEST"}

if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=8000)