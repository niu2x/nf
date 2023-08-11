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
