# Loops and Arrays

Arrays in Ruota are always declared with square brackets, not curly brackets like C-based languages.

	[0, 1, 2, 3, 4];

Arrays are, like C-based languages, 0-based. The first element is at index 0, then 1, etc., ending at the last element being the size of the array minus 1.

This design choice was made in accordance to how arrays can be declared, specifically, with the range `:` operator:

	[0 : 5];		# [0,1,2,3,4]

Arrays, of course, can also be declared with `:>`, which includes the last index:

	[0 :> 5];		# [0,1,2,3,4,5]

The above code generates an exclusive array from 0 to 5, which can then be used in for-loops like so:

	i in [0 : 5] ->> {
		...
	};

The for-loop will assign each value in the array to `i`, then execute the inner code. `i` will always refer to the exact data point within the array, so if the array happened to be a variable:

	a := [0 : 5];
	i in a ->> {
		...
	};

Then the values are mutable from within the for-loops execution.

Putting all this together, let's learn how to make our first function, a sum function if you will. This function will have one parameter, the array we're summing, and one output, the sum.

To begin, we first have to declare the name of our function. Let's call it `sum`:

	sum(array) =>

This is how a function is declared, the function name comes first, then the parameters in parantheses, and then the `=>` operator, designating that the code following it should be assigned to the function.

So let's begin writing the interior of the function:

	sum(array) => {

	};

Here we're using curly-brackets to designate the function's inner scope. All the commands of the function will be put within.

We also need to designate a variable to return, to do that we'll use the `from` keyword:

	sum(array) => s from {

	};

The `from` keyword pulls a variable from inside the scope, allowing us to designate the value we'd like to recieve.

Next we can begin writing the _actual_ meat of the function:

	sum(array) => s from {
		s = 0;
		i in array ->> {
			s = s + i;
		};
	};

Notice that we initialize `s` to 0, which is not necessary but helps to establish that it is indeed a numerical value.

As for the for-loop, it follows the same mechanics we saw above. The `i` is the iterator value, and the `array` variable is what we're indexing. So it goes through each value in `array`, assigns it to `i`, and then executes the interior, which takes `s` and adds `i` to it.

That is the complete function! Always remember to test it out to make sure it works!

	sum([1,2,3])		# 6
	sum([1:5])			# 10
	sum([1:>5])			# 15
	sum("Hello World")	# 1052 - Remember that strings are just arrays of chars
						# "Hello World" in this case is [72, 101, 108, 108, 111, 32, 87, 111, 114, 108, 100]