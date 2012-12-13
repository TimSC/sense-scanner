
import multiprocessing, sys

if __name__=="__main__":
	print "Hello world"
	running = 1

	while running:
		li = sys.stdin.readline().rstrip()

		#print li, len(li)

		if li == "QUIT":
			running = 0

		if li == "PAUSE":
			pass

		if li == "RUN":
			pass

		if li[0:11] == "DATA_BLOCK=":
			args = sys.stdin.readline().rstrip()
			si = int(li[11:])
			dataBlock = sys.stdin.read(si)



