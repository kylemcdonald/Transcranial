import pyprocessing as p5
import pyaudio

import matplotlib.pyplot as plt

from sklearn import lda
from sklearn.svm import SVC

import scipy.fftpack as sfft
from scipy.io import wavfile
from scipy.signal import blackman

import numpy as np
import math
from collections import defaultdict

from OSC import OSCServer, OSCClient, OSCMessage

import os

from time import time

N = 1024

p5.size(1024, 512)

client = OSCClient()
client.connect(('127.0.0.1', 9999))

recent = []
window = blackman(N)
vectors = []
labels = []

def getAmplitude(data):
	freq = sfft.fft(data * window)
	nyquist = len(freq)/2
	return np.absolute(freq[:nyquist])

clf = SVC(probability=True, kernel = 'rbf')
model = lda.LDA(n_components=2)
projection = []
p_min = None
p_max = None
predicted = None

def buildLDA(vectors, labels):
	global model, projection, p_min, p_max
	if not len(vectors):
		print('No data to learn from.')
		return
	t0 = time()
	X = np.array(map(getAmplitude, vectors))
	y = np.array(labels)
	X.flat[::X.shape[1] + 1] += 0.01  # Make X invertible
	projection = model.fit_transform(X, y)
	clf.fit(X,y)

	p_min, p_max = np.min(projection, 0), np.max(projection, 0)
	projection = (projection - p_min) / (p_max - p_min)
	tdelta = time() - t0
	print('Projected {} vectors in {} seconds.'.format(len(vectors), tdelta))

def send_osc_message(name, *args):
	msg = OSCMessage(name)
	for arg in args:
		msg.append(arg)
	try:
		client.send(msg, 0)
	except Exception, e:
		pass
#	msg.clear
	return 

def draw():
	p5.colorMode(p5.RGB)
	p5.background(0)

	if len(projection):
		p5.pushMatrix()
		p5.colorMode(p5.HSB)
		p5.translate(width/4, height/4)
		p5.scale(width/2, height/2)
		for point, label in zip(projection, labels):
			p5.stroke(p5.color(label * 26., 255, 255))
			p5.point(point[0], point[1])			

		p5.popMatrix()
        #send osc to MaxPatch
		probability_lda = model.predict_proba([getAmplitude(recent)])
		send_osc_message("/lda",probability_lda)

		probability_svc = clf.predict_proba([getAmplitude(recent)])
		send_osc_message("/svm",probability_svc)

		cur = model.transform([getAmplitude(recent)])
		cur = cur[0]
		cur = (cur - p_min) / (p_max - p_min)
		global predicted
		if predicted == None:
			predicted = cur
		else:
			predicted = predicted * .9 + cur * .1
		p5.stroke(p5.color(0, 0, 255))
		p5.ellipse(width/4 + predicted[0] * width/2, height/4 + predicted[1] * height/2, 10, 10)

	elif len(recent):
		# draw time-amplitude
		p5.pushMatrix()
		p5.translate(0, height/2)
		p5.scale(width / N, height/2)
		p5.stroke(255)
		p5.noFill()
		p5.beginShape()
		for x, y in enumerate(recent):
			p5.vertex(x, y)
		p5.endShape()
		p5.popMatrix()

		# draw frequency-amplitude
		amp = getAmplitude(recent)
		p5.pushMatrix()
		p5.translate(0, height)
		p5.scale(width, -height)
		p5.stroke(255)
		p5.noFill()
		p5.beginShape()
		for x, y in enumerate(amp):
			p5.vertex(math.log(1+x, len(amp)), pow(y, .5))
		p5.endShape()
		p5.popMatrix()

def keyPressed():
	if p5.key.char == ' ':
		buildLDA(vectors, labels)

def audioInput(in_data, frame_count, time_info, status):
	global recent
	recent = np.fromstring(in_data, 'Int32').astype(float)
	recent /= np.iinfo(np.int32).max
	if p5.key.pressed and p5.key.char >= '1' and p5.key.char <= '9':
		global vectors
		global labels
		vectors.append(recent)
		labels.append(int(p5.key.char) - int('1'))
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
p5.run()
stream.stop_stream()
stream.close()
audio.terminate()
