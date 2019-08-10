from skimage.io import imread
import numpy as np
import sys
from os.path import join

fname_w_ext = sys.argv[1]
fname, ext = tuple(fname_w_ext.split('.'))
im = imread(fname_w_ext)
assert im.shape == (48, 128), im.dtype == np.uint8

im = im // 255
imb = np.zeros((8, 128), dtype=np.uint8)
shifts = (np.arange(6) ** 2).reshape(6, 1)

for i in range(8):
	band = im[i*6:(i+1)*6]
	imb[i] = (band * shifts).sum(0)

with open(join('client', '%s_bitwise.bin' % fname), 'wb') as f:
	f.write(imb.tobytes())
