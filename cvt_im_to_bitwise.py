from skimage.io import imread, imsave
import numpy as np
import sys

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

imsave('%s_bitwise.%s' % (fname, ext), imb)
