import sys, random
#sys.path = ["python-lib", "site-packages", "."]
import multiprocessing, time, pickle, bz2, base64, os, zlib
from PIL import Image
import xml.etree.ElementTree as ET
from reltracker import reltracker

def WorkerProcess(childPipeConn):

	if 0:
		import cProfile
		cProfile.run('Worker(childPipeConn)', 'workerProf')
	else:
		workerObj = Worker(childPipeConn)

class Worker:
	def __init__(self, childPipeConn):
		self.progress = 0.
		self.running = 1
		self.paused = 1
		self.training = 0
		self.imgCount = 0
		self.xmlBlocksCount = 0
		self.xmlTrees = []
		self.trainImgs = {}
		self.xmlDataBlocks = []
		self.modelReady = False
		self.tracker = None
		self.getProgress = False
		self.aliveClock = time.time()
		self.aliveMsgEnabled = True
		self.savedTracker = False
		self.childPipeConn = childPipeConn
		self.workerLog = open("workerLog.txt","wt")
		self.Run()

	def Run(self):


		#random.junk()

		while self.running:
			timeNow = time.time()
			if timeNow > self.aliveClock + 1. and self.aliveMsgEnabled:
				print "ALIVE"
				sys.stdout.flush()
				if self.workerLog is not None:
					self.workerLog.write("ALIVE\n")
					self.workerLog.flush()
				self.aliveClock = timeNow

			#Get all events
			while childPipeConn.poll():
				event = childPipeConn.recv()
				self.HandleEvent(event)
				time.sleep(0.001)

			if (not self.paused and self.training and self.progress < 1.) or self.getProgress:
				assert self.tracker is not None

				if not self.paused and self.training and self.progress < 1.: 
					self.tracker.Update()
				self.progress = self.tracker.GetProgress()
				print "PROGRESS="+str(self.progress)
				self.getProgress = False

				#Save training when training is complete
				if not self.savedTracker and self.tracker.trainingRegressorsCompleteFlag:
					#tracker.PrepareForPickle()
					#pickle.dump(tracker,open("tracker.dat","wb"),protocol=-1)
					#tracker = pickle.load(open("tracker.dat","rb"))
					#tracker.PostUnPickle()
					self.savedTracker = True


			else:
				time.sleep(0.1)

			if self.progress >= 1. and not self.paused:
				self.progress = 1.
				self.paused = 1
				print "PROGRESS="+str(self.progress)
				print "NOW_PAUSED"

			#print "running", self.running
			sys.stdout.flush()
	
		childPipeConn.send("FINISHED")
		print "FINISHED"
		sys.stdout.flush()
		self.workerLog.write("Worker thread finishing\n")
		self.workerLog.close()

	def HandleEvent(self, event):
		if 1:
			if self.workerLog is not None:
				self.workerLog.write(event[0]+"\n")
				self.workerLog.flush()

			#print "Rx",event[0]
			if event[0]=="RUN":
				print "NOW_RUNNING"
				self.paused = 0
			if event[0]=="PAUSE":
				print "NOW_PAUSED"
				self.paused = 1
			if event[0]=="QUIT":
				self.running = 0
			if event[0]=="GET_PROGRESS":
				self.getProgress = True
			if event[0]=="KEEPALIVE":
				#print "ALIVE"
				sys.stdout.flush()

			if event[0]=="TRAINING_DATA_FINISH":
				if len(self.trainImgs) == 0 and not self.modelReady:
					print "Error: No images loaded in algorithm process"
					return
				if len(self.xmlTrees) == 0 and not self.modelReady:
					print "Error: No annotated positions loaded into algorithm process"
					return

				self.modelReady = 1
				self.training = 1
				if self.tracker is None:
					self.tracker = reltracker.RelTracker()
					for tree in self.xmlTrees:
						timestamp = float(tree.attrib['time'])
						xs, ys = [], []
						for child in tree:
							pid = int(child.attrib['id'])
							x = float(child.attrib['x'])
							y = float(child.attrib['y'])
							while pid >= len(xs): xs.append(None)
							while pid >= len(ys): ys.append(None)
							xs[pid] = x
							ys[pid] = y

						if int(round(timestamp*1000.)) not in self.trainImgs:
							print "Image for timestamp",int(round(timestamp*1000.)),"not found"
							return
						im = self.trainImgs[int(round(timestamp*1000.))]
						self.tracker.Add(im, zip(xs,ys))

				self.tracker.TrainingDataComplete()
				#tracker = pickle.load(open("tracker.dat","rb"))

			if event[0]=="SAVE_MODEL":
				if self.paused and self.tracker is not None:
					self.tracker.PrepareForPickle()
					trackerStr = pickle.dumps(self.tracker, protocol=pickle.HIGHEST_PROTOCOL)
					self.tracker.PostUnPickle()

					#fi=open("testpickle.dat","wb")
					#fi.write(trackerStr)
					#fi.close()

					#modelData = "bz2".encode("ascii")+bz2.compress(trackerStr)
					modelData = "raw".encode("ascii")+trackerStr
					modelDataB64 = base64.b64encode(modelData)
					print "DATA_BLOCK={0}".format(len(modelDataB64))
					sys.stdout.write("MODEL\n")
					sys.stdout.flush()
					sys.stdout.write(modelDataB64)
					sys.stdout.flush()
					print "Saved data size", len(modelData)
					print "B64 encoded", len(modelDataB64)
					print "Uncompressed size", len(trackerStr)

			if event[0]=="DATA_BLOCK":
				args = event[1].split(" ")
				print "DATA_BLOCK args", args
				if args[0] == "RGB_IMAGE_DATA":
					#Decode image from raw data block
					args.pop(0)
					pairs = [tmp.split("=") for tmp in args]
					argDict = dict(pairs)
					if 'WIDTH' not in argDict: return
					if 'HEIGHT' not in argDict: return
					if 'TIMESTAMP' not in argDict: return
					width = int(argDict['WIDTH'])
					height = int(argDict['HEIGHT'])
					timestamp = int(argDict['TIMESTAMP'])
					imgRaw = event[2]
					if width * height * 3 != len(imgRaw): 
						print "#Image buffer of incorrect size",width,height,len(imgRaw)
						return

					im = Image.frombuffer("RGB", (width, height), imgRaw, 'raw', "RGB", 0, 1)
					if not self.training:
						#Pre-training image collection
						print "Store image"
						self.trainImgs[timestamp] = im
					self.imgCount += 1

				if args[0]=="XML_DATA":
					#Parse XML from raw data block
					self.xmlDataBlocks.append(event[2])
					#xmlfi = open("xml"+str(self.xmlBlocksCount)+".xml","wt")
					#xmlfi.write(event[2])
					self.xmlBlocksCount += 1

					tree = ET.fromstring(event[2])
					self.xmlTrees.append(tree)
					#timestamp = float(tree.attrib['time'])
					#for child in tree:
					#	pid = int(child.attrib['id'])
					#	x = float(child.attrib['x'])
					#	y = float(child.attrib['y'])
					#	print pid, x, y

				if args[0]=="MODEL":
					binModel = base64.b64decode(event[2])
					modelFormat = binModel[:3]
					print "Loading model from string", len(event[2]), modelFormat
					print "Uncompressed size", len(binModel)
					try:
						modelData = None
						if modelFormat == "bz2":
							modelData = bz2.decompress(binModel[3:])
						if modelFormat == "raw":
							modelData = binModel[3:]
						if modelFormat == "zlb":
							modelData = zlib.decompress(binModel[3:])
						assert modelData is not None
						
						self.tracker = pickle.loads(modelData)
						self.tracker.PostUnPickle()
						print self.tracker
						self.modelReady = True
					except Exception as exErr:
						print "Decompression of data failed", str(exErr)

				if args[0] == "RGB_IMAGE_AND_XML":
					#Decode image from raw data block
					args.pop(0)
					pairs = [tmp.split("=") for tmp in args]
					argDict = dict(pairs)
					if 'WIDTH' not in argDict: return
					if 'HEIGHT' not in argDict: return
					if 'IMGBYTES' not in argDict: return
					if 'XMLBYTES' not in argDict: return
					if 'ID' not in argDict: return
					width = int(argDict['WIDTH'])
					height = int(argDict['HEIGHT'])
					imgBytes = int(argDict['IMGBYTES'])
					xmlBytes = int(argDict['XMLBYTES'])
					reqId = int(argDict['ID'])
					combinedRaw = event[2];
					#if width * height * 3 != imgBytes: 
					#	print "#Image buffer of incorrect size",width,height,len(event[2])
					#	return

					im = Image.frombuffer("RGB", (width, height), combinedRaw[:imgBytes], 'raw', "RGB", 0, 1)
					if self.modelReady:
						#Post-training phase
						#print "Store image"
						#im.save("alg.jpg")

						#Encode xml and make predictions
						outXml = "<prediction>\n"
						xmlData = combinedRaw[imgBytes:imgBytes+xmlBytes].decode('utf8')
						tree = ET.fromstring(xmlData)
						for model in tree:
							outXml += " <model>\n"
							modelList = []
							for pt in model:
								modelList.append((float(pt.attrib['x']), float(pt.attrib['y'])))

							try:
								pred = self.tracker.Predict(im, modelList)
							except:
								#Tracker failed, probably went out of bounds
								pred = modelList
							
							#For out of bounds points, randomly move tracker inside image
							for pi, pt in enumerate(pred):
								margin = 40
								ptl = list(pt) #Ensure this is not a tuple
								if ptl[0] < margin or ptl[0] >= width-margin:
									ptl[0] = random.uniform(margin, width - margin)
								if ptl[1] < margin or ptl[1] >= height-margin:
									ptl[1] = random.uniform(margin, height - margin)
								pred[pi] = ptl

							for pt in pred:
								outXml += "  <pt x=\""+str(pt[0])+"\" y=\""+str(pt[1])+"\"/>\n"
							outXml += " </model>\n"

						outXml += "</prediction>\n"
						outXmlEnc = base64.b64encode(outXml.encode('utf-8'))
						
						print "XML_BLOCK={0}".format(len(outXmlEnc))
						sys.stdout.write("PREDICTION_RESPONSE ID="+str(reqId)+"\n")
						sys.stdout.flush()
						sys.stdout.write(outXmlEnc)
						sys.stdout.flush()

					if self.training and not self.modelReady:
						print "ALG_NOT_READY"

				print "DATA_BLOCK_PROCESSED"


