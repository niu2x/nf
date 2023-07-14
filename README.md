# Design

chunk := stmts

stmts:= 
	| stmt1s

stmt1s:= stmt semi

semi:= 
	| ';'

stmt := 'local' varname optional_var_init
	| single_value operation
optional_var_init := |
	| '=' expr

