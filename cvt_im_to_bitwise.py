from skimage.io import imread, imsave
from skimage.color import rgb2gray
import numpy as np
import sys
from os.path import join

fname_w_ext = sys.argv[1]
fname, ext = tuple(fname_w_ext.split('.'))
im = imread(fname_w_ext)

if im.dtype != np.uint8:
	im = (255*im).astype(np.uint8)

if len(im.shape) == 3 and im.shape[2] == 3:
	im = (255*rgb2gray(im)).astype(np.uint8)

im[im > 0] = 255

imsave(fname_w_ext, im)

#assert im.shape == (48, 128), im.dtype == np.uint8
assert im.shape == (64, 128), im.dtype == np.uint8

im = im // 255
imb = np.zeros((8, 128), dtype=np.uint8)
#shifts = (np.arange(6) ** 2).reshape(6, 1)
shifts = (2 ** np.arange(8)).reshape(8, 1)

for i in range(8):
	#band = im[i*6:(i+1)*6]
	band = im[i*8:(i+1)*8]
	imb[i] = (band * shifts).sum(0)

with open(join('client', '%s_bitwise.bin' % fname), 'wb') as f:
	f.write(imb.tobytes())
