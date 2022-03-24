```
!mkdir images/train images/val images/test annotations/train annotations/val annotations/test
```

```
!python train.py --img 300 --cfg yolov5s.yaml --hyp hyp.scratch.yaml --batch 32 --epochs 100 --data road_sign_data.yaml --weights yolov5s.pt --workers 24 --name dragon-fruit
```

```
python detect.py --source ../Road_Sign_Dataset/images/test/ --weights runs/train/yolo_road_det/weights/best.pt --conf 0.25 --name yolo_road_det
```

```
python path/to/export.py --weights yolov5s.pt --include torchscript onnx
```


