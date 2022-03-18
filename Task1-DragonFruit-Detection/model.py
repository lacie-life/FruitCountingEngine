import torch.onnx
import onnx
import torchvision
from torchvision.models.detection.faster_rcnn import FastRCNNPredictor
from config import ONNX_FILE_PATH


def create_model(num_classes):
    # load Faster RCNN pre-trained model
    model = torchvision.models.detection.fasterrcnn_resnet50_fpn(pretrained=True)

    # get the number of input features
    in_features = model.roi_heads.box_predictor.cls_score.in_features
    # define a new head for the detector with required number of classes
    model.roi_heads.box_predictor = FastRCNNPredictor(in_features, num_classes)
    return model


def convert_to_onnx(model, input):

    torch.onnx.export(
        model, input,
        ONNX_FILE_PATH,
        input_names=["input"], output_names=["output"],
        export_params=True,
        opset_version=11)
    onnx_model = onnx.load(ONNX_FILE_PATH)
    # check that the model converted fine
    onnx.checker.check_model(onnx_model)

    print("Model was successfully converted to ONNX format.")
    print("It was saved to", ONNX_FILE_PATH)

    return ONNX_FILE_PATH

