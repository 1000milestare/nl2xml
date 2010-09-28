/* NotelistParser, version 2.4

Sample parser for Nightingale's notelist files. Reads input from stdin and
writes output to stdout (on Macintosh, using Symantec C's ccommand dialog interface).
For each valid line, fills in fields in a struct, which it does nothing with other
than writing out. The effect is to "canonize" notelists by (1) converting older
formats to the current format and (2) adding explicit default values for omitted
fields. However, it could easily be used as the basis for programs to process
notelists in various ways, especially to modify them for compositional, analytic,
or other purposes.

Original version by John Gibson, 1992; revisions by Donald Byrd, 1997-98.

Revisions by Donald Byrd (with Tim Crawford), Oct. 2000 - September 2001:
	- Make more portable (across platforms)
	- Optionally use an input file given on the command line
	- Improve error handling
	- Parse _every_ field of every line
	- Unless SUMMARY_ONLY is #defined, echo input from the parsed lines
	- Ignore empty lines
	- Make it handle notelist files from all platforms (handle all newline sequences)

Ported to CodeWarrior 5.3 by Donald Byrd, August 2002.

Further improvements from NotelistModify ported to this program, May 2003; the most
important is fixing a memory-management bug that caused crashes on notes with "mod=".

NB: Tab stops in this file should be set to 3 spaces. */


#include "nlParse.h"

/* Formats of lines in a Notelist file:

Note				N t=%ld v=%d npt=%d stf=%d dur=%d dots=%d nn=%d acc=%d eAcc=%d pDur=%d vel=%d %s appear=%d
Rest				R t=%ld v=%d npt=%d stf=%d dur=%d dots=%d %s appear=%d
Grace note		G t=-1 v=%d npt=%d stf=%d dur=%d dots=%d nn=%d acc=%d eAcc=%d pDur=%d vel=%d %c appear=%d
Barline			/ t=%ld type=%d
Clef				C stf=%d type=%d
Key signature	K stf=%d KS=%d %c
Time signature	T stf=%d num=%d denom=%d
Dynamic			D stf=%d type=%d
Text				A v=%d npt=%d stf=%d %c '%s'
Tempo mark		M stf=%d '%s' %c[.]=%s
Tuplet			P v=%d npt=%d num=%d denom=%d appear=%d
Comment			% (anything; lines beginning "%%" should be structured comments)

*/

int main(int, char **);

void printfXMLComment(char *comment);

void ParseAndProcessNotelist(FILE *file);
Boolean ParseAndProcessClef(void);
Boolean ParseAndProcessKeySig(void);
Boolean ParseAndProcessTimeSig(void);
Boolean ParseNote(ANOTE *pNote);
Boolean ParseAndProcessNote(void);
Boolean ParseRest(AREST *pRest);
Boolean ParseAndProcessRest(void);
Boolean ParseAndProcessBarline(void);
Boolean ParseAndProcessTuplet(void);
Boolean ParseAndProcessDynamic(void);
Boolean ParseAndProcessText(void);
Boolean ParseAndProcessTempoMark(void);
Boolean ParseAndProcessBeam(void);

char	gInBuf[MAX_LINELENGTH];
short gErrorCount;
short gNotelistVersion;

short gTiedNoteCount;
short	gLineCount;

short gClefCount = 0;
short	gKeySigCount = 0;
short	gTimeSigCount = 0;
short	gNoteCount = 0;
short	gGraceNoteCount = 0;
short	gRestCount = 0;
short	gBarlineCount = 0;
short	gTupletCount = 0;
short	gDynamicCount = 0;
short	gTextCount = 0;
short	gTempoCount = 0;
short	gBeamCount = 0;

char	gFilename[64];
short	gPartStaves[MAXPARTS+1];	/* 1-based array giving number of staves (value) in each part (index) */
short	gNumNLStaves;					/* number of staves in Notelist system */
short	gFirstMNNumber;				/* number of first measure */
Boolean		gDelAccs;						/* delete redundant accidentals? */

static Boolean ParseAndProcessStructComment(void);

/************************************************************************************/

