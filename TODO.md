# TODO

#### Documentation

[KO] use `markdown-pp` to create the documents.
[KO] rst

#### Library

[OK] create
[OK] join
[KO] thread pool

#### LLVM

[OK] add intrinsic
[OK] lower intrinsic to function call

#### TC

[OK] lexer
[OK] parser
[OK] binder and type checker
[==] async binder
     [OK] in `var x := async foo()` bind `x` to `foo`.
     [OK] in `print_int(x)` set `x` as the first usage of the async var.

##### Optional

[KO] use-list
[KO] complex expressions
