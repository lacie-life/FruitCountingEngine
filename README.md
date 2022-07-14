# Fruit Counting Engine

## Motivation: 

Building a camera module mounted on a UAV that collects data on images of dragon fruit and gives an estimate of dragon fruit production

## Hardware

- [x] Jetson AGX Xavier (32GB)
- [x] ZED 2 camera
- [x] Drone  
- [ ] RTK

## Software

- [x] Jetpack 4.6 (rev.3)
- [x] ZED SDK (ZED SDK 3.6.5 - have some problems with version 3.7)

## Tasks

- [ ] Model dragon fruit detection (SSD-Mobilenet-v2/Yolov5)
  - [x] Train test
  - [x] Dataset (Find more)
  - [x] Evaluate model
  - [ ] Improve model
- [ ] Record data 
  - [x] Print 3D box 
  - [x] Record with Zed 2, T265 and RTK (Phase 1)
  - [ ] Record with Quad Camera, MyntEye and RTK (Phase 2)

### Phase 1
- [ ] Implement model in Jetson AGX and ZED 2  
  - [x] Run TenserRT Engine in ZED 2 Video stream 
    - [x] Build and Run
    - [x] Fixing crash (fixed bug => ZED SDK 3.6.5) 
  - [x] Bounding Box Tracker
    - [x] MO Tracker (testing) 
    - [ ] MO Tracker with VPI (Pending)
  - [x] Counting Object 
    - [x] MO Tracker counting 
    - [ ] Improve counting algorithm (trying bounding box optical flow and SfM)
  - [ ] Improve Tracking and Counting Algorithm
  - [ ] Test with dragon fruit model and data
  - [ ] Simple GUI
    - [x] Control start/stop camera
    - [ ] Counting control

- [ ] Assign 3D position 
  - [ ] Visual Odometry run-time

- [ ] Simple Qt GUI 
  - [ ] ROS bag record
  - [ ] 3D Reconstruction (GTSAM SfM and PMVS2)
 
### Phase 2
- [ ] Box for Mynt and Quad Camera
- [ ] Implement with Quad Camera, MyntEye and RTK
- [ ] 3D Reconstruction (GTSAM SfM and PMVS2) => Have some problem with PMVS 2 and libjpeg of OpenCV (LOL) => GreenHouseAR

## Usage

### 1. Train model

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

### 2. Convert model to TensorRT Engine

```
cd TensorRT-Engine/yolov5

# Copy your trained model to here

mkdir build & cd build
cmake ../yolov5
make

# build engine
./build_engine --model YOLOV5_MODEL.onnx --output yolov5.engine
```

### 3. Using GUI

Updating

## Project report

[Overleaf](https://www.overleaf.com/read/nymnjngppwvc)

## Abstract

- Using TensorRT for optimi model
- Real data collection
- Accuracy

