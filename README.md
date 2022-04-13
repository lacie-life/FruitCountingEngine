# Master-Thesis

## Name: 

Dragon fruit yield estimation system using UAV

## Motivation: 

Building a camera module mounted on a UAV that collects data on images of dragon fruit and gives an estimate of dragon fruit production

## Hardware

- [x] Jetson AGX Xavier (32GB)
- [x] ZED 2 camera
- [x] Drone  

## Software

- [x] Jetpack 4.6 (rev.3)
- [x] ZED SDK (ZED SDK 3.6.5 - have some problems with version 3.7)

## Tasks

- [ ] Model dragon fruit detection (SSD-Mobilenet-v2/Yolov5)
  - [x] Train test
  - [ ] Dataset (Finding)
  - [ ] Evaluate model
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
    - [ ] ZED 2 custom box tracker (pending)
    - [ ] SSD-Tracker (pending)
    - [ ] VPI BBTracker (Testing)
    - [x] LK Tracker (accuracy too low => check LK tracker) 
    - [x] MO TRacker (testing) 
  - [x] Counting Fruit 
    - [x] MO Tracker counting 
    - [ ] LK Tracker counting
    - [ ] Improve counting algorithm (trying bounding box optical flow and SfM)
  - [ ] Improve Tracking and Counting Algorithm
  - [ ] Test with dragon fruit model and data
- [ ] Assign 3D position 
 
### Phase 2
- [ ] Box for Mynt and Quad Camera
- [ ] Implement with Quad Camera, MyntEye and RTK
- [ ] 3D Reconstruction (GTSAM SfM and PMVS2) => Have some problem with PMVS 2 and libjpeg of OpenCV (LOL) => GreenHouseAR


