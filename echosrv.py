import sys, random
#sys.path = ["python-lib", "site-packages", "."]
import multiprocessing, time, pickle, bz2, base64, os, zlib
from PIL import Image
import xml.etree.ElementTree as ET
from reltracker import reltracker

def WorkerProcess(childPipeConn):

	if 0:
		import cProfile
		cProfile.run('WorkerProcessProf(childPipeConn)', 'workerProf')
	else:
		WorkerProcessProf(childPipeConn)


def WorkerProcessProf(childPipeConn):
	progress = 0.
	running = 1
	paused = 1
	training = 0
	imgCount = 0
	xmlBlocksCount = 0
	xmlTrees = []
	trainImgs = {}
	currentFrame = None
	xmlDataBlocks = []
	modelReady = False
	tracker = None
	getProgress = False
	aliveClock = time.time()
	aliveMsgEnabled = False
	savedTracker = False

	#random.junk()

	while running:
		timeNow = time.time()
		if timeNow > aliveClock + 1. and aliveMsgEnabled:
			print "ALIVE"
			sys.stdout.flush()
			aliveClock = timeNow

		if childPipeConn.poll():
			event = childPipeConn.recv()

			#print "Rx",event[0]
			if event[0]=="RUN":
				print "NOW_RUNNING"
				paused = 0
			if event[0]=="PAUSE":
				print "NOW_PAUSED"
				paused = 1
			if event[0]=="QUIT":
				running = 0
			if event[0]=="GET_PROGRESS":
				getProgress = True
			if event[0]=="KEEPALIVE":
				#print "ALIVE"
				sys.stdout.flush()

			if event[0]=="TRAINING_DATA_FINISH":
				if len(trainImgs) == 0 and not modelReady:
					print "Error: No images loaded in algorithm process"
					continue
				if len(xmlTrees) == 0 and not modelReady:
					print "Error: No annotated positions loaded into algorithm process"
					continue

				modelReady = 1
				training = 1
				if tracker is None:
					tracker = reltracker.RelTracker()
					for tree in xmlTrees:
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

						if int(round(timestamp*1000.)) not in trainImgs:
							print "Image for timestamp",int(round(timestamp*1000.)),"not found"
							continue
						im = trainImgs[int(round(timestamp*1000.))]
						tracker.Add(im, zip(xs,ys))

				tracker.TrainingDataComplete()
				#tracker = pickle.load(open("tracker.dat","rb"))

			if event[0]=="SAVE_MODEL":
				if paused and tracker is not None:
					tracker.PrepareForPickle()
					trackerStr = pickle.dumps(tracker, protocol=pickle.HIGHEST_PROTOCOL)
					tracker.PostUnPickle()

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
					if 'WIDTH' not in argDict: continue
					if 'HEIGHT' not in argDict: continue
					if 'TIMESTAMP' not in argDict: continue
					width = int(argDict['WIDTH'])
					height = int(argDict['HEIGHT'])
					timestamp = int(argDict['TIMESTAMP'])
					imgRaw = event[2]
					if width * height * 3 != len(imgRaw): 
						print "#Image buffer of incorrect size",width,height,len(imgRaw)
						continue

					im = Image.frombuffer("RGB", (width, height), imgRaw, 'raw', "RGB", 0, 1)
					if not training:
						#Pre-training image collection
						print "Store image"
						trainImgs[timestamp] = im
					imgCount += 1

				if args[0]=="XML_DATA":
					#Parse XML from raw data block
					xmlDataBlocks.append(event[2])
					#xmlfi = open("xml"+str(xmlBlocksCount)+".xml","wt")
					#xmlfi.write(event[2])
					xmlBlocksCount += 1

					tree = ET.fromstring(event[2])
					xmlTrees.append(tree)
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
						
						tracker = pickle.loads(modelData)
						tracker.PostUnPickle()
						print tracker
						modelReady = True
					except Exception as exErr:
						print "Decompression of data failed", str(exErr)

				if args[0] == "RGB_IMAGE_AND_XML":
					#Decode image from raw data block
					args.pop(0)
					pairs = [tmp.split("=") for tmp in args]
					argDict = dict(pairs)
					if 'WIDTH' not in argDict: continue
					if 'HEIGHT' not in argDict: continue
					if 'IMGBYTES' not in argDict: continue
					if 'XMLBYTES' not in argDict: continue
					if 'ID' not in argDict: continue
					width = int(argDict['WIDTH'])
					height = int(argDict['HEIGHT'])
					imgBytes = int(argDict['IMGBYTES'])
					xmlBytes = int(argDict['XMLBYTES'])
					reqId = int(argDict['ID'])
					combinedRaw = event[2];
					#if width * height * 3 != imgBytes: 
					#	print "#Image buffer of incorrect size",width,height,len(event[2])
					#	continue

					im = Image.frombuffer("RGB", (width, height), combinedRaw[:imgBytes], 'raw', "RGB", 0, 1)
					if modelReady:
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
								pred = tracker.Predict(im, modelList)
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

					if training and not modelReady:
						print "ALG_NOT_READY"

				print "DATA_BLOCK_PROCESSED"

		if (not paused and training and progress < 1.) or getProgress:
			assert tracker is not None

			if not paused and training and progress < 1.: 
				tracker.Update()
			progress = tracker.GetProgress()
			print "PROGRESS="+str(progress)
			getProgress = False

			#Save training when training is complete
			if not savedTracker and tracker.trainingRegressorsCompleteFlag:
				#tracker.PrepareForPickle()
				#pickle.dump(tracker,open("tracker.dat","wb"),protocol=-1)
				#tracker = pickle.load(open("tracker.dat","rb"))
				#tracker.PostUnPickle()
				savedTracker = True


		else:
			time.sleep(0.1)

		if progress >= 1. and not paused:
			progress = 1.
			paused = 1
			print "PROGRESS="+str(progress)
			print "NOW_PAUSED"

		#print "running", running
		sys.stdout.flush()
	
	childPipeConn.send("FINISHED")
	print "FINISHED"
	sys.stdout.flush()

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


