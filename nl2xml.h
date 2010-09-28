/*nl2xml include */

/***************************************************************************
 structs
***************************************************************************/

typedef struct {
/*	unsigned char	modCode;*/
	char			letterSharp;
	char			letterFlat;
	Boolean			sharp;
	Boolean			flat;
} MIDINOTE;


typedef struct {
/*	unsigned char	modCode;*/
	/* need to change this to accomodate 'Percussion' char			sign[12]; */
	char			sign[12];
	short			line;
  short			octaveChange;
} CLEF;


typedef struct {
	char			xmlname[128];
} DURATION;

/***************************************************************************
 function prototype definitions
***************************************************************************/

Boolean replaceSpecialChars (char* oldString);

Boolean init1dAryChar (char **array);
Boolean strcat1d (char **array, int part, char *thisString);
Boolean strcpy1d (char **array, int part, char *thisString); 

Boolean init2dAryChar (char ***array);
Boolean strcat2d (char ***array, int part, int voice, char *thisString);
Boolean strcpy2d (char ***array, int part, int voice, char *thisString); 

Boolean printBarline(int i);

/***************************************************************************
 globals
***************************************************************************/

extern	char gInBuf[MAX_LINELENGTH];
extern	short gErrorCount;
extern	short gNotelistVersion;

extern	short	gLineCount;
extern	short	gClefCount;
extern	short	gKeySigCount;
extern	short	gTimeSigCount;
extern	short	gNoteCount;
extern	short	gGraceNoteCount;
extern	short	gRestCount;
extern	short	gBarlineCount;
extern	short	gTupletCount;
extern	short	gDynamicCount;
extern	short	gTextCount;
extern	short	gTempoCount;

extern	char	gFilename[64];
extern	short	gPartStaves[MAXPARTS+1];	/* 1-based array giving number of staves (value) in each
part (index) */
extern	short	gNumNLStaves;					/* number of staves in Notelist system */
extern	short	gFirstMNNumber;				/* number of first measure */

short gPartCount = 0;

/***************************************************************************
 global elements written to by multiple notelist handling functions
***************************************************************************/
short	gBarlineHere = 0;
short	gBarlineType = 1;
long	gCurrentTime = -1;
char	gAttributes[512] = "";

int gNumParts;

MIDINOTE gMidiNotes[12];
CLEF	 gClefs[14];

/***************************************************************************
 items by part 
***************************************************************************/
Boolean gDivisionsOutput[MAXPARTS+1]; /* if XML <divisions> have been output */
Boolean gNumStavesOutput[MAXPARTS+1]; /* if XML <staves> have been output for each part*/
short	gClefCountPart[MAXPARTS+1]; /*clef number count for this part*/
short gKeySigNumAcc[MAXPARTS+1]; /* global key signature */
short	gClefNum[MAXPARTS+1]; /* global clef value */
char gKeySigShpFlt[MAXPARTS+1]; /* pointers for global strings */
char **gKeySignatures;
char **gTimeSignatures;
char **gClefTags;
char **gDirections;
char **gFinalParts;
//char ***gFinalPartsMeasures;

/***************************************************************************
 items by voice 
***************************************************************************/

Boolean gChordZeroTimeOK[MAXPARTS+1][MAXVOICES+1];
int gDurationCountVoice[MAXPARTS+1][MAXVOICES+1]; /*duration total for this part/voice*/
int gChordPosVoice[MAXPARTS+1][MAXVOICES+1];
int gVoiceTimePosition[MAXPARTS+1][MAXVOICES+1];
int gSlurNum[MAXPARTS+1][MAXVOICES+1]; /* to keep track of slur numbers */ 
int gTupletNumerator[MAXPARTS+1][MAXVOICES+1]; /* tuplet values */
int gTupletDenominator[MAXPARTS+1][MAXVOICES+1];
int gTupletCurrent[MAXPARTS+1][MAXVOICES+1];
int gTupletTotal[MAXPARTS+1][MAXVOICES+1];
int gBeamsTotal[MAXPARTS+1][MAXVOICES+1];
int gBeams[MAXPARTS+1][MAXVOICES+1];
char ***gMeasures; /* pointers for global strings */
char ***gLyrics; 
char ***gNotations;