int main(int argc, char	**argv)
{
	FILE *theFile;
	
#ifdef THINK_C
	/* Adjust size (default is 25 rows of 80 chars) and placement of console window */
	console_options.top = 50;	console_options.left = 10;
	console_options.nrows = 40;	console_options.ncols = 90;
#endif
#if defined(THINK_C) || defined(__MWERKS__)
	argc = ccommand(&argv);						/* provides Unix-like redirection of stdin and stdout */
#endif
	
	(void)Initialize();
	
	if (argc>1)
		theFile = fopen(argv[1], "r");
	else
		theFile = stdin;
	if (!theFile) {
		STATUS_PRINTF("ERROR: can't open the input file.\n");
		return 1;
	}

	/*
   	printf ( "%% NotelistParser 2.4 - sample Nightingale Notelist parser - version of 18 May 2003\n\n");
	*/

	ParseAndProcessNotelist(theFile);

	fclose(theFile); 								/* almost certainly unnecessary */
	(void)CleanUp();

	return 0;
}


void ParseAndProcessNotelist(FILE *file)
{
	char firstChar;
	
	if (!fgetsAllPlatform(gInBuf, MAX_LINELENGTH, file)) {
		STATUS_PRINTF("ERROR: can't read first line of notelist.\n");
		return;
	}
	gNotelistVersion = NotelistVersion();
	if (gNotelistVersion<0) return;

	if (!ParseAndProcessStructComment()) {
		STATUS_PRINTF("FATAL ERROR (ParseAndProcessNotelist): problem with notelist header line.\n");
		return;
	}

	gLineCount = 1;

	/* parse successive lines of input file */
	while (fgetsAllPlatform(gInBuf, MAX_LINELENGTH, file)) {
		gLineCount++;
		
		/* dispatch to specific parsing routines depending on first char */
		sscanf(gInBuf, "%c", &firstChar);
		if (firstChar=='\n' || firstChar=='\r') continue;				/* Ignore empty lines */
		
		switch (firstChar) {
			case CLEF_CHAR:
				if (!ParseAndProcessClef()) gErrorCount++;
				break;
			case KEYSIG_CHAR:
				if (!ParseAndProcessKeySig()) gErrorCount++;
				break;
			case TIMESIG_CHAR:
				if (!ParseAndProcessTimeSig()) gErrorCount++;
				break;
			case NOTE_CHAR:
			case GRACE_CHAR:
				if (!ParseAndProcessNote()) gErrorCount++;
				break;
			case REST_CHAR:
				if (!ParseAndProcessRest()) gErrorCount++;
				break;
			case BAR_CHAR:
				if (!ParseAndProcessBarline()) gErrorCount++;
				break;
			case TUPLET_CHAR:
				if (!ParseAndProcessTuplet()) gErrorCount++;
				break;
			case DYNAMIC_CHAR:
				if (!ParseAndProcessDynamic()) gErrorCount++;
				break;
			case GRAPHIC_CHAR:
				if (!ParseAndProcessText()) gErrorCount++;
				break;
			case TEMPO_CHAR:
				if (!ParseAndProcessTempoMark()) gErrorCount++;
				break;
			case BEAM_CHAR:
				if (!ParseAndProcessBeam()) gErrorCount++;
				break;
			case '*':											/* it's a comment (long-obsolete form) */
			case COMMENT_CHAR:								/* it's a comment */
				CopyLine();
				break;
			default:
				ReportParseFailure("ParseAndProcessNotelist");
				gErrorCount++;
		}
		
			if (gErrorCount>25) {
				STATUS_PRINTF("FATAL ERROR (ParseAndProcessNotelist): TOO MANY ERRORS.\n");
				return;
			}
	}
}


/* Parse a line like this:
 *		C stf=1 type=2
 */

Boolean ParseAndProcessClef()
{
	char		stfStr[32], typeStr[32];
	short		count;
	long		along;
	ACLEF		clef;

	count = sscanf(gInBuf, "%*c%s%s", stfStr, typeStr);
	if (count<2) goto Broken;
	
	if (!ExtractLong(stfStr, &along)) goto Broken;
	clef.stf = along;
	
	if (!ExtractLong(typeStr, &along)) goto Broken;
	clef.type = along;

	/* Now handle the clef as needed. */
	ProcessClef(clef);

	gClefCount++;
	return TRUE;
Broken:
	ReportParseFailure("ParseAndProcessClef");
	return FALSE;
}


/* Parse a line like this:
 *		K stf=1 KS=2 #
 */

