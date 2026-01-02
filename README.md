# mclang

mclang is a sample implement clang for algorithm learnning.

## Front

top to down praser, gen ast with symbol table


## Def-Use chain

design of def-use chain is come from LLVM, but have a little different.
In LLVM, have the `Use(const Use &U) = delete;` , but i use vector to save ary variable
so need using emplace construct class variable, need it.

use it' matin when Value variable created, Value have a list save all use

Instruciont is a User, and User is a Value, so instrucion have a list save all use

when use created?
such as:
```cpp
  BinaryOperator(BasicBlockPtr BB, VariablePtr Left, VariablePtr Right,
                 IROpKind Op)
      : Instruction(BB, Op), Left(this, 0), Right(this, 1) {
    this->Left = Left;
    this->Right = Right;
  }
```
when do `this->Left = Left;` 
will call 

```cpp
  ValuePtr operator=(ValuePtr RHS) {
    set(RHS);
    return RHS;
  }
```

further call

```cpp
void Use::set(ValuePtr V) {
  if (Val)
    removeFromList();
  Val = V;
  if (V)
    V->addUse(this);
}
```

Instrucion -> User -> Value
Use

User ---> | Use |  Use | Use | ... |

## SCCP
Paper:

Constant Propagation with Conditional Branches

Slides:

https://www.cse.iitk.ac.in/users/karkare/Courses/cs738/lecturenotes/11CondConstPropHandout.pdf




## TODO LIST:


support **Doxygen**

all move to C++

IR gen function return a Result, don't passing by arg



support void function


add printAST function

like	
```

​		|
			---  ---
				|
				 --- ---
​		|

​		|

​				|

​				|
```


twine

support CHANGE LOG


Opt?:

SCCP

CodeGen?:

out of ssa

liner scan gen low level machine IR

and do color register allocation

do test for malloc
e.x. int * a = malloc(8);

make malloc can call stdlib after backend

support break

impl FFT algorithm in compiler as test suit.

before test RA algorithm must write a code with spill and
allocate cases, for test RA

## Backend

before RA, mir also is ssa,
but here only have two type variable : register and const
so can repsenation as union.

and will need lower ir => mir
and mir => target mir
at least two step
after do that
need add call prolog, elog

then enter RA algorithm, generate almost asm code

last codegen, gen .data, .text, label with asm format

## Ref
This is the reference implementation of https://www.sigbus.info/compilerbook.

link:  https://github.com/rui314/chibicc