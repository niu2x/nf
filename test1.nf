local a = 1;
print(a);
local f;

block
	local a = 2;
	f = function()
		return a + 2;
	end;
end;

print(a);
