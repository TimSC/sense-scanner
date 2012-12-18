
from PIL import Image
import time, math, pickle
import numpy as np
import sklearn.tree as tree
import sklearn.svm as svm
import sklearn.ensemble as ensemble

def BilinearSample(imgPix, x, y):
	xfrac, xi = math.modf(x)
	yfrac, yi = math.modf(y)

	#Get surrounding pixels
	p00 = imgPix[xi,yi]
	p10 = imgPix[xi+1,yi]
	p01 = imgPix[xi,yi+1]
	p11 = imgPix[xi+1,yi+1]

	#Interpolate colour
	c1 = [p00c * (1.-xfrac) + p10c * xfrac for p00c, p10c in zip(p00, p10)]
	c2 = [p01c * (1.-xfrac) + p11c * xfrac for p01c, p11c in zip(p01, p11)]
	col = [c1c * (1.-yfrac) + c2c * yfrac for c1c, c2c in zip(c1, c2)]

	return col

def GetPixIntensityAtLoc(iml, supportOffsets, loc, rotation = 0.):
	out = []
	for offset in supportOffsets:
		#Apply rotation (anti-clockwise)
		rx = math.cos(rotation) * offset[0] - math.sin(rotation) * offset[1]
		ry = math.sin(rotation) * offset[0] + math.cos(rotation) * offset[1]

		#Get pixel at this location
		try:
			out.append(iml[rx + loc[0], ry + loc[1]])
		except IndexError:
			return None
	return out

def ITUR6012(col): #ITU-R 601-2
	return 0.299*col[0] + 0.587*col[1] + 0.114*col[2]

def RandomDirectionVector(mag):
	ang = np.random.uniform(0, math.pi)
	return (math.cos(ang) * mag, math.sin(ang) * mag)

#*************************************************************

class PredictAxis:
	def __init__(self, xIn, yIn):
		mag = (xIn ** 2. + yIn ** 2.) ** 0.5
		assert mag > 0.
		self.axisX = xIn / mag
		self.axisY = yIn / mag

		self.crossX = -self.axisY
		self.crossY = self.axisX
		self.reg = None
		self.ClearTrainingData()

	def SetTrainData(self, intensitiesIn, offsetsIn, supportPixIntIn):
		assert len(intensitiesIn.shape) == 2
		assert intensitiesIn.shape[0] > 0
		self.labels = []
		for offset in offsetsIn:
			label = offset[0] * self.axisX + offset[1] * self.axisY
			self.labels.append(label)
		self.intensities = intensitiesIn
		self.supportPixInt = supportPixIntIn

	def ClearTrainingData(self):
		self.labels = None
		self.intensities = None
		self.supportPixInt = None

	def Train(self, regIn, regArgs):
		assert self.labels is not None
		self.reg = regIn(**regArgs)
		self.reg.fit(self.intensities - self.supportPixInt, self.labels)

	def Predict(self, intensities):
		assert self.reg is not None
		pass

#******************************************************

