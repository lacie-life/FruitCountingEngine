import cv2
import onnx
import torch
import numpy as np
from albumentations import (Compose, Resize,)
from albumentations.augmentations.transforms import Normalize
from albumentations.pytorch.transforms import ToTensor
from torchvision import models

from config import DETECTION_THRESHOLD, CLASSES
from model import create_model, convert_to_onnx

import tensorrt as trt
import pycuda.driver as cuda
import pycuda.autoinit

# logger to capture errors, warnings, and other information during the build and inference phases
TRT_LOGGER = trt.Logger()

def preprocess_image(img_path):

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

    return image


def postprocess(output_data, img_path):
    orig_image = cv2.imread(img_path)
    image_name = img_path.split('/')[-1].split('.')[0]
    outputs = [{k: v.to('cpu') for k, v in t.items()} for t in output_data]

    if len(outputs[0]['boxes'] != 0):
        boxes = outputs[0]['boxes'].data.numpy()
        scores = outputs[0]['scores'].data.numpy()
        # filter out boxes according to `detection_threshold`
        boxes = boxes[scores >= DETECTION_THRESHOLD].astype(np.int32)
        draw_boxes = boxes.copy()
        # get all the predicited class names
        pred_classes = [CLASSES[i] for i in outputs[0]['labels'].cpu().numpy()]

        # draw the bounding boxes and write the class name on top of it
        for j, box in enumerate(draw_boxes):
            cv2.rectangle(orig_image,
                          (int(box[0]), int(box[1])),
                          (int(box[2]), int(box[3])),
                          (0, 0, 255), 2)
            cv2.putText(orig_image, pred_classes[j],
                        (int(box[0]), int(box[1] - 5)),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0),
                        2, lineType=cv2.LINE_AA)
        cv2.imshow('Prediction', orig_image)
        cv2.waitKey(1)
        cv2.imwrite(f"test_predictions/{image_name}.jpg", orig_image, )


def build_engine(onnx_file_path):
    # initialize TensorRT engine and parse ONNX model
    builder = trt.Builder(TRT_LOGGER)
    network = builder.create_network()
    parser = trt.OnnxParser(network, TRT_LOGGER)

    # allow TensorRT to use up to 1GB of GPU memory for tactic selection
    builder.max_workspace_size = 1 << 30
    # we have only one image in batch
    builder.max_batch_size = 1
    # use FP16 mode if possible
    if builder.platform_has_fast_fp16:
        builder.fp16_mode = True

    # parse ONNX
    with open(onnx_file_path, 'rb') as model:
        print('Beginning ONNX file parsing')
        parser.parse(model.read())
    print('Completed parsing of ONNX file')

    # generate TensorRT engine optimized for the target platform
    print('Building an engine...')
    engine = builder.build_cuda_engine(network)
    context = engine.create_execution_context()
    print("Completed creating Engine")
    return engine, context


def main():
    img_path = "data/test/images/fruit104.png"
    input = preprocess_image(img_path)

    device = torch.device('cuda') if torch.cuda.is_available() else torch.device('cpu')

    model = create_model(num_classes=5)

    model.load_state_dict(torch.load("outputs/model100.pth", map_location=device))
    model.eval()

    output = model(input)

    postprocess(output, img_path)

    onnx_path = convert_to_onnx(model, input)

    # initialize TensorRT engine and parse ONNX model
    engine, context = build_engine(onnx_path)
    # get sizes of input and output and allocate memory required for input data and for output data
    for binding in engine:
        if engine.binding_is_input(binding):  # we expect only one input
            input_shape = engine.get_binding_shape(binding)
            input_size = trt.volume(input_shape) * engine.max_batch_size * np.dtype(np.float32).itemsize  # in bytes
            device_input = cuda.mem_alloc(input_size)
        else:  # and one output
            output_shape = engine.get_binding_shape(binding)
            # create page-locked memory buffers (i.e. won't be swapped to disk)
            host_output = cuda.pagelocked_empty(trt.volume(output_shape) * engine.max_batch_size, dtype=np.float32)
            device_output = cuda.mem_alloc(host_output.nbytes)

    # Create a stream in which to copy inputs/outputs and run inference.
    stream = cuda.Stream()

    # preprocess input data
    host_input = np.array(preprocess_image("turkish_coffee.jpg").numpy(), dtype=np.float32, order='C')
    cuda.memcpy_htod_async(device_input, host_input, stream)

    # run inference
    context.execute_async(bindings=[int(device_input), int(device_output)], stream_handle=stream.handle)
    cuda.memcpy_dtoh_async(host_output, device_output, stream)
    stream.synchronize()

    # postprocess results
    output_data = torch.Tensor(host_output).reshape(engine.max_batch_size, output_shape[0])
    postprocess(output_data)

if __name__ == '__main__':
    main()










