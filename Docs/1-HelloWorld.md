# Hello World Program

The classic _Hello World_ program is very simple to implement! First, we'll need to created a file. Anyways is fine. I'll call mine `HelloWorld.ruo`.

Once you've got the file ready, open it up in your text editor of choice!

The first line will be the command to load a library, in specific, we'll be loading the _System_ library, which can be found in `RuotaCode/` of this repo. All it needs is to be in the same directory as the `HelloWorld.ruo` file. The command is as follows:

	load "System.ruo";

Remember, every line needs to end with a semi-colon (;)! Otherwise, the syntax might fail to evaluate.

The _System_ file/library has various functions that are essentially required for most programming tasks. One of these is the _print_ function, which outputs a sequence of text on the console.

The _print_ function is located within the _Console_ object, declared in the first line of the library. To use it, you call it like so:

	Console.print("Hello World!");

And that's all, quite simple! The full code looks like this:

	load "System.ruo";
	Console.print("Hello World!");

To run it, execute the `ruota.exe` interpreter with the filename as an argument. It should look something like this:

	.\ruota.exe HelloWorld.ruo

And that's it! Should run quite well.