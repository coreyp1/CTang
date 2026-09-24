# The Tang language

**Status:** Specification of the language as implemented at this commit,
with the intended behaviour stated where the implementation departs from it.
Every departure is listed in section 13 with a one-line reproduction, and
each one is a test waiting to be written.

**Version:** none yet. The language is unstable; this document is the thing
that changes first when it does.

This is the reference for *the language*. The embedding API is covered only
as far as a template author needs to understand what the host can and cannot
see; `include/tang/` is the reference for the rest.

---

## 1. Overview

Tang is a template language for embedding in a host program, aimed at
generating HTML. It is loosely typed and garbage collected, and it compiles at
run time either to x86-64 machine code or to bytecode for a virtual machine.
**The two backends are required to agree**: any program that produces
different results under `GTA_PROGRAM_FLAG_DISABLE_BINARY` and
`GTA_PROGRAM_FLAG_DISABLE_BYTECODE` is a defect in one of them, and the test
suite runs under both for that reason.

Three things make it different from the languages it resembles:

- **Strings are sequences of graphemes**, not bytes or code points. Indexing,
  slicing and `length` count user-perceived characters. `"🏴󠁧󠁢󠁳󠁣󠁴󠁿".length`
  is 1; its `byte_length` is 28.
- **Every string carries an output encoding**, and the output buffer is a
  sequence of typed segments. HTML-escaping is not something a template
  remembers to do; it is a property of the string that the host applies when
  it renders.
- **The host supplies every external value through a library**, resolved
  lazily by name at execution time. There is no file system, network, or
  clock unless the host provides one.

A Tang source is either a **script** or a **template**. The host chooses
which (`GTA_PROGRAM_FLAG_IS_TEMPLATE`). A script is code. A template is
literal text with code embedded in tags, and the text outside the tags is
output as-is.

---

## 2. Lexical structure

### 2.1 Encoding

Source is UTF-8. String literals may contain any UTF-8 directly, including
newlines.

### 2.2 Template mode

In a template, the scanner starts in text mode. Everything is literal output
until one of two tags:

| Tag | Meaning |
| --- | --- |
| `<% ... %>` | code. Statements, exactly as in a script. |
| `<%= expression %>` | print. Equivalent to `<% print(expression); %>`. |

A `%>` returns to text mode. Tags may not nest. Text mode ends at end of
input; an unterminated `<%` is not an error, and the code inside it is
compiled as if it were closed.

Literal text is output with the `TRUSTED` encoding (section 8): a template
author's own markup is never escaped. Only values reaching `print` are.

`//` and `/* */` are comments only inside code tags. In text mode they are
text.

There is no comment tag. `<%# ... %>` is a syntax error.

### 2.3 Script mode

The whole source is code. A `%>` outside any tag is an error
(`UNEXPECTEDSCRIPTEND`).

### 2.4 Whitespace and comments

Spaces, tabs and newlines separate tokens and are otherwise ignored.
`// ...` runs to end of line; `/* ... */` may span lines and does not nest.

### 2.5 Identifiers and keywords

An identifier is `[a-zA-Z_][a-zA-Z0-9_]*`. Identifiers are case-sensitive.

Reserved words, which may not be used as identifiers:

```
as  bool  break  continue  do  else  false  float  for  function  global
if  int  null  print  return  string  true  use  while
```

`bool`, `int`, `float` and `string` are cast targets; they are reserved
everywhere, not only after `as`.

### 2.6 Literals

**Integer**: one or more decimal digits. No sign (unary minus is an
operator), no separators, no hexadecimal, octal or exponent forms. A literal
that does not fit in 64 bits is a **syntax error** - it is refused rather than
saturated, since a literal cannot vary and so has nothing to wait for.

Because there are no negative literals and the signed range is asymmetric, the
most negative integer can only be written as unary minus applied to
`9223372036854775808`, which is itself one past the maximum. That magnitude is
therefore accepted in exactly one place - directly after unary minus - and is
a syntax error anywhere else:

```
-9223372036854775808     is the most negative integer
 9223372036854775808     is a syntax error
```

**Float**: digits with a decimal point on at least one side: `3.14`, `1.`,
`.5`. No exponent form; `1e5` is the integer `1` followed by the identifier
`e5`, which is a syntax error.

**Boolean**: `true`, `false`.

**Null**: `null`.

**String**: double-quoted. Three forms select the string's encoding (section
8):

| Literal | Encoding |
| --- | --- |
| `"..."` | `TRUSTED` - emitted as-is |
| `!"..."` | `HTML` - escaped for element content when rendered |
| `%"..."` | `PERCENT` - URL-encoded when rendered |

Escape sequences inside any string form:

| Sequence | Produces |
| --- | --- |
| `\n` `\t` `\r` `\b` `\f` | newline, tab, carriage return, backspace, form feed |
| `\ooo` | one byte, octal, one to three digits; a value over 255 is a syntax error |
| `\xHH` | one byte, hex, one or two digits |
| `\"` `\\` | the character |
| `\` followed by any other character | that character, unchanged (`\u` is `u`) |
| `\` followed by a newline | a newline |
| `\` followed by a digit sequence that is not valid octal (`\48`) | syntax error |

There is no `\u` escape for code points; write the UTF-8 directly.

Single quotes are not string delimiters; `'` is a syntax error.

### 2.7 Operators and punctuation

```
=  +=  -=  *=  /=  %=
+  -  *  /  %  !  <  <=  >  >=  ==  !=  &&  ||  ?  :  .  ,  ;
(  )  [  ]  {  }  <%  <%=  %>
```

`@` is reserved. It begins a date literal that the scanner recognises and the
parser does not yet accept (section 13.13).

---

## 3. Values and types

Tang has these value types. A variable has no type; a value does.

| Type | Literal | Mutable | Passed by |
| --- | --- | --- | --- |
| null | `null` | - | value |
| boolean | `true` `false` | - | value |
| integer | `42` | - | value |
| float | `4.2` | - | value |
| string | `"..."` | no | value |
| array | `[1, 2]` | yes | reference |
| map | `{a: 1}` | yes | reference |
| function | `function f() {}` | no | reference |
| library | via `use` | - | reference |
| rng | `random.seeded(n)` | yes | reference |
| error | produced by failed operations | - | value |

**Integer** is a 64-bit two's-complement value. Arithmetic wraps:
`9223372036854775807 + 1` is `-9223372036854775808`.