Boolean ParseAndProcessKeySig()
{
	char		stfStr[32], numAccStr[32], shpFltStr[32];
	short		count;
	long		along;
	AKEYSIG	keySig;

	count = sscanf(gInBuf, "%*c%s%s%s", stfStr, numAccStr, shpFltStr);
	if (count<3) goto Broken;
	
	if (!ExtractLong(stfStr, &along)) goto Broken;
	keySig.stf = along;
	
	if (!ExtractLong(numAccStr, &along)) goto Broken;
	keySig.numAcc = along;

	if (!(*shpFltStr=='#' || *shpFltStr=='b')) goto Broken;
	keySig.shpFlt = *shpFltStr;
	
	/* Now handle the keySig as needed. */
	ProcessKeySig(keySig);

	gKeySigCount++;
	return TRUE;
Broken:
	ReportParseFailure("ParseAndProcessKeySig");
	return FALSE;
}


/* Parse a line like this:
 *		T stf=1 num=2 denom=2 displ=3
 */

Boolean ParseAndProcessTimeSig()
{
	char		stfStr[32], numStr[32], denomStr[32], displStr[32];
	short		count;
	long		along;
	ATIMESIG	timeSig;

	count = sscanf(gInBuf, "%*c%s%s%s%s", stfStr, numStr, denomStr, displStr);
	if (count<3) goto Broken;
	
	if (!ExtractLong(stfStr, &along)) goto Broken;
	timeSig.stf = along;
	
	if (!ExtractLong(numStr, &along)) goto Broken;
	timeSig.num = along;

	if (!ExtractLong(denomStr, &along)) goto Broken;
	timeSig.denom = along;
	
	timeSig.displ = 1;												/* Default = normal timesig (n/d) */
	if (count>3) {
		if (!ExtractLong(displStr, &along)) goto Broken;
		timeSig.displ = along;
	}
	
	/* Now handle the timeSig as needed. */
	ProcessTimeSig(timeSig);

	gTimeSigCount++;
	return TRUE;
Broken:
	ReportParseFailure("ParseAndProcessTimeSig");
	return FALSE;
}


#define NLERR_BADTIME(t)		(t<0L)
#define NLERR_BADVOICE(v)		(v<1L || v>31L)
#define NLERR_BADPART(npt)		(npt<1L || npt>(long)MAXSTAVES)
#define NLERR_BADSTAFF(stf)		(stf<1L || stf>(long)MAXSTAVES)
#define NLERR_BADNOTENUM(nn)	(nn<0L || nn>127L)
#define NLERR_BADPDUR(pdur)		(pdur<0L || pdur>32000L)
#define NLERR_BADVELOCITY(vel)	(vel<0L || vel>127L)

/* Parse a line like these:
 * 	N t=0 v=1 npt=1 stf=1 dur=4 dots=0 nn=72 acc=0 eAcc=3 pDur=456 vel=75 ...... appear=3
 * 	G t=-1 v=1 npt=1 stf=1 dur=5 dots=0 nn=74 acc=0 eAcc=3 pDur=240 vel=75 . appear=1
 * 	N t=480 v=1 npt=1 stf=1 dur=4 dots=0 nn=70 acc=2 eAcc=2 pDur=456 vel=75 ...... appear=1 mods=14,12
 *
 * This is a bit tricky because note lines contain a variable number of fields, and
 * because grace notes have only one flag and no modifiers. The line will normally have
 * a field of the form "appear=x", but in very early Nightingale notelists, that field is
 * present only if the note/grace note's appearance is something other than NORMAL_VIS (=1).
 * If the note has any modifiers, an optional mods field follows the flags field (or the
 * appearance field, if  there is one). The mods field gives a list of modCodes separated
 * by commas, with optional data values trailing the modCode (and separated from it by a
 * colon). E.g.:
 *				mods=11				[just 1 modifier having modCode=11]
 *				mods=0,2,3			[3 modifiers]
 *				mods=3:50,2			[2 modifiers, the first having a data value of 50]
 * To sort all this out, we note the number of words read by sscanf (returned in <count>)
 * and then switch on that to do the appropriate parsing.
 *
 * IMPORTANT: the calling function must allocate <pNote->modList> before calling this!
 */

