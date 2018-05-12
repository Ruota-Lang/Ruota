#ADDITION-BASED (+)
`a + b`		Adds `a` and `b`
			a:	number, array, char, string (>number)
			b:	number, array, char, string (>number)
`a += b`	Equivalent to `a = a + b`
			a:	number, array, char, string (>number)
			b:	number, array, char, string (>number)
`a .+ b`	Adds arrays `a` and `b` element-wise
			a:	array
			b:	array
`a .+= b`	Equivalent to `a = a .+ b`
			a:	array
			b:	array
#SUBTRACTION-BASED (-)
`- a`		Negates `a`; equivalent to `0 - a`
			a:	number, array, char, string (>number)
			b:	number, array, char, string (>number)
`a - b`		Subtracts `a` and `b`
			a:	number, array, char, string (>number)
			b:	number, array, char, string (>number)
`a -= b`	Equivalent to `a = a - b`
			a:	number, array, char, string (>number)
			b:	number, array, char, string (>number)
`a .- b`	Subtracts arrays `a` and `b` element-wise
			a:	array
			b:	array
`a .-= b`	Equivalent to `a = a .- b`
			a:	array
			b:	array
#MULTIPLICATION-BASED (*)
`a * b`		Multiplies `a` and `b`
			a:	number, array, char, string (>number)
			b:	number, array, char, string (>number)
`a *= b`	Equivalent to `a = a * b`
			a:	number, array, char, string (>number)
			b:	number, array, char, string (>number)
`a .* b`	Multiplies arrays `a` and `b` element-wise
			a:	array
			b:	array
`a .*= b`	Equivalent to `a = a .* b`
			a:	array
			b:	array
#DIVISION-BASED (/)
`a / b`		Divides `a` and `b`
			a:	number, array, char, string (>number)
			b:	number, array, char, string (>number)
`a /= b`	Equivalent to `a = a / b`
			a:	number, array, char, string (>number)
			b:	number, array, char, string (>number)
`a ./ b`	Divides arrays `a` and `b` element-wise
			a:	array
			b:	array
`a ./= b`	Equivalent to `a = a ./ b`
			a:	array
			b:	array
#POWER-BASED (**)
`a ** b`	Raises `a` by the power of `b`
			a:	number, array, char, string (>number)
			b:	number, array, char, string (>number)
`a **= b`	Equivalent to `a = a ** b`
			a:	number, array, char, string (>number)
			b:	number, array, char, string (>number)
`a .** b`	Raises array `a` by the power of array `b` element-wise
			a:	array
			b:	array
`a .**= b`	Equivalent to `a = a .** b`
			a:	array
			b:	array
#MODULUS-BASED (%)
`a % b`		Modulates `a` by `b`
			a:	number, array, char, string (>number)
			b:	number, array, char, string (>number)
`a %= b`	Equivalent to `a = a % b`
			a:	number, array, char, string (>number)
			b:	number, array, char, string (>number)
`a .% b`	Modulates arrays `a` by `b` element-wise
			a:	array
			b:	array
`a .%= b`	Equivalent to `a = a .% b`
			a:	array
			b:	array
#INDEXING
`a[b...]`	Indexes array `a` by any values within []
			a:	array, string
			b:	number, char, string (>number)
#DECLARATION-BASED (=)
`dec a`		Declares variable `a`
`a = b`		Sets value `a` to value `b`
			b:	all
`[a...] = [b...]`	Extension of `=`, sets all values element-wise
			b...:	all
`a := b`	Equivalent to `dec a = b`
			b:	all
`a &= b`	Refers `a` to `b`
`a :&= b`	Equivalent to `dec a &= b`
#CONCATANATION-BASED (++/..)
`a ++ b`	Concatanates arrays `a` and `b`
			a:	array
			b:	array
`a .. b`	Concatanates strings `a` and `b`
			a: number (>string), array (>string), char (>string), string
			b: number (>string), array (>string), char (>string), string
`a ++= b`	Equivalent to `a = a ++ b`
			a:	array
			b:	array
`a ..= b`	Equivalent to `a = a .. b`
			a:	number (>string), array (>string), char (>string), string
			b:	number (>string), array (>string), char (>string), string
#EQUIVALENCY-BASED (==)
`!a`		Negates `a` logically
			a:	number, char (>number), string (>number)
`a == b`	Returns equality of `a` and `b`
			a:	all
			b:	all
`a != b`	Equivalent to `!(a == b)`
			a:	all
			b:	all
`a > b`		Returns whether `a` is greater than `b`
			a:	number, array, char, string (>number)
			b:	number, array, char, string (>number)
`a .> b`	Element-wise `>`
			a:	array
			b:	array
`a >= b`	Returns whether `a` is greater than `b` or equal to `b`; equivalent to `a == b || a > b`
			a:	number, array, char, string (>number)
			b:	number, array, char, string (>number)
`a .>= b`	Element-wise `>=`
			a:	array
			b:	array
`a < b`		Returns whether `a` is less than `b`
			a:	number, array, char, string (>number)
			b:	number, array, char, string (>number)
`a .< b`	Element-wise `<`
			a:	array
			b:	array
`a <= b`	Returns whether `a` is less than `b` or equal to `b`; equivalent to `a == b || a < b`
			a:	number, array, char, string (>number)
			b:	number, array, char, string (>number)
`a .<= b`	Element-wise `<=`
			a:	array
			b:	array
#ARRAY OPERATIONS
`len a`		Returns length of array `a`
			a: string, array
`end a`		Returns last value of array `a`; equivalent to `a[len a - 1]`
			a: string, array
`a push b`	Equivalent to `a ++= [b]`
			a: array
			b: all
`a post b`	Equivalent to `a = [b] ++ a`
			a: array
			b: all
`pop a`		Removes last element of `a`
			a: array
`mov a`		Removes first element of `a`
			a: array