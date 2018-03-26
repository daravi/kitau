class Crane(object):
	"""docstring for Crane"""
	def __init__(self):
		self.x_pos = 0
		self.y_pos = 0
		self.x_limit = 0
		self.y_limit = 0
		self.x_speed = 0
		self.y_speed = 0
		self.running = False
		self.engaged = False
		self.magnet_lvl = 0
	
	def set_x_limit(self, limit):
		self.x_limit = limit

	def set_y_limit(self, limit):
		self.y_limit = limit

	def set_x_speed(self, speed):
		self.x_speed = speed

	def set_y_speed(self, speed):
		self.y_speed = speed

	def start(self):
		self.running = True

	def stop(self):
		self.running = False

	def set_x(self, x_pos):
		if (x_pos > self.x_limit):
			raise Exception("X Position is out of bounds.")
		else:
			print("dsa")

	def send(self):
		str_cmd = ( str(int(self.running)) # 1 bit 
				  + bin(self.x_speed)[2:]  # four bits
				  + bin(self.y_speed)[2:]  # four bits
				  + bin(self.x_pos)[2:]    # four bits
				  + bin(self.y_pos)[2:]    # four bits
				  + "000000000000000"      # 15   bits reserved
				  )

		bin_cmd = int(str_cmd, 2)
		print(str_cmd)


c = Crane()
c.send()










