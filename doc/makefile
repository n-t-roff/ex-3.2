all: exrm.pdf exsummary.pdf ex1.1-2.0.pdf ex2.0-3.1.pdf edittut.pdf viin.pdf \
	viapp.pdf visummary.pdf
# TROFF will frequently be just troff if you have a typesetter
# but no versatec.
TROFF=	troff
DPDF=	dpost | ps2pdf -
exrm.pdf: ex.rm
	${TROFF} -ms ex.rm | ${DPDF} $@
exsummary.pdf: ex.summary
	tbl ex.summary | ${TROFF} -ms | ${DPDF} $@
ex1.1-2.0.pdf: ex1.1-2.0
	${TROFF} -ms ex1.1-2.0 | ${DPDF} $@
ex2.0-3.1.pdf: ex2.0-3.1
	${TROFF} -ms ex2.0-3.1 | ${DPDF} $@
edittut.pdf: edit.tut
	${TROFF} -ms edit.tut | ${DPDF} $@
# index for versatec is different from the one in edit.tut
# because the fonts are different and entries reference page
# rather than section numbers.  if you have a typesetter
# you should just use the index in edit.tut, and ignore editvindex.
editvindex.pdf: edit.vindex
	${TROFF} -ms -n22 edit.vindex | ${DPDF} $@
viin.pdf: vi.in
	tbl vi.in | ${TROFF} -ms | ${DPDF} $@
viapp.pdf: vi.chars
# if using a typesetter use -n20 instead of -n22.
	tbl vi.chars | ${TROFF} -ms -n22 | ${DPDF} $@
visummary.pdf: vi.summary
	tbl vi.summary | ${TROFF} -ms | ${DPDF} $@
clean:
	rm -f *.pdf
