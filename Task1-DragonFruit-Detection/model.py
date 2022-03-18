import torch.onnx
import onnx
import torchvision
from torchvision.models.detection.faster_rcnn import FastRCNNPredictor
from albumentations.pytorch.transforms import ToTensor
import cv2
import numpy as np
from config import ONNX_FILE_PATH


def create_model(num_classes):
    # load Faster RCNN pre-trained model
    model = torchvision.models.detection.fasterrcnn_resnet50_fpn(pretrained=True)

    # get the number of input features
    in_features = model.roi_heads.box_predictor.cls_score.in_features
    # define a new head for the detector with required number of classes
    model.roi_heads.box_predictor = FastRCNNPredictor(in_features, num_classes)
    return model


def convert_to_onnx(model_path, img_path):

    image = cv2.imread(img_path)
    image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB).astype(np.float32)

    # make the pixel range between 0 and 1
    image /= 255.0
    # bring color channels to front
    image = np.transpose(image, (2, 0, 1)).astype(float)
    # convert to tensor
    image = torch.tensor(image, dtype=torch.float)
    # add batch dimension
    image = torch.unsqueeze(image, 0)

    device = torch.device('cuda') if torch.cuda.is_available() else torch.device('cpu')

    model = create_model(num_classes=5).to('cuda')

    model.load_state_dict(torch.load(model_path, map_location='cuda'))
    model.eval()
    model.to('cuda')

    image = image.to('cuda')

    inputs = torch.randn(1, 3, 300, 400, device="cuda")
    inputs = inputs.to('cuda')

    with torch.no_grad():
        outputs = model(image)

    torch.onnx.export(
        model, inputs,
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

