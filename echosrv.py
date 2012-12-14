
import multiprocessing, sys, time
from PIL import Image
import xml.etree.ElementTree as ET

def WorkerProcess(childPipeConn):
	progress = 0.
	running = 1
	paused = 1
	imgCount = 0
	xmlBlocksCount = 0
	xmlTrees = []
	imgs = {}
	xmlDataBlocks = []

	while running:
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
					imgs[timestamp] = im
					im.save("test"+str(imgCount)+".png")
					imgCount += 1

				if args[0]=="XML_DATA":
					#Parse XML from raw data block
					xmlDataBlocks.append(event[2])
					xmlfi = open("xml"+str(xmlBlocksCount)+".xml","wt")
					xmlfi.write(event[2])
					xmlBlocksCount += 1

					tree = ET.fromstring(event[2])
					xmlTrees.append(tree)
					timestamp = float(tree.attrib['time'])
					for child in tree:
						pid = int(child.attrib['id'])
						x = float(child.attrib['x'])
						y = float(child.attrib['y'])
						print pid, x, y

		if not paused:
			print "PROGRESS="+str(progress)

		time.sleep(0.1)
		if not paused:
			progress += 0.01

		if progress >= 1.:
			progress = 1
			running = 0
			print "PROGRESS="+str(progress)

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
	#inputlog = open("inputlog.dat","wb")
	
	fi.write("READY\n")
	fi.flush()
	print "READY"
	sys.stdout.flush()

	p = multiprocessing.Process(target=WorkerProcess, args=(childPipeConn, ))
	p.start()
	
	while running:
		li = sys.stdin.readline()
		if inputlog is not None: inputlog.write(li)
		li = li.rstrip()

		#print li, len(li)
		if len(li) > 0: 
			fi.write(li+"\n")
			fi.flush()

		if li == "QUIT":
			running = 0
			parentPipeConn.send(["QUIT"])

		if li == "PAUSE":
			parentPipeConn.send(["PAUSE"])

		if li == "RUN":
			parentPipeConn.send(["RUN"])

		if li[0:11] == "DATA_BLOCK=":
			args = sys.stdin.readline()
			if inputlog is not None: inputlog.write(args)
			args = args.rstrip()
			si = int(li[11:])
			fi.write(args+"\n")
			fi.write("Attmpt to read " +str(si)+"\n")
			fi.flush()
			dataBlock = sys.stdin.read(si)
			fi.write("Read block " +str(len(dataBlock))+"\n")
			if inputlog is not None:
				inputlog.write(dataBlock)
				inputlog.flush()
			parentPipeConn.send(["DATA_BLOCK", args, dataBlock])

		if parentPipeConn.poll():
			event = parentPipeConn.recv()
			if event == "FINISHED":
				running = 0

		time.sleep(0.01)

	p.join()

