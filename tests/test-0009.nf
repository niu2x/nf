local a = 1
local b = 1
local n = 0

while( n < 20) {
	print(a)
	local tmp = a;
	a = b
	b = tmp+b
	n = n + 1
}