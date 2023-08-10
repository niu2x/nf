local a = 1;
local f;

block
	local a = 101;
	f = function()
		a = a + 1;
		return a + 2;
	end;
	print(f);
	print(f());
end;

print(f);
print(f());
