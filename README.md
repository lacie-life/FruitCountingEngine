# Master-Thesis

## Name: Dragon fruit yield estimation system using UAV

## Motivation: Building a camera module mounted on a UAV that collects data on images of dragon fruit and gives an estimate of dragon fruit production

## Hardware

- [x] Jetson AGX Xavier (32GB)
- [x] ZED 2 camera
- [x] Drone  

## Software

- [x] Jetpack 4.6 (rev.3)
- [x] ZED SDK

## Tasks

- [x] Model dragon fruit detection (SSD-Mobilenet-v2)
- [ ] Record data 
  - [ ] Print 3D box 
  - [ ] Record with Zed 2, T265 and RTK (Phase 1)
  - [ ] Record with Quad Camera, MyntEye and RTK (Phase 2)

### Phase 1
- [ ] Implement model in Jetson AGX and ZED 2  
  - [ ] Run TenserRT Engine in ZED 2 Video stream => Return Bounding Boxes
  - [ ] Tracking Bounding Boxes use VPI KLT Bounding Box Tracker
  - [ ] Counting Fruit (trying bounding box optical flow and SfM)
  - [ ] Improve Tracking and Counting Algorithm
 
### Phase 2
- [ ] Box for Mynt and Quad Camera
- [ ] Implement with Quad Camera, MyntEye and RTK
- [ ] 3D Reconstruction (GTSAM SfM and PMVS2) => Have some problem with PMVS 2 and libjpeg of OpenCV (LOL) => GreenHouseAR


