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

    # Image sample input
    image = cv2.imread(img_path)
    image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB).astype(np.float32)
    image_resized = cv2.resize(image, (256, 256))
    # make the pixel range between 0 and 1
    image_resized /= 255.0
    # bring color channels to front
    image_resized = np.transpose(image_resized, (2, 0, 1)).astype(float)
    # convert to tensor
    image_resized = torch.tensor(image_resized, dtype=torch.float)
    # add batch dimension
    image_resized = torch.unsqueeze(image_resized, 0)

    # Model
    device = torch.device('cuda') if torch.cuda.is_available() else torch.device('cpu')
    model = create_model(num_classes=5).to(device)
    state_dict = torch.load(model_path, map_location='cuda')
    model.load_state_dict(state_dict)
    model.eval()

    model.to(device)
    onnx_input = image_resized.cuda()

    with torch.no_grad():
        outputs = model(onnx_input)

    print(next(model.parameters()).device)
    print(onnx_input.device)

    torch.onnx.export(
        model, onnx_input,
        ONNX_FILE_PATH,
        input_names=["input"],
        output_names=["output"],
        export_params=True,
        verbose=True,
        opset_version=11)
    onnx_model = onnx.load(ONNX_FILE_PATH)
    # check that the model converted fine
    onnx.checker.check_model(onnx_model)

    print("Model was successfully converted to ONNX format.")
    print("It was saved to", ONNX_FILE_PATH)

    return ONNX_FILE_PATH

