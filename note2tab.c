/*
 * Copyright (c) 2013 Marcel Kaiser. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>

#if defined( __FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
# undef __BSD__
# define __BSD__ 1
# include <err.h>
#else	/* !defined(__FreeBSD__) || ... */
# include <stdarg.h>
#endif	/* defined(__FreeBSD__) || ... */

#define MAXACCS	 9	/* Max. # of accidentals in key. */
#define MAXNOTES 6	/* Max. # of notes in a chord */
#define MAXFRET  24

/* G clef */
const char *g_scale[]  = {
	"d", "e", "f", "g", "a", "h", "c"
};

/* F clef */
const char *f_scale[]  = {
	"f", "g", "a", "h", "c", "d", "e"
};

/* Chromatic scale */
const char *c_scale[] = { 
	"c",  "#c", "d", "#d", "e", "f",
	"#f", "g",  "#g", "a", "b", "h"
};

typedef struct note_s {
	char pos;	/* Position on the staff */
	char acc;	/* Accidental */
	char name;	/* Index of the note name in c_scale[] */
} note_t;

struct key_s {
	char   n;	/* # of notes. */
	note_t notes[MAXACCS];
} key;

typedef struct tuning_s {
	const char   string;
	const note_t note;
} tuning_t;

struct note2tab_s {
	char	       fret;
	const tuning_t tuning;
} tab[] = {
	{ -1, { 'E', { -6, 0,  4 } } },
	{ -1, { 'A', { -3, 0,  9 } } },
	{ -1, { 'D', {  0, 0,  2 } } },
	{ -1, { 'G', {  3, 0,  7 } } },
	{ -1, { 'b', {  5, 0, 11 } } },
	{ -1, { 'e', {  8, 0,  4 } } }
};

static void	  translate(char *, int);
static void	  show_tab(void);
static void	  note2tab(note_t *);
static void	  chord2tab(char *, int);
static void	  define_key(char *);
static void	  usage(void);
#ifndef __BSD__
static void	  errx(int, const char *, ...);
#endif

static int	  nnotes = 0;
static char	  clef = 'g';
static bool	  oneline = false;
static note_t	  notes[MAXNOTES];
static const char **scale = g_scale;

int
main(int argc, char *argv[])
{
	int shift, sflag, cflag, kflag;
	
	if (argc == 1)
		usage();
	shift = sflag = cflag = kflag = 0;
	while (argc-- > 1) {
		++argv; nnotes = 0;
		if (strcmp(*argv, "-l") == 0)
			oneline = true;
		else if (strcmp(*argv, "-s") == 0) {
			if (sflag++ != 0)
				usage();
			argc--; argv++;
			shift = strtol(*argv, NULL, 10);
		} else if (strcmp(*argv, "-c") == 0) {
			if (cflag++ != 0)
				usage();
			argc--; argv++;
			if (*argv[0] == 'g') {
				scale = g_scale;
				clef = 'g';
			} else if (*argv[0] == 'f') {
				scale = f_scale;
				clef = 'f';
			} else
				errx(EXIT_FAILURE, "Invalid clef");
		} else if (strcmp(*argv, "-k") == 0) {
			if (kflag++ != 0)
				usage();
			argc--; argv++;
			define_key(*argv);
		} else if (*argv[0] == '(')
			chord2tab(*argv, shift);
		else {
			translate(*argv, shift);
			note2tab(&notes[0]);
			show_tab();
		}
	}
	if (nnotes == 0)
		usage();
	return (0);
}

static void
note2tab(note_t *note)
{
	int i, j, n, p;

	for (i = 0; i < 6; i++)
		tab[i].fret = -1;
	for (i = 0; i < 6; i++) {
		n = tab[i].tuning.note.name;
		p = tab[i].tuning.note.pos + (clef == 'f' ? 12 : 0);
		for (j = 0; j < MAXFRET; j++) {
			if (note->name == n && note->pos == p) {
				tab[i].fret = j;
				break;
			}
			n = (n + 1) % 12;
			if (c_scale[n][0] != '#' && c_scale[n][0] != 'b')
				p++;
		}
	}
}

static void
chord2tab(char *chord, int shift)
{
	int    i, j, k, n;
	bool   unsorted;
	char   strings[6];
	note_t tmp;

	if (*chord == '(')
		chord++;
	for (; (chord = strtok(chord, " )")) != NULL; chord = NULL)
		translate(chord, shift);
	do {
		unsorted = false;
		for (i = 0; i < nnotes - 1; i++) {
			if (notes[i].pos > notes[i + 1].pos) {
				tmp	     = notes[i];
				notes[i]     = notes[i + 1];
				notes[i + 1] = tmp;
				unsorted = true;
			}
		}
	} while (unsorted);

	for (k = 0; 6 - k >= nnotes; k++) {
		for (i = 0; i < 6; i++)
			strings[i] = -1;
		for (i = 0; i < nnotes; i++) {
			note2tab(&notes[i]);
			for (j = k; j < 6; j++) {
				if (strings[j] != -1)
					continue;
				else if (tab[j].fret != -1) {
					strings[j] = tab[j].fret;
					break;
				}
			}
		}
		for (i = n = 0; i < 6; i++) {
			if ((tab[i].fret = strings[i]) != -1)
				n++;
		}
		if (n < nnotes)
			return;	
		if (k > 0 && !oneline)
			(void)putchar('\n');
		show_tab();
	}
}