Boolean ParseNote(ANOTE *pNote)
{
	char		noteKind, timeStr[32], voiceStr[32], partStr[32], staffStr[32], durStr[32],
				dotsStr[32], noteNumStr[32], accStr[32], eAccStr[32], pDurStr[32],
				velStr[32], flagsStr[32], penultStr[64], finalStr[64];
	short		count, numMods, j;
	long		along;
	AMOD		tmpMods[MAX_MODNRS];

	pNote->appearance = NORMAL_VIS;
	pNote->numMods = 0;
	
	count = sscanf(gInBuf, "%c%s%s%s%s%s%s%s%s%s%s%s%s%s%s", &noteKind,
					timeStr, voiceStr, partStr, staffStr, durStr, dotsStr,
					noteNumStr, accStr, eAccStr, pDurStr, velStr, flagsStr,
					penultStr, finalStr);
	if (count<13) goto Broken;
	
	pNote->isGrace = (noteKind=='G');

	if (!ExtractLong(timeStr, &along)) goto Broken;
	if (!pNote->isGrace && NLERR_BADTIME(along)) goto Broken;				/* grace note time is ignored */
	pNote->t = along;
	
	if (!ExtractLong(voiceStr, &along)) goto Broken;
	if (NLERR_BADVOICE(along)) goto Broken;						/* NB: user, not internal, voice */
	pNote->v = along;

	if (!ExtractLong(partStr, &along)) goto Broken;
	if (NLERR_BADPART(along)) goto Broken;
	pNote->npt = along;
	
	if (!ExtractLong(staffStr, &along)) goto Broken;
	if (NLERR_BADSTAFF(along)) goto Broken;
	pNote->stf = along;

	if (!ExtractLong(durStr, &along)) goto Broken;
	pNote->dur = along;

	if (!ExtractLong(dotsStr, &along)) goto Broken;
	pNote->dots = along;

	if (!ExtractLong(noteNumStr, &along)) goto Broken;
	if (NLERR_BADNOTENUM(along)) goto Broken;
	pNote->nn = along;

	if (!ExtractLong(accStr, &along)) goto Broken;
	pNote->acc = along;

	if (!ExtractLong(eAccStr, &along)) goto Broken;
	pNote->eAcc = along;

	if (!ExtractLong(pDurStr, &along)) goto Broken;
	if (!pNote->isGrace && NLERR_BADPDUR(along)) goto Broken;
	pNote->pDur = along;

	if (!ExtractLong(velStr, &along)) goto Broken;
	if (NLERR_BADVELOCITY(along)) goto Broken;
	pNote->vel = along;

	if (!ExtractNoteFlags(flagsStr, pNote)) goto Broken;

	switch (count) {
		case 13:				/* no appearance or mod strings */
			break;
		case 15:				/* has appearance AND mod strings (in that order) */
			if (!ExtractNoteMods(finalStr, &numMods, tmpMods)) goto Broken;
			if (numMods<0 || numMods>=MAX_MODNRS) { printf("BUG: numMods=%d!\n", numMods); goto Broken; }
			pNote->numMods = numMods;
			for (j = 0; j<numMods; j++)
				pNote->modList[j] = tmpMods[j];
		case 14:				/* has appearance OR mod string */
			if (!strncmp(penultStr, "appear=", (size_t)7)) {	/* it's an appearance string */
				if (!ExtractLong(penultStr, &along)) goto Broken;
				pNote->appearance = along;
			}
			else {
				if (!ExtractNoteMods(finalStr, &numMods, tmpMods)) goto Broken;
				if (numMods<0 || numMods>=MAX_MODNRS) { printf("BUG: numMods=%d!\n", numMods); goto Broken; }
				pNote->numMods = numMods;
				for (j = 0; j<numMods; j++)
					pNote->modList[j] = tmpMods[j];
				}
			break;
		default:
			goto Broken;
	}

	return TRUE;

Broken:
	ReportParseFailure("ParseNote");
	return FALSE;
}


Boolean ParseAndProcessNote()
{
	ANOTE		note;
	Boolean	okay;
	
	size_t	modListSize;
	
	modListSize = sizeof(AMOD) * MAX_MODNRS;
	note.modList = (AMOD *)malloc(modListSize);
	if (!note.modList) {
		ReportParseFailure("ParseAndProcessNote/malloc");
		return FALSE;
	}

	okay = ParseNote(&note);
	
	/* Now handle the note as needed. */	
	if (okay) {
		if (note.isGrace) ProcessNote(note, FALSE, TRUE);
		else              ProcessNote(note, FALSE, FALSE);
	}

	/* Don't delete the next line or you'll have a memory leak! */
	if (note.modList) free(note.modList);
	if (note.isGrace) gGraceNoteCount++;
	else					gNoteCount++;

	return okay;
}


