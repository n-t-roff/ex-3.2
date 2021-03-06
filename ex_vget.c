/* Copyright (c) 1979 Regents of the University of California */
#include "ex.h"
#include "ex_tty.h"
#include "ex_vis.h"

/*
 * Input routines for open/visual.
 * We handle upper case only terminals in visual and reading from the
 * echo area here as well as notification on large changes
 * which appears in the echo area.
 */

static void addto(char *, char *);
static int getbr(void);
static int fastpeekkey(void);
static void trapalarm(int);

/*
 * Return the key.
 */
void
ungetkey(int c)
{

	if (Peekkey != ATTN)
		Peekkey = c;
}

/*
 * Return a keystroke, but never a ^@.
 */
int
getkey(void)
{
	int c;

	do {
		c = getbr();
		if (c==0)
			beep();
	} while (c == 0);
	return (c);
}

/*
 * Tell whether next keystroke would be a ^@.
 */
int
peekbr(void)
{

	Peekkey = getbr();
	return (Peekkey == 0);
}

short	precbksl;

/*
 * Get a keystroke, including a ^@.
 * If an key was returned with ungetkey, that
 * comes back first.  Next comes unread input (e.g.
 * from repeating commands with .), and finally new
 * keystrokes.
 *
 * The hard work here is in mapping of \ escaped
 * characters on upper case only terminals.
 */
static int
getbr(void)
{
	int ch;
	register int c, d;
	register char *colp;

getATTN:
	if (Peekkey) {
		c = Peekkey;
		Peekkey = 0;
		return (c);
	}
	if (vglobp) {
		if (*vglobp)
			return (lastvgk = *vglobp++);
		lastvgk = 0;
		return (ESCAPE);
	}
	if (vmacp) {
		if (*vmacp)
			return(*vmacp++);
		/* End of a macro or set of nested macros */
		vmacp = 0;
		if (inopen == -1)	/* don't screw up undo for esc esc */
			vundkind = VMANY;
		inopen = 1;	/* restore old setting now that macro done */
	}
#ifdef TRACE
	if (trace)
		fflush(trace);
#endif
	flusho();
again:
	if (read(0, &ch, 1) != 1) {
		if (errno == EINTR)
			goto getATTN;
		error("Input read error");
	}
	c = ch & TRIM;

#ifdef UCVISUAL
	/*
	 * The algorithm here is that of the UNIX kernel.
	 * See the description in the programmers manual.
	 */
	if (UPPERCASE) {
		if (isupper(c))
			c = tolower(c);
		if (c == '\\') {
			if (precbksl < 2)
				precbksl++;
			if (precbksl == 1)
				goto again;
		} else if (precbksl) {
			d = 0;
			if (islower(c))
				d = toupper(c);
			else {
				colp = "({)}!|^~'~";
				while ((d = *colp++))
					if (d == c) {
						d = *colp++;
						break;
					} else
						colp++;
			}
			if (precbksl == 2) {
				if (!d) {
					Peekkey = c;
					precbksl = 0;
					c = '\\';
				}
			} else if (d)
				c = d;
			else {
				Peekkey = c;
				precbksl = 0;
				c = '\\';
			}
		}
		if (c != '\\')
			precbksl = 0;
	}
#endif
#ifdef TRACE
	if (trace) {
		if (!techoin) {
			tfixnl();
			techoin = 1;
			fprintf(trace, "*** Input: ");
		}
		tracec(c);
	}
#endif
	lastvgk = 0;
	return (c);
}

/*
 * Get a key, but if a delete, quit or attention
 * is typed return 0 so we will abort a partial command.
 */
int
getesc(void)
{
	register int c;

	c = getkey();
	switch (c) {

	case ATTN:
	case QUIT:
		ungetkey(c);
		return (0);

	case ESCAPE:
		return (0);
	}
	return (c);
}

/*
 * Peek at the next keystroke.
 */
int
peekkey(void)
{

	Peekkey = getkey();
	return (Peekkey);
}

/*
 * Read a line from the echo area, with single character prompt c.
 * A return value of 1 means the user blewit or blewit away.
 */
