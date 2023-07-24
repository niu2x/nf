# Design

chunk := stmts

stmts:= 
	| stmt1s

stmt1s:= expr semi

semi:= ';'

expr := 'local' symbol maybe_var_init
	| add_sub_elem '+'/'-' add_sub_elem 
	| symbol '=' expr
	| table_slot '=' expr


table_slot = expr '[' expr ']'

maybe_var_init := 
	| '=' expr


add_sub_elem := mul_div_elem '*'/'/'/'%' mul_div_elem

mul_div_elem := '-' mul_div_elem_no_prefix
	| '#' mul_div_elem_no_prefix


mul_div_elem_no_prefix := symbol



local symbol
symbol
Integer
Number
Table
String

=

-
+

/
*
%


#
-


()
[]