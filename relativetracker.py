
from PIL import Image
import time, math
import numpy as np

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

class PredictAxis:
	def __init__(self, xIn, yIn):
		mag = (xIn ** 2. + yIn ** 2.) ** 0.5
		assert mag > 0.
		self.axisX = xIn / mag
		self.axisY = yIn / mag

		self.crossX = -self.axisY
		self.crossY = self.axisX

	def Train(img, pts):
		pass


class RelativeTracker:
	def __init__(self):
		self.ims = []
		self.imls = None
		self.pointsPosLi = []
		self.progress = 0.
		self.maxSupportOffset = 20.
		self.numSupportPix = 500
		self.numTrainingOffsets = 5000
		self.trainOffsetVar = 5.
		self.supportPixOffsets = None
		self.supportPixCols = None
		self.modelX = PredictAxis(1.,0.)
		self.modelY = PredictAxis(0.,1.)

		#settings = [{'shapeNoise': 12, 'cloudEnabled': 1, 'supportMaxOffset': 39, 'trainVarianceOffset': 41, 'reg': reg}, {'shapeNoise': 1, 'cloudEnabled': 0, 'supportMaxOffset': 20, 'trainVarianceOffset': 5, 'reg': reg}] #"Classic" 0.2 settings

		#numSupportPix = 500, numTrainingOffsets = 5000, 
		#	supportMaxOffset = 90, 
		#	trainVarianceOffset = 23, 
		#	shapeNoise = 16.,
		#	rotationVar = 0.1,

	def Init(self):
		#Generate random support pix offsets
		self.supportPixOffsets = np.random.uniform(-self.maxSupportOffset, self.maxSupportOffset, (self.numSupportPix, 2))
		
		#Create pixel access objects
		if self.imls is None:
			self.imls = [im.load() for im in self.ims]

		#For all training frames
		colStore = []
		for im, iml, imPos in zip(self.ims, self.imls, self.pointsPosLi):
			#For all points
			for ptCount, pt in enumerate(imPos):
				#Store pixel intensity at annotated position
				while ptCount >= len(colStore):
					colStore.append([])
				if pt is None: continue
				col = BilinearSample(iml, pt[0], pt[1])
				colStore[ptCount].append(col)
		
		#Compute average colour for each point
		colStoreArr = np.array(colStore)
		self.supportPixCols = colStoreArr[:,0,:]
		
	def Train(self):
		trainOffsets = np.random.randn(self.numTrainingOffsets, 2) * self.trainOffsetVar
		
		offset = trainOffsets[0]
		


	def AddTrainingData(self, im, pointsPos):
		self.imls = None
		self.ims.append(im)
		self.pointsPosLi.append(pointsPos)
	
	def Update(self):
		self.progress += 0.01
		time.sleep(0.1)

	def GetProgress(self):
		return self.progress

if __name__=="__main__":
	im = Image.open("test0.png")
	iml = im.load()
	tracker = RelativeTracker()
	
	tracker.AddTrainingData(im, [(120,120),(50,50)])
	tracker.AddTrainingData(im, [(140,130),(55,60)])
	tracker.Init()
	tracker.Train()