/***************************************************************************
 items by staff 
***************************************************************************/
short gStaffPartMap[MAXSTAVES+1]; /* maps staves to their corresponding part */
short gStaffToStaffMap[MAXSTAVES+1]; /* maps notelist staff numbers (globally continuous) 
							to their corresponding MusicXML staff numbers (begin again from 1 on each part)*/

/* array to map Notelist durations to MusicXML Type
 index [0] is unknown duration [displayed as a stemless quarter note]
*/
char gDurationTypes[10][8] = {
	"quarter",
	"full",
	"whole",
	"half",
	"quarter",
	"eighth",
	"16th",
	"32nd",
	"64th",
	"128th"};

/* numeric values for note's <duration> */
	int gDurations[10] = {32,256,128,64,32,16,8,4,2,1};

/* basis of time division in MusicXML (based on the smallest division of a 128th note in Notelist
  (we use 32 because a 128th note is 1/32 the duration of a quarter note)
*/
	int gTimeDivisions = 32;

/* array to map Notelist note modifiers to MusicXML Tag */
char gNoteModTags[32][32] = {
	"",
	"<fingering>1</fingering>",
	"<fingering>2</fingering>",
	"<fingering>3</fingering>",
	"<fingering>4</fingering>",
	"<fingering>5</fingering>",
	"",
	"",
	"",
	"",
	"<fermata/>",
	"<trill-mark/>",
	"<accent/>",
	"<strong-accent/>",
	"<staccato/>",
	"<tenuto/>",
	"",
	"<mordent/>",
	"<inverted-mordent/>",
	"<turn/>",
	"",
	"<harmonic/>",
	"<up-bow/>",
	"<down-bow/>",
	"",
	"",
	"",
	"",
	"",
	"",
	"<strong-accent/>",
	"<inverted-mordent/>"};

/* array to map Notelist note modifiers to corresponding MusicXML tag's parent tag */
char gNoteModParentTags[32][32] = {
	"",
	"technical",
	"technical",
	"technical",
	"technical",
	"technical",
	"",
	"",
	"",
	"",
	"notations",
	"ornaments",
	"articulations",
	"articulations",
	"articulations",
	"articulations",
	"",
	"ornaments",
	"ornaments",
	"ornaments",
	"",
	"technical",
	"technical",
	"technical",
	"",
	"",
	"",
	"",
	"",
	"",
	"articulations",
	"ornaments"
};

/* array to map Notelist notehead types to corresponding MusicXML types */
char gNoteheadTags[11][16] = {
	"none",
	"",
	"x",
	"diamond",
	"square",
	"square",
	"diamond",
	"diamond",
	"",
	"slash",
	"none"
};

/* Array to tell us whether or not a Notelist notehead type is filled or not.
 Tried these as booleans, but doesn't work so well, because some should not
 display at all, others should be yes and still others should be explicity "no" 
*/
char gNoteheadTagsFilled[11][3] = {
	"",
	"",
	"no",
	"no",
	"yes",
	"no",
	"yes",
	"",
	"",
	""
};

/* array to map accidentals */
char gAccidentalVals[6][32] = {
	"",
	"flat-flat",
	"flat",
	"natural",
	"sharp",
	"double-sharp"
};

short gAccidentalDoubleFlat = 1;
short gAccidentalFlat = 2;
short gAccidentalNatural = 3;
short gAccidentalSharp = 4;
short gAccidentalDoubleSharp = 5;

/* array to map dynamics */
char gDynamicsMap[24][16] = {
	"",
	"pppp",
	"ppp",
	"pp",
	"p",
	"mp",
	"mf",
	"f",
	"ff",
	"fff",
	"ffff",
	"piu piano",
	"meno piano",
	"meno forte",
	"piu forte",
	"sf",
	"fz",
	"sfz",
	"rf",
	"rfz",
	"fp",
	"sfp",
	"diminuendo",
	"crescendo"
};
