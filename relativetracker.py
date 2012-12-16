
import time

class RelativeTracker:
	def __init__(self):
		self.ims = []
		self.pointsPosLi = []
		self.progress = 0.

	def AddTrainingData(self, im, pointsPos):
		self.ims.append(im)
		self.pointsPosLi.append(pointsPos)
	
	def Update(self):
		self.progress += 0.01
		time.sleep(0.1)

	def GetProgress(self):
		return self.progress

