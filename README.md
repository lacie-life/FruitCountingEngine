# Fruit Counting Engine

## Motivation: 

Building a camera module mounted on a UAV that collects data on images of dragon fruit and gives an estimate of dragon fruit production

## Hardware

- [x] Jetson AGX Xavier (32GB)
- [x] ZED 2 camera
- [x] Drone  
- [x] RTK

## Software

- [x] Jetpack 4.6 (rev.3)
- [x] ZED SDK (ZED SDK 3.6.5 - have some problems with version 3.7)

## Tasks

- [x] Model dragon fruit detection (SSD-Mobilenet-v2/Yolov5)
  - [x] Train test
  - [x] Dataset
  - [x] Evaluate model

### Phase 1
- [x] Implement model in Jetson AGX and ZED 2  
  - [x] Run TenserRT Engine in ZED 2 Video stream 
    - [x] Build and Run
    - [x] Fixing crash (fixed bug => ZED SDK 3.6.5) 
  - [x] Bounding Box Tracker
    - [x] MO Tracker (testing) 
  - [x] Counting Algorithm
  - [x] Test with dragon fruit model and data
  - [x] Simple GUI
    - [x] Control start/stop camera
    - [x] Counting control

## Usage

### 1. Install requierment

### 2. Train model

#### 2.1. YOLOv5

- Install YoLov5 requiements

```
pip install -r Model/Yolo/yolov5/requirements.txt

# Install ONNX lib
pip install onnx
```

- Use [this file](https://github.com/lacie-life/FruitCountingEngine/blob/main/Model/Yolo/yolo-demo.ipynb) for split data to train, test and vali folder with yolo data format (only support VOC data format).

- Change path of dataset and class name in [this file](https://github.com/lacie-life/FruitCountingEngine/blob/main/Model/Yolo/yolov5/data/fruit.yaml)

- Train model by using [this file](https://github.com/lacie-life/FruitCountingEngine/blob/main/Model/Yolo/yolov5/train.py)

- Export model to ONNX format by using [this file](https://github.com/lacie-life/FruitCountingEngine/blob/main/Model/Yolo/yolov5/models/export.py)

#### 2.2. SSD-MobileNet

- Install requirement

```
cd Model/ssd/
pip install -r requirements.txt
```

- Using [this file](https://github.com/lacie-life/FruitCountingEngine/blob/main/Model/ssd/SSD_test.ipynb) for convert data to VOC format

- Download pre-train net
```
wget -P models https://storage.googleapis.com/models-hao/mobilenet-v1-ssd-mp-0_675.pth
```
- Train model by using [this file](https://github.com/lacie-life/FruitCountingEngine/blob/main/Model/ssd/train_ssd.py)

- Evaluation model by using [this file](https://github.com/lacie-life/FruitCountingEngine/blob/main/Model/ssd/eval_ssd.py)

- Convert to ONNX by using [this file](https://github.com/lacie-life/FruitCountingEngine/blob/main/Model/ssd/onnx_export.py)

### 3. Convert model to TensorRT Engine

```
# gennerate .wst file
cd Models/Yolo/yolov5
python gen_wts.py <path to .pt file>

cd TensorRT-Engine/yolov5/

# Copy your trained model to here

mkdir build & cd build
cmake ..
make

# Copy .wts file generated to build folder

# build engine
./yolov5_zed -s best.wts test-11.engine s
```

### 4. Using GUI

```
# Copy .engine file to Engine/Data/model
# Build GUI
cd Engine
mkdir build
cd build
cmake ..
make

# Run
./EngineGUI
```
<Updating>

## Project report

[Overleaf](https://www.overleaf.com/read/nymnjngppwvc)

## Abstract

- Using TensorRT for optimi model
- Real data collection
- Accuracy

## Ref

https://blog.paperspace.com/train-yolov5-custom-data/
  
https://github.com/jkjung-avt/tensorrt_demos
