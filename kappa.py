import socket
import time
import sys

class Kappa:
	socket = None
	connected = False
	nickname = 'feedthespam'
	password = 'oauth:'
	channels = ['#feedthespam']

	def __init__(self):
		self.begin()
		self.socket = socket.socket()
		self.socket.connect(('irc.twitch.tv', 6667))
		self.send("PASS %s" % self.password)
		self.send("NICK %s" % self.nickname)

		while True:
			buf = self.socket.recv(4096)
			lines = buf.split("\n")
			for data in lines:
				data = str(data).strip()

				if data == '':
					continue
				print "I<", data

				# server ping/pong?
				if data.find('PING') != -1:
					n = data.split(':')[1]
					self.send('PONG :' + n)
				if self.connected == False:
					self.perform()
					self.connected = True

				args = data.split(None, 3)
				if len(args) != 4:
					continue
				ctx = {}
				ctx['sender'] = args[0][1:]
				ctx['type']   = args[1]
				ctx['target'] = args[2]
				ctx['msg']    = args[3][1:]

				# whom to reply?
				target = ctx['target']
				if ctx['target'] == self.nickname:
					target = ctx['sender'].split("!")[0]


	def send(self, msg):
		print msg
		self.socket.send(msg+"\r\n")

	def say(self, msg, to):
		self.send("PRIVMSG %s :%s" % (to, msg))

	def perform(self):
		for c in self.channels:
			self.send("JOIN %s" % c)
			# say hello to every channel
			self.say('Are we ready? Kappa', c)

	def quit(self):
		print "Terminating echo.."
		for c in self.channels:
			self.say("Echo out.")
			time.sleep(1)
			self.send("PART %s" % c)


	def begin(self):
		print 'Welcome to echo.\n'
		s = raw_input('Please type in your oauth token here:')
		if (s.find('oauth:') != -1):
			self.password = s
		else:
			self.password += s

def main():
	try:
		Kappa()
	except KeyboardInterrupt:
		Kappa.quit()
	sys.exit(0)

if __name__ == "__main__":
	main()
