
from PIL import Image
import time, math

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
		self.pointsPosLi = []
		self.progress = 0.

		#settings = [{'shapeNoise': 12, 'cloudEnabled': 1, 'supportMaxOffset': 39, 'trainVarianceOffset': 41, 'reg': reg}, {'shapeNoise': 1, 'cloudEnabled': 0, 'supportMaxOffset': 20, 'trainVarianceOffset': 5, 'reg': reg}] #"Classic" 0.2 settings


	def AddTrainingData(self, im, pointsPos):
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
	print iml[50, 50]
	print iml[51, 50]
	print iml[50, 51]
	print iml[51, 51]

	print BilinearSample(iml, 50.5, 50.5)

