local a = 1  
local b = 2
if(b == a){
	print("b == a")
	print('s')
}
else{
	print("b != a")
	if(1 == 1){
		print("1 == 1")
		print("1 == 1")
	}
	else{
		print("b != a")
	}
}

a = 2
b = 1 + 1


if(b == a){
	print("b == a")
	if(b+2-1 == a+1) {
		print("b+2-1 == a+1")
		print("b+2-1 == a+1")
	}

	if(b*8 == a+a+a+a+a+a+a+a) {
		print("b*8 == a+a+a+a+a+a+a+a")
		print("b*8 == a+a+a+a+a+a+a+a")
	}

	if(b*8 != a+a+a+a+a+a+a+a) {
		print("b*8 != a+a+a+a+a+a+a+a")
		print("b*8 != a+a+a+a+a+a+a+a")
	}

	if(b*1 != a+a+a+a+a+a+a+a) {
		print("b*1 != a+a+a+a+a+a+a+a")
		print("b*1 != a+a+a+a+a+a+a+a")
	}
}
else{
	print("b != a")
	if(1 == 1){
		print("b == a")
		print("b == a")
	}
	else{
		print("b != a")
	}
}



locaf f

if(2>11) {
	local x = 1
	f = function(){
		return x;
	}
}
else {
	
	local x = 2
	f = function(){
		return x;
	}

}

print(f())