/* Parse a line like these:
 *		R t=9810 v=1 npt=1 stf=1 dur=3 dots=0 ...... appear=1 mods=10
 *		R t=480 v=1 npt=1 stf=1 dur=4 dots=0 ...... appear=1
 *		??Of the flags, we handle only <inTuplet>; actually, the slurred and
 *			and tied flags can occur on rests, though they rarely do.
 *
 * IMPORTANT: the calling function must allocate <pRest->modList> before calling this!
 */

Boolean ParseRest(AREST *pRest)
{
	char		timeStr[32], voiceStr[32], partStr[32], staffStr[32], durStr[32],
				dotsStr[32], flagsStr[32], appearStr[32], modsStr[64];
	short		count, numMods, j;
	long		along;
	ANOTE		tempNote;
	AMOD		tmpMods[MAX_MODNRS];

	count = sscanf(gInBuf, "%*c%s%s%s%s%s%s%s%s%s", timeStr, voiceStr, partStr, staffStr,
					durStr, dotsStr, flagsStr, appearStr, modsStr);
	if (count<8) goto Broken;
	
	if (!ExtractLong(timeStr, &along)) goto Broken;
	if (NLERR_BADTIME(along)) goto Broken;
	pRest->t = along;
	
	if (!ExtractLong(voiceStr, &along)) goto Broken;
	if (NLERR_BADVOICE(along)) goto Broken;						/* NB: user, not internal, voice */
	pRest->v = along;

	if (!ExtractLong(partStr, &along)) goto Broken;
	if (NLERR_BADPART(along)) goto Broken;
	pRest->npt = along;
	
	if (!ExtractLong(staffStr, &along)) goto Broken;
	if (NLERR_BADSTAFF(along)) goto Broken;
	pRest->stf = along;

	if (!ExtractLong(durStr, &along)) goto Broken;
	pRest->dur = along;

	if (!ExtractLong(dotsStr, &along)) goto Broken;
	pRest->dots = along;

	tempNote.isGrace = FALSE;											/* Rest lines have all note flags */
	if (!ExtractNoteFlags(flagsStr, &tempNote)) goto Broken;
	pRest->inTuplet = tempNote.inTuplet;

	if (!ExtractLong(appearStr, &along)) goto Broken;
	pRest->appearance = along;

	pRest->numMods = 0;

	if (count>=9) {
		if (!ExtractNoteMods(modsStr, &numMods, tmpMods)) goto Broken;
		if (numMods<0 || numMods>=MAX_MODNRS) { printf("BUG: numMods=%d!\n", numMods); goto Broken; }
		pRest->numMods = numMods;
		for (j = 0; j<numMods; j++)
			pRest->modList[j] = tmpMods[j];
	}

	return TRUE;

Broken:
	ReportParseFailure("ParseRest");
	return FALSE;
}


Boolean ParseAndProcessRest()
{
	AREST		rest;
	Boolean	okay;
	size_t	modListSize;
	
	modListSize = sizeof(AMOD) * MAX_MODNRS;
	rest.modList = (AMOD *)malloc(modListSize);
	if (!rest.modList) {
		ReportParseFailure("ParseAndProcessRest/malloc");
		return FALSE;
	}

	ANOTE note;
	
	okay = ParseRest(&rest);
	
	/* Now handle the rest as needed. */	

	if (okay) {
		/* here's a hack call ProcessNote with our rest struct */
		/* and assign it all the values we need from our AREST */

		note.t = rest.t;
		note.v = rest.v;
		note.npt = rest.npt;
		note.stf = rest.stf;
		note.dur = rest.dur;
		note.dots = rest.dots;
		note.nn = ' ';
		note.acc = 0;
		note.eAcc = 0;
		note.vel = ' ';
		note.pDur = 0;
		note.tiedL = FALSE;
		note.tiedR = FALSE;
		note.slurredL = FALSE;
		note.slurredR = FALSE;
		note.inTuplet = rest.inTuplet;
		note.inChord = FALSE;
		note.mainNote = FALSE;
		note.isGrace = FALSE;
		note.appearance = rest.appearance;
		note.numMods = rest.numMods;
/*	AMOD		*modList;*/

/*		ProcessRest(rest); */

		/* for some reason first rest on each staff contains null/zero values */
		if (note.dur != 0) {
			ProcessNote(note, TRUE, FALSE);
		}
	}
	/* Don't delete the next line or you'll have a memory leak! */
	if (rest.modList) free(rest.modList);
	gRestCount++;

	return okay;
}


