local g = function(n) 
	local add = function() 
		n = n + 1;
		return n+1;
	end;
	return add;
end;


local add_1 = g(1);
local add_2 = g(2);

print(add_1());
print(add_1());
print(add_1());


print(add_2());
print(add_2());
print(add_2());


local a = 1;
local f;

block
	local a = 101;
	f = function()
		a = a + 1;
		return a + 2;
	end;
	print(f());
	print(f());
end;

print(f());
print(f());
