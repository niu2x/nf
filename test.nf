local add = function(a)
	return function(k)
		return a+k;
	end;
end;

add = add(2);

print(add(1));