**Float** is an IEEE-754 double.

**String** is an immutable sequence of graphemes with an encoding tag.
Operations that would modify a string return a new one. Assigning through an
index (`s[0] = "z"`) has no effect (13.9).

**Array** is a mutable ordered sequence of values. `y = x` makes `y` refer to
the same array; `y[0] = 9` is visible through `x`. Passing an array to a
function passes the reference. `x + [3]` produces a *new* array and leaves
`x` alone.

**Map** is a mutable association from strings to values. Literal keys are
bare identifiers (`{a: 1}`); at run time keys are strings (`m["a"]`). A
trailing comma is permitted in the literal. The empty map is `{:}` - `{}` is
an empty block.

**Function** values are first-class: they can be assigned, passed and
called through any expression that evaluates to one.

**Error** values are what failed operations return (section 10). They are
ordinary values in that execution continues; they are falsy, and any
operation applied to one produces another error.

### 3.1 Truthiness

Where a boolean is required - `if`, `while`, `for`, `?:`, `&&`, `||`, `!`,
`as bool` - a value converts as follows:

| Value | Truthy when |
| --- | --- |
| null | never |
| boolean | it is `true` |
| integer | non-zero |
| float | non-zero (`-0.0` is zero) |
| string | non-empty (in bytes) |
| array, map | **intended:** non-empty. **Currently:** never (13.5) |
| error | never |
| function, library, rng | unspecified |

---

## 4. Expressions

### 4.1 Precedence

From loosest to tightest. Operators on one row share a level.

| Level | Operators | Associativity |
| --- | --- | --- |
| 1 | `=` `+=` `-=` `*=` `/=` `%=` `? :` | right |
| 2 | `\|\|` | left |
| 3 | `&&` | left |
| 4 | `==` `!=` | left |
| 5 | `<` `<=` `>` `>=` | left |
| 6 | `+` `-` | left |
| 7 | `*` `/` `%` | left |
| 8 | unary `-` `!` `as` | right |
| 9 | call `()` index `[]` slice `[:]` attribute `.` | left |

So `"1" as int + 1` is `("1" as int) + 1`, and `a = b = 3` assigns both.
Comparison does not chain: `1 < 2 < 3` compares a boolean with an integer,
which is an error.

### 4.2 Arithmetic: `+` `-` `*` `/` `%`, unary `-`

| Operands | Result |
| --- | --- |
| integer, integer | integer. `/` truncates toward zero (`-10 / 3` is `-3`); `%` takes the sign of the dividend (`-10 % 3` is `-1`, `42 % -10` is `2`) |
| integer, float or float, float | float, in either order |
| array `+` array | a new array, the concatenation |
| string `+` anything printable | a new string, the concatenation (4.2.1) |
| anything else | error `Not supported` |

Division or modulo by zero, integer or float, is an error (`Divide by zero`,
`Modulo by zero`). `%` on floats is `Not supported`.

Unary minus applies to integers and floats. On anything else it is an error.

**Integer arithmetic that overflows does not wrap.** `+`, `-`, `*` and unary
`-` report a result that does not fit the same way the casts do, with
`[INTEGER TOO LARGE]` or `[INTEGER TOO SMALL]` according to the direction:

```
9223372036854775807 + 1        is [INTEGER TOO LARGE]
9223372036854775807 * 2        is [INTEGER TOO LARGE]
-(0 - 9223372036854775807 - 1) is [INTEGER TOO LARGE]
```

Division overflows for exactly one pair of operands, `GTA_INTEGER_MIN / -1`,
whose quotient is one past the maximum; that is `[INTEGER TOO LARGE]` too.
Modulo does not overflow at all - `x % -1` is `0` for every `x` - and answers
`0` for that pair, even though the hardware's modulo instruction computes the
quotient and would trap on it.

Float arithmetic is unaffected: it has infinities and follows the usual
floating-point rules.

#### 4.2.1 `+` on strings

`+` concatenates when either operand is a string. The other operand is
converted to text **exactly as printing it would convert it**, so `"a" + b`
and `print("a"); print(b);` produce the same bytes:

```
"ab" + "cd"       is "abcd"
"count: " + 5     is "count: 5"
5 + " apples"     is "5 apples"
"b=" + true       is "b=true"
"a=" + [1, 2]     is "a=[1, 2]"
```

Two consequences follow from tying it to printing:

- A value that cannot be printed cannot be concatenated. `null`, a function
  and a library are errors (`Not supported`) rather than contributing nothing
  to the text.
- **An error operand is the result**, not text inside it. `"big: " +
  (9223372036854775807 + 1)` is `[INTEGER TOO LARGE]`, not the string
  `"big: [INTEGER TOO LARGE]"`. Concatenating the marker's text would make the
  result an ordinary string, and everything after it would succeed on prose
  that nothing downstream can tell apart from output the author meant.

The result is a **multi-segment string, each segment keeping its own tag**
(section 8), which is why concatenation joins `GTA_Unicode_String`s rather
than bytes. `"<b>" + !"<i>"` renders as `<b>&lt;i&gt;`: the trusted half is
untouched and the untrusted half is escaped, exactly as if the two had been
printed one after the other. Collapsing the result to a single tag would
either emit untrusted input unescaped or double-escape the markup around it.
Slicing the join keeps the boundary where it was.

Note that `+` is the only meaning available, so the operands' runtime types
decide whether an expression adds or concatenates: `a + 1` is `6` when `a` is
the integer `5` and `"51"` when `a` is the string `"5"`.

### 4.3 Comparison: `<` `<=` `>` `>=`

Defined between integers and floats, in any combination, with the usual
numeric meaning. Every other pairing is an error. Strings do not compare
(13.3).

### 4.4 Equality: `==` `!=`

| Operands | Result |
| --- | --- |
| integer or float, integer or float | numeric equality; `0 == 0.0` is true |
| array, array | true when the same length and every element is `==`, recursively |
| string, string; boolean, boolean; null, null | **intended:** value equality. **Currently:** `Not implemented` (13.3) |
| map, map | `Not supported` |
| different types otherwise | `Not supported` |

### 4.5 Logical: `&&` `\|\|` `!`

`&&` and `||` short-circuit and **return an operand**, not a boolean:
`3 && 4` is `4`; `0 || "x"` is `"x"`; `null || 5` is `5`.

`!` returns a boolean: the negation of the operand's truthiness.

### 4.6 Conditional: `c ? a : b`

