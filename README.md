# Tang
Tang is a new **T**emplate L**ang**uage intended to be embedded into a host program and specifically targeted to generate HTML.

Tang is a loosely-typed, garbage-collected scripting language that is compiled at runtime to binary using a JIT (currently only x86_64 is supported) and falls back to a Bytecode interpreter if the JIT compilation fails or is not available.

## State/Stability
Tang is under active development.  It is unstable (in features, not in performance).  This is it's 3rd incarnation.

The original author wrote the first incarnation in C++ with an AST tree walking interpreter and relied on C++ `shared_ptr` for memory management (i.e., only reference counting).

The second incarnation was written in C++ with a bytecode interpreter, but still relied on the `shared_ptr` for memory management.  It was done as part of a 100+ episode [YouTube series](https://www.youtube.com/playlist?list=PLZqirAnnqaCZ8lT8w7p2PUB7tqrId7d89).  It is boring.  It is live coding.  Nothing is scripted.  Watch it at your own risk.

This is the third incarnation, written in C, with a JIT, bytecode VM fallback, proper garbage collection, etc.

It is intended to be compiled into a shared library.  There is a command to do so in the `Makefile`, but only for Linux.

## Syntax
In Tang, code is interspersed into the literal template text (like PHP).  Quick print tags are supported.  Use `{` and `}` for code blocks.  I like semicolons, so they are here, too.  Tang has arrays, maps, a slice operator, etc.

Examples are the most beneficial, so here are a few:

### Example 1
Suppose a variable named `user` exists, and it is a string containg "Alice".

If this is your template:
```
Welcome, <%= user %>!
```

Then this will be your output:
```
Welcome, Alice!
```

### Example 2
Suppose a variable named `users` exists, and it is a list of names: "Alice",
"Bob", "Carol", and "Frank".

If this is your template:
```
<ul>
<%
for (name : users) {
  print!("  <li>");
  print(name);
  print!("</li>\n");
}
</ul>
```

Then this will be your output:
```
<ul>
  <li>Alice</li>
  <li>Bob</li>
  <li>Carol</li>
  <li>Frank</li>
</ul>
```

Alternatively, the following code could have been used as well, although it
will include a few additional line breaks:
```
<ul>
<% for (name : names) { %>
  <li><%= name %></li>
<% } %>
</ul>
```

## Features: What makes this language different.

### 1. Graphemes.
Tang strings operate on **graphemes** by default.  Just about every other language operates on **bytes**.  If you're lucky, the language will support **Utf-8 encoding**, but that is still not enough.  The flag of Scotland requires **28 bytes** in UTF-8 encoding (`\xF0\x9F\x8F\xB4\xF3\xA0\x81\xA7\xF3\xA0\x81\xA2\xF3\xA0\x81\xB3\xF3\xA0\x81\xA3\xF3\xA0\x81\xB4\xF3\xA0\x81\xBF`) but it is only one grapheme.  Most languages get this very, very wrong and will mangle the text when performing a slice, substring, or string reversal.  I felt that being Grapheme-aware was imperative for a template language!

### 2. Sandboxed Control.
The use case of this language is so that designers can write code that will not adversely affect the host program.  Features are intentionally not included such as accessing the file system or network (although the plugin system does not prevent someone from allowing this behavior should it be required for a particular application).  Limits can be set on memory use or execution time.  Tang was meant to be a tool for the host program to provide freedom within practical and tunable bounds.

## But... Why?
That's like asking a woodworker why he didn't just go to IKEA.

This is a fun project.  I actually like programming in C.  I find beauty in creating a system that has good unit test coverage, no memory leaks, and is intellectually stimulating to work on.

## Technical Details.

I'm still working on this part.  I use Ubuntu 22.04 (haven't upgraded to 24.04 yet).  Please just let me know if I've missed a step.

Here's the broad plan:

### Ghoti.io CUtil
Tang makes use of another library of mine called CUtil.  Compile and intall it following the instructions [here](https://github.com/Ghoti-io/CUtil).

### Install packages
First, the necessary packages must be installed.
```
sudo apt install g++ make bison flex build-essential pkgconf libgtest-dev googletest
```

Additional packages must be installed in order to create documentation and
enhance the build experience.
```
sudo apt install doxygen graphviz texlive-latex-base texlive-latex-extra cloc inotify-tools valgrind gdb
```

If you're working in the WSL, you can install the `wslu` package, so that you can view files (such as the documentation) using a cli command.
```
sudo apt install wslu
```

Example use:
```
make docs
wslview ./docs/html/index.html
```

### Compile source code
```
make
sudo make install
```

For additional `make` commands/options, run:
```
make help
```

### For development.
When developing, I have a script that watches for changes and, when I save a file, it automatically recompiles and runs the tests.
```
make test-watch
```

## Next Steps
There are so many things to flesh out, and I'm working on this in my spare time (although hopefully you can see that I *do* make progress on it over time).

I would like to have help getting this running on Windows.  I have a few things already stubbed out, but nothing is tested yet.  I haven't done proper Windows development in a long time.  Have any insight?  Send me an email. `pennycuff.c` is the first part, and Gmail is my email provider.

There's plenty of other things (non-Windows) that could be done, too, such as supporting other architectures for the JIT, additional built-in libraries (math, date), compiler optimizations, etc.  If you want to work on something, but don't know what, just send me an email and we can talk!
