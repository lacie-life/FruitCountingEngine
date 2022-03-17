from datasetLoader import TrainDataset, TestDataset, inverse_normalize

import torch.nn as nn
import torch.nn.functional as F
import torch
import torch.optim as optim
from torch.utils import data as data_

from config import opt


class Net(nn.Module):
    def __init__(self):
        super().__init__()
        self.conv1 = nn.Conv2d(3, 6, 5)
        self.pool = nn.MaxPool2d(2, 2)
        self.conv2 = nn.Conv2d(6, 16, 5)
        self.fc1 = nn.Linear(16 * 5 * 5, 120)
        self.fc2 = nn.Linear(120, 84)
        self.fc3 = nn.Linear(84, 10)

    def forward(self, x):
        x = self.pool(F.relu(self.conv1(x)))
        x = self.pool(F.relu(self.conv2(x)))
        x = torch.flatten(x, 1)  # flatten all dimensions except batch
        x = F.relu(self.fc1(x))
        x = F.relu(self.fc2(x))
        x = self.fc3(x)
        return x


if __name__ == "__main__":

    train_data = TrainDataset(opt)
    print('load train data')
    train_dataloader = data_.DataLoader(train_data, \
                                  batch_size=16, \
                                  shuffle=True, \
                                  # pin_memory=True,
                                  num_workers=opt.num_workers)

    test_data = TestDataset(opt)
    print('load test data')
    test_dataloader = data_.DataLoader(test_data,
                                       batch_size=16,
                                       num_workers=opt.test_num_workers,
                                       shuffle=False, \
                                       pin_memory=True
                                       )

    net = Net()
    criterion = nn.CrossEntropyLoss()
    optimizer = optim.SGD(net.parameters(), lr=0.001, momentum=0.9)

    device = torch.device('cuda:0' if torch.cuda.is_available() else 'cpu')

    net.to(device)
    net.cuda()

    for epoch in range(10):  # loop over the dataset multiple times

        running_loss = 0.0
        for i, data in enumerate(train_dataloader, 0):
            # get the inputs; data is a list of [inputs, labels]
            inputs, labels = data[0].to(device), data[1].to(device)

            # zero the parameter gradients
            optimizer.zero_grad()

            # forward + backward + optimize
            outputs = net(inputs)
            loss = criterion(outputs, labels)
            loss.backward()
            optimizer.step()

            # print statistics
            running_loss += loss.item()
            if i % 2000 == 1999:  # print every 2000 mini-batches
                print(f'[{epoch + 1}, {i + 1:5d}] loss: {running_loss / 2000:.3f}')
                running_loss = 0.0

    print('Finished Training')




