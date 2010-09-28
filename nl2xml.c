/*****************************************************************************
 This is based on Process.c as distributed with Nightingale and has been 
 modified to output a MusicXML representation of the given notelist file.
 Modificaions by Geoff Chirgwin email: <geoffrey at chirgwin.com>.
 
 Function ProcessNote now handles grace notes (was ProcessGraceNote) 
 and rests (was ProcessRest)
 
 Original comments from Process.c:
 Process objects found by nlParse.c routines in a notelist. The routines in
 this file are sample versions that just print out a few of each object's 
 fields; they're intended to be re-written for each application. 
 
 
 recent changes (as of March 9, 2009):
    now output partwise scores instead of timewise
	added beam handling
	correct ties from <tied/> to <tie/>
	moved ties up from <notations/> to their correct location in <note/> 
    
 
******************************************************************************/

#include "nlParse.h"
#include "nl2xml.h"

Boolean Initialize()
{
	/***************************************************************************
	 define MIDI note mappings
	***************************************************************************/
	gMidiNotes[0].letterSharp = 'C';
	gMidiNotes[0].sharp	= FALSE;
	gMidiNotes[0].letterFlat = 'C';
	gMidiNotes[0].flat	= FALSE;
	gMidiNotes[1].letterSharp = 'C';
	gMidiNotes[1].sharp	= TRUE;
	gMidiNotes[1].letterFlat = 'D';
	gMidiNotes[1].flat	= TRUE;
	gMidiNotes[2].letterSharp = 'D';
	gMidiNotes[2].sharp	= FALSE;
	gMidiNotes[2].letterFlat = 'D';
	gMidiNotes[2].flat	= FALSE;
	gMidiNotes[3].letterSharp = 'D';
	gMidiNotes[3].sharp	= TRUE;
	gMidiNotes[3].letterFlat = 'E';
	gMidiNotes[3].flat	= TRUE;
	gMidiNotes[4].letterSharp = 'E';
	gMidiNotes[4].sharp	= FALSE;
	gMidiNotes[4].letterFlat = 'E';
	gMidiNotes[4].flat	= FALSE;
	gMidiNotes[5].letterSharp = 'F';
	gMidiNotes[5].sharp	= FALSE;
	gMidiNotes[5].letterFlat = 'F';
	gMidiNotes[5].flat	= FALSE;
	gMidiNotes[6].letterSharp = 'F';
	gMidiNotes[6].sharp	= TRUE;
	gMidiNotes[6].letterFlat = 'G';
	gMidiNotes[6].flat	= TRUE;
	gMidiNotes[7].letterSharp = 'G';
	gMidiNotes[7].sharp	= FALSE;
	gMidiNotes[7].letterFlat = 'G';
	gMidiNotes[7].flat	= FALSE;
	gMidiNotes[8].letterSharp = 'G';
	gMidiNotes[8].sharp	= TRUE;
	gMidiNotes[8].letterFlat = 'A';
	gMidiNotes[8].flat	= TRUE;
	gMidiNotes[9].letterSharp = 'A';
	gMidiNotes[9].sharp	= FALSE;
	gMidiNotes[9].letterFlat = 'A';
	gMidiNotes[9].flat	= FALSE;
	gMidiNotes[10].letterSharp = 'A';
	gMidiNotes[10].sharp	= TRUE;
	gMidiNotes[10].letterFlat = 'B';
	gMidiNotes[10].flat	= TRUE;
	gMidiNotes[11].letterSharp = 'B';
	gMidiNotes[11].sharp	= FALSE;
	gMidiNotes[11].letterFlat = 'B';
	gMidiNotes[11].flat	= FALSE;

	/***************************************************************************
	 define clef mappings
	***************************************************************************/
	/* index 0 is empty and unused in notelist */
	strcpy(gClefs[0].sign, "");
	gClefs[0].line = 0;
	gClefs[0].octaveChange = 0;
	/* treble clef with 8va sign above */
	strcpy(gClefs[1].sign, "G");
	gClefs[1].line = 2;
	gClefs[1].octaveChange = 1;
	/* French violin clef (treble clef on bottom line)*/
	strcpy(gClefs[2].sign, "G");
	gClefs[2].line = 1;
	gClefs[2].octaveChange = 0;
	/* treble clef */
	strcpy(gClefs[3].sign, "G");
	gClefs[3].line = 2;
	gClefs[3].octaveChange = 0;
	/* soprano clef (C clef on bottom line) */
	strcpy(gClefs[4].sign, "C");
	gClefs[4].line = 1;
	gClefs[4].octaveChange = 0;
	/*mezzo-soprano clef (C clef on 4th line from top)*/
	strcpy(gClefs[5].sign, "C");
	gClefs[5].line = 2;
	gClefs[5].octaveChange = 0;
	/* alto clef */
	strcpy(gClefs[6].sign, "C");
	gClefs[6].line = 3;
	gClefs[6].octaveChange = 0;
	/* treble-tenor clef (treble clef with 8va sign) */
	strcpy(gClefs[7].sign, "G");
	gClefs[7].line = 2;
	gClefs[7].octaveChange = -1;
	/* tenor clef (C clef on 2nd line from top)*/
	strcpy(gClefs[8].sign, "C");
	gClefs[8].line = 4;
	gClefs[8].octaveChange = 0;
	/* baritone clef (C clef on top line)*/
	strcpy(gClefs[9].sign, "C");
	gClefs[9].line = 5;
	gClefs[9].octaveChange = 0;
	/* bass clef */
	strcpy(gClefs[10].sign, "F");
	gClefs[10].line = 4;
	gClefs[10].octaveChange = 0;
	/* bass clef with 8va sign below */
	strcpy(gClefs[11].sign, "F");
	gClefs[11].line = 4;
	gClefs[11].octaveChange = -1;
	/* percussion clef */
	strcpy(gClefs[12].sign, "Percussion");
	gClefs[12].line = 3;
	gClefs[12].octaveChange = 0;

	/* try encoding = UTF-8 ??? */
	printf("<?xml version=\"1.0\" encoding=\"ISO-8859-1\" standalone=\"no\"?>\n");
	printf("<!DOCTYPE score-partwise PUBLIC \"-//Recordare//DTD MusicXML 2.0 Partwise//EN\"");

	printf(" \"http://www.musicxml.org/dtds/partwise.dtd\">\n");
	/*
		or, alternately reference a local copy of the DTD, like:
		printf(" \"/home/nl2xml/dtds/partwise.dtd\">\n");
	*/
	printf("<score-partwise>\n");

	gErrorCount = 0;

	return TRUE;
}

