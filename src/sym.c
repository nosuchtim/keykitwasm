/*
 *	Copyright 1996 AT&T Corp.  All rights reserved.
 */

#define OVERLAY8

#include "key.h"
#include "gram.h"

Htablep Keywords = NULL;
Htablep Macros = NULL;
Htablep Topht = NULL;
static Htablep Freeht = NULL;

Context *Currct = NULL;
Context *Topct = NULL;

Symlongp Merge, Now, Clicks, Debug, Sync, Optimize, Mergefilter, Nowoffset;
Symlongp Mergeport1, Mergeport2;
Symlongp Debugwait, Debugmidi, Debugrun, Debugfifo, Debugmouse, Debuggesture;
Symlongp Clocksperclick, Clicksperclock, Graphics, Debugdraw, Debugmalloc;
Symlongp Millicount, Throttle2, Loadverbose, Warnnegative, Midifilenoteoff;
Symlongp Drawcount, Mousedisable, Forceinputport, Showsync, Echoport;
Symlongp Inputistty, Debugoff, Fakewrap, Mfsysextype;
Symlongp Tempotrack, Onoffmerge, Defrelease, Grablimit, Mfformat, Defoutport;
Symlongp Filter, Record, Recsched, Throttle, Recfilter, Recinput, Recsysex;
Symlongp Lowcorelim, Arraysort, Midithrottle, Defpriority;
Symlongp Taskaddr, Debuginst, Usewindfifos, Prepoll, Printsplit;
Symlongp Novalval, Eofval, Intrval, Debugkill, Debugkill1, Linetrace;
Symlongp Abortonint, Abortonerr, Redrawignoretime, Resizeignoretime;
Symlongp Consecho, Checkcount, Isofuncwarn, Consupdown, Monitor_fnum;
Symlongp Consecho_fnum, Slashcheck, Directcount, SubstrCount;
Symlongp Mousefnum, Consinfnum, Consoutfnum, Midi_in_fnum, Mousefifolimit;
Symlongp Saveglobalsize, Warningsleep, Millires, Milliwarn, Resizefix;
Symlongp Deftimeout;
Symlongp Minbardx, Kobjectoffset, Midi_out_fnum, Mousemoveevents;
Symlongp Numinst1, Numinst2, Offsetpitch, Offsetfilter, DoDirectinput;
Symlongp Offsetportfilter;
Datum *Rebootfuncd, *Nullfuncd, *Errorfuncd;
Datum *Intrfuncd, *Nullvald;
Datum Zeroval, Noval, Nullval, _Dnumtmp_;
Datum *Colorfuncd;
Datum *Redrawfuncd;
Datum *Resizefuncd;
Datum *Exitfuncd;
Htablepp Track, Wpick;
Htablepp Chancolormap;
Symlongp Chancolors;

void
newcontext(Symbolp s, int sz)
{
	Context *c;

	c = (Context *) kmalloc(sizeof(Context),"newcontext");
	c->symbols = newht(sz);
	c->func = s;
	c->next = Currct;
	c->localnum = 1;
	c->paramnum = 1;
	Currct = c;
}

void
popcontext(void)
{
	Context *nextc;
	if ( Currct == NULL || Currct->next == NULL )
		execerror("popcontext called too many times!\n");
	nextc = Currct->next;
	kfree((char*)Currct);
	Currct = nextc;
}

Symbolp Free_sy = NULL;

Symbolp
newsy(void)
{
	static Symbolp lastsy;
	static int used = ALLOCSY;
	Symbolp s;

	/* First check the free list and use those nodes, before using */
	/* the newly allocated stuff. */
	if ( Free_sy != NULL ) {
		s = Free_sy;
		Free_sy = Free_sy->next;
		goto getout;
	}

	/* allocate a BUNCH of new ones at a time */
	if ( used == ALLOCSY ) {
		used = 0;
		lastsy = (Symbolp) kmalloc(ALLOCSY*sizeof(Symbol),"newsy");
	}
	used++;
	s = lastsy++;

    getout:
	s->stype = UNDEF;
	s->stackpos = 0;	/* i.e. it's global */
	s->flags = 0;
	s->onchange = NULL;
	s->next = NULL;
	return(s);
}

void
freesy(register Symbolp sy)
{
	/* Add it to the list of free symbols */
	sy->next = Free_sy;
	Free_sy = sy;
}

Symbolp
findsym(register char *p,Htablep symbols)
{
	Datum key;
	Hnodep h;

	key = strdatum(p);
	h = hashtable(symbols,key,H_LOOK);
	if ( h )
		return h->val.u.sym;
	else
		return NULL;
}

Symbolp
findobjsym(char *p,Kobjectp o,Kobjectp *foundobj)
{
	Datum key;
	Hnodep h;
	Htablep symbols = o->symbols;
	Symbolp s;

	if ( symbols == NULL ) {
		mdep_popup("Internal error - findobjsym finds NULL symbols!");
		return NULL;
	}
	key = strdatum(p);
	h = hashtable(symbols,key,H_LOOK);
	if ( h ) {
		if ( foundobj )
			*foundobj = o;
		return h->val.u.sym;
	}

	/* Not found, try inherited objects */
	if ( o->inheritfrom != NULL ) {
		Kobjectp o2;
		for ( o2=o->inheritfrom; o2!=NULL; o2=o2->nextinherit ) {
			s=findobjsym(p,o2,foundobj);
			if ( s != NULL ) {
				return(s);
			}
		}
	}
	return NULL;
}

Symbolp
uniqvar(char* pre)
{
	static long unum = 0;
	char buff[32];

	if ( pre == NULL )
		pre = "";
	strncpy(buff,pre,20);	/* 20 is 32 - (space for NONAMEPREFIX + num) */
	buff[20] = 0;
	sprintf(strend(buff),"%s%ld",NONAMEPREFIX,unum++);
	if ( unum > (MAXLONG-4) )
		execerror("uniqvar() has run out of names!?");
	return globalinstallnew(uniqstr(buff),VAR);
}

/* lookup(p) - find p in symbol table */
Symbolp
lookup(char *p)
{
	Symbolp s;

	if ( (s=findsym(p,Currct->symbols)) != NULL )
		return(s);
	if ( Currct != Topct && (s=findsym(p,Topct->symbols)) != NULL )
		return(s);
	return(NULL);
}

Symbolp
localinstall(Symstr p,int t)
{
	Symbolp s = syminstall(p,Currct->symbols,t);
	if ( Inparams )
		s->stackpos = Currct->paramnum++;
	else
		s->stackpos = -(Currct->localnum++);	/* okay if chars are unsigned*/
	return(s);
}

int Starting = 1;

Symbolp
globalinstall(Symstr p,int t)
{
	Symbolp s;
	if ( (s=findsym(p,Topct->symbols)) != NULL )
		return s;
	s = syminstall(p,Topct->symbols,t);
	return(s);
}

/* Use this variation if you know that the symbol is new */
Symbolp
globalinstallnew(Symstr p,int t)
{
	return syminstall(p,Topct->symbols,t);
}

Symbolp
syminstall(Symstr p,Htablep symbols,int t)
{
	Symbolp s;
	Hnodep h;
	Datum key;

	key = strdatum(p);
	h = hashtable(symbols,key,H_INSERT);
	if ( h==NULL )
		execerror("Unexpected h==NULL in syminstall!?");
	if ( isnoval(h->val) ) {
		s = newsy();
		s->name = key;
		s->stype = t;
		s->stackpos = 0;
		s->sd = Noval;
		h->val = symdatum(s);
	}
	else {
		if ( h->val.type != D_SYM )
			execerror("Unexpected h->val.type!=D_SYM in syminstall!?");
		s = h->val.u.sym;
	}
	return s;
}

void
clearsym(register Symbolp s)
{
#ifdef OLDSTUFF
	Codep cp;
	BLTINCODE bc;
#endif

	if ( s->stype == VAR ) {
		Datum *dp = symdataptr(s);
		switch (dp->type) {
		case D_ARR:
			if ( dp->u.arr ) {
				arrdecruse(dp->u.arr);
				dp->u.arr = NULL;
			}
			break;
		case D_PHR:
			if ( dp->u.phr != NULL ) {
				phdecruse(dp->u.phr);
				dp->u.phr = NULL;
			}
			break;
		case D_CODEP:

			/* BUG FIX - 5/4/97 - no longer free it */
#ifdef OLDSTUFF
			cp = dp->u.codep;
			bc = ((cp==NULL) ? 0 : BLTINOF(cp));
			/* If it's a built-in function, then the codep was */
			/* allocated by funcdp(), so we can free it. */
			if ( bc != 0 ) {
				/* kfree(cp); */
				dp->u.codep = NULL;
			}
#endif
			dp->u.codep = NULL;
			break;
		default:
			break;
		}
	}
}

static struct {		/* Keywords */
	char	*name;
	int	kval;
} keywords[] = {
	{ "function",	FUNC },
	{ "return",	RETURN },
	{ "if",		IF },
	{ "else",		ELSE },
	{ "while",	WHILE },
	{ "for",		FOR },
	{ "in",		SYM_IN },  /* to avoid conflict on windows */
	{ "break",	BREAK },
	{ "continue",	CONTINUE },
	{ "task",		TASK },
	{ "eval",		EVAL },
	{ "vol",		VOL },	/* sorry, I'm just used to 'vol' */
	{ "volume",	VOL },
	{ "vel",		VOL },
	{ "velocity",	VOL },
	{ "chan",		CHAN },
	{ "channel",	CHAN },
	{ "pitch",	PITCH },
	{ "time",		TIME },
	{ "dur",		DUR },
	{ "duration",	DUR },
	{ "length",	LENGTH },
	{ "number",	NUMBER },
	{ "type",		TYPE },
	{ "defined",	DEFINED },
	{ "undefine",	UNDEFINE },
	{ "delete",	SYM_DELETE },
	{ "readonly",	READONLY },
	{ "onchange",	ONCHANGE },
	{ "flags",	FLAGS },
	{ "varg",		VARG },
	{ "attrib",	ATTRIB },
	{ "global",	GLOBALDEC },
	{ "class",	CLASS },
	{ "method",	METHOD },
	{ "new",		KW_NEW },
	{ "nargs",	NARGS },
	{ "typeof",	TYPEOF },
	{ "xy",		XY },
	{ "port",		PORT },
	{ 0,		0 },
};