/* Parse a line like this:
 *		/ t=1440 type=1
 */

Boolean ParseAndProcessBarline()
{
	char		timeStr[32], typeStr[32];
	short		count;
	long		along;
	ABARLINE	barLine;

	count = sscanf(gInBuf, "%*c%s%s", timeStr, typeStr);
	if (count<2) goto Broken;
	
	if (!ExtractLong(timeStr, &along)) goto Broken;
	if (NLERR_BADTIME(along)) goto Broken;
	barLine.t = along;
	
	if (!ExtractLong(typeStr, &along)) goto Broken;
	barLine.type = along;
	
	/* Now handle the barline as needed. */
	ProcessBarline(barLine);

	gBarlineCount++;
	return TRUE;
Broken:
	ReportParseFailure("ParseAndProcessBarline");
	return FALSE;
}


/* Parse a line like this:
 *		P v=2 npt=1 num=3 denom=2 appear=101
 */

Boolean ParseAndProcessTuplet()
{
	char		voiceStr[32], partStr[32], numStr[32], denomStr[32], appearStr[32];
	short		count;
	long		along;
	ATUPLET	tuplet;

	count = sscanf(gInBuf, "%*c%s%s%s%s%s", voiceStr, partStr, numStr, denomStr, appearStr);
	if (count<5) goto Broken;
	
	if (!ExtractLong(voiceStr, &along)) goto Broken;
	tuplet.v = along;

	if (!ExtractLong(partStr, &along)) goto Broken;
	tuplet.npt = along;
	
	if (!ExtractLong(numStr, &along)) goto Broken;
	tuplet.num = along;

	if (!ExtractLong(denomStr, &along)) goto Broken;
	tuplet.denom = along;
	
	if (!ExtractLong(appearStr, &along)) goto Broken;
	tuplet.appearance = along;

	/* Now handle the tuplet as needed. */
	ProcessTuplet(tuplet);

	gTupletCount++;
	return TRUE;
Broken:
	ReportParseFailure("ParseAndProcessTuplet");
	return FALSE;
}


/* Parse a line like this:
 *		D stf=1 type=2
 */

Boolean ParseAndProcessDynamic()
{
	char		stfStr[32], typeStr[32];
	short		count;
	long		along;
	ADYNAMIC	dynamic;

	count = sscanf(gInBuf, "%*c%s%s", stfStr, typeStr);
	if (count<2) goto Broken;
	
	if (!ExtractLong(stfStr, &along)) goto Broken;
	if (NLERR_BADSTAFF(along)) goto Broken;
	dynamic.stf = along;
	
	if (!ExtractLong(typeStr, &along)) goto Broken;
	dynamic.type = along;

	/* Now handle the dynamic as needed. */
	ProcessDynamic(dynamic);

	gDynamicCount++;
	return TRUE;
Broken:
	ReportParseFailure("ParseAndProcessDynamic");
	return FALSE;
}


/* Parse a line like this:
 *		A v=2 npt=1 stf=2 S3 'strepitoso'
 */

