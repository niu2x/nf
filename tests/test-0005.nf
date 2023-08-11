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


local a = function(arg_1)
	local b = function(arg_2)
		local c = function(arg_3)
			arg_1 = arg_1 + 1;
			arg_2 = arg_2 + 1;
			arg_3 = arg_3 + 1;
			return arg_1 + arg_2 + arg_3;
		end;
		return c;
	end;
	return b;
end;

local f = a(1)(2);
local g = a(2)(3);

print(f(0));
print(f(0));
print(f(1));


print(g(0));
print(g(0));
print(g(1));