long
neednum(char *s,Datum d)
{
	if ( d.type != D_NUM && d.type != D_DBL )
		execerror("%s expects a number, got %s!",s,atypestr(d.type));
	return ( roundval(d) );
}

Codep
needfunc(char *s,Datum d)
{
	if ( d.type != D_CODEP )
		execerror("%s expects a function, got %s!",s,atypestr(d.type));
	return d.u.codep;
}

Kobjectp
needobj(char *s,Datum d)
{
	if ( d.type != D_OBJ )
		execerror("%s expects an object, got %s!",s,atypestr(d.type));
	return d.u.obj;
}

Fifo *
needfifo(char *s,Datum d)
{
	long n;
	Fifo *f;

	if ( d.type != D_NUM && d.type != D_DBL ) {
		execerror("%s expects a fifo id (i.e. a number), but got %s!",
			s,atypestr(d.type));
	}
	n = roundval(d);
	f = fifoptr(n);
	return f;
}

Fifo *
needvalidfifo(char *s,Datum d)
{
	Fifo *f;

	f = needfifo(s,d);
	if ( f == NULL )
		execerror("%s expects a fifo id, and %ld is not a valid fifo id!",s,numval(d));
	return f;
}

char *
needstr(char *s,Datum d)
{
	if ( d.type != D_STR )
		execerror("%s expects a string, got %s!",s,atypestr(d.type));
	return ( d.u.str );
}

Htablep
needarr(char *s,Datum d)
{
	if ( d.type != D_ARR )
		execerror("%s expects an array, got %s!",s,atypestr(d.type));
	return ( d.u.arr );
}

Phrasep
needphr(char *s,Datum d)
{
	if ( d.type != D_PHR )
		execerror("%s expects a phrase, got %s!",s,atypestr(d.type));
	return ( d.u.phr );
}

Symstr
datumstr(Datum d)
{
	char buff[32];
	char *p;
	long id;

	if ( isnoval(d) )
		execerror("Attempt to convert uninitialized value (Noval) to string!?");

	switch ( d.type ) {
	case D_NUM:
		(void) prlongto(d.u.val,p=buff);
		break;
	case D_PHR:
		p = phrstr(d.u.phr,0);
		break;
	case D_DBL:
		sprintf(p=buff,"%g",d.u.dbl);
		break;
	case D_STR:
		p = d.u.str;
		break;
	case D_ARR:
		/* we re-use the routines in main.c for doing */
		/* printing into a buffer.  I supposed we could really */
		/* do this for all the data types here. */
		stackbuffclear();
		prdatum(d,stackbuff,1);
		p = stackbuffstr();
		break;
	case D_OBJ:
		buff[0] = '$';
		id = (d.u.obj?d.u.obj->id:-1) + *Kobjectoffset;
		(void) prlongto(id,buff+1);
		p = buff;
		break;
	default:
		p = "";
		break;
	}
	p = uniqstr(p);
	return p;
}

Datum
newarrdatum(int used,int size)
{
	Datum d;

	d.type = D_ARR;
	d.u.arr = newht(size>0 ? size : ARRAYHASHSIZE);
	
#ifdef OLDSTUFF
if(Debug&&*Debug>1)eprint("newarrdatum(%d), d.u.arr=%ld\n",used,(long)(d.u.arr));
#endif
	d.u.arr->h_used = used;
	d.u.arr->h_tobe = 0;
	return d;
}

Htablepp
globarray(char *name)
{
	Datum *dp;
	Symbolp s;

	s = globalinstall(name,VAR);
	s->stype = VAR;
	dp = symdataptr(s);
	*dp = newarrdatum(1,0);
	return &(dp->u.arr);
}

Datum
phrsplit(Phrasep p)
{
#define MAXSIMUL 128
	Noteptr activent[MAXSIMUL];
	long activetime[MAXSIMUL];
	Noteptr n, newn;
	Phrasep p2;
	Symbolp s;
	Datum d, da;
	long tm2;
	int i, j, k;
	Htablep arr;
	int samesection, nactive = 0;
	long t, elapse, closest, now = 0L;
	long arrnum = 0L;

	da = newarrdatum(0,0);
	arr = da.u.arr;

	n = firstnote(p);

	while ( n!=NULL || nactive>0 ) {

		/* find out which event is closer: the end of a pending */
		/* note, or the start of the next one (if there is one). */

		if ( n != NULL ) {
			closest = n->clicks - now;
			samesection = 1;
		}
		else {
			closest = 30000;
			samesection = 0;
		}

		/* Check the ending times of the pending notes, to see */
		/* if any of them end before the next note starts.  If so, */
		/* we want to create a new section.  */
		for ( k=0; k<nactive; k++ ) {
#ifdef OLDSTUFF
			if ( durof(activent[k]) <= 0 )
				continue;
#endif
			if ( (t=activetime[k]) <= closest ) {
				closest = t;
				samesection = 0;
			}
		}

		/* We want to let that amount of time elapse. */
		elapse = closest;
		if ( samesection!=0 && (closest == 0 || nactive == 0) )
			goto addtosame;

		/* We're going to create a new element in the split array */

		d = numdatum((long)arrnum++);
		s = arraysym(arr,d,H_INSERT);
		p2 = newph(1);

		/* add all active notes to the phrase */
		tm2 = now+elapse;
		for ( k=0; k<nactive; k++ ) {
			newn = ntcopy(activent[k]);
			if ( timeof(newn) < now ) {
				long overhang = now - timeof(newn);
				timeof(newn) += overhang;
				durof(newn) -= overhang;
			}
			if ( endof(newn) > tm2 ) {
				durof(newn) = tm2-timeof(newn);
			}
			ntinsert(newn,p2);
		}
		p2->p_leng = tm2;
		*symdataptr(s) = phrdatum(p2);

		/* If any notes are pending, take into account elapsed */
		/* time, and if they expire, get rid of them. */
		for ( i=0; i<nactive; i++ ) {
			if ( (activetime[i] -= elapse) <= 0L ) {
				/* Remove this note from the list by */
				/* shifting everything down. */
				for ( j=i+1; j<nactive; j++ ) {
					activetime[j-1] = activetime[j];
					activent[j-1] = activent[j];
				}
				nactive--;
				i--;	/* don't advance loop index */
			}
		}

	addtosame:
		if ( samesection ) {
			/* add this new note (and all others that start at the */
			/* same time) to the list of active ones */
			long thistime = timeof(n);
			while ( n!=NULL && timeof(n) == thistime ) {
				if ( nactive >= MAXSIMUL )
					execerror("Too many simultaneous notes in expression (limit is %d)\n",MAXSIMUL);
				activent[nactive] = n;
				activetime[nactive++] = durof(n);
				/* advance to next note */
				n = nextnote(n);
			}
		}
		now += elapse;
	}
	return(da);
}

/* sep contains the list of possible separator characters. */
/* multiple consecutive separator characters are treated as one. */

Datum
strsplit(char *str,char *sep)
{
	char buffer[128];
	char *buff, *p, *endp;
	char *word = NULL;
	int isasep;
	Datum da;
	Htablep arr;
	long n;
	int slen = (int)strlen(str);
	int state;

	/* avoid kmalloc if we can use small buffer */
	if ( slen >= (int)sizeof(buffer) )
		buff = kmalloc((unsigned)(slen+1),"strsplit");
	else
		buff = buffer;
	strcpy(buff,str);
	endp = buff + slen;

	/* An inital scan to figure out how big an array we need */
	for ( state=0,n=0,p=buff; state >= 0 && p < endp ; p++ ) {
		isasep = (strchr(sep,*p) != NULL);
		switch ( state ) {
		case 0:	/* before word */
			if ( ! isasep )
				state = 1;
			break;
		case 1:	/* scanning word */
			if ( isasep ) {
				n++;
				state=0;
			}
			break;
		}
	}
	if ( state == 1 )
		n++;

	da = newarrdatum(0,(int)n);
	arr = da.u.arr;

	for ( state=0,n=0,p=buff; state >= 0 && p < endp ; p++ ) {

		isasep = (strchr(sep,*p) != NULL);
		switch ( state ) {
		case 0:	/* before word */
			if ( ! isasep ) {
				word = p;
				state = 1;
			}
			break;
		case 1:	/* scanning word */
			if ( isasep ) {
				*p = '\0';
				setarrayelem(arr,n++,word);
				state=0;
			}
			break;
		}
	}
	if ( state == 1 ) {
		*p = '\0';
		setarrayelem(arr,n++,word);
	}
	if ( slen >= (int)sizeof(buffer) )
		kfree(buff);
	return(da);
}

void
setarraydata(Htablep arr,Datum i,Datum d)
{
	Symbolp s;
	if ( isnoval(i) )
		execerror("Can't use undefined value as array index\n");
	s = arraysym(arr,i,H_INSERT);
	*symdataptr(s) = d;
}

void
setarrayelem(Htablep arr,long n,char *p)
{
	setarraydata(arr,numdatum(n),strdatum(uniqstr(p)));
}

void
fputdatum(FILE *f,Datum d)
{
	char *str = datumstr(d);
	int c;
	int i = 0;
	int nl = (*Printsplit)!=0;

	if ( d.type == D_STR )
		putc('"',f);
	for ( ; (c=(*str)) != '\0'; str++ ) {
		char *p = NULL;
		i++;
		if ( nl>0 && i>nl ) {
			i = 0;
			fputs("\\\n",f);
		}
		switch (c) {
		case '\n':
			p = "\\n";
			break;
		case '\r':
			p = "\\r";
			break;
		case '"':
			p = "\\\"";
			break;
		case '\\':
			p = "\\\\";
			break;
		}
		if ( p )
			fputs(p,f);
		else
			putc(c,f);
	}
	if ( d.type == D_STR )
		putc('"',f);
}