Boolean ParseAndProcessText()
{
	char		voiceStr[32], partStr[32], staffStr[32], codeStr[32];
	char		textStr[256], styleDigit;
	short		count;
	long		along;
	ATEXT		text;

	count = sscanf(gInBuf, "%*c%s%s%s%s", voiceStr, partStr, staffStr, codeStr);
	if (count<4) goto Broken;
	
	if (!ExtractLong(voiceStr, &along)) goto Broken;
	text.v = along;

	if (!ExtractLong(partStr, &along)) goto Broken;
	text.npt = along;
	
	if (!ExtractLong(staffStr, &along)) goto Broken;
	text.stf = along;

	if (!(*codeStr=='S' || *codeStr=='L')) goto Broken;
	text.code = *codeStr;
	
	styleDigit = *(codeStr+1);
	if (isdigit(styleDigit))
		text.style = (short)styleDigit-(short)'0';				/* Should be 1 to 5 */

	else
		text.style = 0;													/* "Unknown" */

	/* Get the quoted string, which can contain embedded whitespace. NB: the call below
	 * assumes the preceding stuff on the line doesn't contain a single quote: that's
	 * not completely safe, but I don't see an easy way to start scanning in the
	 * correct position of <gInBuf>.
	 */
	if (ExtractDelimString(gInBuf, '\'', '\'', textStr, 256)<0) goto Broken;

	/*
	 * Storing <textStr> is somewhat of a problem: it could be pretty long but very
	 * rarely will be. So storing it in a manner that's grossly wasteful of space is
	 * trivial, but being efficient is much harder (though still not difficult). For
	 * now, we don't bother with efficiency.
	 */
	strcpy(text.textStr, textStr);
	 
	/* Now handle the text as needed. */
	ProcessText(text);
	
	gTextCount++;
	return TRUE;
Broken:
	ReportParseFailure("ParseAndProcessText");
	return FALSE;
}


/* Parse a line like this:
 *		M stf=1 'Prestissimo' q=100
 *		??The "100" is really a string (e.g., "c.100", "96-104"), and I think it can contain
 *			embedded space: we handle strings but not with embedded space.
 */

Boolean ParseAndProcessTempoMark()
{
	char		staffStr[32], tempoStr[256], metroStr[256], metroUnit, dotOrEquals;
	short		count;
	long		along, startPos, startMetroPos, whitespaceLen;
	ATEMPO	tempo;
	Boolean	dotted;

	count = sscanf(gInBuf, "%*c%s", staffStr);
	if (count<1) goto Broken;

	/* Get the 1st quoted string, which can contain embedded whitespace. NB: the call
	 * below assumes the preceding stuff on the line doesn't contain a single quote:
	 * that's not completely safe, but I don't see an easy way to start scanning in the
	 * correct position of <gInBuf>.
	 */
	startPos = ExtractDelimString(gInBuf, '\'', '\'', tempoStr, 256);
	if (startPos<0) goto Broken;

	/* Get the metronome-unit character and the metronome mark. Note these are optional:
	 * if they're absent, we'll set <metroUnit> to the newline (or maybe return) char.
	 */
	startMetroPos = startPos+strlen(tempoStr)+2;
	whitespaceLen = strspn(gInBuf+startMetroPos, " \t");
	startMetroPos += whitespaceLen;								/* Advance to metronome unit */

	sscanf(gInBuf+startMetroPos, "%c%c", &metroUnit, &dotOrEquals);
	dotted = (dotOrEquals=='.');
	startMetroPos += (dotted? 3 : 2);							/* Advance past equals sign */
	sscanf(gInBuf+startMetroPos, "%s", metroStr);
	
	if (!ExtractLong(staffStr, &along)) goto Broken;
	tempo.stf = along;

	/*
	 * Storing <tempoStr> and <metroStr> have an efficiency problem like <textStr>,
	 * but they can't be very long. We just go ahead and waste space.
	 */
	strcpy(tempo.tempoStr, tempoStr);
	tempo.metroUnit = metroUnit;
	tempo.dotted = dotted;
	strcpy(tempo.metroStr, metroStr);

	/* Now handle the tempo mark as needed. */
	ProcessTempoMark(tempo);
	
	gTempoCount++;
	return TRUE;
Broken:
	ReportParseFailure("ParseAndProcessTempoMark");
	return FALSE;
}

/* Parse a line like this:
 *		B v=1 npt=1 count=2
 */

Boolean ParseAndProcessBeam()
{
	char		voiceStr[32], nptStr[32], beamCountStr[32];
	short		count;
	long		along;
	ABEAM		beam;

	count = sscanf(gInBuf, "%*c%s%s%s", voiceStr, nptStr, beamCountStr);
	if (count<3) goto Broken;
	
	if (!ExtractLong(voiceStr, &along)) goto Broken;
	beam.v = along;
	
	if (!ExtractLong(nptStr, &along)) goto Broken;
	beam.npt = along;

	if (!ExtractLong(beamCountStr, &along)) goto Broken;
	beam.count = along;

	/* Now handle the beam as needed. */
	ProcessBeam(beam);

	gBeamCount++;
	return TRUE;
Broken:
	ReportParseFailure("ParseAndProcessBeam");
	return FALSE;
}


