from enum import Enum
import json
import tcpClient

class MsgID(Enum):
	getInstanceList = 101
	getInstanceInfo = 102
	getCurInstance = 103
	setActiveInstance = 104
	getRegGroup = 105
	getRegList = 106
	getReg = 107
	setReg = 108
	getMem = 109
	setMem = 110
	getBreakPoint = 201
	setBreakPoint = 202
	delBreakPoint = 203
	getWatchPoint = 204
	setWatchPoint = 205
	delWatchPoint = 206
	cmdRun = 210
	cmdStop = 211
	cmdStep = 212


class VSpaceCmdProtocol:
	def __init__(self, ip, port) -> None:
		self.client = tcpClient.tcpClient(ip = ip, port = port)
		self.sessionID = 0

	def resultCheck(self, res) -> bool:
		if 'result' in res and res['result'] == 'success':
			return True
		else:
			if 'errcode' in res :
				print ("errcode:0x%x" % res['errcode'])
			return False

	def checkError(self, res) -> None:
		if 'errcode' in res :
			print ("errcode:0x%x" % res['errcode'])
		return False

	def login(self) -> bool:
		self.client.connect()
		request = json.dumps({'msg':'login'})
		self.client.send(request)

		recv = self.client.recv()
		response = json.loads(recv)
		
		ret = self.resultCheck(response)
		if ret == True:
			self.sessionID = response['sessionID']
			return True
		else:
			return False

	def logout(self) -> bool:
		if self.sessionID == 0:
			return False
		sdate = {'msg':'logout'}
		sdate['sessionID'] = self.sessionID
		request = json.dumps(sdate)
		self.client.send(request)

		rdate = self.client.recv()
		response = json.loads(rdate)
		ret = self.resultCheck(response)
		if ret == True:
			self.sessionID = 0
			return True
		else:
			return False

	def keepAlive(self) -> bool:
		if self.sessionID == 0:
			return False
		sdate = {'msg':'keepAlive'}
		sdate['sessionID'] = self.sessionID
		request = json.dumps(sdate)
		self.client.send(request)

		rdate = self.client.recv()
		response = json.loads(rdate)
		return self.resultCheck(response)

	def getInstanceList(self) -> list:
		if self.sessionID == 0:
			return []
		sdate = {}
		sdate['msgID'] = MsgID.getInstanceList.value
		sdate['sessionID'] = self.sessionID
		request = json.dumps(sdate)
		self.client.send(request)

		rdate = self.client.recv()
		response = json.loads(rdate)
		if 'list' in response:
			return response['list']
		else:
			self.checkError(response)
			return []

	def getInstanceInfo(self, id:int) -> dict:
		if self.sessionID == 0:
			return {}
		sdate = {}
		sdate['msgID'] = MsgID.getInstanceInfo.value
		sdate['id'] = id
		sdate['sessionID'] = self.sessionID
		request = json.dumps(sdate)
		self.client.send(request)

		rdate = self.client.recv()
		response = json.loads(rdate)
		
		if 'info' in response:
			return response['info']
		else:
			self.checkError(response)
			return {}

	def getCurInstance(self) -> int:
		if self.sessionID == 0:
			return -1
		sdate = {}
		sdate['msgID'] = MsgID.getCurInstance.value
		sdate['sessionID'] = self.sessionID
		request = json.dumps(sdate)
		self.client.send(request)
		
		rdate = self.client.recv()
		response = json.loads(rdate)
	
		if 'active' in response:
			return response['active']
		else:
			self.checkError(response)
			return -1

	def setActiveInstance(self, id:int) -> bool:
		if self.sessionID == 0:
			return False
		sdate = {}
		sdate['msgID'] = MsgID.setActiveInstance.value
		sdate['sessionID'] = self.sessionID
		sdate['active'] = id
		request = json.dumps(sdate)
		self.client.send(request)

		rdate = self.client.recv()
		response = json.loads(rdate)
		return self.resultCheck(response)

	def getRegList(self, group:str) -> list:
		if self.sessionID == 0:
			return []
		sdate = {}
		sdate['msgID'] = MsgID.getRegList.value
		sdate['sessionID'] = self.sessionID
		sdate['group'] = group
		request = json.dumps(sdate)
		self.client.send(request)
		
		rdate = self.client.recv()
		response = json.loads(rdate)
		if 'list' in response:
			return response['list']
		else:
			self.checkError(response)
			return []

	def RegR(self, name:str) -> dict:
		if self.sessionID == 0:
			return {}
		sdate = {}
		sdate['msgID'] = MsgID.getReg.value
		sdate['sessionID'] = self.sessionID
		sdate['name'] = name
		request = json.dumps(sdate)
		self.client.send(request)
		
		rdate = self.client.recv()
		response = json.loads(rdate)
		
		if 'info' in response:
			return response['info']
		else:
			self.checkError(response)
			return {}

	def RegW(self, name:str, value:int) -> bool:
		if self.sessionID == 0:
			return False
		sdate = {}
		sdate['msgID'] = MsgID.setReg.value
		sdate['sessionID'] = self.sessionID
		sdate['name'] = name
		sdate['value'] = value
		request = json.dumps(sdate)
		self.client.send(request)

		rdate = self.client.recv()
		response = json.loads(rdate)
		return self.resultCheck(response)

	def memoryRead(self, addr:int, len:int) -> list:
		if self.sessionID == 0:
			return []
		sdate = {}
		sdate['msgID'] = MsgID.getMem.value
		sdate['sessionID'] = self.sessionID
		sdate['addr'] = addr
		sdate['len'] = len
		request = json.dumps(sdate)
		self.client.send(request)

		rdate = self.client.recv()
		response = json.loads(rdate)
		if 'value' in response:
			return response['value']
		else:
			self.checkError(response)
			return []


	def memoryWirte(self, addr:int, len:int, value:list) -> bool:
		if self.sessionID == 0:
			return False
		sdate = {}
		sdate['msgID'] = MsgID.setMem.value
		sdate['sessionID'] = self.sessionID
		sdate['addr'] = addr
		sdate['len'] = len
		sdate['value'] = value
		request = json.dumps(sdate)
		self.client.send(request)

		rdate = self.client.recv()
		response = json.loads(rdate)
		return self.resultCheck(response)

	def getBreakpoint(self) -> list:
		if self.sessionID == 0:
			return []
		sdate = {}
		sdate['msgID'] = MsgID.getBreakPoint.value
		sdate['sessionID'] = self.sessionID
		request = json.dumps(sdate)
		self.client.send(request)

		rdate = self.client.recv()
		response = json.loads(rdate)

		if 'list' in response:
			return response['list']
		else:
			self.checkError(response)
			return []

	def setBreakpoint(self, addr:int) -> bool:
		if self.sessionID == 0:
			return False
		sdate = {}
		sdate['msgID'] = MsgID.setBreakPoint.value
		sdate['sessionID'] = self.sessionID
		sdate['addr'] = addr
		request = json.dumps(sdate)
		self.client.send(request)

		rdate = self.client.recv()
		response = json.loads(rdate)
		return self.resultCheck(response)

	def delBreakpoint(self, addr:int) -> bool:
		if self.sessionID == 0:
			return False
		sdate = {}
		sdate['msgID'] = MsgID.delBreakPoint.value
		sdate['sessionID'] = self.sessionID
		sdate['addr'] = addr
		request = json.dumps(sdate)
		self.client.send(request)

		rdate = self.client.recv()
		response = json.loads(rdate)
		return self.resultCheck(response)

	def getWatchpoint(self) -> list:
		if self.sessionID == 0:
			return []
		sdate = {}
		sdate['msgID'] = MsgID.getWatchPoint.value
		sdate['sessionID'] = self.sessionID
		request = json.dumps(sdate)
		self.client.send(request)

		rdate = self.client.recv()
		response = json.loads(rdate)
	
		if 'list' in response:
			return response['list']
		else:
			self.checkError(response)
			return []

	def setWatchpoint(self, addr:int, len:int, attr:str) -> bool:
		if self.sessionID == 0:
			return False
		sdate = {}
		sdate['msgID'] = MsgID.setWatchPoint.value
		sdate['sessionID'] = self.sessionID
		sdate['addr'] = addr
		sdate['len'] = len
		sdate['attr'] = attr
		request = json.dumps(sdate)
		self.client.send(request)

		rdate = self.client.recv()
		response = json.loads(rdate)
		return self.resultCheck(response)


	def delWatchpoint(self, addr:int, len:int, attr:str) -> bool:
		if self.sessionID == 0:
			return False
		sdate = {}
		sdate['msgID'] = MsgID.delWatchPoint.value
		sdate['sessionID'] = self.sessionID
		sdate['addr'] = addr
		sdate['len'] = len
		sdate['attr'] = attr
		request = json.dumps(sdate)
		self.client.send(request)

		rdate = self.client.recv()
		response = json.loads(rdate)
		return self.resultCheck(response)

	def cmdRun(self) -> bool:
		if self.sessionID == 0:
			return False
		sdate = {}
		sdate['msgID'] = MsgID.cmdRun.value
		sdate['sessionID'] = self.sessionID
		request = json.dumps(sdate)
		self.client.send(request)

		rdate = self.client.recv()
		response = json.loads(rdate)
		return self.resultCheck(response)

	def cmdStop(self) -> bool:
		if self.sessionID == 0:
			return False
		sdate = {}
		sdate['msgID'] = MsgID.cmdStop.value
		sdate['sessionID'] = self.sessionID
		request = json.dumps(sdate)
		self.client.send(request)

		rdate = self.client.recv()
		response = json.loads(rdate)
		return self.resultCheck(response)

	def cmdStep(self, n:int) -> bool:
		if self.sessionID == 0:
			return False
		sdate = {}
		sdate['msgID'] = MsgID.cmdStep.value
		sdate['sessionID'] = self.sessionID
		sdate['steps'] = n
		request = json.dumps(sdate)
		self.client.send(request)

		rdate = self.client.recv()
		response = json.loads(rdate)
		return self.resultCheck(response)