static struct binum {
	char *name;
	long val;
	Symlongp *ptovar;
} binums[] = {
	{ "Noval", MAXLONG, &Novalval },
	{ "Eof", MAXLONG-1, &Eofval },
	{ "Interrupt", MAXLONG-2, &Intrval },
	{ "Merge", 0L, &Merge },
	{ "Mergeport1", 0L, &Mergeport1 },    // default output
	{ "Mergeport2", -1L, &Mergeport2 },   //
	{ "Mergefilter", 0L, &Mergefilter },
	{ "Clicks", (long)(DEFCLICKS), &Clicks },
	{ "Debug", 0L, &Debug },
	{ "Optimize", 1L, &Optimize },
	{ "Debugwait", 0L, &Debugwait },
	{ "Debugoff", 0L, &Debugoff },
	{ "Fakewrap", 0L, &Fakewrap },
	{ "Debugrun", 0L, &Debugrun },
	{ "Debuginst", 0L, &Debuginst },
	{ "Debugkill", 0L, &Debugkill },
	{ "Debugfifo", 0L, &Debugfifo },
	{ "Debugmalloc", 0L, &Debugmalloc },
	{ "Debugdraw", 0L, &Debugdraw },
	{ "Debugmouse", 0L, &Debugmouse },
	{ "Debugmidi", 0L, &Debugmidi },
	{ "Debuggesture", 0L, &Debuggesture },
	{ "Now", -1L, &Now },
	{ "Nowoffset", 0L, &Nowoffset },
	{ "Sync", 0L, &Sync },
	{ "Showsync", 0L, &Showsync },
	{ "Clocksperclick", 1L, &Clocksperclick },
	{ "Clicksperclock", 1L, &Clicksperclock },
	{ "Filter", 0L, &Filter },	/* bitmask for message filtering */
	{ "Record", 1L, &Record },		/* If 0, recording is disabled */
	{ "Recsched", 0L, &Recsched },	/* If 1, record scheduled stuff */
	{ "Recinput", 1L, &Recinput },	/* If 1, record midi input */
	{ "Recsysex", 1L, &Recsysex },	/* If 1, record sysex */
	{ "Recfilter", 0L, &Recfilter },	/* per-channel bitmask turns off recording */
	{ "Lowcore", DEFLOWLIM, &Lowcorelim },
	{ "Millicount", 0L, &Millicount },	/* see mdep.c */
	{ "Throttle2", 100L, &Throttle2 },
	{ "Drawcount", 8L, &Drawcount },
	{ "Mousedisable", 0L, &Mousedisable },
	{ "Forceinputport", -1L, &Forceinputport },
	{ "Checkcount", 20L, &Checkcount },
	{ "Loadverbose", 0L, &Loadverbose },
	{ "Warnnegative", 1L, &Warnnegative },
	{ "Midifilenoteoff", 1L, &Midifilenoteoff },
	{ "Isofuncwarn", 1L, &Isofuncwarn },
	{ "Inputistty", 0L, &Inputistty },
	{ "Arraysort", 0, &Arraysort },
	{ "Taskaddr", 0, &Taskaddr },
	{ "Tempotrack", 0, &Tempotrack },
	{ "Onoffmerge", 1, &Onoffmerge },
	{ "Defrelease", 0, &Defrelease },
	{ "Defoutport", 0, &Defoutport },
	{ "Echoport", 0, &Echoport },
	{ "Grablimit", 1000, &Grablimit },
	{ "Mfformat", 0, &Mfformat },
	{ "Mfsysextype", 0, &Mfsysextype },
	{ "Trace", 1, &Linetrace },
	{ "Abortonint", 0, &Abortonint },
	{ "Abortonerr", 0, &Abortonerr },
	{ "Debugkill1", 0, &Debugkill1 },
	{ "Consecho", 1, &Consecho },
	{ "Slashcheck", 1, &Slashcheck },
	{ "Directcount", 0, &Directcount },
	{ "SubstrCount", 0, &SubstrCount },
	{ "Consupdown", 0, &Consupdown },
	{ "Prepoll", 0, &Prepoll },
	{ "Printsplit", 77, &Printsplit },
	{ "Midithrottle", 128, &Midithrottle },
	{ "Throttle", 100, &Throttle },
	{ "Defpriority", 500, &Defpriority },
	{ "Redrawignoretime", 100L, &Redrawignoretime },
	{ "Resizeignoretime", 100L, &Resizeignoretime },
	{ "Graphics", 1, &Graphics },
	{ "Consinfifo", -1, &Consinfnum },
	{ "Consoutfifo", -1, &Consoutfnum },
	{ "Mousefifo", -1, &Mousefnum },
	{ "Midiinfifo", -1, &Midi_in_fnum },
	{ "Midioutfifo", -1, &Midi_out_fnum },
	{ "Monitorfifo", -1, &Monitor_fnum },
	{ "Consechofifo", -1, &Consecho_fnum },
	{ "Saveglobalsize", 256L, &Saveglobalsize },
	{ "Warningsleep", 0L, &Warningsleep },
	{ "Deftimeout", 2L, &Deftimeout },
	{ "Millires", 1L, &Millires },
	{ "Milliwarn", 2L, &Milliwarn },
	{ "Resizefix", 1L, &Resizefix },
	{ "Mousemoveevents", 0L, &Mousemoveevents },
	{ "Objectoffset", 0L, &Kobjectoffset },
	{ "Showtext", 1, &Showtext },
	{ "Showbar", 4*DEFCLICKS, &Showbar },
	{ "Sweepquant", 1, &Sweepquant },
	{ "Menuymargin", 2, &Menuymargin },
	{ "Menusize", 12, &Menusize },
	{ "Dragquant", 1, &Dragquant },
	{ "Menuscrollwidth", 15, &Menuscrollwidth },
	{ "Textscrollsize", 200, &Textscrollsize },
	{ "Menujump", 0, &Menujump },
	{ "Panraster", 1, &Panraster },
	{ "Bendrange", 1024*16, &Bendrange },
	{ "Bendoffset", 64, &Bendoffset },
	{ "Volstem", 0, &Volstem },
	{ "Volstemsize", 4, &Volstemsize },
	{ "Colors", 2, &Colors },
	{ "Colornotes", 1, &Colornotes },
	{ "Chancolors", 0, &Chancolors },
	{ "Inverse", 0, &Inverse },
	{ "Usewindfifos", 0, &Usewindfifos },
	{ "Mousefifolimit", 1L, &Mousefifolimit },
	{ "Minbardx", 8L, &Minbardx },
	{ "Numinst1", 0L, &Numinst1 },
	{ "Numinst2", 0L, &Numinst2 },
	{ "Directinput", 0L, &DoDirectinput },
	{ "Offsetpitch", 0L, &Offsetpitch },
	{ "Offsetportfilter", -1L, &Offsetportfilter },
	{ "Offsetfilter", 1<<9, &Offsetfilter },/* per-channel bitmask turns off
		offset effect, default turns it off for channel 10, drums */
	{ 0, 0, 0 }
};

Phrasepp Currphr, Recphr;

static struct biphr {
	char *name;
	Phrasepp *ptophr;
} biphrs[] = {
	{ "Current", &Currphr },
	{ "Recorded", &Recphr },
	{ 0, 0 }
};

Symstrp Keypath, Machine, Keyerasechar, Keykillchar, Keyroot;
Symstrp Printsep, Printend, Musicpath;
Symstrp Pathsep, Dirseparator, Devmidi, Version, Initconfig, Keypagepersistent, Nullvalsymp;
Symstrp Fontname, Icon, Windowsys, Drawwindow, Picktrack;

static struct bistr {
	char *name;
	char *val;
	Symstrp *ptostr;
} bistrs[] = {
	{ "Keyroot", "", &Keyroot },
	{ "Keypath", "", &Keypath },
	{ "Musicpath", "", &Musicpath },
	{ "Machine", MACHINE, &Machine },
	{ "Devmidi", "", &Devmidi },
	{ "Printsep", " ", &Printsep },
	{ "Printend", "\n", &Printend },
	{ "Pathseparator", PATHSEP, &Pathsep },
	{ "Dirseparator", SEPARATOR, &Dirseparator },
	{ "Version", KEYVERSION, &Version },
	{ "Initconfig", "", &Initconfig },
	{ "Keypagepersistent", "", &Keypagepersistent },
	{ "Killchar", "", &Keykillchar },
	{ "Erasechar", "", &Keyerasechar },
	{ "Font", "", &Fontname },
	{ "Icon", "", &Icon },
	{ "Windowsys", "", &Windowsys },
	{ "Nullval", "", &Nullvalsymp },
	{ 0, 0, 0 }
};

void
installnum(char *name,Symlongp *pvar,long defval)
{
	Symbolp s;
	name = uniqstr(name);
	/* Only install and set value if not already present */
	if ( (s=lookup(name)) == NULL ) {
		s = globalinstallnew(name,VAR);
		*symdataptr(s) = numdatum(defval);
	}
	*pvar = (Symlongp)( &(symdataptr(s)->u.val) ) ;
}

void
installstr(char *name,char *str)
{
	Symbolp s = globalinstallnew(uniqstr(name),VAR);
	*symdataptr(s) = strdatum(uniqstr(str));
}

/* build a Datum that is a function pointer, pointing to a built-in function */
Datum
funcdp(Symbolp s, BLTINCODE f)
{
	Codep cp;
	Datum d;
	int sz;

	sz = Codesize[IC_BLTIN] + varinum_size(0) + Codesize[IC_SYM];
	cp = (Codep) kmalloc(sz,"funcdp");
	// keyerrfile("CP 0 = %lld, sz=%d\n", (intptr_t)cp,sz);

	*Numinst1 += sz;

	d.type = D_CODEP;
	d.u.codep = cp;

	cp = put_bltincode(f,cp);
	// keyerrfile("CP 3 = %lld\n", (intptr_t)cp);
	cp = put_numcode(0,cp);
	// keyerrfile("CP 4 = %lld\n", (intptr_t)cp);
	cp = put_symcode(s,cp);
	// keyerrfile("CP 5 = %lld\n", (intptr_t)cp);
	return d;
}