Evaluates `c` for truthiness and yields `a` or `b`. Right-associative:
`p ? 1 : q ? 2 : 3`.

### 4.7 Cast: `expression as type`

`type` is `bool`, `int`, `float` or `string`.

| From \ To | `bool` | `int` | `float` | `string` |
| --- | --- | --- | --- | --- |
| null | `false` | `0` | `0.0` | `"null"` |
| boolean | itself | `1` / `0` | `1.0` / `0.0` | `"true"` / `"false"` |
| integer | `!= 0` | itself | exact | decimal digits |
| float | `!= 0.0` | truncate toward zero, or a marker if it does not fit (below) | itself | see 4.12 |
| string | non-empty | leading decimal integer (`"12abc"` is `12`), or a marker if there is none or it does not fit (below) | leading decimal, same rule | itself |
| array | crash (13.6) | crash | crash | crash |
| map | crash (13.6) | `Not supported` | `Not supported` | `Not supported` |
| error | - | - | - | `Not implemented` |

**A value that does not fit in an integer does not produce a number.** The
integer range is `[-2^63, 2^63)`, and a value outside it becomes an error
value that prints as a marker:

| Value | `as int` |
| --- | --- |
| at least `2^63`, or `inf` | `[INTEGER TOO LARGE]` |
| below `-2^63`, or `-inf` | `[INTEGER TOO SMALL]` |
| a NaN | `[NOT A NUMBER]` |

This applies to both sources that can exceed the range - a float and a string
- and the marker is deliberately the same for both, because it means the same
thing. Integer arithmetic that overflows reports with the same two markers
(4.2). `99999999999999999999999.0 as int` and `"99999999999999999999999" as
int` are both `[INTEGER TOO LARGE]`.

A string with **no** leading number is `[NOT A NUMBER]`, for `as int` and
`as float` alike: `"abc" as int` and `"" as int` are both `[NOT A NUMBER]`,
while `"0" as int` is `0`. Those are two different situations and used to give
the same answer. A string that merely *continues* past its number is
unaffected - `"12abc" as int` is still `12`, and leading whitespace is still
skipped.

These are error values, so they are false in a condition and arithmetic on
them yields an error, but unlike other errors they print as the marker rather
than as nothing - the alternative is not silence but a number that is untrue.

Note that `9223372036854775807.0` is `[INTEGER TOO LARGE]`: the largest
integer is not representable as a float and the literal rounds up to `2^63`,
which does not fit. The largest float that does convert is
`9223372036854774784.0`. `-9223372036854775808.0` converts exactly, being a
power of two. The string spellings of the bounds do convert, since they are
parsed as integers rather than through a float: `"9223372036854775807" as int`
and `"-9223372036854775808" as int` are both exact.

### 4.8 Index: `a[i]`

**Array** with integer `i`: element `i`, counting from zero; a negative `i`
counts from the end (`a[-1]` is the last). Out of range either way yields
`null`. A non-integer index is an error (`Invalid index`).

**String** with integer `i`: the grapheme at `i`, as a one-grapheme string,
with the same negative rule. Out of range yields `""`. A non-integer index is
an error.

**Map** with string `k`: the value for `k`, or `null` if absent. A non-string
key is an error (`Map key is not a string`).

Indexing `null` or a number is `Not supported`.

### 4.9 Slice: `a[start:end]` and `a[start:end:step]`

Arrays and strings. Semantics are Python's:

- Each part is optional. `start` defaults to the beginning (or the end when
  `step` is negative); `end` to the end (or the beginning); `step` to `1`.
- Negative `start` and `end` count from the end. Values past either end are
  clamped, so `[-20:-15]` on a short sequence is empty and `[-34::3]` starts
  at the beginning.
- `end` is exclusive.
- A negative `step` walks backwards: `[-2::-1]` is everything but the last
  element, reversed. `[::-1]` reverses.
- `step` of `0` is an error (`Invalid index`).

A slice of an array is a new array. A slice of a string is a new string.

### 4.10 Attribute: `a.name`

Reads a named attribute of a value. Which names exist depends on the type:

| Type | Attributes |
| --- | --- |
| string | `length`, `byte_length`, `html`, `html_attribute`, `percent`, `javascript`, `render`, `raw` (section 8) |
| array | `size` |
| library | its members (section 9) |
| rng | `next_int`, `next_float`, `next_bool`, `set_seed`; and five declared but unimplemented names (9.2) |
| map, integer, float, boolean, null, function | none; `Not implemented` |

Attributes are values, not calls. Something that needs no arguments is an
attribute (`s.length`, `r.next_int`), never `s.length()`. An attribute that
does take arguments is a function value, called with `()`: `r.set_seed(3)`.

Maps are **not** indexed by `.`: `m.a` is `Not implemented`; write `m["a"]`
(13.7 - and note the asymmetry with assignment in 4.13).

### 4.11 Call: `f(args)`

Calls a function value with positional arguments. The argument count must
equal the parameter count; otherwise the call yields `Argument Count
Mismatch` and the body does not run. Calling a non-function yields `Invalid
function call` (but see 13.16).

### 4.12 `print(expression)`

`print` is an expression with the syntax of a call. It appends the value to
the output and yields `null`. What each type appends:

| Type | Output |
| --- | --- |
| string | its text, tagged with its encoding (section 8) |
| integer | decimal digits |
| float | fixed notation with six decimals, trailing zeros removed, decimal point kept: `3.5`, `0.333333`, `100.`, `1.` |
| null | nothing |
| array | `[` elements separated by `, ` `]`, each element as `print` would show it, recursively: `[1, [2, 3], x]` |
| boolean | `true` / `false` |
| map | `{` entries separated by `, ` `}`, each entry a quoted key, `": "`, and the value as `print` would show it, recursively: `{"a": 1, "b": [2, 3]}` |
| function, library, rng, error | nothing |

`as string` on a float uses the same formatting, so `3.0 as string` is `"3."`.

### 4.13 Assignment: `target = expression`

Assignment is an expression; its value is the assigned value, which is what
makes `a = b = 3` work. The target may be:

- an **identifier**: binds the variable in the current scope (section 6).
- an **index** `a[i]` on an array: sets element `i`, negative indices as in
  4.8. An index at or past the end grows the array, filling with `null`
  (`a[6] = 42` on a four-element array gives seven elements). A negative
  index past the beginning is an error.