Boolean CleanUp()
{

    /* output all aggregated parts */
	int i;
	for (i = 1; i<=gNumParts; i++) {
		printf("	<part id=\"P%d\">\n", i);
		printf(gFinalParts[i]);
		printf("	</part>\n");

	}
	
	/*print musicxml closing*/
	printf("</score-partwise>\n\n");

	/* do some Notelist error handling here... */
	if (gErrorCount>0) {
		fprintf(stderr, "%% %d error(s) found in Notelist file.\n", gErrorCount);
	}

	free(gKeySignatures);
	free(gTimeSignatures);
	free(gClefTags);
	free(gDirections);
	free(gMeasures);
	free(gLyrics); 
	free(gNotations);
	free(gFinalParts);

	return TRUE;
}

void CopyLine()
{
	printf("%s", gInBuf);
}

/* Item-processing functions */

Boolean ProcessClef(ACLEF clef)
{
	char tempStr[256];

	/* output clef only if if's different from our previous clef 
	 or it's on a different staff of a multi-staff part
	 also ensure we don't output a number value of 0 or less */

	short thisStaffNL = (short) clef.stf;
	int thisStaffXML = gStaffToStaffMap[thisStaffNL] > 0? gStaffToStaffMap[thisStaffNL] : 1;
	int thisPart = gStaffPartMap[thisStaffNL];
	short thisClefType = (short) clef.type;

	if (gClefNum[thisStaffNL] != thisClefType ||
			gPartStaves[thisPart] > 1) {
		sprintf(tempStr, "				<clef number=\"%d\">\n", thisStaffXML);
		strcat1d(gClefTags, thisPart, tempStr);
		sprintf(tempStr, "					<sign>%s</sign>\n",	gClefs[thisClefType].sign);
		strcat1d(gClefTags, thisPart, tempStr);

		sprintf(tempStr, "					<line>%d</line>\n", gClefs[thisClefType].line);
		strcat1d(gClefTags, thisPart, tempStr);

		if (gClefs[thisClefType].octaveChange != 0) {
			sprintf(tempStr, "					<clef-octave-change>%d</clef-octave-change>\n",
			gClefs[thisClefType].octaveChange);
			strcat1d(gClefTags, thisPart, tempStr);
		}
		strcat1d(gClefTags, thisPart, "				</clef>\n");

		gClefCountPart[thisPart]++;
		gClefNum[thisStaffNL] = thisClefType;

	}

	return TRUE;
}


Boolean ProcessKeySig(AKEYSIG keySig)
{
	/* missing tag: <mode>major</mode>	*/

	char tempStr[256];
	short thisPart = (short) gStaffPartMap[(short) keySig.stf];
	/* only need one key signature per part, so if we have one already 
	 or if we have have no key signature, don't think we have to output anything 
	 check for key changes, and only output if this key signature is different from previous
	*/
	if (strlen(gKeySignatures[thisPart]) <= 0 &&
			keySig.numAcc != gKeySigNumAcc[thisPart] && 
			keySig.shpFlt != gKeySigShpFlt[thisPart]) {
		strcat1d(gKeySignatures, thisPart, "				<key>\n");
		sprintf(tempStr, "					<fifths>%s%d</fifths>\n", (keySig.shpFlt=='b')? "-" : "", keySig.numAcc);
		strcat1d(gKeySignatures, thisPart, tempStr);
		strcat1d(gKeySignatures, thisPart, "				</key>\n");
		gKeySigNumAcc[thisPart] = keySig.numAcc;
		gKeySigShpFlt[thisPart] = keySig.shpFlt;
	}
	return TRUE;
}

Boolean ProcessTimeSig(ATIMESIG timeSig)
{
	/*
		unused notelist attribute(s): timeSig.displ
		char* thisTimeSig = gTimeSignaturesPart[gStaffPartMap[timeSig.stf]];
	*/

	short thisPart = (short) gStaffPartMap[(short) timeSig.stf];
	char tempStr[256];

	/* only need one time signature per part, so if we have one already, do nothing*/
	if (strlen(gTimeSignatures[thisPart]) <= 0) {
		strcat1d(gTimeSignatures, thisPart, "				<time>\n");
		sprintf(tempStr, "					<beats>%d</beats>\n", timeSig.num);
		strcat1d(gTimeSignatures, thisPart, tempStr);
		sprintf(tempStr, "					<beat-type>%d</beat-type>\n",
		timeSig.denom);
		strcat1d(gTimeSignatures, thisPart, tempStr);
		strcat1d(gTimeSignatures, thisPart, "				</time>\n");
	}

	return TRUE;
}

