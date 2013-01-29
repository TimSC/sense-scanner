
import multiprocessing, sys, time, pickle, bz2, base64
from PIL import Image
import xml.etree.ElementTree as ET
from reltracker import reltracker

def WorkerProcess(childPipeConn):
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

	while running:
		timeNow = time.time()
		if timeNow > aliveClock + 1. and aliveMsgEnabled:
			print "ALIVE"
			aliveClock = timeNow

		if childPipeConn.poll():
			event = childPipeConn.recv()

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
			if event[0]=="TRAIN":
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
					modelData = bz2.compress(trackerStr)
					print "DATA_BLOCK={0}".format(len(modelData))
					sys.stdout.write("MODEL\n")
					sys.stdout.flush()
					sys.stdout.write(modelData)
					sys.stdout.flush()
					print "Saved data size", len(modelData)
					print "Uncompressed size", len(trackerStr)

			if event[0]=="DATA_BLOCK":
				args = event[1].split(" ")
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
					if width * height * 3 != len(event[2]): 
						print "#Image buffer of incorrect size",width,height,len(event[2])
						continue

					im = Image.frombuffer("RGB", (width, height), event[2], 'raw', "RGB", 0, 1)
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
					print "Loading model from string", len(event[2])
					try:
						modelData = bz2.decompress(base64.b64decode(event[2]))
						print "Uncompressed size", len(modelData)
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
					if width * height * 3 != imgBytes: 
						print "#Image buffer of incorrect size",width,height,len(event[2])
						continue

					im = Image.frombuffer("RGB", (width, height), event[2][:imgBytes], 'raw', "RGB", 0, 1)
					if training:
						#Post-training phase
						#print "Store image"
						#im.save("alg.jpg")

						outXml = "<prediction>\n"
						xmlData = event[2][imgBytes:imgBytes+xmlBytes].decode('utf8')
						tree = ET.fromstring(xmlData)
						for model in tree:
							outXml += " <model>\n"
							modelList = []
							for pt in model:
								modelList.append((float(pt.attrib['x']), float(pt.attrib['y'])))

							pred = tracker.Predict(im, modelList)
							
							for pt in pred:
								outXml += "  <pt x=\""+str(pt[0])+"\" y=\""+str(pt[1])+"\"/>\n"
							outXml += " </model>\n"

						outXml += "</prediction>\n"
						outXmlEnc = outXml.encode('utf-8')

						print "XML_BLOCK={0}".format(len(outXmlEnc))
						sys.stdout.write("PREDICTION_RESPONSE ID="+str(reqId)+"\n")
						sys.stdout.flush()
						sys.stdout.write(outXmlEnc)
						sys.stdout.flush()

					else:
						print "ALG_NOT_READY"

				print "DATA_BLOCK_PROCESSED"

		if (not paused and training and progress < 1.) or getProgress:
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


if __name__=="__main__":
	running = 1
	parentPipeConn, childPipeConn = multiprocessing.Pipe()

	fi = open("log.txt","wt")
	inputlog = None
	inputlog = open("inputlog.dat","wb")
	
	fi.write("READY\n")
	fi.flush()
	print "READY"
	sys.stdout.flush()

	p = multiprocessing.Process(target=WorkerProcess, args=(childPipeConn, ))
	p.start()
	
	while running:
		li = sys.stdin.readline()
		if inputlog is not None: 
			inputlog.write(li)
			inputlog.flush()
		li = li.rstrip()

		#print li, len(li)
		if len(li) > 0: 
			fi.write(li+"\n")
			fi.flush()

		handled = 0

		if li == "QUIT":
			running = 0
			parentPipeConn.send(["QUIT"])
			handled = 1

		if li[0:11] == "DATA_BLOCK=":
			args = sys.stdin.readline()
			if inputlog is not None: 
				inputlog.write(args)
				inputlog.flush()
			args = args.rstrip()
			si = int(li[11:])
			fi.write(args+"\n")
			fi.write("Attempt to read " +str(si)+"\n")
			fi.flush()
			dataBlock = sys.stdin.read(si)

			parentPipeConn.send(["DATA_BLOCK", args, dataBlock])
			handled = 1

		#Send all misc commands to worker thread
		if not handled:
			parentPipeConn.send([li,])

		if parentPipeConn.poll():
			event = parentPipeConn.recv()
			if event == "FINISHED":
				running = 0

		time.sleep(0.01)

	p.join()

