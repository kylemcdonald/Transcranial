import argparse
import matplotlib.pyplot as plt
import scipy.fftpack as sfft
from scipy.io import wavfile
from scipy.signal import blackman
import numpy as np
import os

parser = argparse.ArgumentParser(
  description='Convert audio from a folder to frequency domain feature vectors.')
parser.add_argument('-i', '--input', default='wav')
parser.add_argument('-m', '--maxfreq', type=int, default=500)
parser.add_argument('-c', '--chunksize', type=int, default=2**14)
args = parser.parse_args()

n = args.chunksize
print('{0} chunk size'.format(n))
window = blackman(n)
vectors = []
labels = []
for f in os.listdir(args.input):
	path = os.path.join(args.input, f)
	sampleRate, data = wavfile.read(path)
	total = len(data)
	chunks = [data[i:i+n] for i in range(0, total-n, n)]
	label = f.split('.')[0]
	for i, chunk in enumerate(chunks):
		freq = sfft.fft(chunk * window)
		bins = len(freq)/2
		usable = args.maxfreq * n / sampleRate
		if usable == 0:
			usable = bins
		amp = np.absolute(freq[:usable])
		vectors.append(amp)
		labels.append(label)
		print('{0}: {1}/{2} chunks {3} Hz {4} bins {5} Hz/bin {6} usable'.format(
			f, i+1, len(chunks), sampleRate, bins, sampleRate/n, usable))
output = '{0}hz.{1}ch'.format(args.maxfreq, n)
if not os.path.exists(output):
	os.makedirs(output)
vectors = np.array(vectors)
print('Removing mean.')
vectors = vectors - vectors.mean(axis=0)
print('Scaling to global standard deviation.')
vectors = vectors / vectors.std()
print('Saving vectors.')
np.savetxt('{0}/vectors'.format(output), vectors, fmt='%.8f', delimiter='\t')
print('Saving labels.')
np.savetxt('{0}/labels'.format(output), labels, fmt='%s')