- an **index** `m[k]` on a map: sets or adds the key.
- an **attribute** `a.name`: intended for map members only (`m.b = 2` adds
  `b`). It is currently accepted on every value and on every name, with the
  consequences in 13.10.

Any other target - a slice, a call, a literal - is not a valid assignment
target. It is currently accepted and does the wrong thing (13.11).

#### 4.13.1 Compound assignment: `target += expression`

`+=`, `-=`, `*=`, `/=` and `%=`. `a += b` means exactly `a = a + b`, with the
same precedence and right-associativity as `=`, so `a += 2 * 3` adds six and
`a += b += 1` works. Everything the operator does comes from the plain binary
form: `+=` concatenates when either side is a string (4.2.1), and an overflow
is reported rather than wrapped, so

```
a = 9223372036854775807; a += 1;   leaves a as [INTEGER TOO LARGE]
s = ""; for (i = 0; i < 3; i += 1) { s += "x"; }   leaves s as "xxx"
```

**The target must be an identifier.** Unlike plain assignment, `a[i] += b` and
`a.name += b` are syntax errors; write `a[i] = a[i] + b`. The restriction is
deliberate rather than pending: desugaring an index target would evaluate the
index expression twice, so `a[f()] += 1` would call `f` twice, and a compound
assignment that silently calls a function twice is worse than one that does
not exist. A form that evaluates its target once would need the target
compiled as a reference, which is a larger change than this spelling is worth
on its own.

### 4.14 Array and map literals

`[e1, e2, ...]` evaluates its elements in order. `[]` is empty.

`{k1: e1, k2: e2,}` with bare-identifier keys; a trailing comma is allowed.
`{:}` is empty. Duplicate keys: last wins.

---

## 5. Statements

A program is a sequence of statements. Every statement except a block and a
function declaration ends with `;`. An empty statement (`;` on its own) is
not permitted.

### 5.1 Expression statement

`expression ;` - evaluated for its effect. **The value of the last statement
executed is the program's result** (section 11), so a script that ends with
`x;` returns `x`.

### 5.2 Block

`{ statements }` groups statements. **A block does not introduce a scope.**
A variable assigned inside `if` or a loop is visible afterwards.

### 5.3 `if`

```
if (condition) statement
if (condition) statement else statement
```

`else` binds to the nearest unmatched `if`. Conditions use truthiness (3.1).

### 5.4 `while` and `do`

```
while (condition) statement
do statement while (condition);
```

### 5.5 `for`

```
for (init; condition; step) statement
```

All three parts are optional; `for (;;)` loops until `break`. The loop
variable, being an ordinary variable, is visible afterwards with its final
value.

### 5.6 Ranged `for`

```
for (name : expression) statement
```

Iterates the elements of an **array**, binding each to `name` in turn. `name`
keeps the last element afterwards. Mutating the array during iteration is
permitted and affects later iterations.

`name` is bound to the element, not to a copy of it, which follows from
arrays and maps being reference types (section 3): `for (x : a) { x[0] = 9; }`
is visible through `a`. Rebinding `name` is not mutation and does not reach
the array - `for (x : a) { x = 99; }` leaves `a` alone.

Only arrays are iterable. Strings are `Not implemented`; maps and everything
else are `Not supported` (14).

### 5.7 `break` and `continue`

Inside a loop, the usual meanings, applying to the innermost loop.

Outside any loop the behaviour is defined and unusual: at top level, `break`
or `continue` **ends the program**; inside a function, either one **returns
from the function** with `null`. Neither is an error.

### 5.8 `return`

`return;` or `return expression;`. Inside a function, returns. At top level,
ends the program with the given value as its result.

### 5.9 Function declaration

```
function name(param, param, ...) { statements }
```

A statement, permitted at top level and inside other functions. Section 7.

### 5.10 `use`

```
use name;
use name.member.member as alias;
```

Binds a library, or a member reached through a dotted path, to a variable.
Section 9. A dotted path without `as` is a syntax error.

### 5.11 `global`

```
global name;
global name = expression;
```

Only inside a function: declares that `name` refers to the top-level variable
rather than a function-local one. At top level it is a compile error.
Section 6.

---

## 6. Scope

There are exactly two kinds of scope: the **program scope** and one **local
scope per function invocation**. Blocks do not create scope.

- Assigning to an identifier at top level binds it in the program scope.
- Assigning to an identifier inside a function binds it in that function's
  local scope - even if a program-scope variable of the same name exists.
  There is no implicit capture: a function body cannot read a top-level
  variable without `global`.
- `global name;` inside a function makes `name` refer to the program-scope
  variable for the rest of that function; `global name = expr;` does the same
  and assigns.
- Reading an identifier that has never been assigned yields `null`. It is not
  an error.
- A variable's binding lasts until the program ends. Nothing is ever unbound.

Function declarations bind their name in the scope where they appear. A
function declared inside another function is local to it and cannot be called
from outside.

Functions are not closures. A function sees its parameters, its own locals,
and (via `global`) the program scope. It does not see the locals of the
function that declared it.

---

## 7. Functions

```
function add(a, b) {
  return a + b;
}
```

- Parameters are bound by position. The call must supply exactly as many
  arguments as there are parameters (4.11).
- Parameter names must be distinct (13.17).
- Arrays, maps, functions and rngs are passed by reference; everything else
  by value.
- `return expr;` returns the value; `return;` or falling off the end returns
  `null`.
- Recursion is permitted. There is no depth limit (13.14).
- A function is a value. `f = add; f(1, 2)` works, as does passing one as an
  argument.
- A function must be **declared before it is called**, textually. Calling a
  name that is declared later in the source is, in the current
  implementation, a crash rather than an error (13.15). Whether declarations
  should be hoisted is an open question (14).
- Declaring a function whose name is already bound - by another function or
  by a variable - is an error at compile time (13.18).

**Native functions.** The host can expose a C function as a Tang function
value (`gta_computed_value_function_native_create`). A native function
receives the argument list and may carry a bound object, which is how
per-object methods such as `rng.set_seed` are built. Arity is the native
function's own business.

---

## 8. Strings and output encoding

Every string carries one of five encodings. The encoding says what must
happen to the text *when it is rendered*, not what has happened to it:

| Encoding | Meaning | Rendered by |
| --- | --- | --- |
| `TRUSTED` | emit as-is | nothing |
| `HTML` | element content | `<` `>` `&` → `&lt;` `&gt;` `&amp;` |
| `HTML_ATTRIBUTE` | attribute value | the above, plus `"` → `&quot;` and `'` → `&#39;` |
| `PERCENT` | URL query component | letters, digits, `-` `_` `.` `~` kept; space → `+`; everything else → `%XX` per byte |
| `JAVASCRIPT` | inside a script string | `'` `"` `\` → backslash-escaped; newline, CR, tab → `\n` `\r` `\t`; `<` `>` `&` → `<` `>` `&` |

A literal is `TRUSTED`, `HTML` or `PERCENT` according to its prefix (2.6).
Template text is `TRUSTED`. Values printed from integers and floats contain
nothing that any encoder changes.

### 8.1 String attributes

| Attribute | Yields |
| --- | --- |
| `length` | grapheme count, integer |
| `byte_length` | UTF-8 byte count |
| `html` | the same text, tagged `HTML` |
| `html_attribute` | the same text, tagged `HTML_ATTRIBUTE` |
| `percent` | the same text, tagged `PERCENT` |
| `javascript` | the same text, tagged `JAVASCRIPT` |
| `raw` | the same text, tagged `TRUSTED` |
| `render` | a **new** `TRUSTED` string containing the text *after* its encoding has been applied |

So `"a&b".html` is still the three characters `a&b`, tagged for escaping;
`"a&b".html.render` is the five characters `a&amp;b`, trusted; and
`"a&b".html.render.html` renders as `a&amp;amp;b` - double-encoding is
possible and explicit.

### 8.2 The output buffer

`print` appends the value's text to the program's output **with its tag**.
The output is therefore a sequence of typed segments, not a flat string. The
host reads it in one of two ways:

- `context->output->buffer` - the raw bytes, every segment unencoded. This is
  the wrong thing to send to a browser.
- `gta_unicode_string_render(context->output)` - every segment encoded per
  its tag. This is the thing to send to a browser.

The `tang` command-line tool and `GTA_PROGRAM_FLAG_PRINT_TO_STDOUT` write the
**raw** form (13.19).

Tagging is per segment, so `print("<b>"); print(!"<i>")` renders as
`<b>&lt;i&gt;` - and so does `print("<b>" + !"<i>")`, because concatenation
joins the segment lists rather than the bytes (4.2.1).

### 8.3 What this buys

Escaping happens at the boundary, once, driven by a property of the data.
A value that came from an untrusted source and was tagged `HTML` by the host's
library cannot be printed unescaped by accident, only by writing `.raw`,
which is greppable.

---

## 9. Libraries

A library is a named value the host makes available. `use` brings one into
scope:

```
use math;                       // math is now a variable holding the library
use math as m;                  // aliased
use math.pi as pi;              // one member, aliased (as is required)
use random.global.next_int as n; // any depth
```

### 9.1 Resolution

Libraries are looked up **by name at execution time**, in three tiers,
innermost first:

1. the execution context's library (`context->library`) - per run;
2. the program's library (`program->library`) - per compiled program;
3. the language's library (`language->library`) - shared by every program;
   this is where `math` and `random` live.

The first match wins. A library is instantiated only when a `use` for it is
executed, through the callback the host registered
(`gta_library_add_library_from_string(library, "name", callback)`), so an
unused library costs nothing.

A `use` of a name that no tier provides binds the variable to `null`. It is
not an error, and the program continues.

**External variables are libraries.** A host that wants a template to see
`user` registers a callback under `"user"` that returns the value. The
template writes `use user;`. This is deliberately the only channel in; it is
what makes the sandbox a sandbox.

The variable bound by `use` is an ordinary variable. `use a; a = 42;`
replaces it locally and does not touch the library.

### 9.2 Built-in libraries

**`math`**

| Member | Value |
| --- | --- |
| `pi` | float, π |

Everything else on the `math` list in `TODO.md` is unimplemented.
Reading an absent member is `Not implemented`, not `null`.

**`random`** - Mersenne Twister, MT19937-64. The sequence for a seed is
required to match `std::mt19937_64` with the same seed, and the tests check
it.

| Member | Value |
| --- | --- |
| `global` | the process-wide generator, seeded at startup. Shared by every program. Its seed cannot be changed. |
| `default` | a new generator with a fresh seed |
| `seeded(n)` | a new generator seeded with the integer `n` |

An `rng` value has these attributes:

| Attribute | Yields |
| --- | --- |
| `next_int` | the next 64-bit output, as a (possibly negative) integer |
| `next_float` | the next output divided by the maximum, a float in [0, 1] |
| `next_bool` | the low bit of the next output |
| `set_seed` | a function; `r.set_seed(n)` reseeds `r`. On `random.global` it yields the error `Cannot change the seed of the global random number generator` |

Declared in the attribute table and **not implemented** - each yields `Not
implemented` or `Invalid function call`: `next_int_range`,
`next_float_range`, `next_gaussian`, `shuffle`, `sample`, `choose`.

---

## 10. Errors

### 10.1 Syntax errors

A syntax error is any input the grammar in section 12 does not accept, plus
the scanner-level errors: a malformed string (unterminated, or a trailing
backslash), an octal escape over 255, a `%>` outside a tag in script mode,
and any character that is not a token.

**The contract is that program creation fails**: `gta_program_create()`
returns `NULL`, and the `tang` tool exits with status 1 and "failed to
compile". Nothing runs.

The implementation honours this (13.1). It did not always: a parse that
failed used to be indistinguishable from an empty source, so the program was
created, ran, produced no output and a `null` result, and the host had no way
to tell that from a correct empty template.

Compile-time errors that are not syntax errors - `global` at top level, a
function or identifier declared twice - also fail creation, with a message on
stderr. See 13.18 for the ones that currently abort instead.

### 10.2 Run-time errors

A run-time error is a **value**. The operation that failed yields an error
value instead of a result, and execution **continues** with it. An error
value is falsy, prints as nothing, and every operation applied to one yields
another error. It therefore propagates through an expression but does not
unwind a statement: `x = 1 / 0; print("after"); x;` prints `after` and the
program's result is the `Divide by zero` error.

The errors:

| Error | Raised by |
| --- | --- |
| `Divide by zero` | `/` with a zero divisor |
| `Modulo by zero` | `%` with a zero divisor |
| `Not supported` | an operation the operand types do not define: `"a" + 1`, `1 < 2 < 3`, `%` on floats, indexing `null` |
| `Not implemented` | an operation that is defined but not yet written (13.3, 13.7) |
| `Invalid index` | a non-integer array or string index; a slice step of 0; an out-of-range negative index in assignment |
| `Map key is not a string` | indexing a map with a non-string |
| `Invalid function call` | calling something that is not a function |
| `Argument Count Mismatch` | calling with the wrong number of arguments |
| `Cannot change the seed of the global random number generator` | `random.global.set_seed(n)` |
| `Iterator end` | internal; leaks as a program result (13.12) |
| `Out of memory`, `Invalid bytecode` | internal |

The host sees the last one as `context->result`, and only if it was the
last statement's value. An error in the middle of a program is lost unless
the program stores it. `gta_program_execute()` returns `true` regardless.
This is a design gap (14).

### 10.3 Limits

None. There is no cap on execution time, recursion depth, memory, or output
size. `while (true) {}` runs until the host kills it, and unbounded
recursion overflows the C stack (13.14). The README's statement that limits
can be set describes intent, not the implementation.

---

## 11. Execution model

1. **Parse.** Source → AST. Failure is a syntax error (10.1).
2. **Simplify.** Constant folding and variable-map construction; `-3` becomes
   one literal, and `"a" + "b"` would, if `+` were defined on strings.
3. **Compile.** AST → bytecode, and AST → x86-64 machine code unless disabled.
   Every literal is interned once per program as a **singleton** value; the
   program that references `42` in three places has one `42`.
4. **Execute.** Machine code if it was generated, else bytecode. A run needs
   an execution context, which holds the output buffer, the result, the
   per-run library, and the garbage collector's roots. One program may be
   executed many times with fresh contexts.

**The result** of a program is the value of the last statement executed - the
expression statement's value, `return`'s operand, or `null` when the last
statement was a loop, a block that ended with one, `print`, or nothing.

**Flags** (`gta_program_create_with_flags`): `IS_TEMPLATE`, `DISABLE_BINARY`,
`DISABLE_BYTECODE`, `DEBUG`, `DUPLICATE_CODE` (copy the source rather than
adopting it), `PRINT_TO_STDOUT` (write output as it is produced, raw, and do
not fill the buffer), `IGNORE_ENVIRONMENT`. Unless the last is set, the
environment variables `TANG_DEBUG`, `TANG_DISABLE_BYTECODE` and
`TANG_DISABLE_BINARY` set the corresponding flags.

**The `tang` tool** reads a template from a file or stdin (`-s` for a script,
`-e` for inline source), runs it, and writes the raw output to stdout.

---

## 12. Grammar

Terminals in quotes; `ε` is empty. This is the bison grammar with the
actions removed and the open/closed-statement split (which exists only to
resolve the dangling `else`) collapsed.

```
program        : expression | statements | ε

