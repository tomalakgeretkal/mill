Mill
====

Mill is a dynamically typed programming language with a focus on clarity,
concurrency and specification. It strives towards the following goals:

- Lightweight threading.
- Terrific interop with other languages.
- Formal specification of preconditions, postconditions, exceptions, invariants
  and types.
- A focus on immutable variables and data structures.
- A familiar, eye-pleasing syntax.

Hello, world!
-------------

The hello world program is as follows::

    use std::io;

    MAIN {
        io::writeln("Hello, world!");
    }

Implementation
--------------

The implementation consists of a compiler called millc and a VM called mill.
The compiler is written in Perl and the VM is written in C++.

Contributing
------------

Like to contribute? Comment on one of the GitHub issues!
