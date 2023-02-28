<p align="center">
  <img src="https://raw.githubusercontent.com/Lartu/nari2023/main/nari.png">
</p>

**Nari** (2023) is a new iteration of my Nari programming language. I develop it to relax, so I say that working on Nari is programming language programming as therapy. Nari is inspired by Forth, or how I imagine Forth to be, as I've never written a single line of Forth. Nari is a weakly typed language. Its only data types are strings and integers. I don't know if it's fast or not (it probably isn't). I might expand it over time. I also might not.

In this particular repository, unsolicited *feature* pull requests are not welcome. Sorry. This is a personal project and I'd like to keep it that way. Ideas and bug-fixes are welcome, though.

### Some Examples

Add two numbers and print them followed by a line break:
```
10 5 + lf .
```

[Disan Count](https://esolangs.org/wiki/Disan_Count) from 50 to 1 (print all even numbers):

```
{ even? 2 % 0 = }

50 $i [ @i 0 >= :
    @i even? (
        @i "  is even." lf .
    )
    @i 1 - $i
]
```

Simple dictionary implementation:

```
# Set value #
# Value  #  " This is the value!"
# Key -> #  " myKey"
$

# Get value and print it #
" myKey" @ lf .
```

### Usages

Relaxing. Nari can probably be also used for other things. It can probably be embedded within C++ source code as an embeddable interpreted language that you can probably hook up to C++ functions within your own code. It maybe might be used for code golfing? I don't have any particular usecases in mind. You are welcome to share yours.


### Etcetera

Nari was my late guinea pig. She lived to the ripe age of eight years old!