Boolean ProcessNote(ANOTE note, Boolean isRest, Boolean isGraceNote)
{

	/*
		unused notelist attribute(s): note.t, note.v, note.pDur, note.vel
	
		Still unaccounted for Notelist modifiers:
		[15]	wedge
		[20]	plus sign	+ (should probably just be text of '+' )
		[24]	tremolo (1 slash)
	
		[25]	tremolo (2 slashes)
		[26]	tremolo (3 slashes)
		[27]	tremolo (4 slashes)
		[28]	tremolo (5 slashes)
	
		[29]	tremolo (6 slashes)
	
	*/

	char* attributes = gAttributes;

	/* these are only written to by ProcessNote */
	char technical[1024] = "";

	char articulations[1024] = "";
	char ornaments[1024] = "";

	char tempStr[1024];

	char noteAppearance[32] = "";
	short i;
	short thisStaffNum = gStaffToStaffMap[(short) note.stf];
	short thisVoice = (note.v) ? (short) note.v : 1;
	short thisPart = (note.npt) ? (short) note.npt : 1;
	short thisAppearance = (short) note.appearance;

	short midiNoteIdx;

	short durIdx = (int) note.dur;

	char *noteType = gDurationTypes[durIdx];
	int duration = gDurations[durIdx];

	midiNoteIdx = note.nn - ( ((int) (note.nn / 12)) * 12);

	/*increment duration based on number of dots */
	i = 1;

	while (i <= note.dots) {
		duration += duration/2;
		i ++;
	}

	if (durIdx >= -1) {
		strcat2d(gMeasures, thisPart, thisVoice, "			<note>\n");
	}

	/* this is a grace note */
	if (isGraceNote) {
		strcat2d(gMeasures, thisPart, thisVoice, "				<grace/>\n");
	}

	if (gBeams[thisPart][thisVoice] > 0)
	{
		if (gBeams[thisPart][thisVoice] == gBeamsTotal[thisPart][thisVoice])
		{
			strcat2d(gMeasures, thisPart, thisVoice, "				<beam>begin</beam>\n");
		} else if (gBeams[thisPart][thisVoice] == 1)
		{
			strcat2d(gMeasures, thisPart, thisVoice, "				<beam>end</beam>\n");
		} else 
		{
			strcat2d(gMeasures, thisPart, thisVoice, "				<beam>continue</beam>\n");
		}
		gBeams[thisPart][thisVoice] --;
	}
    
	/* this is a rest */
	if (isRest) {
		/* add current duration to duration count global for this part/voice */
		gDurationCountVoice[thisPart][thisVoice] += duration;
		/* handle multiple measure rests */
		if (durIdx <= -2) {
			strcat(attributes, "				<measure-style>\n");
			sprintf(attributes + strlen(attributes), "					<multiple-rest>%d</multiple-rest>\n", -(durIdx));
			strcat(attributes, "				</measure-style>\n");

			/* nothing else to do for multi-measure rests */
			return TRUE;
		} else {
			/* this is a whole rest, so set duration index accordingly */
			if (durIdx == -1) {
				durIdx = 2;

				/* reset pointer to note type and duration value with new duration index */
				noteType = gDurationTypes[durIdx];
				duration = gDurations[durIdx];
			}
			strcat2d(gMeasures, thisPart, thisVoice, "				<rest/>\n");
		}
	} else { 	/* not a rest, do the stuff that only applies to notes/grace notes, not rests */
		/* print chord tag for all notes in chord, except for first note,
			 which is assumed, I think ... this still isn't right... */
		if (note.inChord) {
			/*if current time position matches previous time position, output chord tag */
			if (note.t == gChordPosVoice[thisPart][thisVoice] && 
					(note.t != 0 || gChordZeroTimeOK[thisPart][thisVoice])) {
				strcat2d(gMeasures, thisPart, thisVoice, "				<chord/>\n");
			} else {	/* otherwise, we know we have a new chord, so output nothing, 
										but reset chord time position for this part to current time
										MusicXML assumes all subsequent notes with <chord/> tag are 
										grouped with the preceding base note (last one w/o <chord/> */
				gChordPosVoice[thisPart][thisVoice] = note.t;
				if (note.t == 0) {
					gChordZeroTimeOK[thisPart][thisVoice] = TRUE;
				}
				/* add current duration to duration count global for this part/voice */
				gDurationCountVoice[thisPart][thisVoice] += duration;
			}
		} else
		{
			/* add current duration to duration count global for this part/voice */
			gDurationCountVoice[thisPart][thisVoice] += duration;		
		}
		strcat2d(gMeasures, thisPart, thisVoice, "				<pitch>\n");

		/* we'll try to use enharmonics that match the key signature */
	    /* also try to match pitch spelling to accidental, if there is one*/
		/* TODO: improve enharmonic spelling */		
		if (gKeySigShpFlt[thisPart] == 'b' || note.acc == gAccidentalFlat || note.eAcc == gAccidentalFlat) {
			sprintf(tempStr, "					<step>%c</step>\n", gMidiNotes[midiNoteIdx].letterFlat);
			strcat2d(gMeasures, thisPart, thisVoice, tempStr);
		} else {
			sprintf(tempStr, "					<step>%c</step>\n", gMidiNotes[midiNoteIdx].letterSharp);
			strcat2d(gMeasures, thisPart, thisVoice, tempStr);
		}

		/*again, we'll try to use enharmonic spellings that match the key signature or match the accidental */
		if (note.acc == gAccidentalFlat || note.eAcc == gAccidentalFlat) {
			strcat2d(gMeasures, thisPart, thisVoice, "					<alter>-1</alter>\n");
		} else if (note.acc == gAccidentalSharp || note.eAcc == gAccidentalSharp) {
			strcat2d(gMeasures, thisPart, thisVoice, "					<alter>1</alter>\n");
		} else if (note.acc == gAccidentalDoubleSharp || note.eAcc == gAccidentalDoubleSharp) {
			strcat2d(gMeasures, thisPart, thisVoice, "					<alter>2</alter>\n");
		} else if (note.acc == gAccidentalDoubleFlat || note.eAcc == gAccidentalDoubleFlat) {
			strcat2d(gMeasures, thisPart, thisVoice, "					<alter>-2</alter>\n");
		}

		sprintf(tempStr, "					<octave>%d</octave>\n", (int) (note.nn /
	12) - 1);
		strcat2d(gMeasures, thisPart, thisVoice, tempStr);
		strcat2d(gMeasures, thisPart, thisVoice, "				</pitch>\n");

	}
	
	/* grace notes have no duration in musicXML*/

	if (!isGraceNote) {

		sprintf(tempStr, "				<duration>%d</duration>\n", duration);

		strcat2d(gMeasures, thisPart, thisVoice, tempStr);
	}

	if (note.tiedL || note.tiedR) {
		strcat2d(gMeasures, thisPart, thisVoice, "				<tie type=\"");
		strcat2d(gMeasures, thisPart, thisVoice,  (note.tiedL? "stop" : (note.tiedR? "start" : "")));
		strcat2d(gMeasures, thisPart, thisVoice,  "\"/>\n");
	}	
	
	sprintf(tempStr, "				<voice>%d</voice>\n", thisVoice);
	strcat2d(gMeasures, thisPart, thisVoice, tempStr);

	sprintf(tempStr, "				<type>%s</type>\n", noteType);
	strcat2d(gMeasures, thisPart, thisVoice, tempStr);

	/*now loop trough dots again and output the correct number of dot tags */
	i = 1;
	while (i <= note.dots) {
		strcat2d (gMeasures, thisPart, thisVoice, "				<dot/>\n");
		i ++;
	}

	/* Accidentals note.acc */
	if (note.acc != 0) {
		sprintf (tempStr, "				<accidental>%s</accidental>\n", gAccidentalVals[(short) note.eAcc]);
		strcat2d(gMeasures, thisPart, thisVoice, tempStr);
	}

	/* is inTuplet is true, we need to output current time-modification tag (see
		processTuplet) here to make sure it's in the context of the current note */
	if (note.inTuplet) {
		strcat2d(gMeasures, thisPart, thisVoice, "				<time-modification>\n");

		sprintf (tempStr, "					<actual-notes>%d</actual-notes>\n", gTupletNumerator[thisPart][thisVoice]);
		strcat2d(gMeasures, thisPart, thisVoice, tempStr);
		sprintf (tempStr, "					<normal-notes>%d</normal-notes>\n", gTupletDenominator[thisPart][thisVoice]);
		strcat2d(gMeasures, thisPart, thisVoice, tempStr);

		strcat2d(gMeasures, thisPart, thisVoice, "				</time-modification>\n");

		/* not sure if this will work for all cases (assumes first note of tuplet is basis note 
		length for tuplet, which is not necessarily true , but we'll need to tally all tuplets 
		or somehow determine when the tuplet is done, so we can print the "stop" tuplet tag */
		if (gTupletTotal[thisPart][thisVoice] == 0) {
			/*tupletDurationBasis[thisPart][thisVoice] = duration;*/

			gTupletTotal[thisPart][thisVoice] = (duration * gTupletNumerator[thisPart][thisVoice]);
		}
		gTupletCurrent[thisPart][thisVoice] +=	duration;
	}

	/* we don't really have a mapping for types 1 (ngale "normal") and 8
	 (ngale explicit half-note notehead), so we'll ignore them for now
	 Maybe we need to handle appearance difference for rests (isRest == TRUE) ?? */

	if (!isRest && thisAppearance != 1 && thisAppearance != 8) {
		if (strlen(gNoteheadTagsFilled[thisAppearance])>0) {
			sprintf(noteAppearance + strlen(noteAppearance)," filled=\"%s",
				gNoteheadTagsFilled[thisAppearance]);
			strcat(noteAppearance,"\"");
		}

		sprintf (tempStr, "				<notehead%s>%s</notehead>\n", noteAppearance, gNoteheadTags[thisAppearance]);
		strcat2d(gMeasures, thisPart, thisVoice, tempStr);

	}

		sprintf(tempStr, "				<staff>%d</staff>\n", thisStaffNum);
		strcat2d(gMeasures, thisPart, thisVoice, tempStr);

	if (note.numMods>0) {
		for (i = 0; i<note.numMods; i++) {


	/* append all modifiers to correct parent tag string */
	/* articulations */
	if (strcmp(gNoteModParentTags[note.modList[i].modCode], "articulations") == 0) {
		strcat(articulations, "						");
				strcat(articulations, gNoteModTags[note.modList[i].modCode]);
				strcat(articulations, "\n");

			 }
			/* technical */
			if (strcmp(gNoteModParentTags[note.modList[i].modCode], "technical") == 0) {
				strcat(technical, "					");
				strcat(technical, gNoteModTags[note.modList[i].modCode]);
				strcat(technical, "\n");
			}
			/* ornaments */
			if (strcmp(gNoteModParentTags[note.modList[i].modCode], "ornaments") == 0) {
				strcat(ornaments, "					");
				strcat(ornaments, gNoteModTags[note.modList[i].modCode]);
				strcat(ornaments, "\n");
			}
			/* notations */
			if (strcmp(gNoteModParentTags[note.modList[i].modCode], "notations") == 0) {
				strcat2d(gNotations, thisPart, thisVoice, "					");
				strcat2d(gNotations, thisPart, thisVoice, gNoteModTags[note.modList[i].modCode]);
				strcat2d(gNotations, thisPart, thisVoice, "\n");
			}
		}
	}

	/* increment slur number if we're at the start of a slur */
	if (note.slurredR) {
		if (gSlurNum[thisPart][thisVoice] < 0) {
			gSlurNum[thisPart][thisVoice] = 0;
		} else {
			gSlurNum[thisPart][thisVoice] ++;
		}
	}

/* add <slur> start or stop	<notations>, also ensure we don't erroneously 
output a value for the number attribute of 0 or less */

	if (note.slurredL || note.slurredR) {
		sprintf(tempStr, "					<slur type=\"%s\" number=\"%d\"/>\n", (note.slurredL? "stop" :
(note.slurredR? "start" : "")), (gSlurNum[thisPart][thisVoice] > 0? gSlurNum[thisPart][thisVoice] : 1));
		strcat2d(gNotations, thisPart, thisVoice, tempStr);
	}
	/* decrement slur number if we've just ended a slur*/
	if (note.slurredL) {
			gSlurNum[thisPart][thisVoice] --;
	}

	/* tuplet (nested in notations)
	 this is the last note in the tuplet, so print tuplet stop tag */

	if (note.inTuplet && gTupletCurrent[thisPart][thisVoice] == gTupletTotal[thisPart][thisVoice]
			&& gTupletTotal[thisPart][thisVoice] != 0) {

		strcat2d (gNotations,  thisPart, thisVoice, "					<tuplet type=\"stop\"/>\n");

		/*zero these globals out (this may be redundant-- it happens in processTuplet),

		but not really sure how this works */
		gTupletNumerator[thisPart][thisVoice] = 0;
		gTupletDenominator[thisPart][thisVoice] = 0;
		gTupletCurrent[thisPart][thisVoice] = 0;
		gTupletTotal[thisPart][thisVoice] = 0;

	}

	/* output tags we've been adding to throughout */
	/* <technical>, nested in <notations> */
	if (strlen(technical) != 0) {
		strcat2d(gNotations, thisPart, thisVoice, "					<technical>\n");
		strcat2d(gNotations, thisPart, thisVoice, technical);
		strcat2d(gNotations, thisPart, thisVoice, "					</technical>\n");
	}
	/* <ornaments>, nested in <notations> */
	if (strlen(ornaments) != 0) {
		strcat2d(gNotations, thisPart, thisVoice, "					<ornaments>\n");
		strcat2d(gNotations, thisPart, thisVoice, ornaments);
		strcat2d(gNotations, thisPart, thisVoice, "					</ornaments>\n");
	}

	/* notations */
	if (strlen(gNotations[thisPart][thisVoice]) != 0 || strlen(articulations) != 0) {
		strcat2d(gMeasures, thisPart, thisVoice, "				<notations>\n");
		strcat2d(gMeasures, thisPart, thisVoice, gNotations[thisPart][thisVoice]);
	}

	/* <articulations>, nested in <notations>*/
	if (strlen(articulations) != 0) {
		strcat2d(gMeasures, thisPart, thisVoice, "					<articulations>\n");
		strcat2d(gMeasures, thisPart, thisVoice, articulations);
		strcat2d(gMeasures, thisPart, thisVoice, "					</articulations>\n");
	}

	if (strlen(gNotations[thisPart][thisVoice]) != 0 || strlen(articulations) != 0) {
		strcat2d(gMeasures, thisPart, thisVoice, "				</notations>\n");
	}

	/* nest lyric tag within this note */
	if (strlen(gLyrics[thisPart][thisVoice]) != 0) {
		strcat2d(gMeasures, thisPart, thisVoice, gLyrics[thisPart][thisVoice]);

		/* empty out lyric global for this part*/
		strcpy2d(gLyrics,thisPart,thisVoice, "");
	}
	strcat2d(gMeasures, thisPart, thisVoice, "			</note>\n");

	/*reset this global for next note for this part/voice:*/
	strcpy2d(gNotations, thisPart, thisVoice, "");

	return TRUE;

}

