# TODO

#### Documentation

[KO] use `markdown-pp` to create the documents.
[KO] rst

#### Library

[KO] create
[KO] join
[KO] thread pool

#### LLVM

[KO] add intrinsic
[KO] lower intrinsic to function call

#### TC

[OK] lexer
[OK] parser
[OK] binder and type checker
[==] async binder
     [KO] in `var x := async foo()` bind `x` to `foo`.
     [KO] in `print_int(x)` set `x` as the first usage of the async var.

##### Optional

[KO] use-list
[KO] complex expressions
