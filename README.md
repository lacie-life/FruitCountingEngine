# Master-Thesis

## Name: Dragon fruit yield estimation system using UAV

## Motivation: Building a camera module mounted on a UAV that collects data on images of dragon fruit and gives an estimate of dragon fruit production

## Hardware

- [x] Jetson AGX Xavier (32GB)
- [x] ZED 2 camera
- [x] Drone  

## Software

- [x] Jetpack 4.6 (rev.3)
- [x] ZED SDK (ZED SDK 3.6.5 - have some problems with version 3.7)

## Tasks

- [x] Model dragon fruit detection (SSD-Mobilenet-v2/Yolov5)
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
    - [x] Fixing crash (finding bug) 
  - [ ] Bounding Box Tracker
    - [ ] ZED 2 custom box tracker (checking)
    - [ ] SSD-Tracker (pending)
  - [ ] Counting Fruit (trying bounding box optical flow and SfM)
  - [ ] Improve Tracking and Counting Algorithm
 
### Phase 2
- [ ] Box for Mynt and Quad Camera
- [ ] Implement with Quad Camera, MyntEye and RTK
- [ ] 3D Reconstruction (GTSAM SfM and PMVS2) => Have some problem with PMVS 2 and libjpeg of OpenCV (LOL) => GreenHouseAR


