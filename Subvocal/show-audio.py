# to get the latest pyprocessing:
# pip install --allow-unverified pyprocessing pyprocessing==0.1.3.22

import matplotlib.pyplot as plt

import scipy.fftpack as sfft
from scipy.io import wavfile
from scipy.signal import blackman

import numpy as np
import math

import os, time
import pyaudio
from pyprocessing import *

N = 1024

size(1024, 512)
background(0)

recent = []
window = blackman(N)

def draw():
	background(0)

	if len(recent):
		# draw time-amplitude
		pushMatrix()
		translate(0, height/2)
		scale(width / N, height/2)
		stroke(255)
		noFill()
		beginShape()
		for x, y in enumerate(recent):
			vertex(x, y)
		endShape()
		popMatrix()

		# draw frequency-amplitude
		freq = sfft.fft(recent * window)
		nyquist = len(freq)/2
		amp = np.absolute(freq[:nyquist])
		pushMatrix()
		translate(0, height)
		scale(width, -height)
		stroke(255)
		noFill()
		beginShape()
		for x, y in enumerate(amp):
			vertex(math.log(1+x, nyquist), pow(y, .5))
		endShape()
		popMatrix()

def audioInput(in_data, frame_count, time_info, status):
	global recent
	recent = np.fromstring(in_data, 'Int32').astype(float)
	recent /= np.iinfo(np.int32).max
	return (None, pyaudio.paContinue)

audio = pyaudio.PyAudio()
format = pyaudio.paInt32
stream = audio.open(format=format, # bytes per sample
					channels=1,
					rate=48000,
					input=True,
					output=False,
					frames_per_buffer=N,
					stream_callback=audioInput)
stream.start_stream()
run()
stream.stop_stream()
stream.close()
audio.terminate()
