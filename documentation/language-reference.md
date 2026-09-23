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
compiled as if it were closed (see 13.1 for what "not an error" costs).

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
that does not fit in 64 bits saturates to `9223372036854775807`.

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
=  +  -  *  /  %  !  <  <=  >  >=  ==  !=  &&  ||  ?  :  .  ,  ;
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
| 1 | `=` `? :` | right |
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
| anything else | error `Not supported` |

Division or modulo by zero, integer or float, is an error (`Divide by zero`,
`Modulo by zero`). `%` on floats is `Not supported`.

Unary minus applies to integers and floats. On anything else it is an error.

`+` on strings is **not** concatenation; it is currently `Not implemented`
(13.2). This is the largest open decision in section 14.

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
| string | non-empty | leading decimal integer, else `0` (`"12abc"` is `12`, `"abc"` is `0`), or a marker if it does not fit (below) | leading decimal, else `0.0` | itself |
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
thing. `99999999999999999999999.0 as int` and `"99999999999999999999999" as
int` are both `[INTEGER TOO LARGE]`.

A string that is not a number at all is still `0`, not a marker: `"abc" as
int` is `0`. That is a different question from this one, which is only about
values that do not fit.

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
| boolean | **intended:** `true` / `false`. **Currently:** nothing (13.8) |
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
`<b>&lt;i&gt;`.

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

The implementation honours this only sometimes (13.1). When it does not, the
program is created, runs, produces no output and a `null` result, and the
host has no way to tell that from a correct empty template. This is the
first defect to fix, because everything that depends on Tang inherits it.

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
| `Not implemented` | an operation that is defined but not yet written (13.2, 13.3, 13.7, 13.8) |
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

Every item is reproducible with `tang -s -e '<code>'`, or with the test
harness in `test/`. Numbers are for cross-reference from the text above, not
priority - except that 13.1 comes first.

1. **Syntax errors are swallowed when they occur before the parser has
   reduced the start symbol.** `GTA_Parser_error()` replaces the AST with a
   parse-error node only if an AST already exists; otherwise it leaves it
   `NULL`, and `gta_program_create()` treats `NULL` as an empty program.
   Which errors fail and which are swallowed depends on the parser's
   reduction timing, not on anything the author can predict: `1e5` and
   `1; ; 2;` fail; `;;`, `'x'`, `%>`, `"unterminated`, `use math.pi;`, and
   in template mode `A <% 1 + %> C` all silently produce an empty program.
   Fix: treat `parseError != NULL` as failure regardless of the AST.

2. **String concatenation is not implemented.** `"ab" + "cd"` is
   `Not implemented`. So is `"a" + 1`, which is `Not supported`.

3. **Equality and ordering are not implemented for strings, booleans or
   null.** `"a" == "a"`, `true == true`, `null == null`, `"a" < "b"` all
   yield `Not implemented`. Only numbers and arrays compare.

4. **Float literals with the same integer part are the same literal.**
   `gta_program_get_singleton()` takes the interning key as `GTA_UInteger`,
   and `astNodeFloat.c` passes the double straight in, so `0.25` and `0.5`
   both key as `0` and the second resolves to the first. `0.5 + 0.25` is `1.`;
   `0.1 + 0.2` is `0.2`; `0.3 - 0.1` is `0.`; `1.5 + 1.75` is `3.`. Literals
   whose integer parts differ, and values that did not come from literals,
   are unaffected - which is why `Binary.Add`'s float cases, which use library
   values, pass. Fix: hash the double's bit pattern.

5. **Arrays and maps are always falsy.** `if ([1])` takes the else branch;
   `![1]` is `true`; `[1] && 2` is `[1]`.

6. **`as` on an array crashes** with a null-context assertion in
   `gta_computed_value_array_create()` for every target type. `[] as bool`
   segfaults. `{:} as bool` also crashes; the other map casts return
   `Not supported`.

7. **`m.name` does not read a map member** (`Not implemented`) even though
   `m.name = v` writes one. `m["name"]` works.

8. **`print(true)` prints nothing** - the boolean vtable's `print` is
   `not_supported`.

9. **Assigning through a string index is silently ignored.**
   `s = "abc"; s[0] = "z"; s;` is `"abc"`. Should be an error, since strings
   are immutable.

10. **Attribute assignment is accepted on every value and every name, and
    shadows built-ins.** `x = [1, 2]; x.size = 5; x.size;` is `5`;
    `"abc".length = 1` likewise; `use math; math.pi = 3;` rebinds `pi`;
    `null.x = 1` succeeds. Worse, an attribute assignment on a **non-map
    variable rebinds the variable**: `f = 1; f.g.h = 2; f;` is `2`.

11. **Assignment to a slice rebinds the variable** instead of being rejected:
    `x = [1, 2, 3]; x[1:2] = [9]; x;` is `[9]`, and `x[1:] = 7; x;` is `7`.

12. **A ranged `for` leaks its iterator sentinel as the program result.**
    `for (i : [1, 2]) {}` as the last statement gives the result
    `Iterator end` (an error) rather than `null`. Visible in every template
    that ends with a loop.

13. **Date literals are scanned but not parsed.** `@` puts the scanner into a
    date state that recognises `now`, `today`, `+3d`, ISO dates and time
    zones, but no grammar rule consumes the tokens. `@now;` is currently
    swallowed under 13.1 and yields `null`.

14. **No recursion limit.** `function f(n) { return f(n + 1); } f(0);`
    overflows the C stack and segfaults the host process. A template can take
    the server down.

15. **Calling a function before its declaration crashes** with
    `free(): invalid pointer`: `foo(); function foo() {}`.

16. **The two backends disagree on calling a non-function.** `f = 3; f();`
    is `Invalid function call` under the JIT and `3` under the bytecode VM;
    `(1)(2)` segfaults the VM.

17. **Duplicate parameter names hang the compiler.**
    `function f(a, a) {}` never returns from `gta_program_create()`.

18. **Redeclaration aborts instead of failing compilation.** The parse-error
    singletons `function_redeclared` and `identifier_redeclared` exist, but
    `function f() {} function f() {}` and `x = 1; function x() {}` both hit
    an assertion and abort the process.

19. **`PRINT_TO_STDOUT` and the `tang` tool emit the raw buffer**, with no
    encoding applied, so `!"<b>"` reaches stdout as `<b>`. Either is
    defensible as a debugging aid; neither is what a user of a *template*
    tool expects, and the tool has no flag to render.

20. **Percent-encoding of non-ASCII bytes is wrong.** `unicodeString.c`
    indexes the hex table with `string->buffer[i] >> 4` on a signed `char`,
    so a byte ≥ 0x80 produces garbage: `"é".percent.render` is `%l3%f9`
    instead of `%C3%A9`. Fix: cast to `unsigned char`.

21. **Storing an array inside itself aborts.** `x = [1, 2]; x[0] = x;`
    fails the same assertion as 13.6 under the JIT and segfaults the VM.

22. **The README's `print!(...)` is not syntax.** The examples there predate
    the `!"..."` prefix and do not parse.

---

## 14. Open questions

Things this document deliberately does not decide, because they are choices
rather than defects. Each needs a decision, then a test, then code.

- **String concatenation.** `+`, following most of the family; or a
  dedicated operator, so that `"1" + 1` cannot mean two things. Whatever is
  chosen has to say what the *encoding* of the result is when the operands
  differ - `"<b>" + !"x"` - and the answer that keeps the guarantees of
  section 8 is that the result is a multi-segment string and each segment
  keeps its own tag, exactly as the output buffer does. The infrastructure
  for that exists (`gta_unicode_string_concat`).
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