/* ---------------------------------------- ParseAndProcessStructComment and allies -- */

/* Given a pointer to the beginning of one of the optional fields on the header
	structured comment, parse it and return a pointer to the char after its end. If
	we don't recognize the field, or there's no non-whitespace in the string, just
	return NULL. */

static char *ParseField(char *p, Boolean *pOkay);
static char *ParseField(char *p, Boolean *pOkay)
{
	char str[256];
	short ans, number;

	*pOkay = TRUE;
	
	/* Point to first non-whitespace char. If there's nothing following, do nothing. */
	for (p++; *p; p++)
		if (!isspace((int)*p)) break;
	ans = sscanf(p, "%s", str);
	if (ans<1) return NULL;
	
	if (strcmp(str, "delaccs")==0) {
		gDelAccs = TRUE;
		p += strlen(str);
		return p;
	}
	else if (strncmp(str, "startmeas=", strlen("startmeas="))==0) {
		p = strchr(p, '=');															/* Skip to the next '=' in input */
		if (!p) return (char *)0;
		p++;
		ans = sscanf(p, "%s", str);
		p += strlen(str);
		number = (short)strtol(str, (char **)NULL, 10);
		gFirstMNNumber = number;
		return p;
	}
	else {
		*pOkay = FALSE;
		return NULL;		/* Unrecognized field */
	}
}


/* Parse a line like this (a header):
		%%Score file='Untitled'  partstaves=1 1 1 0
	In the future there may be other kinds of structured comment.
*/

#define COMMENT_SCORE		"%%Score"
#define COMMENT_NOTELIST	"%%Notelist"

static Boolean ParseAndProcessStructComment()
{
	char			str[64], *p, nstaves;
	short			ans, i;
	long			startPos;
	Boolean		okay;

	if (gNotelistVersion<=1)
		okay = (strncmp(gInBuf, COMMENT_SCORE, strlen(COMMENT_SCORE))==0);
	else
		okay = (strncmp(gInBuf, COMMENT_NOTELIST, strlen(COMMENT_NOTELIST))==0);

	/* Extract score name, which is enclosed in single-quotes and can contain whitespace.
		It should not be longer than 31 chars (excluding terminating null).
	*/
	startPos = ExtractDelimString(gInBuf, '\'', '\'', gFilename, 64);
	if (startPos<0) goto Broken;
	p = gInBuf+startPos+1;						/* Advance past the opening delimiter */
	p = strchr(p, '\'');							/* Advance to the closing delimiter */

	gNumNLStaves = 0;
	gFirstMNNumber = 	1;

	/* Construct a 1-based array giving the arrangement of parts and staves.
		Indices represent part numbers; values represent the number of staves
		in a part.
	*/
	for (i=0; i<=MAXPARTS; i++)
		gPartStaves[i] = 0;
	p = strchr(p, '=');															/* Skip to the next '=' in input */
	if (!p) goto Broken;
	for (p++, i=1; i<=MAXPARTS; i++, p++) {
		ans = sscanf(p, "%s", str);
		if (ans<1) goto Broken;
		nstaves = (char)strtol(str, (char **)NULL, 10);
		if (nstaves<=0) {
			/* Found the terminator for the part/staff info: skip over it. */
			p = strchr(p, ' ');
			if (!p) goto Done;
			break;
		}
		gPartStaves[i] = nstaves;
		gNumNLStaves += nstaves;
		p = strchr(p, ' ');
		if (!p) goto Done;
	}

	/* Parse and make use of any optional fields that appear at the end of the line. If any
	* are unrecognized, complain, but don't return failure. */
	do
		p = ParseField(p, &okay);
	while (p);
	if (!okay) {
		STATUS_PRINTF("ERROR (ParseAndProcessStructComment): the Notelist's header line contained unrecognized field(s).\n");
	}
	 
Done:
	return ProcessStructComment();
	
Broken:
	ReportParseFailure("ParseAndProcessStructComment");
	return FALSE;
}


void printfXMLComment(char *comment)
{
  printf("<!-- some comment");
//  printf(comment);
  printf("-->");
}