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
For compiling on BSD, Linux and Solaris, autoconfiguration is required:
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
* There are currently issues with lines longer than the screen width.
  Make the screen wide enough before opening vi or break long lines
  before they exceed the screen width.
* When inserting an unnamed buffer with `P` or `p` after a macro
  the whole file is inserted instead.
  (Note that e.g. the arrow keys are considered macros.)
  Either use named buffers or 'h', 'j', 'k', 'l', '^F', '^B'
  for motions *between* yank and put commands.
  (This bug is fixed in ex version 3.4.)
* If the screen with is not a multiple of the tab width
  tab characters are displayed wrong
  after screen updates
  which results in a left shift of the subsequent text.
  (This bug is fixed in vi version 3.4.)
  The display is fixed with the following actions:

  * The current line is always displayed correct after `^L`.
  * All screen lines are fixed with screen updates after
    `^F` `^B`, `''` `''`. `^^` `^^` and so on.
  * The issue does not occur if the terminal width is set
    to a multiple of the tab width (e.g. a multiple of 8)
    *before* vi is started.

Features which had been invented after version 3.2:

* `ZZ`, `:x` (use `:wq`)
  (New in ex-3.3.)
* `:map!`
  (New in ex-3.3.)
* Job control
  (New in ex-3.4.)
* The comment character `"`.
  If you need to comment something in `~/.exrc`,
  put these lines to the end of the file
  and insert an empty line before them.
  (New in ex-3.4.)
* There is no read-only mode.
  Program name `view` and option `-R` did not exist.
  (New in ex-3.4.)
* The documents
  [viin.pdf](http://n-t-roff.github.io/ex/3.2/viin.pdf)
  and
  [exrm.pdf](http://n-t-roff.github.io/ex/3.2/exrm.pdf)
  describe vi version 3.2 in detail,
  [ex3.1-3.5.pdf](http://n-t-roff.github.io/ex/3.6/ex3.1-3.5.pdf)
  shows the differences to later vi versions.