static void
show_tab()
{
	int i;
	
	if (oneline) {
		for (i = 0; i < 6; i++) {
			if (tab[i].fret != -1) {
				(void)printf("%c%d%s", tab[i].tuning.string,
				    tab[i].fret, i < 5 ? " " : "\n");
			} else if (i == 5)
				(void)putchar('\n');
		}
		return;
	}
	for (i = 5; i >= 0; i--) {
		if (tab[i].fret != -1) {
			if (tab[i].fret >= 10) {
				(void)printf("%c|--%d--|\n",
				    tab[i].tuning.string, tab[i].fret);
			} else {
				(void)printf("%c|---%d--|\n",
				    tab[i].tuning.string, tab[i].fret);
			}
		} else
			(void)printf("%c|------|\n", tab[i].tuning.string);
	}
}

static void
translate(char *str, int shift)
{
	int  i, j, l, pos, n, k;

	switch (*str) {
	case '#':
		notes[nnotes].acc = 1;
		str++;
		break;
	case 'b':
		notes[nnotes].acc = -1;
		str++;
		break;
	case '%':
		notes[nnotes].acc = '%';
		str++;
		break;
	default:
		if ((str[0] != '-' && !isdigit(str[1])) &&
		    !isdigit(str[0]))
			errx(EXIT_FAILURE, "'%c': Invalid accidental", *str);
		notes[nnotes].acc = 0;
	}
	if (nnotes >= MAXNOTES) {
		errx(EXIT_FAILURE, "A chord must not exceed %d notes",
		    MAXNOTES);
	}
	notes[nnotes].pos = pos = strtol(str, NULL, 10);

	for (j = pos; j < 0; j += 7)
		;
	j %= 7;
	for (i = 0; i < 12; i++) {
		if (strcmp(scale[j], c_scale[i]) == 0)
			break;
	}
	for (k = 0; k < key.n && notes[nnotes].acc != '%'; k++) {
		if (abs(notes[nnotes].pos - key.notes[k].pos) % 7 == 0) {
			notes[nnotes].acc += key.notes[k].acc;
			break;
		}
	}
	for (k = notes[nnotes].acc; k < 0 && notes[nnotes].acc != '%'; k++) {
		/* b accidental. */
		i = (i + 12 - 1) % 12;
		notes[nnotes].pos--;
	}
	for (k = notes[nnotes].acc; k > 0 && notes[nnotes].acc != '%'; k--) {
		/* # accidental. */
		i = (i + 1) % 12;
		if (c_scale[i][0] != '#' && c_scale[i][0] != 'b')
			notes[nnotes].pos++;
	}
	if (shift != 0)
		l = k = shift / abs(shift);
	for (n = abs(shift); n > 0; n--) {
		if (c_scale[(i + l + 12) % 12][0] != '#' &&
		    c_scale[(i + l + 12) % 12][0] != 'b')
			notes[nnotes].pos += k;
		l += k;
	}
	while (shift < 0)
		shift += 12;
	i = (i + shift) % 12;
	notes[nnotes].name = i;
	nnotes++;
}

static void
define_key(char *s)
{
	int pos;

	for (key.n = 0; (s = strtok(s, " ")) != NULL; s = NULL) {
		if (key.n >= MAXACCS)
			errx(EXIT_FAILURE, "Too many accidentals");
		if (*s != 'b' && *s != '#')
			errx(EXIT_FAILURE, "'%c': invalid accidental", *s);
		key.notes[(int)key.n].acc = *s == 'b' ? -1 : 1;
		s++;
		pos = strtol(s, NULL, 10);
		if (pos < 0 || pos > 11)
			errx(EXIT_FAILURE, "%d out of range", pos);
		key.notes[(int)key.n].pos = pos;
		key.n++;
	}
}

static void
usage()
{

	(void)printf("Usage: %s [-l][-k key][-s shift][-c clef] " \
	    "[(][#|b|%%]pos ...[)]\n", PROGRAM);
	exit(1);
}

#ifndef __BSD__
static void
errx(int eval, const char *fmt, ...)
{
	va_list argvp;

	va_start(argvp, fmt);
	(void)fprintf(stderr, "%s: ", PROGRAM);
	(void)vfprintf(stderr, fmt, argvp);
	exit(eval);
}
#endif	/* !__BSD__ */

