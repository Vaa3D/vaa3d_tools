import myFunction
import gwdtFunction
import tifffile
import math
from concurrent.futures import ThreadPoolExecutor, ProcessPoolExecutor
import os

'''
dir_path = 'E:/programData_Yanyan/77_Thread1'
save_path = 'F:/QualityControlProject/Data/Ex_488_Em_525/results'

sigma = 2
K_size = 2 * math.ceil(sigma) + 1
bkg_thresh = 2000

rowsDir = os.listdir(dir_path)
countAll = 0
for a in rowsDir:
    colsPath = dir_path + '/' + a
    colsDir = os.listdir(colsPath)
    if not os.path.isfile(colsPath):
        count = 0
        for b in colsDir:
            imagePath = colsPath + '/' + b
            saveName = save_path + '/' + b + '.marker'
            if os.path.isfile(imagePath) and imagePath.endswith('.tif'):
                count += 1
        countAll += count
        print(count)
print(countAll)
'''

dir_path = 'E:/PavelData2D'
save_path = 'E:/Pavel_DownSampled'
myFunction.dirDownSampled(dir_path, save_path)