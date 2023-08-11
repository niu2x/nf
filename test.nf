local a = function(arg_1)
	local b = function(arg_2)
		local c = function(arg_3)
			return arg_1 + arg_2 + arg_3;
		end;
		return c;
	end;
	return b;
end;

print(a(1)(2)(3));