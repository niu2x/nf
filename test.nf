local g = function(n) 
	local add = function() 
		return 1 + n;
	end;
	return add;
end;


local add_1 = g(2);
local add_2 = g(3);

print(add_1());
print(add_2());