Boolean ProcessBarline(ABARLINE barLine)
{
	char tempStr[8192];
	
	short i;
	short j;
	short k;

	char* attributes = gAttributes;
	gBarlineType = barLine.type;

	gPartCount = 0;

	/*output the entire measure if we need to*/
	for (i = 1; i<=gNumParts; i++) {
		sprintf(tempStr, "		<measure number=\"%d\">\n", gFirstMNNumber + gBarlineCount);
		strcat1d(gFinalParts, i, tempStr);
		/*nothing left, so move on*/
		if (gPartStaves[i]<=0) break;
		/*only output attributes tag if it's got something in it */
		if (!gDivisionsOutput[i] || !gNumStavesOutput[i] || strlen(gKeySignatures[i]) > 0 || 
				strlen(gTimeSignatures[i]) > 0 || gPartStaves[i] > 1 ||
				strlen(gClefTags[i]) > 0 || strlen (attributes) > 0 ) {
			/*output attributes */
			strcat1d(gFinalParts, i, "			<attributes>\n");
	
			/* only need to output attributes once per part */			
			if (!gDivisionsOutput[i]) {
				sprintf(tempStr, "				<divisions>%d</divisions>\n", gTimeDivisions);
				strcat1d(gFinalParts, i, tempStr);
				gDivisionsOutput[i] = TRUE;
			}
			/* only need to output number of staves once per part */			
			if (!gNumStavesOutput[i] && gPartStaves[i] > 1) {
				sprintf(tempStr,"				<staves>%d</staves>\n", gPartStaves[i]);
				strcat1d(gFinalParts, i, tempStr);
				gNumStavesOutput[i] = TRUE;
			}
			sprintf(tempStr, "%s", gKeySignatures[i]);
			strcat1d(gFinalParts, i, tempStr);
			
			strcpy(gKeySignatures[i], "");

			sprintf(tempStr, "%s", gTimeSignatures[i]);
			strcat1d(gFinalParts, i, tempStr);
			
			sprintf(tempStr, "%s", gClefTags[i]);
			strcat1d(gFinalParts, i, tempStr);
			
			if (strlen(attributes) != 0) {
				sprintf(tempStr, "%s", attributes);
				strcat1d(gFinalParts, i, tempStr);
				
				/* empty out attributes global*/
				strcpy(attributes, "");
			}
			strcat1d(gFinalParts, i, "			</attributes>\n");			
		}
		/* nest direction tag within this measure */
		if (strlen(gDirections[i]) != 0) {
			sprintf(tempStr, "%s", gDirections[i]);
			strcat1d(gFinalParts, i, tempStr);
			
			/* empty out direction global for this part*/
			strcpy1d(gDirections,i , "");
		}
		
		for (j = 1; j<=MAXVOICES; j++) {
			/*print all voice specific stuff, if there's anything there; add <backup/>s to
				backup the amount we went forward on the previous voice(s)*/
			/*printf("strlen(gMeasures[%d][%d]) = %d", i, j, strlen(gMeasures[i][j]));*/
			if (strlen(gMeasures[i][j]) > 0) {
			    for (k = j; k >= 1; k--) {
					if (j > 1 && gDurationCountVoice[i][k] > 0) {
						strcat1d(gFinalParts, i, "			<backup>\n");
						sprintf(tempStr, "				<duration>%d</duration>\n", gDurationCountVoice[i][k]);
						strcat1d(gFinalParts, i, tempStr);
						strcat1d(gFinalParts, i, "			</backup>\n");
					}
				}
				sprintf(tempStr, "%s", gMeasures[i][j]);
				strcat1d(gFinalParts, i, tempStr);
			}
			/* end of measure, so reset duration count for next measure */
			gDurationCountVoice[i][j] = 0;
		}
		printBarline(i);
		strcat1d(gFinalParts, i, "		</measure>\n");
	}

	/*reset each parts gMeasures, etc. to empty strings*/

	for (i = 1; i<=gNumParts; i++) {
			for (j = 1; j<=MAXVOICES; j++) {
				/*set each part's gMeasures to an empty string*/
				strcpy2d(gMeasures, i, j, "");
			}

		strcpy1d(gTimeSignatures, i, "");
		strcpy1d(gClefTags, i, "");

	}
	return TRUE;

}

