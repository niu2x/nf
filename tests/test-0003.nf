local x = 3
local y = 1

local g = function() {
	local x = 3
	x = 100
	return y+x+4
}

print(x)
print(g())
print(x)
y = 2
print(g())
print(x)
print(y)