statements     : statement | statements statement

statement      : "if" "(" expression ")" statement
               | "if" "(" expression ")" statement "else" statement
               | "while" "(" expression ")" statement
               | "do" statement "while" "(" expression ")" ";"
               | "for" "(" opt-expr ";" opt-expr ";" opt-expr ")" statement
               | "for" "(" IDENTIFIER ":" expression ")" statement
               | "function" IDENTIFIER "(" params ")" block
               | block
               | "return" ";" | "return" expression ";"
               | "break" ";" | "continue" ";"
               | expression ";"
               | TEMPLATESTRING
               | "<%=" expression "%>"
               | "use" IDENTIFIER ";"
               | "use" library-path "as" IDENTIFIER ";"
               | "global" IDENTIFIER ";"
               | "global" IDENTIFIER "=" expression ";"

block          : "{" "}" | "{" statements "}"
params         : ε | IDENTIFIER | params "," IDENTIFIER
opt-expr       : ε | expression
library-path   : IDENTIFIER | library-path "." IDENTIFIER | library-path "." "global"

expression     : "null" | "true" | "false" | INTEGER | FLOAT | STRING | IDENTIFIER
               | expression "=" expression
               | expression "?" expression ":" expression
               | expression "||" expression | expression "&&" expression
               | expression "==" expression | expression "!=" expression
               | expression "<" expression  | expression "<=" expression
               | expression ">" expression  | expression ">=" expression
               | expression "+" expression  | expression "-" expression
               | expression "*" expression  | expression "/" expression
               | expression "%" expression
               | "-" expression | "!" expression
               | expression "as" ("int" | "float" | "bool" | "string")
               | "print" "(" expression ")"
               | expression "." IDENTIFIER | expression "." "global"
               | expression "[" expression "]"
               | expression "[" opt-expr ":" opt-expr "]"
               | expression "[" opt-expr ":" opt-expr ":" opt-expr "]"
               | expression "(" args ")"
               | "[" args "]"
               | "{" ":" "}" | "{" map-entries "}" | "{" map-entries "," "}"
               | "(" expression ")"

