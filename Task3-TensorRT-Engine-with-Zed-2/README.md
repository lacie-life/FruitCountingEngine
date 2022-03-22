## Setting


IMAGES=/home/jun/Github/Master-Thesis/Task1-DragonFruit-Detection/data/test/Images

./ssd-detect --model=/home/jun/Github/Master-Thesis/Task1-DragonFruit-Detection/model/ssd-mobilenet.onnx  --labels=/home/jun/Github/Master-Thesis/Task1-DragonFruit-Detection/model/labels.txt --input-blob=input_0 --output-cvg=scores --output-bbox=boxes "$IMAGES/fruit*.png" $IMAGES/test/fruit_%i.png