Boolean ProcessTuplet(ATUPLET	tuplet)
{

/*	printf("%c v=%d npt=%d num=%d denom=%d appear=%03d\n", TUPLET_CHAR,
				tuplet.v, tuplet.npt, tuplet.num, tuplet.denom, tuplet.appearance);
*/

	char tempStr[1024];

	short thisPart = (short) tuplet.npt;
	short thisVoice = (short) tuplet.v;

	strcat2d (gNotations,  thisPart, thisVoice, "					<tuplet type=\"start\" placement=\"above\" ");

	sprintf (tempStr, "show-number=\"%s\" bracket=\"%s\"/>\n",
		((tuplet.appearance - 100) >= 10 || tuplet.appearance >= 10)? "actual" : ((tuplet.appearance >=
100)? "actual" : "none"),
		(tuplet.appearance % 10 == 0)? "no" : "yes");

 	strcat2d(gNotations, thisPart, thisVoice, tempStr);

	/* initialize (or reset) tuplet globals for this part*/
	gTupletNumerator[thisPart][thisVoice] = tuplet.num;
	gTupletDenominator[thisPart][thisVoice] = tuplet.denom;
	gTupletCurrent[thisPart][thisVoice] = 0;
	gTupletTotal[thisPart][thisVoice] = 0;

	return TRUE;
}

