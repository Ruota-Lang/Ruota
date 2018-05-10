![RUOTA](https://raw.githubusercontent.com/Agilulfulus/Ruota/master/LogoIdeas/LogoBanner.png)

# Ruota Programming Language

Side project of mine - all features below (unless otherwise specified) should work. Unfortunately, it is a little slow right now during runtime, but as the language and interpreter improves I hope that I may get it to the speed of other major interpreted languages.

Ruota is a high-level abstract weakly-typed langauge. Variable types are, where applicable, implicitly converted. The language is both functional and object-oriented. The essential foundation is that everything, including functions, are variables and are stored as such. "Objects" or "Classes" as they may be called are essentially scopes and functions (variables) that are cloned and manipulated across mutliple instances. 

The language is designed to give a lot of freedom to the programmer in regards to the format, style, and functionality. In addition, the overall goal of Ruota is to make available an extensible and efficient interpreted code that can be embedded into any application. I also hope that the language may be more intuitive for mathematics while style mantaining a C-style foundation.

At the moment, the language has no implementations outside of the command line.

# Contact

Pop in here: https://discord.gg/UCWbyb9

# Planned Features

There are, of course, a few features that I wish to see implemented in the future:

1.	Proper memory management (moving away from using std::shared_ptr) and overall optimization
2.	Ruota to C++/C compiler
3.	Inline C++/C code - probably will come with point 2
4.	DLL Loading for more usability (i.e., a curses module, graphicals module, etc.)
5.	Full fledged documentation

# Basic Syntax

Variables are declared using `:=`, but once declared they can be set using `=`. Using `:=` again would reset the variable's declaration.

There is no typing system, types are automatically assigned based on context:

	>>>	a := 1;					# a is of type 'number'
	>>>	a := [1,2,3];			# a is of type 'array'
	>>>	a := new A;				# a is of type 'object' (prototyped by the structure A)
	>>>	a := "Hello";			# a is of type 'string' (strings are arrays, they can be indexed and iterated with restrictions: an indexed value of a string can not be set to any value other than a char)
	>>>	a := 'H';				# a is of type 'char'
	>>>	a := (x) -> { x + 1 };	# a is of type 'lambda'
	>>>	a := null;				# a is of type (and value) 'null'
	>>> a := struct { x:=1, y:=2 };	# a is of type 'object' with properties x and y, equal to 1 and 2 respectively

Functions can be declared anonymously:

	(args) -> { function };
	(x) -> { x + 1 };

They may also be assigned a token:

	f(x) => { x + 1 };
	f := (x) -> { x + 1 };

The `=>` operator is essential shorthand for declaring a function, notice that the examples above have identical results. There is a minor difference however, the `=>` operator cannot be used to overwrite a variable/function that has already been declared.

Objects are declared with the following syntax:

	static/dynamic ObjectName :: {
		...properties...
	};
	static A :: {
		value := 0;
		square(x) => { x ** 2 };
	};

There are two types of objects, `static` and `dynamic.` Static objects cannot be instantiated with `new,` whereas dynamic objects can only be instantiated. In the above example, one may do `A.value` and recieve no errors, but if A was declared like so:

	dynamic A :: {
		...
	};

Then A cannot be indexed for its properties. However, declaring an object instantiation of A allows you to use a copy of the object.

	a := new A;
	a.value = 10;

If A were static, the opposite would be true. Therein only `A.value` can be accessed, `a = new A` would return invalid syntax.

	static A :: {
		...
	};

Objects can within themselves hold other objects:

	static A :: {
		a_value := 0;
		static B :: {
			b_value := 0;
		};
	};

Accessing `B` here requires only `A.B`.

# Lists and Loops

Lists/Arrays are 0-indexed.

There is a foreach loop, which operates through the use of the `:` operator. This operator denotes the boundaries of an exclusive (up to) range:

	a := [ 0 : 10 ];		# [0,1,2,3,4,5,6,7,8,9]

It can be further narrowed by specifying a step:

	a := [ 0 : 10 : 2 ];		# [0,2,4,6,8]

If one wished to specify an _inclusive_ range, the operator `:>` is available:

	a := [ 0 :> 10 ];		# [0,1,2,3,4,5,6,7,8,9,10]

Lists can be further specifed using the `->>` operator. It follows the syntax `a ->> b`, where `a` is a list of input values, and `b` is a lambda with a single input, denoting how the input values are transformed.

	a := [ 1 :> 10 ] ->> ( (x) -> { x ** 2 } );	# [1,4,9,16,25,36,49,64,81,100]

Putting this all together, we can use the Foreach loop, with the `in` operator:

	i in [ 0 : 10 ] do {
		...
	};
	iterator in list do {
		...
	};

The list may be specified using any means, the only part that must remain is the iterator variables, in the above example: i. The iterator is mutable from within the loop.

The while loop is contructed similarily:

	a == true do {
		...
	};

	statement do {
		...
	};

# If-Then Statements

If-then statements use the keywords `then` and `else`.

	statement then ... ;

	statement then ... else ... ;

	statement then {
		...
	} else {
		...
	};

	statement then {
		...
	} else statement_2 then {
		...
	} else {
		..
	};

# Notable Operators

	a ** b		The exponential of `a` to the power of `b`
	len a		Size of array `a`
	num a		Numerical value of string-array `a`
	chr a		Char value of number `a`
	str a		String value of `a`
				condenses array into a string format
	arr a		Array value of string `a`
				`str (arr "Hello World" + 1)` adds 1 to each char in the string and then converts it back to string
	a push b	Pushes value `b` to the end of `a`
	a post b	Pushes value `b` to the front of `a`
	pop a		Pops value from end of `a`
	mov a		Pops value from front of `a`
	end a		Returns value from end of `a`
	a ++ b		Array concatanation of `a` and `b`
	a .. b		String concatanation of `a` and `b`
	a &= b		`a` refers to `b` - anything done to `a` is done to `b`
				The syntax `a .= null` removes the connection
	a :&= b		Declare `a`, then refer it to `b`
	type a		Returns a string-array declaring the type of data `a` holds
