# ex-3.2 (from 3BSD release)
This is `vi` version 3.2 taken from 3BSD.
It had been released in January 1980.
## Installation notes
The software is downloaded with
```sh
git clone https://github.com/n-t-roff/ex-3.2.git
```
and can be kept up-to-date with
```sh
git pull
```
Some configuration (e.g. installation paths) can be done in the
[`makefile`](https://github.com/n-t-roff/ex-3.2/blob/master/Makefile.in).
For compiling it on BSD and Linux autoconfiguration is required:
```sh
$ ./configure
```
The software is build with
```sh
$ make
```
and installed with
```
$ su
# make install
# exit
```
All generated files are removed with
```sh
$ make distclean
```
## Usage notes for version 3.2
* PAGE-UP, PAGE-DOWN keys may work on most terminals by putting
  `map  ^[[5~ ^B` and `map  ^[[6~ ^F` into `~/.exrc`.
* The are issues with Undo.
  For the moment save more often to not loose important changes.
* There are currently issues with lines longer than the screen width.
* When inserting an unnamed buffer with `P` or `p` after a macro
  the whole file is inserted instead.
  (Note that e.g. the arrow keys are considered macros.)
  Either use named buffers or 'h', 'j', 'k', 'l', '^F', '^B'
  for motions *between* yank and put commands.

Features which had been invented after version 3.2:

* Job control
* The comment character `"`.
  If you need to comment something in `~/.exrc`,
  put these lines to the end of the file
  and insert an empty line before them.
* `ZZ`, `:x` and `:map!`
* The documents
  [viin.pdf](http://n-t-roff.github.io/ex/3.2/viin.pdf)
  and
  [exrm.pdf](http://n-t-roff.github.io/ex/3.2/exrm.pdf)
  describe vi version 2.2 in detail.

**Attention**:
The original `vi` had not been 8-bit clean!
Moreover it does automatically change all 8-bit characters to 7-bit
in the whole file even if no editing is done!
This will e.g. destroy all UTF-8 characters.