Boolean ProcessDynamic(ADYNAMIC dynamic)
{

/*missing:	printf("		<sound dynamics=\"54\"/>\n");
*/
	short thisStaffNL = (short) dynamic.stf;
	short thisStaffXML = gStaffToStaffMap[thisStaffNL];
	short thisPart = (short) gStaffPartMap[thisStaffNL];
	short thisType = (short) dynamic.type;
	
	char tempStr[1024];
	
	strcat1d(gDirections, thisPart, "			<direction>\n");
	strcat1d(gDirections, thisPart, "				<direction-type>\n");

	/* other-dynamics */
	/* piu piano, meno piano, meno forte, piu forte, rf */
	if ((thisType >= 11 && thisType <= 14) || thisType == 18) {
	
		strcat1d(gDirections, thisPart, "					<dynamics>\n");
		strcat1d(gDirections, thisPart, "						<other-dynamics/>\n");
		strcat1d(gDirections, thisPart, "					</dynamics>\n");

		sprintf(tempStr, "					<words>%s</words>\n", gDynamicsMap[thisType]);
		strcat1d(gDirections, thisPart, tempStr);

	/* diminuendo,	crescendo */
	}	else if (thisType >= 22) {
		sprintf(tempStr, "					<wedge type=\"%s\" spread=\"0\" />\n", gDynamicsMap[thisType]);

		strcat1d(gDirections, thisPart, tempStr);

	/* everything else, just a regular dynamic */
	} else {
		strcat1d(gDirections, thisPart, "				<dynamics>\n");
		sprintf(tempStr, "					<%s/>\n", gDynamicsMap[thisType]);

		strcat1d(gDirections, thisPart, tempStr);
		strcat1d(gDirections, thisPart, "				</dynamics>\n");
	}

	strcat1d(gDirections, thisPart, "				</direction-type>\n");
	sprintf(tempStr, "				<staff>%d</staff>\n", thisStaffXML);
	strcat1d(gDirections, thisPart, tempStr);
	strcat1d(gDirections, thisPart, "			</direction>\n");

	/* need to close the crescedo/diminuendo wedge.	Since notelist doesn't store this, 
		we'll just pick some arbitrary, hopefull average numbers for positioning */
	if (thisType >= 22) {
		strcat1d(gDirections, thisPart, "			<direction>\n");
		strcat1d(gDirections, thisPart, "				<direction-type>\n");
		strcat1d(gDirections, thisPart, "					<wedge type=\"stop\" relative-x=\"50\" spread=\"10\"/>\n");
		strcat1d(gDirections, thisPart, "				</direction-type>\n");
		sprintf(tempStr, "				<staff>%d</staff>\n", thisStaffXML);
		strcat1d(gDirections, thisPart, tempStr);
		strcat1d(gDirections, thisPart, "			</direction>\n");
	}

	return TRUE;
}

Boolean ProcessText(ATEXT text)
{
/* unused: text.v, text.npt, text.stf, text.code, text.style */

/* 		think part=-2 and voice=-2 means above top of system, not attached to staff/part (?) 
			for now, we'll make sure these are positive, otherwise
			we'd end up core dumping from trying to use negative pointers 
*/

	short thisStaffNL = (short) text.stf;
	short thisPart = text.npt > 0? (gStaffPartMap[thisStaffNL] > 0? gStaffPartMap[thisStaffNL] : text.npt) : 1;
	short thisVoice = text.v > 0? text.v : 1;
	short thisStaffXML = text.stf > 0? gStaffToStaffMap[thisStaffNL] : 1;

	char syllabic[8] = "single";
	char directionPlacement[8] = "above";

	char tempStr[256];

	/* strip out/replace all special chars */
	replaceSpecialChars(text.textStr);

	/* It's a lyric */

	if (text.code == 'L') {

		sprintf(tempStr, "				<lyric number=\"%d\">\n", thisVoice);
		
		strcat2d(gLyrics, thisPart, thisVoice, tempStr);
		sprintf(tempStr, "					<syllabic>%s</syllabic>\n", syllabic);
		strcat2d(gLyrics, thisPart, thisVoice, tempStr);
		sprintf(tempStr, "					<text>%s</text>\n", text.textStr);
		strcat2d(gLyrics, thisPart, thisVoice, tempStr);
		strcat2d(gLyrics, thisPart, thisVoice, "				</lyric>\n");
	} else {
		/* score marking, default to position above the staff
		 should make a point of putting these on the correct staff of a multi-staff part
		*/

		sprintf(tempStr, "			<direction placement=\"%s\">\n", directionPlacement);
		strcat1d(gDirections, thisPart, tempStr);
		strcat1d(gDirections, thisPart, "				<direction-type>\n");
		sprintf(tempStr, "					<words>%s</words>\n", text.textStr);
		strcat1d(gDirections, thisPart, tempStr);
		strcat1d(gDirections, thisPart, "				</direction-type>\n");
		sprintf(tempStr, "				<staff>%d</staff>\n", thisStaffXML);
		strcat1d(gDirections, thisPart, tempStr);
		strcat1d(gDirections, thisPart, "			</direction>\n");
	}

	return TRUE;
}

