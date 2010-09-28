/* nlParse.h for Nightingale notelist parser. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#if defined(THINK_C) || defined(__MWERKS__)
	#include <console.h>
#endif

#define TRUE	1
#define FALSE	0
#ifndef THINK_C
	#define Boolean unsigned char 
#endif

/* The following structs are related to but are not at all the same as Nightingale's
structs with the same names. */

typedef struct {
	char	stf;
	char	type;
} ACLEF;

typedef struct {
	char	stf;
	char	numAcc;
	char	shpFlt;
	char	filler;
} AKEYSIG;

typedef struct {
	char	stf;
	char	num;
	char	denom;
	char	displ;
} ATIMESIG;

typedef struct {
	unsigned char	modCode;
	char				data;
} AMOD;

typedef struct {
	long		t;
	char		v;
	char		npt;
	char		stf;
	char		dur;
	char		dots;
	char		nn;
	char		acc;
	char		eAcc;
	char		vel;
	short		pDur;
	Boolean	tiedL:1;					/* TRUE if note is tied to previous note */
	Boolean	tiedR:1;					/* TRUE if note is tied to next note */
	Boolean	slurredL:1;				/* TRUE if a slur extends from this note to the left */
	Boolean	slurredR:1;				/* TRUE if a slur extends from this note to the right */
	Boolean	inTuplet:1;				/* TRUE if note is in a tuplet */
	Boolean	inChord:1;				/* TRUE if note is in a chord (in same voice on same staff) */
	Boolean	mainNote:1;				/* TRUE if note marks termination of the stem */
	Boolean	isGrace:1;				/* TRUE if note is a grace note */
	char		appearance;
	short		numMods;
	AMOD		*modList;				/* pointer to list of mods, dynamically allocated */
} ANOTE;

typedef struct {
	long		t;
	char		v;
	char		npt;
	char		stf;
	char		dur;
	char		dots;
	Boolean	inTuplet:1;				/* TRUE if rest is in a tuplet */
	Boolean	bFiller:7;
	char		appearance;
	char		filler;
	short		numMods;
	AMOD		*modList;				/* pointer to list of mods, dynamically allocated */
} AREST;

typedef struct {
	long	t;
	char	type;
} ABARLINE;

typedef struct {
	char	v;
	char	npt;
	char	num;
	char	denom;
	char	appearance;
} ATUPLET;


typedef struct {
	char	stf;
	char	type;
} ADYNAMIC;

typedef struct {
	long	t;						/* Not in Notelist but required by MEF */
	char	v;
	char	npt;
	char	stf;
	char	code;
	char	style;				/* from 1 to 5, or 0 = unknown */
	char	textStr[256];
} ATEXT;

typedef struct {
	char		stf;
	char		metroUnit;
	Boolean	dotted;
	char		filler;
	char		tempoStr[64];		/* Nightingale's max length: cf. its InsertTempo()  */
	char		metroStr[64];		/* Nightingale's max length: cf. its InsertTempo()  */
} ATEMPO;


typedef struct {
	char	v;
	char	npt;
	char    count;
} ABEAM;

enum {									/* modCode values */
	MOD_FERMATA=10,					/* Leave 0 thru 9 for digits */
	MOD_TRILL,
	MOD_ACCENT,
	MOD_HEAVYACCENT,
	MOD_STACCATO,
	MOD_WEDGE,
	MOD_TENUTO,
	MOD_MORDENT,
	MOD_INV_MORDENT,
	MOD_TURN,
	MOD_PLUS,
	MOD_CIRCLE,
	MOD_UPBOW,
	MOD_DOWNBOW,
	MOD_TREMOLO1,
	MOD_TREMOLO2,
	MOD_TREMOLO3,
	MOD_TREMOLO4,
	MOD_TREMOLO5,
	MOD_TREMOLO6,
	MOD_HEAVYACC_STACC,
	MOD_LONG_INVMORDENT
};


enum {								/* Notehead appearances: */
	NO_VIS=0,						/* Invisible */
	NORMAL_VIS,						/* Normal appearance */
	X_SHAPE,							/* "X" head (notes only) */
	HARMONIC_SHAPE,				/* "Harmonic" hollow head (notes only) */
	SQUAREH_SHAPE,					/* Square hollow head (notes only) */
	SQUAREF_SHAPE,					/* Square filled head (notes only) */
	DIAMONDH_SHAPE,				/* Diamond-shaped hollow head (notes only) */
	DIAMONDF_SHAPE,				/* Diamond-shaped filled head (notes only) */
	HALFNOTE_SHAPE,				/* Halfnote head (for Schenker, etc.) (notes only) */
	SLASH_SHAPE						/* Chord slash */
};


#define MAX_LINELENGTH 1024

#define MAXVOICES 100					/* Maximum voice no. in score, from Ngale's <NLimits.h> */
#define MAXSTAVES 64						/* Maximum number of staves attached to a system, from Ngale's <NLimits.h> */
#define MAXPARTS MAXSTAVES				/* Maximum number of parts */
#define MAX_MODNRS 20					/* Max. modifiers on a note/grace note/rest */

#define STATUS_PRINTF printfXMLComment			/* Print status/error information (perhaps to <stderr>) */

#define NOTE_CHAR			'N'
#define GRACE_CHAR		'G'
#define REST_CHAR			'R'
#define TUPLET_CHAR		'P'
#define BAR_CHAR			'/'
#define CLEF_CHAR			'C'
#define KEYSIG_CHAR		'K'
#define TIMESIG_CHAR		'T'
#define TEMPO_CHAR		'M'
#define GRAPHIC_CHAR		'A'
#define DYNAMIC_CHAR		'D'
#define BEAM_CHAR			'B'
#define COMMENT_CHAR		'%'


/* Prototypes for public functions */

Boolean ExtractLong(char *, long *);
Boolean ExtractNoteFlags(char *, ANOTE *);
Boolean ExtractNoteMods(char *modStr, short *numMods, AMOD tmpMods[]);
long ExtractDelimString(char string[], char delimL, char delimR, char subStr[], long subStrMaxLen);
short NotelistVersion(void);
void ReportParseFailure(char *);
char *fgetsAllPlatform(char *s, int n, FILE *fp);

void CopyLine(void);

Boolean ProcessClef(ACLEF clef);
Boolean ProcessKeySig(AKEYSIG keySig);
Boolean ProcessTimeSig(ATIMESIG timeSig);
/* Boolean ProcessGraceNote(ANOTE note); */
Boolean ProcessNote(ANOTE note, Boolean isRest, Boolean isGraceNote);
Boolean ProcessRest(AREST rest);
Boolean ProcessBarline(ABARLINE barLine);
Boolean ProcessTuplet(ATUPLET tuplet);
Boolean ProcessDynamic(ADYNAMIC dynamic);
Boolean ProcessText(ATEXT text);
Boolean ProcessTempoMark(ATEMPO tempo);
Boolean ProcessBeam(ABEAM beam);
Boolean ProcessStructComment(void);

Boolean Initialize(void);
Boolean CleanUp(void);
