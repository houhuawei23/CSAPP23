with open("export_results.txt") as f:
	data = f.read()

base = []
for i in range(0, len(data), 4):
	base.append(int(data[i+2:i+4] + data[i:i+2], 16))

for i in range(1, 127):
	if base[i] & 2:
		print(chr(i), end='')