/* Pre-defined macros.  It is REQUIRED that these values match the */
/* corresponding values in phrase.h and grid.h.  For example, the value */
/* of P_STORE must match STORE, NT_NOTE must match NOTE, etc.  */

static char *Stdmacros[] = {

	/* These are values for nt.type, also used as bit-vals for  */
	/* the value of Filter. */
	"MIDIBYTES 1", /* NT_LE3BYTES is not here - not user-visible */
	"NOTE 2",
	"NOTEON 4",
	"NOTEOFF 8",
	"CHANPRESSURE 16", "CONTROLLER 32", "PROGRAM 64", "PRESSURE 128",
		"PITCHBEND 256", "SYSEX 512", "POSITION 1024", "CLOCK 2048",
		"SONG 4096", "STARTSTOPCONT 8192", "SYSEXTEXT 16384",

	"Nullstr \"\"",

	/* Values for action() types.  The values are intended to not */
	/* overlap the values for interrupt(), to avoid misuse and */
	/* also to leave open the possibility of merging the two. */
	"BUTTON1DOWN 1024", "BUTTON2DOWN 2048", "BUTTON12DOWN 4096",
	"BUTTON1UP 8192", "BUTTON2UP 16384", "BUTTON12UP 32768",
	"BUTTON1DRAG 65536", "BUTTON2DRAG 131072", "BUTTON12DRAG 262144",
	"MOVING 524288",
	/* values for setmouse() and sweep() */
	"NOTHING 0", "ARROW 1", "SWEEP 2", "CROSS 3",
		"LEFTRIGHT 4", "UPDOWN 5", "ANYWHERE 6", "BUSY 7",
		"DRAG 8", "BRUSH 9", "INVOKE 10", "POINT 11", "CLOSEST 12",
		"DRAW 13",
	/* values for cut() */
	"NORMAL 0", "TRUNCATE 1", "INCLUSIVE 2",
	"CUT_TIME 3", "CUT_FLAGS 4", "CUT_TYPE 5",
	"CUT_CHANNEL 6", "CUT_NOTTYPE 7",
	/* values for menudo() */
	"MENU_NOCHOICE -1", "MENU_BACKUP -2", "MENU_UNDEFINED -3",
	"MENU_MOVE -4", "MENU_DELETE -5",
	/* values for draw() */
	"CLEAR 0", "STORE 1", "XOR 2",
	/* values for window() */
	"TEXT 1", "PHRASE 2",
	/* values for style() */
	"NOBORDER 0", "BORDER 1", "BUTTON 2", "MENUBUTTON 3", "PRESSEDBUTTON 4",
	/* values for kill() signals */
	"KILL 1",
	NULL
};

/* initsyms() - install constants and built-ins in table */
void
initsyms(void)
{
	int i;
	Symbolp s;
	Datum *dp;
	char *p;

	Zeroval = numdatum(0L);
	Noval = numdatum(MAXLONG);
	Nullstr = uniqstr("");

	Keywords = newht(113);	/* no good reason for 113 */ 
	for (i = 0; (p = keywords[i].name) != NULL; i++) {
		(void)syminstall(uniqstr(p), Keywords, keywords[i].kval);
	}

	for (i=0; (p=binums[i].name)!=NULL; i++) {
		/* Don't need to uniqstr(p), because installnum does it. */ 
		installnum(p,binums[i].ptovar,binums[i].val);
	}
	
	for (i=0; (p=biphrs[i].name)!=NULL; i++) {
		s = globalinstallnew(uniqstr(p),VAR);
		dp = symdataptr(s);
		*dp = phrdatum(newph(1));
		*(biphrs[i].ptophr) = &(dp->u.phr);
		s->stackpos = 0;	/* i.e. it's global */
	}
	
	for (i=0; (p=bistrs[i].name)!=NULL; i++) {
		s = globalinstallnew(uniqstr(p),VAR);
		dp = symdataptr(s);
		*dp = strdatum(uniqstr(bistrs[i].val));
		*(bistrs[i].ptostr) = &(dp->u.str);
	}
	
	for (i=0; (p=builtins[i].name)!=NULL; i++) {
		s = globalinstallnew(uniqstr(p), VAR);
		dp = symdataptr(s);
		*dp = funcdp(s,builtins[i].bltindex);
	}

	Rebootfuncd = symdataptr(lookup(uniqstr("Rebootfunc")));
	Nullfuncd = symdataptr(lookup(uniqstr("nullfunc")));
	Errorfuncd = symdataptr(lookup(uniqstr("Errorfunc")));
	Intrfuncd = symdataptr(lookup(uniqstr("Intrfunc")));
	Nullvald = symdataptr(lookup(uniqstr("Nullval")));
	Nullval = *Nullvald;

	Colorfuncd = symdataptr(lookup(uniqstr("Colorfunc")));
	Redrawfuncd = symdataptr(lookup(uniqstr("Redrawfunc")));
	Resizefuncd = symdataptr(lookup(uniqstr("Resizefunc")));
	Exitfuncd = symdataptr(lookup(uniqstr("Exitfunc")));
	Track = globarray(uniqstr("Track"));
	Chancolormap = globarray(uniqstr("Chancolormap"));

	Macros = newht(113);	/* no good reason for 113 */

	for ( i=0; (p=Stdmacros[i]) != NULL;  i++ ) {
		/* Some compilers make strings read-only */
		p = strsave(p);
		macrodefine(p,0);
		free(p);
	}
	sprintf(Msg1,"MAXCLICKS=%ld",(long)(MAXCLICKS));
	macrodefine(Msg1,0);
	sprintf(Msg1,"MAXPRIORITY=%ld",(long)(MAXPRIORITY));
	macrodefine(Msg1,0);

	*Inputistty = mdep_fisatty(Fin) ? 1 : 0;
	if ( *Inputistty == 0 )
		*Consecho = 0;
	Starting = 0;

	*Keypath = uniqstr(mdep_keypath());
	*Musicpath = uniqstr(mdep_musicpath());
}

void
initsyms2(void)
{
	if ( **Keyerasechar == '\0' ) {
		char str[2];
		str[0] = Erasechar;
		str[1] = '\0';
		*Keyerasechar = uniqstr(str);
	}
	if ( **Keykillchar == '\0' ) {
		char str[2];
		str[0] = Killchar;
		str[1] = '\0';
		*Keykillchar = uniqstr(str);
	}
}

Datum Str_x0, Str_y0, Str_x1, Str_y1, Str_x, Str_y, Str_button;
Datum Str_type, Str_mouse, Str_drag, Str_move, Str_up, Str_down;
Datum Str_highest, Str_lowest, Str_earliest, Str_latest, Str_modifier;
Datum Str_default, Str_w, Str_r, Str_init;
Datum Str_get, Str_set, Str_newline;
Datum Str_red, Str_green, Str_blue, Str_grey, Str_surface;
Datum Str_finger, Str_hand, Str_xvel, Str_yvel;
Datum Str_proximity, Str_orientation, Str_eccentricity;
Datum Str_width, Str_height;
#ifdef MDEP_OSC_SUPPORT
Datum Str_elements, Str_seconds, Str_fraction;
#endif

void
initstrs(void)
{
	Str_type = strdatum(uniqstr("type"));
	Str_mouse = strdatum(uniqstr("mouse"));
	Str_drag = strdatum(uniqstr("mousedrag"));
	Str_move = strdatum(uniqstr("mousemove"));
	Str_up = strdatum(uniqstr("mouseup"));
	Str_down = strdatum(uniqstr("mousedown"));
	Str_x = strdatum(uniqstr("x"));
	Str_y = strdatum(uniqstr("y"));
	Str_x0 = strdatum(uniqstr("x0"));
	Str_y0 = strdatum(uniqstr("y0"));
	Str_x1 = strdatum(uniqstr("x1"));
	Str_y1 = strdatum(uniqstr("y1"));
	Str_button = strdatum(uniqstr("button"));
	Str_modifier = strdatum(uniqstr("modifier"));
	Str_highest = strdatum(uniqstr("highest"));
	Str_lowest = strdatum(uniqstr("lowest"));
	Str_earliest = strdatum(uniqstr("earliest"));
	Str_latest = strdatum(uniqstr("latest"));
	Str_default = strdatum(uniqstr("default"));
	Str_w = strdatum(uniqstr("w"));
	Str_r = strdatum(uniqstr("r"));
	Str_init = strdatum(uniqstr("init"));
	Str_get = strdatum(uniqstr("get"));
	Str_set = strdatum(uniqstr("set"));
	Str_newline = strdatum(uniqstr("\n"));
	Str_red = strdatum(uniqstr("red"));
	Str_green = strdatum(uniqstr("green"));
	Str_blue = strdatum(uniqstr("blue"));
	Str_grey = strdatum(uniqstr("grey"));
	Str_surface = strdatum(uniqstr("surface"));
	Str_finger = strdatum(uniqstr("finger"));
	Str_hand = strdatum(uniqstr("hand"));
	Str_xvel = strdatum(uniqstr("xvel"));
	Str_yvel = strdatum(uniqstr("yvel"));
	Str_proximity = strdatum(uniqstr("proximity"));
	Str_orientation = strdatum(uniqstr("orientation"));
	Str_eccentricity = strdatum(uniqstr("eccentricity"));
	Str_height = strdatum(uniqstr("height"));
	Str_width = strdatum(uniqstr("width"));
#ifdef MDEP_OSC_SUPPORT
	Str_elements = strdatum(uniqstr("elements"));
	Str_seconds = strdatum(uniqstr("seconds"));
	Str_fraction = strdatum(uniqstr("fraction"));
#endif
}

static FILE *Mf;

void
pfprint(char *s)
{
	fputs(s,Mf);
}

void
phtofile(FILE *f,Phrasep p)
{
	Mf = f;
	phprint(pfprint,p,0);
	putc('\n',f);
	if ( fflush(f) )
		mdep_popup("Unexpected error from fflush()!?");
}

