## Setting

### SSD 

IMAGES=/home/jun/Github/Master-Thesis/Task1-DragonFruit-Detection/data/test/Images

./ssd-detect --model=/home/jun/Github/Master-Thesis/Task1-DragonFruit-Detection/model/ssd-mobilenet.onnx  --labels=/home/jun/Github/Master-Thesis/Task1-DragonFruit-Detection/model/labels.txt --input-blob=input_0 --output-cvg=scores --output-bbox=boxes "$IMAGES/fruit*.png" $IMAGES/test/fruit_%i.png

### Yolov5

```
cd yolov5
mkdir build & cd build
cmake ..
make

# Convert onnx to tersorRT engine
./build_engine --model ONNX_MODEL --output OUTPUT_ENGINE


```