class RelativeTracker:
	def __init__(self):
		self.ims = []
		self.imls = None
		self.pointsPosLi = []
		self.progress = 0.
		self.maxSupportOffset = 20.
		self.numSupportPix = 200 #500
		self.numTrainingOffsets = 5000
		self.trainOffsetVar = 5.
		self.supportPixOffsets = None
		self.supportPixCols = None
		self.models = []
		self.numTrackers = None
		
		#settings = [{'shapeNoise': 12, 'cloudEnabled': 1, 'supportMaxOffset': 39, 'trainVarianceOffset': 41, 'reg': reg}, {'shapeNoise': 1, 'cloudEnabled': 0, 'supportMaxOffset': 20, 'trainVarianceOffset': 5, 'reg': reg}] #"Classic" 0.2 settings

		#numSupportPix = 500, numTrainingOffsets = 5000, 
		#	supportMaxOffset = 90, 
		#	trainVarianceOffset = 23, 
		#	shapeNoise = 16.,
		#	rotationVar = 0.1,

	def Init(self):
		#Get number of tracking points
		self.numTrackers = len(self.pointsPosLi[0])
		for pts in self.pointsPosLi:
			assert len(pts) == self.numTrackers

		#Generate random support pix offsets
		self.supportPixOffsets = []
		for count in range(self.numTrackers):
			self.supportPixOffsets.append(np.random.uniform(-self.maxSupportOffset, 
				self.maxSupportOffset, (self.numSupportPix, 2)))
		
		#Create pixel access objects
		if self.imls is None:
			self.imls = [im.load() for im in self.ims]

	def GenerateTrainingForTracker(self, trNum):
		
		spOffsets = self.supportPixOffsets[trNum]

		#Count number of annotated frames
		numAnnotatedFrames = 0
		for iml, framePositions in zip(self.imls, self.pointsPosLi):
			loc = framePositions[trNum]
			if loc is not None: numAnnotatedFrames += 1

		#For each annotated frame, get intensity at annotated position
		supportPixIntCollect = []
		for iml, framePositions in zip(self.imls, self.pointsPosLi):
			loc = framePositions[trNum]
			if loc is None: continue
			supportColours = GetPixIntensityAtLoc(iml, spOffsets, loc)
			if supportColours is None: continue
			supportInt = []
			for col in supportColours:
				supportInt.append(ITUR6012(col))
			supportPixIntCollect.append(supportInt)
		supportPixIntAv = np.array(supportPixIntCollect).mean(axis=0)

		#For each annotated frame, generate training data
		trainingIntensities = []
		trainingOffsets = []
		for iml, framePositions in zip(self.imls, self.pointsPosLi):
			loc = framePositions[trNum]
			if loc is None: continue

			trainingOnThisFrame = int(round(self.numTrainingOffsets / numAnnotatedFrames))
			if trainingOnThisFrame == 0: trainingOnThisFrame = 1

			for trainCount in range(trainingOnThisFrame):
				#Generate random offset
				trainOffsetsMag = np.random.randn() * self.trainOffsetVar	
				trainOffset = RandomDirectionVector(trainOffsetsMag)

				#Get intensity at training offset
				offsetLoc = (loc[0] + trainOffset[0], loc[1] + trainOffset[1])
				supportColours = GetPixIntensityAtLoc(iml, spOffsets, offsetLoc)
				if supportColours is None: continue
				supportInt = []
				for col in supportColours:
					supportInt.append(ITUR6012(col))
				#print trainOffset

				trainingIntensities.append(supportInt)
				trainingOffsets.append(trainOffset)

		trainingIntensitiesArr = np.array(trainingIntensities)
		trainingOffsetsArr = np.array(trainingOffsets)

		assert len(trainingIntensitiesArr.shape) == 2
		assert len(trainingOffsetsArr.shape) == 2

		#Create a pair of axis trackers for this data and copy training data
		axisX = PredictAxis(1.,0.)
		axisX.SetTrainData(trainingIntensitiesArr, trainingOffsetsArr, supportPixIntAv)

		axisY = PredictAxis(0.,1.)
		axisY.SetTrainData(trainingIntensitiesArr, trainingOffsetsArr, supportPixIntAv)

		return (axisX, axisY)

	def Train(self):
		self.models = []
		for i in range(self.numTrackers):
			print i, self.numTrackers
			model = self.GenerateTrainingForTracker(i)

			regArgs = {'n_estimators':20, 'n_jobs':-1, 'compute_importances': True}
			reg = ensemble.RandomForestRegressor
			model[0].Train(reg, regArgs)
			model[1].Train(reg, regArgs)
		
			model[0].ClearTrainingData()
			model[1].ClearTrainingData()
		self.models.append(model)

	def AddTrainingData(self, im, pointsPos):
		self.imls = None
		self.ims.append(im)
		self.pointsPosLi.append(pointsPos)
	
	def ClearTrainingImages(self):
		self.ims = []
		self.imls = None

	def Update(self):
		self.progress += 0.01
		time.sleep(0.1)
		if len(self.models) == 0:
			self.Init()
			self.Train()
			self.ClearTrainingImages()
			pickle.dump(self, open("tracker.dat","wb"))

	def GetProgress(self):
		return self.progress

if __name__=="__main__":
	tracker = pickle.load(open("tracker.dat","rb"))
	print len(tracker.models)


if 0:
	im = Image.open("test0.png")
	iml = im.load()
	tracker = RelativeTracker()
	
	tracker.AddTrainingData(im, [(120,120),(50,50)])
	tracker.AddTrainingData(im, [(140,130),(20,60)])
	tracker.Init()
	tracker.Train()
	tracker.ClearTrainingImages()
	pickle.dump(tracker, open("tracker.dat","wb"))