void
vartofile(Symbolp s, char *fname)
{
	FILE *f;
	
	if ( fname==NULL || *fname == '\0' )
		return;

	if ( stdioname(fname) )
		f = stdout;
	else if ( *fname == '|' ) {
#ifdef PIPES
		f = popen(fname+1,"w");
		if ( f == NULL ) {
			eprint("Can't open pipe: %s\n",fname+1);
			return;
		}
#else
		eprint("No pipes!\n");
		return;
#endif
	}
	else {
		f = getnopen(fname,"w");
		if ( f == NULL ) {
			eprint("Can't open %s\n",fname);
			return;
		}
	}

	phtofile(f,symdataptr(s)->u.phr);
	
	if ( f != stdout ) {
		if ( *fname != '|' )
			getnclose(fname);
#ifdef PIPES
		else {
			if ( pclose(f) < 0 )
				eprint("Error in pclose!?\n"); 
		}
#endif
	}
}

/* Map the contents of a file (or output of a pipe) into a phrase */
/* variable. Note that if the file can't be read or the pipe can't */
/* be opened, it's a silent error. */

void
filetovar(register Symbolp s, char *fname)
{
	FILE *f;
	Phrasep ph;

	if ( fname==NULL || *fname == '\0' )
		return;

	if ( stdioname(fname) )
		f = stdin;
	else if ( *fname == '|' ) {
		/* It's a pipe... */
#ifdef PIPES
		f = popen(fname+1,"r");
#else
		warning("No pipes!");
		return;
#endif
	}
	else {
		/* a normal file */

		/* Use KEYPATH value to look for files. */
		char *pf = mpathsearch(fname);
		if ( pf )
			fname = pf;

		f = getnopen(fname,"r");
	}
	if ( f == NULL || feof(f) )
		return;		/* Silence.  Might be appropriate to */
				/* make some noise when a pipe fails. */
	clearsym(s);
	s->stype = VAR;
	ph = filetoph(f,fname);
	phincruse(ph);
	*symdataptr(s) = phrdatum(ph);

	if ( f != stdin ) {
		if ( *fname != '|' )
			getnclose(fname);
#ifdef PIPES
		else
			if ( pclose(f) < 0 )
				eprint("Error in pclose!?\n"); 
#endif
	}

}

Hnodep Free_hn = NULL;

Hnodep
newhn(void)
{
	static Hnodep lasthn;
	static int used = ALLOCHN;
	Hnodep hn;

	/* First check the free list and use those nodes, before using */
	/* the newly allocated stuff. */
	if ( Free_hn != NULL ) {
		hn = Free_hn;
		Free_hn = Free_hn->next;
		goto getout;
	}

	/* allocate a BUNCH of new ones at a time */
	if ( used == ALLOCHN ) {
		used = 0;
		lasthn = (Hnodep) kmalloc(ALLOCHN*sizeof(Hnode),"newhn");
	}
	used++;
	hn = lasthn++;

    getout:
	hn->next = NULL;
	hn->val = symdatum(NULL);
	/* hn->key = NULL; */
	return(hn);
}

void
freehn(Hnodep hn)
{
	if ( hn == NULL )
		execerror("Hey, hn==NULL in freehn\n");

	switch ( hn->val.type ) {
	case D_SYM:
		if ( hn->val.u.sym ) {
			clearsym(hn->val.u.sym);
			freesy(hn->val.u.sym);
		}
		break;
	case D_TASK:
		if ( hn->val.u.task ) {
			freetp(hn->val.u.task);
		}
		break;
	case D_FIFO:
		if ( hn->val.u.fifo )
			freeff(hn->val.u.fifo);
		break;
	case D_WIND:
		break;
	default:
		eprint("Hey, type=%d in clearhn, should something go here??\n",hn->val.type);
		break;
	}
	hn->val = Noval;

	hn->next = Free_hn;
	Free_hn = hn;
}

#ifdef OLDSTUFF
void
chkfreeht() {
	register Htablep ht;
	if ( Freeht == NULL || Freeht->h_next == NULL )
		return;
	for ( ht=Freeht->h_next; ht!=NULL; ht=ht->h_next ) {
		if ( ht == Freeht ) {
			eprint("INFINITE LOOP IN FREEHT LIST!!!\n");
			abort();
		}
	}
}
#endif

/* To avoid freeing and re-allocating the large chunks of memory */
/* used for the hash tables, we keep them around and reuse them. */

Htablep
newht(int size)
{
	register Hnodepp h, pp;
	register Htablep ht;

/* eprint("(newht(%d ",size); */
	/* See if there's a saved table we can use */
	for ( ht=Freeht; ht!=NULL; ht=ht->h_next ) {
		if ( ht->size == size )
			break;
	}
	if ( ht != NULL ) {
		/* Remove from Freeht list */
		if ( ht->h_prev == NULL ) {
			/* it's the first one in the Freeht list */
			Freeht = ht->h_next;
			if ( Freeht != NULL )
				Freeht->h_prev = NULL;
		}
		else if ( ht->h_next == NULL ) {
			/* it's the last one in the Freeht list */
			ht->h_prev->h_next = NULL;
		}
		else {
			ht->h_next->h_prev = ht->h_prev;
			ht->h_prev->h_next = ht->h_next;
		}
	}
	else {
		ht = (Htablep) kmalloc( sizeof(Htable), "newht" );
		h = (Hnodepp) kmalloc( size * sizeof(Hnodep), "newht" );
		ht->size = size;
		ht->nodetable = h;
		/* initialize entire table to NULLS */
		pp = h + size;
		while ( pp-- != h )
			*pp =  NULL;
	}

	ht->count = 0;
	ht->h_used = 0;
	ht->h_tobe = 0;
	ht->h_next = NULL;
	ht->h_prev = NULL;
	ht->h_state = 0;
	if ( Topht != NULL ) {
		Topht->h_prev = ht;
		ht->h_next = Topht;
	}
	Topht = ht;
	return(ht);
}

void
clearht(Htablep ht)
{
	register Hnodep hn, nexthn;
	register Hnodepp pp;
	register int n = ht->size;
	
	pp = ht->nodetable;
	/* as we're freeing the Hnodes pointed to by this hash table, */
	/* we zero out the table, in preparation for its reuse. */
#ifdef lint
	nexthn = 0;
#endif
	if ( ht->count != 0 ) {
		while ( --n >= 0 ) {
			for ( hn=(*pp); hn != NULL; hn=nexthn ) {
				nexthn = hn->next;
/* if(*Debug>0)eprint("freehn being called from clearht\n"); */
				freehn(hn);
			}
			*pp++ = NULL;
		}
	}
	ht->count = 0;
}

void
freeht(Htablep ht)
{
	register Htablep ht2;

	clearht(ht);

	/* If it's in the Htobechecked list... */
	for ( ht2=Htobechecked; ht2!=NULL; ht2=ht2->h_next ) {
		if ( ht2 == ht )
			break;
	}
	/* remove it */
	if ( ht2 != NULL ) {
		if ( ht2->h_next )
			ht2->h_next->h_prev = ht2->h_prev;
		if ( ht2 == Htobechecked )
			Htobechecked = ht2->h_next;
		else
			ht2->h_prev->h_next = ht2->h_next;
	}

	for ( ht2=Freeht; ht2!=NULL; ht2=ht2->h_next ) {
		if ( ht == ht2 ) {
			eprint("HEY!, Trying to free an ht node (%lld) that's already in the Free list!!\n",(intptr_t)ht);
			abort();
		}
	}
	/* Add to Freeht list */
	if ( Freeht )
		Freeht->h_prev = ht;
	ht->h_next = Freeht;
	ht->h_prev = NULL;
	ht->h_used = 0;
	ht->h_tobe = 0;
	ht->h_state = 0;
	Freeht = ht;
}

void
htlists(void)
{
	Htablep ht3;
	eprint("   Here's the Freeht list:");
	for(ht3=Freeht;ht3!=NULL;ht3=ht3->h_next)eprint("(%lld,sz%d,u%d,t%d)",(intptr_t)ht3,ht3->size,ht3->h_used,ht3->h_tobe);
	eprint("\n");
	eprint("   Here's the Htobechecked list:");
	for(ht3=Htobechecked;ht3!=NULL;ht3=ht3->h_next)eprint("(%lld,sz%d,u%d,t%d)",(intptr_t)ht3,ht3->size,ht3->h_used,ht3->h_tobe);
	eprint("\n");
	eprint("   Here's the Topht list:");
	for(ht3=Topht;ht3!=NULL;ht3=ht3->h_next)eprint("(%lld,sz%d,u%d,t%d)",(intptr_t)ht3,ht3->size,ht3->h_used,ht3->h_tobe);
	eprint("\n");
}

/* ========== Interned String Table with Incremental Tri-Color GC ========== */

static Strtable *Strtab = NULL;

/* Strnode pool allocator — mirrors newhn/freehn pattern */
#define ALLOCSN 128
static Strnode *Free_sn = NULL;

static Strnode *
newsn(void)
{
	static Strnode *lastsn;
	static int used = ALLOCSN;
	Strnode *sn;

	if ( Free_sn != NULL ) {
		sn = Free_sn;
		Free_sn = Free_sn->next;
	} else {
		if ( used == ALLOCSN ) {
			used = 0;
			lastsn = (Strnode *)kmalloc(ALLOCSN * sizeof(Strnode), "newsn");
		}
		used++;
		sn = lastsn++;
	}
	sn->next = NULL;
	sn->str = NULL;
	sn->gc_color = GC_WHITE;
	return sn;
}

static void
freesn(Strnode *sn)
{
	sn->str = NULL;
	sn->next = Free_sn;
	Free_sn = sn;
}

static Strtable *
new_strtable(int size)
{
	Strtable *st = (Strtable *)kmalloc(sizeof(Strtable), "new_strtable");
	st->size = size;
	st->count = 0;
	st->buckets = (Strnode **)kmalloc(size * sizeof(Strnode *), "strtable_buckets");
	memset(st->buckets, 0, size * sizeof(Strnode *));
	return st;
}

