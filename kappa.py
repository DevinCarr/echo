import socket
import time
import sys

class Kappa:
	socket = None
	connected = False
	nickname = 'feedthespam'
	password = 'oauth:'
	channel = '#'
	watcher = None
	frequency = 4

	def __init__(self):
		self.begin()
		self.watcher = Watch(self.frequency)
		self.socket = socket.socket()
		self.socket.connect(('irc.twitch.tv', 6667))
		self.send("PASS %s" % self.password)
		self.send("NICK %s" % self.nickname)

	def start(self):
		while True:
			buf = self.socket.recv(4096)
			lines = buf.split("\n")
			for data in lines:
				data = str(data).strip()

				if data == '':
					continue
				#print "<", data

				# Server Ping/Pong response
				if data.find('PING') != -1:
					print '< PING'
					n = data.split(':')[1]
					self.send('PONG :' + n)
				if self.connected == False:
					self.perform()
					self.connected = True

				args = data.split(None, 3)
				# Check for PRIVMSG's not server messages
				if len(args) != 4:
					continue

				if 'PRIVMSG' not in args[1]:
					continue

				# Add every message that comes into the queue
				msg = args[3][1:]
				self.watcher.add(msg)

				# Watch for spamming and then join the fun
				spam = self.watcher.check()
				if spam != None:
					print 'Got some spam: ', spam
					print 'Joining in..'
					self.say(spam,self.channel)

	def send(self, msg):
		print '>', msg
		self.socket.send(msg+"\r\n")

	def say(self, msg, to):
		self.send("PRIVMSG %s :%s" % (to, msg))

	def perform(self):
		self.send("JOIN %s" % self.channel)
		# say hello to every channel
		self.say('Are we ready? Kappa', self.channel)

	def quit(self):
		print 'Terminating echo..'
		self.say('Echo out.', self.channel)
		time.sleep(1)
		self.send("PART %s" % self.channel)

	def begin(self):
		print 'Welcome to echo.\n'
		print 'We will now be operating at a spam frequency of 1/%s' % self.frequency
		s = raw_input('\nPlease type in your oauth token here: ')
		if (s.find('oauth:') != -1):
			self.password = s
		else:
			self.password += s

		self.channel += raw_input('Channel to join: ')
		print 'Let\'s Begin.'

class Watch:
	spamQueue = []
	size = 0
	repeat = 0
	timeout = -1
	timeoutLimit = 10
	delay = False
	wait = 0

	def __init__(self,maxSize=1):
		self.spamQueue = []
		self.size = maxSize*10
		self.repeat = self.size / 10
		self.wait = self.repeat

	def add(self,msg):
		current = len(self.spamQueue)
		if current < self.size or current == 0:
			self.spamQueue.append(msg)
		else:
			self.spamQueue.pop(0)
			self.spamQueue.append(msg)

	def check(self):
		# Wait a bit if we have already contributed
		l = len(self.spamQueue)
		if self.timeout == -1:
			if self.wait > 0:
				if self.spamQueue.count(self.spamQueue[l-1]) > self.repeat:
					self.timeout = time.time()
					self.delay = True
					self.wait = self.repeat
					return self.spamQueue[l-1]
			else:
				self.wait -= 1
			# else wait for spam to happen
		elif (time.time() - self.timeout) > self.timeoutLimit:
			self.timeout = -1
			return None
		else:
			if len(self.spamQueue) != 0:
				self.spamQueue.pop(0)
			if self.delay:
				print 'Gonna wait a bit to not get auto-banned.'
				self.delay = False

def main():
	k = Kappa()
	try:
		k.start()
	except KeyboardInterrupt:
		k.quit()
	sys.exit(0)

if __name__ == "__main__":
	main()
