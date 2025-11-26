import ultralytics
import cv2
import preprocessing
from pathlib import Path

# Preprocessing
test_data_path = Path(__file__).parent / "../test/data"
processed_data_path = Path(__file__).parent / "../test/data/processed"
processed_data_path.mkdir(exist_ok=True)

for image_path in test_data_path.iterdir():
    if image_path.is_file():
        processed_image = preprocessing.process_image(image_path=image_path)
        output_file = processed_data_path / f"processed_{image_path.name}"
        cv2.imwrite(str(output_file), processed_image)

# Model Testing
model_m_path = Path(__file__).parent / "../trained_models/local/best_m.pt"
project_path = Path(__file__).parent / "../test/results"
project_path.mkdir(exist_ok=True)

local_m_model = ultralytics.YOLO(model_m_path)
local_m_model(processed_data_path,project=project_path,name="local_m_model",save=True)