/* ---------- String GC state ---------- */

int Strgc_state = STRGC_IDLE;
static int Strgc_allocs = 0;
static int Strgc_threshold = 5000;
static int Strgc_work = 100;  /* items per step */
static int Strgc_cursor = 0;  /* bucket or root-group cursor */

/* Mark phase sub-cursors */
static int Strgc_root_group = 0;   /* which root group (0-13) */
static int Strgc_sub_cursor = 0;   /* position within current root group */
static Htablep Strgc_sub_ht = NULL; /* current hash table being scanned */
static int Strgc_sub_bucket = 0;    /* bucket within that hash table */

/* Gray list for nested arrays encountered during marking */
typedef struct GrayEntry {
	Htablep ht;
	int bucket;             /* resume position within this table */
	struct GrayEntry *next;
} GrayEntry;

static GrayEntry *Gray_list = NULL;
static GrayEntry *Free_gray = NULL;

static GrayEntry *
new_gray(Htablep ht)
{
	GrayEntry *g;
	if ( Free_gray != NULL ) {
		g = Free_gray;
		Free_gray = Free_gray->next;
	} else {
		g = (GrayEntry *)kmalloc(sizeof(GrayEntry), "new_gray");
	}
	g->ht = ht;
	g->bucket = 0;
	g->next = NULL;
	return g;
}

static void
free_gray(GrayEntry *g)
{
	g->next = Free_gray;
	Free_gray = g;
}

/* Forward declarations for GC internals */
static void strgc_mark_str(Symstr s);
static void strgc_mark_datum(Datum d);
static void strgc_mark_phrase(Phrasep ph);
static void strgc_scan_ht(Htablep ht, int *work_left);
static void strgc_mark_step(void);
static void strgc_sweep_step(void);
static void strgc_reset_colors(void);

/* Mark a single interned string as BLACK (reachable). */
/* Since strings have no outgoing references, they go directly to BLACK. */
static void
strgc_mark_str(Symstr s)
{
	Strnode *sn;
	int v;
	register unsigned int t = 0;
	register int c;
	register char *p;

	if ( s == NULL || Strtab == NULL )
		return;

	/* Hash to find the right bucket */
	p = s;
	while ( (c=(*p++)) != '\0' ) {
		t += c;
		t <<= 3;
	}
	v = t % (Strtab->size);

	/* Search chain for the exact pointer */
	for ( sn = Strtab->buckets[v]; sn != NULL; sn = sn->next ) {
		if ( sn->str == s ) {
			if ( sn->gc_color == GC_WHITE )
				sn->gc_color = GC_BLACK;
			return;
		}
	}
}

/* Mark a Datum's string references. Pushes arrays onto gray list. */
static void
strgc_mark_datum(Datum d)
{
	switch ( d.type ) {
	case D_STR:
		strgc_mark_str(d.u.str);
		break;
	case D_ARR:
		if ( d.u.arr != NULL ) {
			/* Push onto gray list for incremental scanning */
			GrayEntry *g = new_gray(d.u.arr);
			g->next = Gray_list;
			Gray_list = g;
		}
		break;
	case D_PHR:
		if ( d.u.phr != NULL )
			strgc_mark_phrase(d.u.phr);
		break;
	/* D_NUM, D_DBL, D_SYM, D_CODEP, etc.: no string references */
	default:
		break;
	}
}

/* Mark strings reachable from a phrase's note attrib fields */
static void
strgc_mark_phrase(Phrasep ph)
{
#ifdef NTATTRIB
	Noteptr n;
	if ( ph == NULL )
		return;
	for ( n = firstnote(ph); n != NULL; n = nextnote(n) ) {
		if ( attribof(n) != NULL && attribof(n) != Nullstr )
			strgc_mark_str(attribof(n));
	}
#endif
}

/* Scan up to *work_left Hnodes from a hash table, marking Datums. */
/* Resumes from Strgc_sub_bucket. Returns when work exhausted or table done. */
static void
strgc_scan_ht(Htablep ht, int *work_left)
{
	Hnodep h;
	int i;

	if ( ht == NULL || ht->nodetable == NULL )
		return;

	for ( i = Strgc_sub_bucket; i < ht->size && *work_left > 0; i++ ) {
		for ( h = ht->nodetable[i]; h != NULL && *work_left > 0; h = h->next ) {
			strgc_mark_datum(h->key);
			strgc_mark_datum(h->val);
			(*work_left)--;
		}
	}
	Strgc_sub_bucket = i;
}

/* Drain gray list entries, scanning up to work_left Hnodes total. */
/* Returns remaining work budget. */
static int
strgc_drain_gray(int work_left)
{
	while ( Gray_list != NULL && work_left > 0 ) {
		GrayEntry *g = Gray_list;
		Strgc_sub_bucket = g->bucket;
		strgc_scan_ht(g->ht, &work_left);
		if ( Strgc_sub_bucket >= g->ht->size ) {
			/* This table is fully scanned */
			Gray_list = g->next;
			free_gray(g);
		} else {
			/* Save resume position */
			g->bucket = Strgc_sub_bucket;
			break;  /* work exhausted */
		}
	}
	return work_left;
}

/* ---------- Mark step: walk root groups incrementally ---------- */

/* External declarations needed for root scanning */
extern Htablep Keywords, Macros, Topht;
extern Context *Topct, *Currct;
extern Phrasep Tobechecked;
extern Htablep Htobechecked;
extern Kobjectp Topobj;
extern Htablep Tasktable;
extern Midiport Midiinputs[];
extern Midiport Midioutputs[];

/* These are declared in other files; we declare them here for GC access */
extern Fifo *Topfifo;
extern Htablep Fifotable;
extern Lknode *Toplk;

/* Str_* Datum globals — declared earlier in this file */
extern Datum Str_x0, Str_y0, Str_x1, Str_y1, Str_x, Str_y, Str_button;
extern Datum Str_type, Str_mouse, Str_drag, Str_move, Str_up, Str_down;
extern Datum Str_highest, Str_lowest, Str_earliest, Str_latest, Str_modifier;
extern Datum Str_default, Str_w, Str_r, Str_init;
extern Datum Str_get, Str_set, Str_newline;
extern Datum Str_red, Str_green, Str_blue, Str_grey, Str_surface;
extern Datum Str_finger, Str_hand, Str_xvel, Str_yvel;
extern Datum Str_proximity, Str_orientation, Str_eccentricity;
extern Datum Str_width, Str_height;

/* Mark built-in Str_* Datum globals (root group 0) */
static void
strgc_mark_builtins(void)
{
	strgc_mark_datum(Str_x0); strgc_mark_datum(Str_y0);
	strgc_mark_datum(Str_x1); strgc_mark_datum(Str_y1);
	strgc_mark_datum(Str_x); strgc_mark_datum(Str_y);
	strgc_mark_datum(Str_button);
	strgc_mark_datum(Str_type); strgc_mark_datum(Str_mouse);
	strgc_mark_datum(Str_drag); strgc_mark_datum(Str_move);
	strgc_mark_datum(Str_up); strgc_mark_datum(Str_down);
	strgc_mark_datum(Str_highest); strgc_mark_datum(Str_lowest);
	strgc_mark_datum(Str_earliest); strgc_mark_datum(Str_latest);
	strgc_mark_datum(Str_modifier);
	strgc_mark_datum(Str_default); strgc_mark_datum(Str_w);
	strgc_mark_datum(Str_r); strgc_mark_datum(Str_init);
	strgc_mark_datum(Str_get); strgc_mark_datum(Str_set);
	strgc_mark_datum(Str_newline);
	strgc_mark_datum(Str_red); strgc_mark_datum(Str_green);
	strgc_mark_datum(Str_blue); strgc_mark_datum(Str_grey);
	strgc_mark_datum(Str_surface);
	strgc_mark_datum(Str_finger); strgc_mark_datum(Str_hand);
	strgc_mark_datum(Str_xvel); strgc_mark_datum(Str_yvel);
	strgc_mark_datum(Str_proximity); strgc_mark_datum(Str_orientation);
	strgc_mark_datum(Str_eccentricity);
	strgc_mark_datum(Str_width); strgc_mark_datum(Str_height);
	/* Also mark Nullstr */
	strgc_mark_str(Nullstr);
}

/* Mark all tasks' stacks and string fields (root group 4) */
static void
strgc_mark_tasks(int *work_left)
{
	extern Ktaskp Toptp;
	Ktaskp tp;
	for ( tp = Toptp; tp != NULL && *work_left > 0; tp = tp->nxt ) {
		Datum *dp;
		/* Mark task stack */
		if ( tp->stack != NULL && tp->stackp != NULL ) {
			for ( dp = tp->stack; dp < tp->stackp && *work_left > 0; dp++ ) {
				strgc_mark_datum(*dp);
				(*work_left)--;
			}
		}
		/* Mark string fields */
		if ( tp->filename != NULL )
			strgc_mark_str(tp->filename);
		if ( tp->ontaskerrormsg != NULL )
			strgc_mark_str(tp->ontaskerrormsg);
	}
}

/* Mark phrases in Topph and Tobechecked lists (root group 5) */
static void
strgc_mark_phrases(int *work_left)
{
	Phrasep ph;
	for ( ph = Topph; ph != NULL && *work_left > 0; ph = ph->p_next ) {
		strgc_mark_phrase(ph);
		(*work_left)--;
	}
	for ( ph = Tobechecked; ph != NULL && *work_left > 0; ph = ph->p_next ) {
		strgc_mark_phrase(ph);
		(*work_left)--;
	}
}

/* Mark objects' symbol tables (root group 6) */
static void
strgc_mark_objects(int *work_left)
{
	Kobjectp obj;
	for ( obj = Topobj; obj != NULL && *work_left > 0; obj = obj->onext ) {
		if ( obj->symbols != NULL ) {
			GrayEntry *g = new_gray(obj->symbols);
			g->next = Gray_list;
			Gray_list = g;
		}
		(*work_left)--;
	}
}