int
readecho(int c)
{
	register char *sc = cursor;
	void (*OP)();
	bool waste;
	register int OPeek;

	if (WBOT == WECHO)
		vclean();
	else
		vclrech(0);
	splitw++;
	vgoto(WECHO, 0);
	ex_putchar(c);
	vclreol();
	vgoto(WECHO, 1);
	cursor = linebuf; linebuf[0] = 0; genbuf[0] = c;
	if (peekbr()) {
		if (!INS[0] || (INS[-1] & OVERBUF))
			goto blewit;
		vglobp = INS;
	}
	OP = Pline; Pline = normline;
	ignore(vgetline(0, genbuf + 1, &waste));
	vscrap();
	Pline = OP;
	if (Peekkey != ATTN && Peekkey != QUIT && Peekkey != CTRL('h')) {
		cursor = sc;
		vclreol();
		return (0);
	}
blewit:
	OPeek = Peekkey==CTRL('h') ? 0 : Peekkey; Peekkey = 0;
	splitw = 0;
	vclean();
	vshow(dot, NOLINE);
	vnline(sc);
	Peekkey = OPeek;
	return (1);
}

/*
 * A complete command has been defined for
 * the purposes of repeat, so copy it from
 * the working to the previous command buffer.
 */
void
setLAST(void)
{

	if (vglobp)
		return;
	lastreg = vreg;
	lasthad = Xhadcnt;
	lastcnt = Xcnt;
	*lastcp = 0;
	CP(lastcmd, workcmd);
}

/*
 * Gather up some more text from an insert.
 * If the insertion buffer oveflows, then destroy
 * the repeatability of the insert.
 */
void
addtext(char *cp)
{

	if (vglobp)
		return;
	addto(INS, cp);
	if (INS[-1] & OVERBUF)
		lastcmd[0] = 0;
}

void
setDEL(void)
{

	setBUF(DEL);
}

/*
 * Put text from cursor upto wcursor in BUF.
 */
void
setBUF(char *BUF)
{
	register int c;
	register char *wp = wcursor;

	c = *wp;
	*wp = 0;
	BUF[0] = 0;
	BUF[-1] = 0;
	addto(BUF, cursor);
	*wp = c;
}

static void
addto(char *buf, char *str)
{

	if (buf[-1] & OVERBUF)
		return;
	if (strlen(buf) + strlen(str) + 1 >= VBSIZE) {
		buf[-1] |= OVERBUF;
		return;
	}
	ignore(strcat(buf, str));
}

/*
 * Note a change affecting a lot of lines, or non-visible
 * lines.  If the parameter must is set, then we only want
 * to do this for open modes now; return and save for later
 * notification in visual.
 */
int
noteit(bool must)
{
	register int sdl = destline, sdc = destcol;

	if (notecnt < 2 || (!must && state == VISUAL))
		return (0);
	splitw++;
	if (WBOT == WECHO)
		vmoveitup(1, 1);
	vigoto(WECHO, 0);
	ex_printf("%d %sline", notecnt, notesgn);
	if (notecnt > 1)
		ex_putchar('s');
	if (*notenam) {
		ex_printf(" %s", notenam);
		if (*(strend(notenam) - 1) != 'e')
			ex_putchar('e');
		ex_putchar('d');
	}
	vclreol();
	notecnt = 0;
	if (state != VISUAL)
		vcnt = vcline = 0;
	splitw = 0;
	if (state == ONEOPEN || state == CRTOPEN)
		vup1();
	destline = sdl; destcol = sdc;
	return (1);
}

/*
 * Rrrrringgggggg.
 * If possible, use flash (VB).
 */
void
beep(void)
{

	if (VB)
		vputp(VB, 0);
	else
		vputc(CTRL('g'));
}

/*
 * Map the command input character c,
 * for keypads and labelled keys which do cursor
 * motions.  I.e. on an adm3a we might map ^K to ^P.
 * DM1520 for example has a lot of mappable characters.
 */