args           : ε | expression | args "," expression
map-entries    : IDENTIFIER ":" expression | map-entries "," IDENTIFIER ":" expression
```

`TEMPLATESTRING` is a run of literal text in template mode. A
`TEMPLATESTRING` immediately followed by `<%=` is delivered to the parser as
one token; that is an implementation detail and changes nothing above.

`.global` in an attribute or library path is the member named `global`
(`random.global`), which would otherwise be swallowed by the keyword.

---

## 13. Implementation status

Every open item is reproducible with `tang -s -e '<code>'`, or with the test
harness in `test/`. Numbers are for cross-reference from the text above, not
priority, and they are never reused: an item that has been fixed keeps its
number and says so, because the text above points at these by number.

1. **Fixed.** Syntax errors used to be swallowed when they occurred before
   the parser had reduced the start symbol. `GTA_Parser_error()` built a
   parse-error node only if an AST already existed, otherwise it left `NULL` -
   and `NULL` already meant "empty source", which `gta_program_create()`
   turns into an empty program. Which errors failed and which were swallowed
   depended on reduction timing rather than on anything the author could
   predict. The node is now built unconditionally, and a rule action that
   rejects its input (invalid UTF-8, or a failed allocation) produces one too,
   so `NULL` means only that there was nothing to parse.
   Fix: treat `parseError != NULL` as failure regardless of the AST.

2. **Fixed.** String concatenation was not implemented at run time: `"ab" +
   "cd"` was `Not implemented` and `"a" + 1` was `Not supported`. The AST
   simplifier folded string `+` with `gta_unicode_string_concat` all along, so
   a folded program and an executed one disagreed about the same source. `+`
   now concatenates whenever either operand is a string (4.2.1).

   Fixing it made a second defect reachable, since nothing else built a
   multi-segment string: `gta_unicode_string_substring` searched for the
   segment containing the start grapheme with `offset >= grapheme_start`,
   which every segment satisfies when the start is 0, so a whole-string
   substring - which is what printing takes - was given the *last* segment's
   type. A trusted tail laundered an untrusted head, and `!"<i>" + "<b>"`
   would have reached the output as `<i><b>`, unescaped.

3. **Fixed.** Equality and ordering were implemented only for numbers and
   arrays; every other type's virtual table carried the "not implemented"
   stub, so `"a" == "a"`, `true == true` and `null == null` were errors.
   Equality is now defined for strings, booleans and null, and is **strict**
   across types (section 14): a value of one type is never equal to a value
   of another, and asking is not an error, because `x == null` is how a
   template asks whether a value is there at all. Strings also order, by code
   point, which for UTF-8 is byte order - not a collation. Ordering across
   types has no answer and remains an error.

4. **Fixed.** Float literals with the same integer part used to be the same
   literal. `gta_program_get_singleton()` takes the interning key as
   `GTA_UInteger` and `astNodeFloat.c` passed the double straight in, so
   `0.25` and `0.5` both keyed as `0` and the second resolved to the first:
   `0.5 + 0.25` was `1.`, `0.1 + 0.2` was `0.2`. The key is now the double's
   bit pattern. Interning itself still works - one value used twice is still
   one singleton.

5. **Fixed.** Arrays and maps were always falsy: `if ([1])` took the else
   branch and `![1]` was `true`. `is_true` is a stored byte, which both
   engines read straight out of the object rather than computing, and the
   container constructors set it to false and nothing ever set it again. It
   is now set from the number of elements the container is created with, and
   set again by the operations that can grow one from empty. An empty array
   or map is false; one that holds anything is true.

6. **Fixed**, and the old diagnosis was wrong. `[] as bool` did not hit a
   null-context assertion: the array's `cast` slot held
   `gta_computed_value_cast`, which is the generic dispatcher that *reads*
   the cast slot, so every cast called itself until the stack ran out.
   A container now casts to a boolean, which is its truthiness (13.5), and to
   a string, which is its rendering. There is no number a container could
   sensibly be, so `[] as int` is `Not supported`. Maps cast the same way;
   they previously said `Not supported` for all of it.

7. **Fixed.** `m.name` did not read a map member (`Not implemented`) even
   though `m.name = v` wrote one. The map's `period` was the generic
   attribute lookup, and a map has no attributes, so every name missed. It
   now checks for a built-in attribute of the map type first - there are
   none, and the order is what keeps a name like `m.size` available to mean
   the map's own size later rather than a key arriving from untrusted data -
   and then looks the name up as a key. A name that is not a key is
   `Map Key Not Found`, which is what `m["name"]` already said.

8. **Fixed.** `print(true)` used to print nothing: the boolean vtable's
   `print` was `not_supported`, even though its `to_string` already produced
   `true` and `false` and `true as string` already printed them. Booleans now
   print as `true` and `false`.

9. **Not a defect, described wrongly.** Assigning through a string index
   *is* an error: `s = "abc"; s[0] = "z";` yields `Not supported`, and the
   string is unchanged because strings are immutable. What the old text was
   reading was the value of the *next* statement - an error that is not the
   last value of the program is discarded, which is the errors-as-values
   model and the open question in section 14, not something specific to
   strings.

10. **Fixed.** Attribute assignment refused to compile, which - before the
    caller checked for that, see 13.23 - truncated the program, and made it
    look as though the assignment had been accepted on every value and every
    name. It had not: `x = [1, 2]; x.size = 5; x.size;` was `5` because the
    program ended at the assignment and `5` was the assignment's own value,
    not because anything had been written. The same reading produced the
    "rebinds the variable" in the old text of this item and of 13.11.

    `a.b = v` now compiles to what `a["b"] = v` compiles to, so each type
    answers for itself: a map writes the member (4.13), an array says
    `Invalid index` because a string is not an array subscript, and a string,
    a library and null say `Not supported`. The built-in keeps its meaning -
    `x.size` is still the array's size - and the program keeps running.

11. **Fixed.** Assignment to a slice was not rejected, it truncated the
    program (13.23), which read as rebinding the variable. Only a name, a
    member and a subscript can be assigned to; anything else is now rejected
    during analysis, in one place, rather than by each compiler abandoning an
    emission it had half done. `x[1:2] = [9]` fails to compile.

12. **Fixed.** A ranged `for` leaked its iterator sentinel as its value, so
    `for (i : [1, 2]) {}` as the last statement gave the error `Iterator end`
    rather than `null` - visible in every template that ends with a loop.
    Both ways out of the loop landed on the same place. They are now
    separate: running out of elements is the normal end of a loop, and its
    value is `null`; an expression that could not be iterated at all is an
    error and stays one, because that is the only way the author hears about
    it. `break` keeps the value it was given.

13. **Date literals are scanned but not parsed.** `@` puts the scanner into a
    date state that recognises `now`, `today`, `+3d`, ISO dates and time
    zones, but no grammar rule consumes the tokens. `@now;` is currently
    swallowed under 13.1 and yields `null`.

14. **No recursion limit.** `function f(n) { return f(n + 1); } f(0);`
    overflows the C stack and segfaults the host process. A template can take
    the server down.

15. **Fixed** as a crash; the language question is still open. `foo();
    function foo() {}` crashed with `free(): invalid pointer` - see 13.24 for
    why every one of these reported errors crashed rather than failing
    compilation. It now fails compilation, which is what the current design
    (declarations bind where they appear) says it should do. Whether the
    declaration ought to be hoisted instead is still the open question in
    section 14.

16. **Fixed.** `f = 3; f();` was `Invalid function call` under the JIT and
    `3` under the bytecode VM. A call is an expression and has to leave
    exactly one value behind, like every other one; the VM's refusals set
    `context->result` and left nothing, so the stack was short by one - the
    POP the block compiler emits after the statement took the value
    underneath, and `context->result` was then overwritten at the end by
    whatever was on top. The error is now left in the arguments' place, which
    is also what the argument-count mismatch does.

17. **Fixed.** `function f(a, a) {}` never returned from
    `gta_program_create()`. Two parameters with one name take one slot in the
    function's scope, so the scope held fewer variables than the function had
    parameters - and the x86-64 prologue works out how many locals to reserve
    by subtracting one from the other, in `size_t`. The subtraction wrapped,
    and the loop that fills the locals with null then ran about 2^64 times.
    A repeated parameter name is now rejected during analysis, and the
    subtraction says in an assertion what it is relying on.

18. **Fixed.** `function f() {} function f() {}` and `x = 1; function x() {}`
    aborted the process. Two separate defects, the first hiding the second:
    see 13.24 and 13.25. Both now fail compilation.

19. **`PRINT_TO_STDOUT` and the `tang` tool emit the raw buffer**, with no
    encoding applied, so `!"<b>"` reaches stdout as `<b>`. Either is
    defensible as a debugging aid; neither is what a user of a *template*
    tool expects, and the tool has no flag to render.

20. **Fixed.** Percent-encoding of non-ASCII bytes was wrong.
    `unicodeString.c` indexed the hex table with `string->buffer[i] >> 4` on
    a signed `char`, so a byte >= 0x80 produced garbage and
    `"é".percent.render` was `%l3%f9`. The index is now taken through
    `unsigned char`, and the result is `%C3%A9`.

21. **Fixed**, and it was not the same fault as 13.6. `x = [1, 2]; x[0] = x;`
    takes the deep-copy path in `assign_index`, which is the only path that
    uses the execution context - and the x86-64 caller was loading the
    context into the *second* argument register and then popping the index
    over it, so the callee read whatever happened to be in the fourth.
    Assigning a temporary never noticed, because a container adopts those
    without touching the context. The context is now passed in the register
    the ABI puts the fourth argument in. Storing a container into itself
    stores a deep copy of what it held, so no cycle is created.

22. **The README's `print!(...)` is not syntax.** The examples there predate
    the `!"..."` prefix and do not parse.

23. **Fixed.** A statement whose bytecode compile refused truncated the
    program instead of failing it. `gta_program_compile_bytecode()` published
    `program->bytecode` before compiling and never looked at `error_free` on
    the way out, so a refusal left the partial vector installed with no
    `RETURN` appended, `gta_program_create()` saw a non-null bytecode and
    reported success, and the virtual machine - whose loop has no bound - ran
    off the end into the vector's spare capacity, where a zero reads as
    `RETURN` because that opcode is enumerator 0. So the program stopped at
    the refused statement and called it a clean return:
    `print("before"); x.size = 5; print("after");` printed only `before`.
    Nothing reported an error, and neither ASan nor the allocation counters
    could see it, because the read stayed inside the vector's own allocation.
    This is what 13.10 and 13.11 were describing.

24. **Fixed.** Reporting an error by returning a parse-error singleton
    aborted the process with `free(): invalid pointer`.
    `gta_ast_node_destroy()` folded its `is_singleton` test into the
    condition that picks the destructor:

    ```c
    (!self->is_singleton && self->vtable->destroy)
      ? self->vtable->destroy(self)
      : gta_ast_node_null_destroy(self);
    ```

    which sends a singleton to the fallback - and the fallback is a `free()`,
    not a no-op. So the check that exists to protect a singleton was what
    freed it. This is why 13.15 and 13.18 crashed instead of failing
    compilation.

25. **Fixed.** A redeclared function was inserted into the outermost scope's
    `function_scopes` anyway, and then its scope was destroyed - so the hash
    held a dangling pointer, and destroying the outermost scope destroyed it
    a second time. It also dropped the first declaration's scope, which the
    hash owned. The insert is now skipped when the name is already taken.
    Hidden behind 13.24, which aborted first.

26. **Fixed.** The x86-64 engine bound every parameter to the wrong
    argument. The caller walked the arguments backwards while walking the
    slots it wrote them into forwards, so `f(1, 2, 3)` with
    `function f(a, b, c)` bound `a` to 3 and `c` to 1. Silent wrong answers
    rather than a crash, under the engine that runs by default, and the
    bytecode engine disagreed with it. It also evaluated the arguments last
    to first, which the bytecode engine does not. Arguments are now evaluated
    in source order and written to the slot the callee reads that parameter
    from. A native function is handed the block as a C array and reads it the
    other way, so that branch reverses it in place first.

---

## 14. Open questions

Things this document deliberately does not decide, because they are choices
rather than defects. Each needs a decision, then a test, then code.

- ~~**String concatenation.**~~ **Decided:** `+`, the spelling a template
  author reaches for first, rather than a dedicated operator. The cost is
  that the operands' runtime types decide whether an expression adds or
  concatenates, which the author cannot see at the point of use. The result
  is a multi-segment string with each segment keeping its own tag, which is
  the only answer that keeps the guarantees of section 8. See 4.2.1.
- **Equality across types.** Strict (`"3" == 3` is false) or coercing? This
  document assumes strict, because a template language should not have
  PHP's `==`.
- **String ordering.** Byte order, code point order, or collation? Grapheme
  strings argue for at least code point order.
- **Map iteration.** `for (k : m)` over keys, or over `[key, value]` pairs?
  And `m.size`, `m.keys`, `m.has`.
- **String iteration.** `for (g : s)` over graphemes is the obvious meaning.
- **Hoisting.** Should `foo(); function foo() {}` work? JavaScript says yes;
  the current design (declarations bind where they appear) says no. The
  crash in 13.15 has to become an error either way.
- **Closures and anonymous functions.** Neither exists. The `use ... as`
  mechanism covers the template use case for the former; the latter would be
  needed for callbacks (`sort` with a comparator).
- **Block scope.** `if (c) { tmp = 1; }` leaking `tmp` is convenient for
  templates and surprising for everyone else.
- **Error handling.** Errors as values that keep going is a defensible
  choice for templates - a broken expression should not blank the page - but
  the host then needs to *know* an error happened, and today it only finds
  out if the error was the last value. A per-context error list, or a flag
  to halt on first error, or both.
- **Limits.** Instruction count, recursion depth, output size, memory. The
  README promises them; the sandbox story depends on them.
- **Date literals** (13.13) - finish or remove.
- **Number separators** (`1_000_000`), exponent floats (`1e5`).
- **Keys for reconciliation.** cjelly's `docs/semantics.md` needs Tang to
  express a stable identity on repeated elements so that regenerated markup
  can be diffed against the live interface. That is a syntax decision in this
  language, and it should be made with the reconciler in mind rather than
  retrofitted.
