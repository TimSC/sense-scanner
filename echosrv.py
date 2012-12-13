
import multiprocessing, sys, time

def WorkerProcess(childPipeConn):
	progress = 0.
	running = 1
	paused = 1

	while running:

		if childPipeConn.poll():
			event = childPipeConn.recv()
			print "RX",event

			if event=="RUN":
				print "NOW_RUNNING"
				paused = 0
			if event=="PAUSE":
				print "NOW_PAUSED"
				paused = 1
			if event=="QUIT":
				running = 0

		if not paused:
			print "PROGRESS="+str(progress)

		time.sleep(0.01)
		if not paused:
			progress += 0.01

		if progress >= 1.:
			progress = 1
			running = 0

		#print "running", running
	
	print "FINISHED"
	

if __name__=="__main__":
	print "Hello world"
	running = 1
	parentPipeConn, childPipeConn = multiprocessing.Pipe()

	p = multiprocessing.Process(target=WorkerProcess, args=(childPipeConn, ))
	p.start()
	
	while running:
		li = sys.stdin.readline().rstrip()

		#print li, len(li)

		if li == "QUIT":
			running = 0
			parentPipeConn.send("QUIT")

		if li == "PAUSE":
			parentPipeConn.send("PAUSE")

		if li == "RUN":
			parentPipeConn.send("RUN")

		if li[0:11] == "DATA_BLOCK=":
			args = sys.stdin.readline().rstrip()
			si = int(li[11:])
			dataBlock = sys.stdin.read(si)

	p.join()

