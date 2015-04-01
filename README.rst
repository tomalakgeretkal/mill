Mill
====

Mill is a dynamically typed programming language with a focus on clarity,
concurrency and specification.

Hello, world!
-------------

The hello world program reads as follows::

    MAIN {
        outln("Hello, world!");
    }

Influence
---------

Mill takes ideas from many languages, in no particular order:

- Clojure: protocols.
- D: contracts, exceptions.
- Eiffel: contracts, CQS.
- Elixir: pipelines.
- F#: type system.
- Go: concurrency, control structures, I/O.
- Haskell: monad syntax.
- Perl: control structures, special variables.
- Scala: pattern matching.
- Spec#: contracts.

Implementation
--------------

The implementation consists of a compiler called millc and a VM called mill.
The compiler is written in Perl and the VM is written in C++.

Contributing
------------

Like to contribute? Comment on one of the GitHub issues or create a new one!
