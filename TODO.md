# Public TODO List
This is a list of tasks that need to be done and hopefully will be a guide for anyone who wants to actually work on something.

The order does not correlate to difficulty.  Just look through everything and find something that sounds interesting to work on!

## Utility Library Overview
A library is something that should be accessed with the `use` keyword.  An example script using the yet-to-be-written `math` library might be:
```
use math;

print("Pi is ");
print(math.pi);
print("\n");
print("sin(4) = ")
print(math.sin(4));
print("\n");
```

## Library Concepts
In order to understand the Library implementation, you must understand the problem that it is trying to solve.

Remember that the use case for Tang is that a program may have *thousands* of templates.  Depending on the templates, the program may want some templates to have access to a library that is not available to other templates (for security reasons).  I implemented a 3-tiered approach.
  1. **Language libraries** - There should be a standard set of libraries that are always available by default as part of the language itself, such as `math`.  There should, therefore, be a registry of libraries that are shared by all scripts/templates.  This is implemented on the Language object and is populated when the Language object is created.
  2. **Program libraries** - There should be a way to declare libraries which `program` objects may consult when a library load is requested, but that are not available to other Program objects.  These libraries are stored on the Program object itself.
  3. **Execution-specific libraries** - There must be a way to declare libraries on a particular program execution.  This is the most granular option, and these libraries are stored on the Execution Context object.

Understand that there are 2 uses for Libraries in Tang.
  1. Generic functions or constants, such as a `math` library.
  2. To provide external **variables** to the script execution (as demonstrated many times in `test-tangLanguageExecuteSimple.cpp`).  Such variables may exist in any of the 3 scopes mentioned above.

External variables are supplied to the program via a Library.  It may seem "weird" to think of external variables being implemented as a type of library, but the weirdness is not noticed by the end user.

Examples of the 3 Library applications:
  1. **Language libraries** - already mentioned.  Examples include `math` and `random`.
  2. **Program libraries** - Imagine a template for an article.  `Article` may be a library containing information about the article, but it doesn't need to be available to a template for an image.
  3. **Execution-specific libraries** - This is a library that may be present or not depending on factors related to the specific execution context, or perhaps a library that changes behavior based on some external factor.

## New Libraries To Implement
These are not necessarily stand-alone (e.g., `date`).  Libraries should make use of singleton objects, if possible.

### `math`
**Started.**  This library should contain common math constants and trigonometric functions.  If a function can be supplied as an attribute on an object, then that is preferred.

For example, a `ceil` or `floor` is always associated with a value, and could be added to the integer and float attributes rather than putting it into the `math` library.  Note that this is a departure from most other languages.

We also need to come up with a representation of `+ infinity`, `- infinity`, and perhaps `NAN`.

(Strikethrough indicates that feature is added.)
  * ~~`pi` - Provide the constant.~~
  * `e` - Provide the constant.
  * `ln(x)` - Calculate the natural log of a number.
  * `lg(x)` - Calculate the log base 2 of a number.
  * `log(x,base)` - Calculate the natural log of a number.
  * `pow(x,y)` - Calculate `x ^ y`.
  * `degrees(x)` - Convert `x` radians to degrees.
  * `radians(x)` - Convert `x` degrees to radians.
  * Trigonometric functions (`x` in radians)
    * `acos(x)`
    * `asin(x)`
    * `atan(x)`
    * `cos(x)`
    * `sin(x)`
    * `tan(x)`
  * Hyperbolic functions
    * `acosh(x)`
    * `asinh(x)`
    * `atanh(x)`
    * `cosh(x)`
    * `sinh(x)`
    * `tanh(x)`
  * `lerp(a,b,t)` - Compute a linear interpolation `a + t(b-a)`.

### ~~`random`~~

*Done.  Mersenne Twister random library added to CUtil, and global/local RNG object libraries added.*

~~Should probably be implemented as a [Mersenne Twister](https://en.wikipedia.org/wiki/Mersenne_Twister), although I am open to suggestions.  Features desired:~~

  * ~~Request a random number from a global RNG (seed set at startup).~~
  * ~~Request a custom generator with a specific seed (which will be a new `ComputedValue` type).~~
  * ~~Request the next random `float` or `int` from the generator (either global or custom).~~
  * ~~Request a `unit` random value (a `float` from `0` to `1`) from the generator (either global or custom).~~
  * ~~The global generator will need Mutex protection.  See the [Ghoti.io/cutil](https://github.com/Ghoti-io/CUtil) library for a cross-platform mutex implementation.~~

### `date`
Date/time functions are going to take some research.  They are evil.  I can see at least 2 fundamental parts of a proper date library:
  1. A way to represent dates/times with a timezone.  Need to consider historical calendars, historical dates (before the Unix epoch).
  2. A way to represent intervals of time (years, months, days, etc.).

See [Falsehoods programmers believe about time](https://gist.github.com/timvisee/fcda9bbdff88d45cc9061606b4b923ca).

### `color`
Perhaps a library for dealing with color values (a template might have to generate color codes!).  RGB, but maybe also RGBA or HSV.  Perhaps a LERP from a color ramp?  Color mixing?

## Attributes Needed On Existing Object Types
An attribute on an object can provide a function, but if the function must then be immediately called without arguments, then it would be better for the attribute to return the value that would result.

In other words, don't do ```"abc".length()```, but ```"abc".length```.  Don't do ```(3.5).ceil()```, but ```(3.5).ceil```.

### Integer/Float
  * `ceil` - Return the ceiling of the number.
  * `floor` - Return the floor of the number.
  * `round` - Return the rounded value of the number.
  * `abs` - Return the absolute value of the number.
  * `sqrt` - Calculate the square root of the number.
  * `format(pattern)` - Convert a number to a string using the supplied pattern.  Suggest using the `printf` formatting standard.

### String
We are not currently respecting the `GTA_UNICODE_STRING_TYPE_TRUSTED`, `GTA_UNICODE_STRING_TYPE_HTML`, and `GTA_UNICODE_STRING_TYPE_PERCENT` string specifiers.  `TRUSTED` strings should output as-is (the current behavior).  `HTML` should be HTML encoded.  `PERCENT` should be URL encoded (a.k.a. percent encoded).
  * `rendered` - Should provide a `TRUSTED` version of the string, with the proper encoding applied.
  * `percent` - Should provide a copy of the string with the encoding set to `PERCENT`.
  * `html` - Should provide a copy of the string with the encoding set to `HTML`.

## Language Enhancements (involves use of `flex` and `bison`)
These are just nice-to-have, ergonomic enhancements to the language:
  * Number seperators: Many modern languages support an underscore as part of the numbers to make it easier to visually gauge values.

    Example: `10000000.00001` vs `10_000_000.000_1`

## More to come...
