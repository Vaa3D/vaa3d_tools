import torch.nn as nn
import torch
import torch.nn.functional as F
import numpy as np
import random

seed = 42
torch.manual_seed(seed)
torch.cuda.manual_seed(seed)
torch.cuda.manual_seed_all(seed)
np.random.seed(seed)
random.seed(seed)
torch.backends.cudnn.deterministic = True

class AlexNet(nn.Module):
    def __init__(self, num_classes=11, init_weights=False):
        super(AlexNet, self).__init__()
        self.features = nn.Sequential(
            nn.Conv3d(1, 48, kernel_size=11, stride=4, padding=2),  #48,31,31,31
            nn.LeakyReLU(inplace=True),                             #48,31,31,31
            nn.MaxPool3d(kernel_size=3, stride=2),                  #48,15,15,15
            nn.Conv3d(48, 128, kernel_size=5, padding=2),           #128,15,15,15
            nn.LeakyReLU(inplace=True),                             #128,15,15,15
            nn.MaxPool3d(kernel_size=3, stride=2),                  #128,7,7,7
            nn.Conv3d(128, 192, kernel_size=3, padding=1),          #192,7,7,7
            nn.LeakyReLU(inplace=True),                             #192,7,7,7
            nn.Conv3d(192, 192, kernel_size=3, padding=1),          #192,7,7,7
            nn.LeakyReLU(inplace=True),                             #192,7,7,7
            nn.Conv3d(192, 128, kernel_size=3, padding=1),          #128,7,7,7
            nn.LeakyReLU(inplace=True),                             #128,7,7,7
            nn.MaxPool3d(kernel_size=3, stride=2),                  #128,3,3,3
        )
        self.classifier = nn.Sequential(
            nn.Dropout(p=0.5),                                      #3456
            nn.Linear(128 * 3 * 3 * 3, 4096),                       #4096
            nn.LeakyReLU(inplace=True),                             #4096
            nn.Dropout(p=0.5),                                      #4096
            nn.Linear(4096, 4096),                                  #4096
            nn.LeakyReLU(inplace=True),                             #4096
            nn.Linear(4096, num_classes),                           #10
        )
        if init_weights:
            self._initialize_weights()

    def forward(self, x):
        x = self.features(x)
        x = torch.flatten(x, start_dim=1)                           #8,3456
        x = self.classifier(x)

        x=nn.Sigmoid()(x)
        return x

    def _initialize_weights(self):
        for m in self.modules():
            if isinstance(m, nn.Conv2d):
                nn.init.kaiming_normal_(m.weight, mode='fan_out', nonlinearity='relu')
                if m.bias is not None:
                    nn.init.constant_(m.bias, 0)
            elif isinstance(m, nn.Linear):
                nn.init.normal_(m.weight, 0, 0.01)
                nn.init.constant_(m.bias, 0)
