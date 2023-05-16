
import time
from vcp import VSpaceCmdProtocol


def getInstanceList(client):
	instList = client.getInstanceList()
	if instList:
		print('get instance list success:', instList)
	else:
		print('get instance list fail')

def getInstanceInfo(client):
	info = client.getInstanceInfo(1)
	if info:
		print('get instance info success:', info)
	else:
		print('get instance info fail')

def getCurInstance(client):
	id = client.getCurInstance()
	if id > 0:
		print('get current Instance success:', id)
	else:
		print('get current Instance fail')

def setActiveInstance(client):
	if client.getCurInstance(1):
		print('set active Instance success')
	else:
		print('set active Instance fail')

def getRegsiter(client):
	pass

def setRegsiter(client):
	if client.RegW(name ='PC', value = 0x12345678):
		print('set active Instance success')
	else:
		print('set active Instance fail')

def readMemory(client):
	list = client.memoryRead(addr = 0x80000000, len = 1)
	if list:
		print('memory value:', str(list))
	else:
		print('read memory fail')

def writeMemory(client):
	if client.memoryWirte(addr = 0x80000000, len = 1, value = [0x01]):
		print('write memory success')
	else:
		print('write memory fail')

def getBreakpoint(client):
	list = client.getBreakpoint()
	if list:
		print('breakpoint list:', str(list))
	else:
		print('get breakpoint fail')

def setBreakpoint(client):
	if client.setBreakpoint(addr = 0x80000000):
		print('set breakpoint success')
	else:
		print('set breakpoint fail')

def delBreakpoint(client):
	if client.delBreakpoint(addr = 0x80000000):
		print('delete breakpoint success')
	else:
		print('delete breakpoint fail')

def getWatchpoint(client):
	list = client.getWatchpoint()
	if list:
		print('watchpoint list:', str(list))
	else:
		print('get watchpoint fail')

def setWatchpoint(client):
	if client.setWatchpoint(addr = 0x80000000, len = 1, attr = 'r'):
		print('set watchpoint success')
	else:
		print('set watchpoint fail')

def delWatchpoint(client):
	if client.delWatchpoint(addr = 0x80000000, len = 1, attr = 'r'):
		print('set watchpoint success')
	else:
		print('set watchpoint fail')


if __name__ == '__main__':
	
	client = VSpaceCmdProtocol(ip = "127.0.0.1", port = 8000)
	client.login()
	curTime = time.time()
	getInstanceList(client = client)

	try:
		while True:
			interval = time.time() - curTime
			if interval >= 5:
				getInstanceList(client = client)
				curTime = time.time()
	except KeyboardInterrupt:
		pass
	else:
		pass


	'''
	client = VSpaceCmdProtocol(ip = "127.0.0.1", port = 8000)
	print('client login')
	curTime = time.time()

	getInstanceList(client = client)
	getInstanceInfo(client = client)
	getCurInstance(client = client)
	getRegsiter(client = client)


	try:
		while True:
			interval = time.time() - curTime
			if interval >= 5:
				client.keepAlive()
				curTime = time.time()
				print('keep alive')
			
			time.sleep(1)
	except KeyboardInterrupt:
		client.logout()
		print('client logout')
	else:
		pass
	'''