Boolean ProcessTempoMark(ATEMPO tempo)
{
/* unused: tempo.metroUnit, tempo.metroStr, tempo.stf, tempo.dotted */

/*	should figure out what staff/part these should go on
		have had some problems doing this, so hardcoding to top part (number="1") for now
*/
	short thisPart = 1;
	char tempStr[128];

	strcat1d(gDirections, thisPart, "			<direction placement=\"above\">\n");
	strcat1d(gDirections, thisPart, "				<direction-type>\n");

/*		still need to properly translate Notelist note equals string
			and handle durations other than quarter note =
 */
		sprintf(tempStr, "					<words>%s%s</words>\n", tempo.tempoStr,tempo.metroStr);
		strcat1d(gDirections, thisPart, tempStr);
		strcat1d(gDirections, thisPart, "				</direction-type>\n");
		sprintf(tempStr, "				<sound tempo=\"%d\"/>\n", atoi(tempo.metroStr));
		strcat1d(gDirections, thisPart, tempStr);
		strcat1d(gDirections, thisPart, "			</direction>\n");

	return TRUE;
}

#define COMMENT_NLHEADER2	"%%Notelist-V2 file="	/* start of structured comment: Ngale 99 and after*/

/* can these be other comments? Or are they necessarily the first-line header comment?
	 If not, we should handle and preserve "% comments" as "<!-- comments -->"
 */
Boolean ProcessStructComment(void)
{
	/*	unused Notelist elements: startmeas=gFirstMNNumber */

	short i;
	short staffNum;
	short stavesMaxThisPart;

	short thisPart;
	short nlStaff = 1;
	short XMLStaff;

	short partNum;

	printf("	<work>\n");
	printf("		<work-title>%s</work-title>\n", gFilename);
	printf("	</work>\n");

	printf("	<identification>\n");

	printf("		<encoding>\n");
	printf("			<software>nl2xml v 0.9 : Nightingale Notelist to MusicXML translator. ");
	printf(" Notelist file version %d</software>\n", gNotelistVersion);
	printf("		</encoding>\n");
	printf("	</identification>\n");

	printf("	<part-list>\n");
	for (i = 1; i<=MAXPARTS; i++) {
		/* no staff, so at last used staff; move on */
		if (gPartStaves[i]<=0) break;

		printf("		<score-part id=\"P%d\">\n", i);
		printf("			<part-name>Part %d</part-name>\n", i);
		printf("		</score-part>\n");
	}

	gNumParts = i-1;
	
	printf("	</part-list>\n");

	/* map each staff to a corresponding parent part
	 create an array mapping staves to parts */
	gStaffPartMap[0] = 0;

	staffNum = 1;


	for (partNum = 1; partNum <= gNumParts; partNum++) {
		stavesMaxThisPart = staffNum+gPartStaves[partNum];
		while (staffNum < stavesMaxThisPart) {
			gStaffPartMap[staffNum] = partNum;
			/* prevent a possible endless loop */
			if (staffNum > MAXSTAVES+1) {
				break;
			}
			staffNum++;
		}

		/* prevent a possible endless loop */
		if (partNum > MAXPARTS+1) {
			break;
		}
	}

	/* in music XML, staff numbers begin incrementing again from 1 with each new
		 part, but in notelist stafff numbers increment globally across all parts */
	for (thisPart = 1; thisPart <= MAXPARTS; thisPart ++) {
		if (gPartStaves[thisPart]<=0) break;
		for (XMLStaff = 1; XMLStaff <= gPartStaves[thisPart]; XMLStaff ++) {
			gStaffToStaffMap[nlStaff] = XMLStaff;				

			nlStaff++;
		}
	}

	/* since we now know how many parts we have, allocate memory for global arrays 
	   based on that number */
	gKeySignatures = (char **) malloc((gNumParts+1) * sizeof(char *));
	if (gKeySignatures == NULL) {
		fprintf(stderr,"Out of memory\n");
		exit(1);
	}
	init1dAryChar(gKeySignatures);

	gTimeSignatures = (char **) malloc((gNumParts+1) * sizeof(char *));
	if (gTimeSignatures == NULL) {
		fprintf(stderr,"Out of memory\n");
		exit(1);
	}
	init1dAryChar(gTimeSignatures);

	gClefTags = (char **) malloc((gNumParts+1) * sizeof(char *));
	if (gClefTags == NULL) {
		fprintf(stderr,"Out of memory\n");
		exit(1);
	}
	init1dAryChar(gClefTags);

	gFinalParts = (char **) malloc((gNumParts+1) * sizeof(char *));
	if (gFinalParts == NULL) {
		fprintf(stderr,"Out of memory\n");
		exit(1);
	}
	init1dAryChar(gFinalParts);

	gDirections = (char **) malloc((gNumParts+1) * sizeof(char *));
	if (gDirections == NULL) {
		fprintf(stderr,"Out of memory\n");
		exit(1);
	}
	init1dAryChar(gDirections);

	gMeasures = (char ***) malloc((gNumParts+1) * sizeof(char **));
	if (gMeasures == NULL) {
		fprintf(stderr,"Out of memory\n");
		exit(1);
	}
	init2dAryChar(gMeasures);

	gLyrics = (char ***) malloc((gNumParts+1) * sizeof(char **));
	if (gLyrics == NULL) {
		fprintf(stderr,"Out of memory\n");
		exit(1);
	}
	init2dAryChar(gLyrics);

	gNotations = (char ***) malloc((gNumParts+1) * sizeof(char **));
	if (gNotations == NULL) {
		fprintf(stderr,"Out of memory\n");
		exit(1);
	}
	init2dAryChar(gNotations);

	return TRUE;
}