/* Mark lock name fields (root group 7) */
static void
strgc_mark_locks(void)
{
	Lknode *lk;
	for ( lk = Toplk; lk != NULL; lk = lk->next ) {
		if ( lk->name != NULL )
			strgc_mark_str(lk->name);
	}
}

/* Mark fifo data chains (root group 8) */
static void
strgc_mark_fifos(int *work_left)
{
	Fifo *ff;
	for ( ff = Topfifo; ff != NULL && *work_left > 0; ff = ff->next ) {
		Fifodata *fd;
		for ( fd = ff->head; fd != NULL && *work_left > 0; fd = fd->next ) {
			strgc_mark_datum(fd->d);
			(*work_left)--;
		}
	}
}

/* Mark context chain symbol tables (root group 9) */
static void
strgc_mark_contexts(int *work_left)
{
	Context *ct;
	for ( ct = Topct; ct != NULL && *work_left > 0; ct = ct->next ) {
		if ( ct->symbols != NULL ) {
			GrayEntry *g = new_gray(ct->symbols);
			g->next = Gray_list;
			Gray_list = g;
		}
		(*work_left)--;
	}
}

/* Mark MIDI port name fields (root group 11) */
static void
strgc_mark_midiports(void)
{
	int i;
	for ( i = 0; i < MIDI_IN_DEVICES; i++ ) {
		if ( Midiinputs[i].name != NULL )
			strgc_mark_str(Midiinputs[i].name);
	}
	for ( i = 0; i < MIDI_OUT_DEVICES; i++ ) {
		if ( Midioutputs[i].name != NULL )
			strgc_mark_str(Midioutputs[i].name);
	}
}

/* Incremental mark step — walks root groups and gray list */
static void
strgc_mark_step(void)
{
	int work_left = Strgc_work;

	/* First drain gray list (nested arrays discovered during marking) */
	work_left = strgc_drain_gray(work_left);
	if ( work_left <= 0 )
		return;

	/* Walk root groups */
	while ( Strgc_root_group <= 13 && work_left > 0 ) {
		switch ( Strgc_root_group ) {
		case 0: /* Built-in Str_* Datums + Nullstr */
			strgc_mark_builtins();
			Strgc_root_group++;
			work_left -= 30;
			break;

		case 1: /* Keywords hash table */
			if ( Keywords != NULL ) {
				Strgc_sub_bucket = Strgc_sub_cursor;
				strgc_scan_ht(Keywords, &work_left);
				if ( Strgc_sub_bucket >= Keywords->size ) {
					Strgc_sub_cursor = 0;
					Strgc_root_group++;
				} else {
					Strgc_sub_cursor = Strgc_sub_bucket;
				}
			} else {
				Strgc_root_group++;
			}
			break;

		case 2: /* Macros hash table */
			if ( Macros != NULL ) {
				Strgc_sub_bucket = Strgc_sub_cursor;
				strgc_scan_ht(Macros, &work_left);
				if ( Strgc_sub_bucket >= Macros->size ) {
					Strgc_sub_cursor = 0;
					Strgc_root_group++;
				} else {
					Strgc_sub_cursor = Strgc_sub_bucket;
				}
			} else {
				Strgc_root_group++;
			}
			break;

		case 3: /* Topht — all active hash tables */
			{
				Htablep ht;
				if ( Strgc_sub_ht == NULL )
					Strgc_sub_ht = Topht;
				while ( Strgc_sub_ht != NULL && work_left > 0 ) {
					Strgc_sub_bucket = Strgc_sub_cursor;
					strgc_scan_ht(Strgc_sub_ht, &work_left);
					if ( Strgc_sub_bucket >= Strgc_sub_ht->size ) {
						Strgc_sub_ht = Strgc_sub_ht->h_next;
						Strgc_sub_cursor = 0;
					} else {
						Strgc_sub_cursor = Strgc_sub_bucket;
						break;
					}
				}
				if ( Strgc_sub_ht == NULL ) {
					Strgc_sub_cursor = 0;
					Strgc_root_group++;
				}
			}
			break;

		case 4: /* Task stacks */
			strgc_mark_tasks(&work_left);
			Strgc_root_group++;
			break;

		case 5: /* Topph + Tobechecked phrases */
			strgc_mark_phrases(&work_left);
			Strgc_root_group++;
			break;

		case 6: /* Topobj object symbol tables */
			strgc_mark_objects(&work_left);
			Strgc_root_group++;
			break;

		case 7: /* Lock name fields */
			strgc_mark_locks();
			Strgc_root_group++;
			work_left--;
			break;

		case 8: /* Fifo data chains */
			strgc_mark_fifos(&work_left);
			Strgc_root_group++;
			break;

		case 9: /* Context chain */
			strgc_mark_contexts(&work_left);
			Strgc_root_group++;
			break;

		case 10: /* Special tables: Windhash, Tasktable, Fifotable */
			{
				extern Htablep Windhash;
				Htablep tables[3];
				int ntables = 0, ti;
				if ( Windhash != NULL ) tables[ntables++] = Windhash;
				if ( Tasktable != NULL ) tables[ntables++] = Tasktable;
				if ( Fifotable != NULL ) tables[ntables++] = Fifotable;
				for ( ti = 0; ti < ntables; ti++ ) {
					GrayEntry *g = new_gray(tables[ti]);
					g->next = Gray_list;
					Gray_list = g;
				}
				Strgc_root_group++;
			}
			break;

		case 11: /* MIDI port names */
			strgc_mark_midiports();
			Strgc_root_group++;
			work_left--;
			break;

		case 12: /* Htobechecked list */
			{
				Htablep ht;
				for ( ht = Htobechecked; ht != NULL && work_left > 0; ht = ht->h_next ) {
					GrayEntry *g = new_gray(ht);
					g->next = Gray_list;
					Gray_list = g;
					work_left--;
				}
				Strgc_root_group++;
			}
			break;

		case 13: /* Done — drain any remaining gray list */
			work_left = strgc_drain_gray(work_left);
			if ( Gray_list == NULL ) {
				/* All roots scanned, gray list empty — move to sweep */
				Strgc_state = STRGC_SWEEP;
				Strgc_cursor = 0;
				return;
			}
			/* Gray list not yet empty, stay in group 13 */
			return;
		}

		/* After each group, drain gray list */
		work_left = strgc_drain_gray(work_left);
	}

	/* If we've exhausted all groups and gray list is empty */
	if ( Strgc_root_group > 13 && Gray_list == NULL ) {
		Strgc_state = STRGC_SWEEP;
		Strgc_cursor = 0;
	}
}

/* Incremental reset: set all Strnodes to WHITE */
static void
strgc_reset_colors(void)
{
	int work_left = Strgc_work;
	int i;
	Strnode *sn;

	for ( i = Strgc_cursor; i < Strtab->size && work_left > 0; i++ ) {
		for ( sn = Strtab->buckets[i]; sn != NULL; sn = sn->next ) {
			sn->gc_color = GC_WHITE;
			work_left--;
		}
	}
	Strgc_cursor = i;
	if ( i >= Strtab->size ) {
		/* Reset complete, transition to MARK */
		Strgc_state = STRGC_MARK;
		Strgc_root_group = 0;
		Strgc_sub_cursor = 0;
		Strgc_sub_ht = NULL;
		Strgc_sub_bucket = 0;
		Gray_list = NULL;
	}
}

/* Incremental sweep: free WHITE strings, reset BLACK to WHITE */
static void
strgc_sweep_step(void)
{
	int work_left = Strgc_work;
	int i;
	Strnode *sn, *prev, *next;
	int freed = 0;

	for ( i = Strgc_cursor; i < Strtab->size && work_left > 0; i++ ) {
		prev = NULL;
		for ( sn = Strtab->buckets[i]; sn != NULL; sn = next ) {
			next = sn->next;
			if ( sn->gc_color == GC_WHITE ) {
				/* Unreachable — free it */
				if ( prev == NULL )
					Strtab->buckets[i] = next;
				else
					prev->next = next;
				if ( sn->str != NULL )
					kfree(sn->str);
				freesn(sn);
				Strtab->count--;
				freed++;
			} else {
				/* Survived — reset to WHITE for next cycle */
				sn->gc_color = GC_WHITE;
				prev = sn;
			}
			work_left--;
		}
	}
	Strgc_cursor = i;

	if ( i >= Strtab->size ) {
		/* Sweep complete */
		if ( *Debug > 1 )
			eprint("strgc: sweep done, freed %d strings, %d remain\n",
				freed, Strtab->count);
		Strgc_state = STRGC_IDLE;
		Strgc_allocs = 0;
	}
}

/* Main incremental GC driver — call from interpreter loop */
void
strgc_step(void)
{
	if ( Strtab == NULL )
		return;

	switch ( Strgc_state ) {
	case STRGC_IDLE:
		if ( Strgc_allocs >= Strgc_threshold ) {
			if ( *Debug > 1 )
				eprint("strgc: starting cycle, %d allocs, %d strings\n",
					Strgc_allocs, Strtab->count);
			Strgc_state = STRGC_MARK_INIT;
			Strgc_cursor = 0;
		}
		break;
	case STRGC_MARK_INIT:
		strgc_reset_colors();
		break;
	case STRGC_MARK:
		strgc_mark_step();
		break;
	case STRGC_SWEEP:
		strgc_sweep_step();
		break;
	}
}

/* Run a complete GC cycle (non-incremental) — used by garbcollect() */
void
strgc_full(void)
{
	if ( Strtab == NULL )
		return;

	/* Force start if idle */
	if ( Strgc_state == STRGC_IDLE ) {
		Strgc_state = STRGC_MARK_INIT;
		Strgc_cursor = 0;
	}

	/* Run to completion */
	while ( Strgc_state != STRGC_IDLE ) {
		/* Use a large work budget per step for full collection */
		int save_work = Strgc_work;
		Strgc_work = 100000;
		strgc_step();
		Strgc_work = save_work;
	}
}

/* uniqstr: intern a string, returning a unique pointer for each unique value.
 * Now uses Strnode/Strtable instead of Hnode/Htable, supporting GC. */