def ReadDataBlock(parentPipeConn, inputlog, fi):
	args = sys.stdin.readline()
	if inputlog is not None: 
		inputlog.write(args)
		inputlog.flush()
	args = args.rstrip()
	si = int(li[11:])
	if fi is not None:
		fi.write(args+"\n")
		fi.write("Attempt to read " +str(si)+"\n")
		fi.flush()
	dataBlock = sys.stdin.read(si)

	parentPipeConn.send(["DATA_BLOCK", args, dataBlock])


if __name__=="__main__":
        
	running = 1
	parentPipeConn, childPipeConn = multiprocessing.Pipe()

	if sys.platform == "win32":
		import msvcrt

		#Set console to open in binary mode (on windows)
		msvcrt.setmode(sys.stdin.fileno(), os.O_BINARY)
		msvcrt.setmode(sys.stdout.fileno(), os.O_BINARY)

	#fi = None
	fi = open("log.txt","wt")
	inputlog = None
	#inputlog = open("inputlog.dat","wb")
	
	if fi is not None:
		fi.write("READY\n")
        fi.flush()
	print "READY"
	sys.stdout.flush()

	p = multiprocessing.Process(target=WorkerProcess, args=(childPipeConn, ))
	p.start()
	
	while running:
		#startItTi = time.time()
		#if fi is not None: fi.write("Waiting for stdin data:"+str(startItTi)+"\n")
		li = sys.stdin.readline()
		#endItTi = time.time()
		#if fi is not None: fi.write("Rx stdin data:"+str(endItTi)+","+str(endItTi-startItTi)+"\n")
		if inputlog is not None: 
			inputlog.write(li)
			inputlog.flush()
		li = li.rstrip()

		#print li, len(li)
		if len(li) > 0 and fi is not None: 
			fi.write(li+"\n")
			fi.flush()

		handled = 0

		if li == "QUIT":
			running = 0
			parentPipeConn.send(["QUIT"])
			handled = 1

		if li[0:11] == "DATA_BLOCK=":
			ReadDataBlock(parentPipeConn, inputlog, fi)
			handled = 1

		if li == "KEEPALIVE":
			if not p.is_alive():
				print "INTERNAL_ERROR"
				sys.stdout.flush()
			else:
				parentPipeConn.send(["KEEPALIVE"])
			handled = 1

		#Send all misc commands to worker thread
		if not handled:
			parentPipeConn.send([li,])

		if parentPipeConn.poll():
			event = parentPipeConn.recv()
			if event == "FINISHED":
				running = 0

		time.sleep(0.01)

	if fi is not None: fi.flush()
	p.join()