Boolean printBarline (int i) {
	/* output the barline itself */

	strcat1d(gFinalParts, i, "			<barline location=\"right\">\n");

	switch (gBarlineType)
	{
		/*	normal bar line */
		case 1:
			strcat1d(gFinalParts, i, "				<bar-style>regular</bar-style>\n");
			strcat1d(gFinalParts, i, "			</barline>\n");
		break;
		/* double bar line */
		case 2:
			strcat1d(gFinalParts, i, "				<bar-style>light-light</bar-style>\n");
			strcat1d(gFinalParts, i, "			</barline>\n");
		break;
		/* final double bar line */
		case 3:
			strcat1d(gFinalParts, i, "				<bar-style>light-heavy</bar-style>\n");
			strcat1d(gFinalParts, i, "			</barline>\n");
		break;
		/* heavy double bar line */
		case 4:
			strcat1d(gFinalParts, i, "				<bar-style>heavy-heavy</bar-style>\n");
			strcat1d(gFinalParts, i, "			</barline>\n");
		break;
		/* repeat-left double bar line (||:) */
		case 5:
			strcat1d(gFinalParts, i, "				<bar-style>heavy-light</bar-style>\n");
			strcat1d(gFinalParts, i, "				<repeat direction=\"forward\" />\n");
			strcat1d(gFinalParts, i, "			</barline>\n");
		break;

		/* repeat-right double bar line (:||) */
		case 6:
			strcat1d(gFinalParts, i, "				<bar-style>light-heavy</bar-style>\n");
			strcat1d(gFinalParts, i, "				<repeat direction=\"backward\" />\n");
			strcat1d(gFinalParts, i, "			</barline>\n");
		break;
		/* repeat-both double bar line (:||:) still not sure about this one*/
		case 7:
			strcat1d(gFinalParts, i, "				<bar-style>light-light</bar-style>\n");
			strcat1d(gFinalParts, i, "				<repeat direction=\"forward\" />\n");
			strcat1d(gFinalParts, i, "			</barline>\n");
	}
	return TRUE;
}

Boolean ProcessBeam(ABEAM beam)
{
	gBeams[beam.npt][beam.v] = beam.count;
}

/* this is a hack,
		should make a better effort at character encoding and use UTF-8
        need to srip out newlines
*/
Boolean replaceSpecialChars (char *oldString)
{
	int i;
	int j;
	char newString[1024] = "";
	char from[] = "‘’“”¸°";
	/*may want to end up replacing these with strings ... (to represent encoded values)*/
	char to[] = "''''	";
	Boolean badChar = FALSE;

	for (i = 0; i < strlen(oldString); i ++) {
		badChar = FALSE;

		for (j = 0; j < strlen(from); j ++) {
			if (oldString[i] == from[j] ) {
				badChar = TRUE;
				break;
			}
		}
		if (!badChar) {
			sprintf (newString + strlen(newString), "%c", oldString[i]);
		} else {

			sprintf (newString + strlen(newString), "%c", to[j]);
		}
	}

	strcpy (oldString, "");
	strcpy (oldString, newString);

	return TRUE;
}

Boolean init1dAryChar (char **array) {
	int i = 0;

	for(i = 0; i <= gNumParts; i++){
		array[i] = (char *) malloc(1);
		if (array[i] == NULL){
			fprintf(stderr,"Out of memory\n");
			exit(1);
		}
		strcpy(array[i], "");
	}

	return TRUE;
}

Boolean strcpy1d (char **array, int part, char *thisString) {
	if (part < 0) {
		fprintf(stderr,"index in function strcpy1d cannot be negative\n");
		return FALSE;
	}
	free(array[part]);
	array[part] = (char *) malloc(strlen(thisString)+1);

	if (array[part] == NULL){
		fprintf(stderr,"Out of memory\n");

		exit(1);
	}
	strcpy(array[part], thisString);

	return TRUE;
}

Boolean strcat1d (char **array, int part, char *thisString) {
	if (part < 0) {
		fprintf(stderr,"index in function strcat1d cannot be negative\n");
		return FALSE;
	}
	array[part] = (char *) realloc(array[part], strlen(array[part]) + strlen(thisString) + 2);

	if (array[part] == NULL){
		fprintf(stderr,"Out of memory\n");
		exit(1);
	}

	strcat(array[part], thisString);

	return TRUE;
}

Boolean init2dAryChar (char ***array) {
	int i = 0, j = 0;

	for(i = 0; i <= gNumParts; i++){
		array[i] = (char **) malloc(sizeof(char *) * (MAXVOICES+1));
		if (array[i] == NULL){
			fprintf(stderr,"Out of memory\n");
			exit(1);
		}
		for(j = 0; j <= MAXVOICES; j++) {
			array[i][j] = (char *) malloc(1);
			if (array[i][j] == NULL){
				fprintf(stderr,"Out of memory\n");
				exit(1);
			}
			strcpy(array[i][j], "");
		}

	}
	return TRUE;
}

Boolean strcpy2d (char ***array, int part, int voice, char *thisString) {
	if (part < 0 || voice < 0) {
		fprintf(stderr,"indexes in function strcpy2d cannot be negative\n");
		return FALSE;
	}
	free(array[part][voice]);
	array[part][voice] = (char *) malloc(strlen(thisString)+1);
	if (array[part][voice] == NULL){
		fprintf(stderr,"Out of memory\n");
		exit(1);
	}
	strcpy(array[part][voice], thisString);

	return TRUE;
}

Boolean strcat2d (char ***array, int part, int voice, char *thisString) {
	if (part < 0 || voice < 0) {
		fprintf(stderr,"indexes in function strcat2d cannot be negative\n");
		return FALSE;
	}
	array[part][voice] = (char *) realloc(array[part][voice], strlen(array[part][voice]) + strlen(thisString) + 2);
	if (array[part][voice] == NULL){
		fprintf(stderr,"Out of memory\n");
		exit(1);
	}
	strcat(array[part][voice], thisString);

	return TRUE;
}


