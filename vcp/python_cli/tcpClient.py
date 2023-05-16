
import struct
import socket

class tcpClient:
	def __init__(self, ip, port) -> None:
		self.ip = ip
		self.port = port
		self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		timeval = struct.pack('ll', 5, 0)
		self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVTIMEO, timeval)
		self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_SNDTIMEO, timeval)

	def checksum(self, data, size):
		csum = 0
		for i in range(0, size//2):
			csum += (data[i*2 ] << 8) 
			csum += data[i*2 + 1] & 0x00FF
		if size % 2 == 1:
			csum += data[size - 1] << 8
		while (csum >> 16):
			csum = (csum >> 16) + (csum & 0xFFFF)

		return ~csum & 0xFFFF

	def connect(self):
		self.sock.connect((self.ip, self.port))

	def recv(self):
		rhdr = self.sock.recv(12)
		headr = struct.unpack('!lHHl', rhdr)
		if headr[0] != 0x7E766370 :
			return ''

		length = headr[3]
		payload = self.sock.recv(length - 12)
		csum = self.checksum(payload, length - 12)
		if csum != headr[2]:
			return ''

		return payload.decode()

	def recvlong(self) -> str:
		payload = bytes()
		try:
			rhdr = self.sock.recv(12)
			headr = struct.unpack('!lHHl', rhdr)
			if headr[0] != 0x7E766370:
				return ''
			
			length = headr[3] - 12
			while length > len(payload) :
				data = self.sock.recv(4096)
				if not data:
					break
				payload += data
		
		except BlockingIOError:
			pass

		csum = self.checksum(payload, len(payload))
		if csum != headr[2]:
			return ''

		return payload.decode()

	def send(self, buf):
		csum = self.checksum(buf.encode(), len(buf.encode()))
		sendData = struct.pack('!lHHl', 0x7E766370, 0, csum, len(buf.encode()) + 12) + buf.encode()
		print (sendData[:12])
		self.sock.send(sendData)

	def __del__(self):
		self.sock.close()
