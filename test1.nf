local a = 1;
local f;

block
	local a = 2;
	f = function()
		return a + 2;
	end;
	print(f);
	print(f());
end;

print(f);
print(f());
