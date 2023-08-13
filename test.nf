local a = 1
print(a)

local a = {2;}
print(a)

local b = {
	local a = 123 
	a*2
}

print(a)
print(b)

if(a > b)
	print("a > b")

if(b>a)
	print("b>a")