int
map(int c, struct maps *maps)
{
	register int d;
	register char *p, *q;
	char b[10];	/* Assumption: no keypad sends string longer than 10 */

	/*
	 * Mapping for special keys on the terminal only.
	 * BUG: if there's a long sequence and it matches
	 * some chars and then misses, we lose some chars.
	 *
	 * For this to work, some conditions must be met.
	 * 1) Keypad sends SHORT (2 or 3 char) strings
	 * 2) All strings sent are same length & similar
	 * 3) The user is unlikely to type the first few chars of
	 *    one of these strings very fast.
	 * Note: some code has been fixed up since the above was laid out,
	 * so conditions 1 & 2 are probably not required anymore.
	 * However, this hasn't been tested with any first char
	 * that means anything else except escape.
	 */
#ifdef MDEBUG
	if (trace)
		fprintf(trace,"map(%c): ",c);
#endif
	b[0] = c;
	b[1] = 0;
	for (d=0; maps[d].mapto; d++) {
#ifdef MDEBUG
		if (trace)
			fprintf(trace,"d=%d, ",d);
#endif
		if ((p = maps[d].cap)) {
			for (q=b; *p; p++, q++) {
#ifdef MDEBUG
				if (trace)
					fprintf(trace,"q->b[%d], ",q-b);
#endif
				if (*q==0) {
					/*
					 * This test is oversimplified, but
					 * should work mostly. It handles the
					 * case where we get an ESCAPE that
					 * wasn't part of a keypad string.
					 */
					if ((c=='#' ? peekkey() : fastpeekkey()) == 0) {
#ifdef MDEBUG
					if (trace)
						fprintf(trace,"fpk=0: return %c",c);
#endif
						macpush(&b[1],1);
						return(c);
					}
					*q = getkey();
					q[1] = 0;
				}
				if (*p != *q)
					goto contin;
			}
			macpush(maps[d].mapto,1);
			c = getkey();
#ifdef MDEBUG
	if (trace)
		fprintf(trace,"Success: return %c",c);
#endif
			return(c);	/* first char of map string */
			contin:;
		}
	}
#ifdef MDEBUG
	if (trace)
		fprintf(trace,"Fail: return %c",c); /* DEBUG */
#endif
	macpush(&b[1],0);
	return(c);
}

/*
 * Push st onto the front of vmacp. This is tricky because we have to
 * worry about where vmacp was previously pointing. We also have to
 * check for overflow (which is typically from a recursive macro)
 * Finally we have to set a flag so the whole thing can be undone.
 * canundo is 1 iff we want to be able to undo the macro.  This
 * is false for, for example, pushing back lookahead from fastpeekkey(),
 * since otherwise two fast escapes can clobber our undo.
 */
void
macpush(char *st, int canundo)
{
	char tmpbuf[BUFSIZ];

	if (st==0 || *st==0)
		return;
#ifdef TRACE
	if (trace)
		fprintf(trace, "macpush(%s)",st);
#endif
	if (vmacp && strlen(vmacp) + strlen(st) > BUFSIZ)
		error("Macro too long@ - maybe recursive?");
	if (vmacp) {
		strcpy(tmpbuf, vmacp);
		canundo = 0;	/* can't undo inside a macro anyway */
	}
	strcpy(vmacbuf, st);
	if (vmacp)
		strcat(vmacbuf, tmpbuf);
	vmacp = vmacbuf;
	/* arrange to be able to undo the whole macro */
	if (canundo) {
		inopen = -1;	/* no need to save since it had to be 1 or -1 before */
		otchng = tchng;
		vsave();
		saveall();
		vundkind = VMANY;
	}
#ifdef TRACE
	if (trace)
		fprintf(trace, "saveall for macro: undkind=%d, unddel=%d, undap1=%d, undap2=%d, dol=%d, unddol=%d, truedol=%d\n", undkind, lineno(unddel), lineno(undap1), lineno(undap2), lineno(dol), lineno(unddol), lineno(truedol));
#endif
}

/*
 * Get a count from the keyed input stream.
 * A zero count is indistinguishable from no count.
 */
int
vgetcnt(void)
{
	register int c, cnt;

	cnt = 0;
	for (;;) {
		c = getkey();
		if (!isdigit(c))
			break;
		cnt *= 10, cnt += c - '0';
	}
	ungetkey(c);
	Xhadcnt = 1;
	Xcnt = cnt;
	return(cnt);
}

/*
 * fastpeekkey is just like peekkey but insists the character come in
 * fast (within 1 second). This will succeed if it is the 2nd char of
 * a machine generated sequence (such as a function pad from an escape
 * flavor terminal) but fail for a human hitting escape then waiting.
 */
static int
fastpeekkey(void)
{
	register int c;

	if (inopen == -1)	/* don't work inside macros! */
		return (0);
	signal(SIGALRM, trapalarm);
	alarm(1);
	CATCH
		c = peekkey();
#ifdef MDEBUG
	if (trace)
		fprintf(trace,"[OK]",c);
#endif
		alarm(0);
	ONERR
		c = 0;
#ifdef MDEBUG
	if (trace)
		fprintf(trace,"[TOUT]",c);
#endif
	ENDCATCH
#ifdef MDEBUG
	if (trace)
		fprintf(trace,"[fpk:%o]",c);
#endif
	return(c);
}

static void
trapalarm(int i) {
	(void)i;
	alarm(0);
	longjmp(vreslab,1);
}
