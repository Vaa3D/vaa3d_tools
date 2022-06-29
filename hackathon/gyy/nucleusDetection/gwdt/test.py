import tifffile
import numpy as np

from scipy import misc
from PIL import Image

a = np.load('files/17_rgb.npy')
tifffile.imwrite('C:/Users/admin/Desktop/data/test/17_rgb.tif', a)
# im=Image.fromarray(a)
# tif = TIFF.open("rgb.tif",mode='w')
# tif.write_image(im,compression=None)
# tif.close()