Symstr
uniqstr(char *s)
{
	Strnode **buckets;
	Strnode *sn, *topsn;
	int v;
	int is_new = 0;

	if ( s == NULL ) {
		eprint("uniqstr: NULL string passed!\n");
		s = "";
	}

	if ( Strtab == NULL ) {
		char *p = getenv("STRHASHSIZE");
		char *g, *w;
		Strtab = new_strtable( p ? atoi(p) : 1009 );
		g = getenv("STRGCTHRESHOLD");
		if ( g ) Strgc_threshold = atoi(g);
		w = getenv("STRGCWORK");
		if ( w ) Strgc_work = atoi(w);
	}

	{
		register unsigned int t = 0;
		register int c;
		register char *p = s;

		/* compute hash value of string */
		while ( (c=(*p++)) != '\0' ) {
			t += c;
			t <<= 3;
		}
		v = t % (Strtab->size);
	}

	buckets = Strtab->buckets;
	topsn = buckets[v];
	if ( topsn == NULL ) {
		/* no collision */
		sn = newsn();
		sn->str = kmalloc((unsigned)strlen(s)+1,"uniqstr");
		strcpy((char*)(sn->str), s);
		is_new = 1;
	}
	else {
		Strnode *prev;

		/* quick test for first node in list, most common case */
		if ( strcmp(topsn->str, s) == 0 ) {
			/* Write barrier */
			if ( Strgc_state != STRGC_IDLE )
				topsn->gc_color = GC_BLACK;
			return(topsn->str);
		}

		/* Look through entire list */
		sn = topsn;
		for ( prev=sn; (sn=sn->next) != NULL; prev=sn ) {
			if ( strcmp(sn->str, s) == 0 )
				break;
		}
		if ( sn == NULL ) {
			/* string wasn't found, add it */
			sn = newsn();
			sn->str = kmalloc((unsigned)strlen(s)+1,"uniqstr");
			strcpy((char*)(sn->str), s);
			is_new = 1;
		}
		else {
			/* Symstr found.  Delete it from its current */
			/* position so we can move it to the top. */
			prev->next = sn->next;
		}
	}
	/* Whether we've just allocated a new node, or whether we've */
	/* found the node somewhere in the list, we insert it at the */
	/* top of the list.  Ie. the lists are constantly re-arranging */
	/* themselves to put the most recently seen entries on top. */
	sn->next = topsn;
	buckets[v] = sn;

	if ( is_new ) {
		Strtab->count++;
		Strgc_allocs++;
	}

	/* Write barrier: during active GC, mark any touched string BLACK */
	if ( Strgc_state != STRGC_IDLE )
		sn->gc_color = GC_BLACK;

	return(sn->str);
}

int
isundefd(Symbolp s)
{
	Datum d;

	if ( s->stype == UNDEF )
		return(1);
	d = *symdataptr(s);
	if ( isnoval(d) )
		return(1);
	else
		return(0);
}

/*
 * Look for an element in the hash table.
 * Values of 'action':
 *     H_INSERT ==> look for, and if not found, insert
 *     H_LOOK ==> look for, but don't insert
 *     H_DELETE ==> look for and delete
 */

Hnodep
hashtable(Htablep ht,Datum key,int action)
{
	Hnodepp table;
	Hnodep h, toph, prev;
	int v, nc;

	table = ht->nodetable;

	/* base the hash value on the 'uniqstr'ed pointer */
	switch ( key.type ) {
	case D_NUM:
		v = ((unsigned int)(key.u.val)) % (ht->size);
		break;
	case D_STR:
		v = ((intptr_t)(key.u.str)>>2) % (ht->size);
		break;
	case D_OBJ:
		v = ((unsigned int)(key.u.obj->id)>>2) % (ht->size);
		break;
	default:
		execerror("hashtable isn't prepared for that key.type");
		break;
	}

	/* look in hash table of existing elements */
	toph = table[v];
	if ( toph != NULL ) {

		/* collision */

		/* quick test for first node in list, most common case */
		if ( dcompare(key,toph->key) == 0 ) {
			if ( action != H_DELETE )
				return(toph);
			/* delete from list and free */
			table[v] = toph->next;
			freehn(toph);
			ht->count--;
			return(NULL);
		}

		/* Look through entire list */
		h = toph;
		nc = 0;
		for ( prev=h; ((h=h->next) != NULL); prev=h ) {
			nc++;
			if ( dcompare(key,h->key) == 0 ) {
				break;
			}
		}
		if ( h != NULL ) {
			/* Found.  Delete it from it's current */
			/* position so we can either move it to the top, */
			/* or leave it deleted. */
			prev->next = h->next;
			if ( action == H_DELETE ) {
				/* delete it */
				freehn(h);
				ht->count--;
				return(NULL);
			}
			/* move it to the top of the collision list */
			h->next = toph;
			table[v] = h;
			return(h);
		}
	}

	/* it wasn't found */
	if ( action == H_DELETE ) {
		return(NULL);
	}

	if ( action == H_LOOK )
		return(NULL);

	h = newhn();
	h->key = key;
	h->val = Noval;
	ht->count++;

	/* Add to top of collision list */
	h->next = toph;
	table[v] = h;

	return(h);
}

/*
 * Look for the symbol for a particular array element, given
 * a pointer to the main array symbol, and the subscript value.
 * Values of 'action':
 *     H_INSERT ==> look for symbol, and if not found, insert
 *     H_LOOK ==> look for symbol, don't insert
 *     H_DELETE ==> look for symbol and delete it
 */

Symbolp
arraysym(Htablep arr,Datum subs,int action)
{
	Symbolp s = NULL;
	Symbolp ns;
	Hnodep h;
	Datum key;

	if ( arr == NULL )
		execerror("Internal error: arr==0 in arraysym!?");

	key = dtoindex(subs);

	switch (action) {
	case H_LOOK:
		h = hashtable(arr,key,action);
		if ( h )
			s = h->val.u.sym;
		break;
	case H_INSERT:
		h = hashtable(arr,key,action);
		if ( isnoval(h->val) ) {
			/* New element, initialized to null string */
			ns = newsy();
			ns->name = key;
			ns->stype = VAR;
			*symdataptr(ns) = strdatum(Nullstr);
			h->val = symdatum(ns);
		}
		s = h->val.u.sym;
		break;
	case H_DELETE:
		(void) hashtable(arr,key,action);
		break;
	default:
		execerror("Internal error: bad action in arraysym!?");
	}
	return(s);
}

int
arrsize(Htablep arr)
{
	return arr->count;
}

int
dtcmp(Datum *d1,Datum *d2)
{
	return dcompare(*d1,*d2);
}

static int elsize;	/* element size */
static INTFUNC2P qscompare;

/*
 * Quick Sort routine.
 * Code by Duane Morse (...!noao!terak!anasazi!duane)
 * Based on Knuth's ART OF COMPUTER PROGRAMMING, VOL III, pp 114-117.
 */

/* Exchange the contents of two vectors.  n is the size of vectors in bytes. */
static void
memexch(register unsigned char *s1,register unsigned char *s2,register int n)
{
	register unsigned char c;
	while (n--) {
		c = *s1;
		*s1++ = *s2;
		*s2++ = c;
	}
}

static void
mysort(unsigned char *vec,int nel)
{
	register short i, j;
	register unsigned char *iptr, *jptr, *kptr;

begin:
	if (nel == 2) {	/* If 2 items, check them by hand. */
		if ((*qscompare)(vec, vec + elsize) > 0)
			memexch(vec, vec + elsize, elsize);
		return;
	}
	j = (short) nel;
	i = 0;
	kptr = vec;
	iptr = vec;
	jptr = vec + elsize * nel;
	while (--j > i) {

		/* From the righthand side, find first value */
		/* that should be to the left of k. */
		jptr -= elsize;
		if ((*qscompare)(jptr, kptr) > 0)
			continue;

		/* Now from the lefthand side, find first value */
		/* that should be to right of k. */

		iptr += elsize;
		while(++i < j && (*qscompare)(iptr, kptr) <= 0)
			iptr += elsize;

		if (i >= j)
			break;

		/* Exchange the two items; k will eventually end up between them. */
		memexch(jptr, iptr, elsize);
	}
	/* Move item 0 into position.  */
	memexch(vec, iptr, elsize);
	/* Now sort the two partitions. */
	if ((nel -= (i + 1)) > 1)
		mysort(iptr + elsize, nel);

	/* To save a little time, just start the routine over by hand. */
	if (i > 1) {
		nel = i;
		goto begin;
	}
}

static void
pqsort(unsigned char *vec,int nel,int esize,INTFUNC2P compptr)
{
	if (nel < 2)
		return;
	elsize = esize;
	qscompare = compptr;
	mysort(vec, nel);
}

/* Return a Noval-terminated list of the index values of an array.  */
Datum *
arrlist(Htablep arr,int *asize,int sortit)
{
	register Hnodepp pp;
	register Hnodep h;
	register Datum *lp;
	register int hsize;
	Datum *list;

	pp = arr->nodetable;
	hsize = arr->size;
	*asize = arrsize(arr);
	list = (Datum *) kmalloc((*asize+1)*sizeof(Datum),"arrlist");

	lp = list;
	/* visit each slot in the hash table */
	while ( hsize-- > 0 ) {
		/* and traverse its list */
		for ( h=(*pp++); h!=NULL; h=h->next ) {
			*lp++ = h->val.u.sym->name;
		}
	}
	*lp++ = Noval;
	if ( sortit )
		pqsort((unsigned char *)list,*asize,(int)sizeof(Datum),(INTFUNC2P)dtcmp);
	return(list);
}

void
hashvisit(Htablep arr,HNODEFUNC f)
{
	register Hnodepp pp;
	register Hnodep h;
	register int hsize;

	pp = arr->nodetable;
	hsize = arr->size;
	/* visit each slot in the hash table */
	while ( hsize-- > 0 ) {
		/* and traverse its list */
		for ( h=(*pp++); h!=NULL; h=h->next ) {
			if ( (*f)(h) )
				return;	/* used to be break, apparent mistake */
		}
	}
}

