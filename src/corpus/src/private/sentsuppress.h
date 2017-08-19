/* This file is automatically generated. DO NOT EDIT!
   Instead, edit gen-sentsuppress.py and re-run.  */

#ifndef SENTSUPPRESS_H
#define SENTSUPPRESS_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

struct sentsuppress_list {
	const char *name;
	int offset;
	int length;
};

static const char *sentsuppress_list_names[] = {
	"english",
	"french",
	"german",
	"italian",
	"portuguese",
	"russian",
	"spanish",
	NULL
};

static struct sentsuppress_list sentsuppress_lists[] = {
	{"english", 0, 146},
	{"french", 147, 82},
	{"german", 230, 241},
	{"italian", 472, 45},
	{"portuguese", 518, 172},
	{"russian", 691, 18},
	{"spanish", 710, 164},
	{NULL, 0, 0}
};

static const char * sentsuppress_strings[] = {
	/* english */
	"A.",
	"A.D.",
	"A.M.",
	"A.S.",
	"AA.",
	"AB.",
	"AD.",
	"Abs.",
	"Adj.",
	"Adv.",
	"Alt.",
	"Approx.",
	"Apr.",
	"Aug.",
	"B.",
	"B.V.",
	"C.F.",
	"C.O.D.",
	"Capt.",
	"Card.",
	"Col.",
	"Comm.",
	"Conn.",
	"Cont.",
	"D.",
	"D.A.",
	"D.C.",
	"DC.",
	"Dec.",
	"Def.",
	"Dept.",
	"Diff.",
	"Dr.",
	"E.",
	"E.G.",
	"E.g.",
	"Ed.",
	"Est.",
	"Etc.",
	"Ex.",
	"Exec.",
	"F.",
	"Feb.",
	"Fn.",
	"Fri.",
	"G.",
	"Gb.",
	"Hon.B.A.",
	"Hz.",
	"I.",
	"I.D.",
	"I.T.",
	"I.e.",
	"Id.",
	"In.",
	"J.B.",
	"J.D.",
	"J.K.",
	"Jam.",
	"Jan.",
	"Job.",
	"Joe.",
	"Jr.",
	"Jul.",
	"Jun.",
	"K.",
	"K.R.",
	"Kb.",
	"L.",
	"L.A.",
	"L.P.",
	"Lev.",
	"Lib.",
	"Lt.",
	"Lt.Cdr.",
	"M.",
	"M.I.T.",
	"M.R.",
	"M.T.",
	"MM.",
	"MR.",
	"Maj.",
	"Mar.",
	"Mart.",
	"Mb.",
	"Md.",
	"Mgr.",
	"Min.",
	"Misc.",
	"Mr.",
	"Mrs.",
	"Ms.",
	"Mt.",
	"Mx.",
	"N.V.",
	"N.Y.",
	"Nov.",
	"Nr.",
	"Num.",
	"O.",
	"Oct.",
	"Op.",
	"Org.",
	"P.M.",
	"P.O.",
	"P.V.",
	"PC.",
	"PP.",
	"Ph.D.",
	"Phys.",
	"Prof.",
	"Pvt.",
	"Q.",
	"R.L.",
	"R.T.",
	"Rep.",
	"Rev.",
	"S.",
	"S.A.",
	"S.A.R.",
	"S.E.",
	"S.p.A.",
	"Sep.",
	"Sept.",
	"Sgt.",
	"Sq.",
	"St.",
	"T.",
	"U.",
	"U.S.",
	"U.S.A.",
	"U.S.C.",
	"VS.",
	"Var.",
	"X.",
	"Yr.",
	"Z.",
	"a.m.",
	"cf.",
	"e.g.",
	"etc.",
	"exec.",
	"i.e.",
	"p.m.",
	"pp.",
	"vs.",
	NULL,

	/* french */
	"All.",
	"C.",
	"Comm.",
	"D.",
	"DC.",
	"Desc.",
	"Inc.",
	"Jr.",
	"L.",
	"M.",
	"MM.",
	"Mart.",
	"Op.",
	"P.",
	"P.-D. G.",
	"P.O.",
	"Prof.",
	"S.A.",
	"S.M.A.R.T.",
	"U.",
	"U.S.",
	"U.S.A.",
	"Var.",
	"W.",
	"acoust.",
	"adr.",
	"anc.",
	"ann.",
	"anon.",
	"ap. J.-C.",
	"append.",
	"aux.",
	"av. J.-C.",
	"avr.",
	"broch.",
	"bull.",
	"cam.",
	"categ.",
	"coll.",
	"collab.",
	"config.",
	"dest.",
	"dict.",
	"dim.",
	"dir.",
	"doc.",
	"d\303\251c.",
	"encycl.",
	"exempl.",
	"fig.",
	"f\303\251vr.",
	"gouv.",
	"graph.",
	"h\303\264p.",
	"ill.",
	"illustr.",
	"imm.",
	"imprim.",
	"indus.",
	"janv.",
	"jeu.",
	"juil.",
	"lun.",
	"mar.",
	"mer.",
	"niv.",
	"nov.",
	"oct.",
	"quart.",
	"r\303\251f.",
	"sam.",
	"sept.",
	"symb.",
	"synth.",
	"syst.",
	"trav. publ.",
	"ven.",
	"voit.",
	"\303\251d.",
	"\303\251dit.",
	"\303\251quiv.",
	"\303\251val.",
	NULL,

	/* german */
	"A.",
	"A.M.",
	"Abs.",
	"Abt.",
	"Abw.",
	"Adj.",
	"Adr.",
	"Akt.",
	"Allg.",
	"Alt.",
	"App.",
	"Apr.",
	"Art.",
	"Aug.",
	"Ausg.",
	"Ausschl.",
	"B.",
	"Bed.",
	"Ben.",
	"Ber.",
	"Best.",
	"Bibl.",
	"C.",
	"Ca.",
	"Chin.",
	"Chr.",
	"Co.",
	"D.",
	"D. h.",
	"Dat.",
	"Dez.",
	"Di.",
	"Dim.",
	"Dipl.-Ing.",
	"Dipl.-Kfm.",
	"Dir.",
	"Do.",
	"Dr.",
	"Dtzd.",
	"Einh.",
	"Erf.",
	"Evtl.",
	"F.",
	"F.f.",
	"Fa.",
	"Fam.",
	"Feb.",
	"Fn.",
	"Folg.",
	"Forts. f.",
	"Fr.",
	"Frl.",
	"G.",
	"Gebr.",
	"Gem.",
	"Geograph.",
	"Ges.",
	"Gesch.",
	"Ggf.",
	"Hbf.",
	"Hptst.",
	"Hr.",
	"Hrn.",
	"Hrsg.",
	"I.",
	"Inc.",
	"Ing.",
	"Inh.",
	"Int.",
	"J.",
	"J.D.",
	"Jahrh.",
	"Jan.",
	"Jr.",
	"Kap.",
	"Kfm.",
	"Kl.",
	"Konv.",
	"Kop.",
	"L.",
	"Ltd.",
	"M.",
	"Max.",
	"Mi.",
	"Min.",
	"Mind.",
	"Mio.",
	"Mo.",
	"Mod.",
	"Mrd.",
	"Msp.",
	"N.",
	"Nov.",
	"Nr.",
	"O.",
	"Obj.",
	"Okt.",
	"Op.",
	"P.",
	"P.M.",
	"PIN.",
	"Pfd.",
	"Phys.",
	"Port.",
	"Prot.",
	"Proz.",
	"Qu.",
	"R.",
	"Rd.",
	"Reg.",
	"Reg.-Bez.",
	"Rel.",
	"Rep.",
	"S.A.",
	"Sa.",
	"Schr.",
	"Sek.",
	"Sep.",
	"Sept.",
	"So.",
	"Spezif.",
	"St.",
	"StR.",
	"Std.",
	"Str.",
	"T.",
	"Tel.",
	"Temp.",
	"Test.",
	"Trans.",
	"T\303\244gl.",
	"U.",
	"U. U.",
	"U.S.",
	"U.S.A.",
	"U.U.",
	"Urspr.",
	"Urspr\303\274ngl.",
	"Verf.",
	"Vgl.",
	"W.",
	"Wg.",
	"Y.",
	"Z.",
	"Z. B.",
	"Z. Zt.",
	"Ztr.",
	"a.D.",
	"a.M.",
	"a.Rh.",
	"a.a.O.",
	"a.a.S.",
	"am.",
	"amtl.",
	"b.",
	"beil.",
	"d.J.",
	"d.\303\204.",
	"e.V.",
	"e.Wz.",
	"e.h.",
	"ehem.",
	"eigtl.",
	"einschl.",
	"entspr.",
	"erw.",
	"ev.",
	"evtl.",
	"exkl.",
	"frz.",
	"geb.",
	"gedr.",
	"gek.",
	"gesch.",
	"gest.",
	"ggf.",
	"ggfs.",
	"hpts.",
	"i.A.",
	"i.B.",
	"i.H.",
	"i.J.",
	"i.R.",
	"i.V.",
	"inkl.",
	"jew.",
	"jhrl.",
	"k. u. k.",
	"k.u.k.",
	"kath.",
	"kfm.",
	"kgl.",
	"led.",
	"m.E.",
	"m.W.",
	"mtl.",
	"m\303\266bl.",
	"n. Chr.",
	"n.u.Z.",
	"n\303\244ml.",
	"o.A.",
	"o.B.",
	"o.g.",
	"od.",
	"p.Adr.",
	"r.",
	"r\303\266m.",
	"r\303\266m.-kath.",
	"s.",
	"s.a.",
	"schles.",
	"schweiz.",
	"schw\303\244b.",
	"sog.",
	"s\303\274dd.",
	"t\303\244gl.",
	"u.",
	"u. Z.",
	"u.A.w.g.",
	"u.U.",
	"u.a.",
	"u.v.a.",
	"u.\303\204.",
	"u.\303\244.",
	"v. Chr.",
	"v. H.",
	"v. u. Z.",
	"v.Chr.",
	"v.H.",
	"v.R.w.",
	"v.T.",
	"v.u.Z.",
	"verh.",
	"verw.",
	"vgl.",
	"z.",
	"z.B.",
	"z.Hd.",
	"z.Z.",
	"zzgl.",
	"\303\266sterr.",
	NULL,

	/* italian */
	"C.P.",
	"Cfr.",
	"D.",
	"DC.",
	"Geom.",
	"Ing.",
	"L.",
	"Liv.",
	"Ltd.",
	"Mod.",
	"N.B.",
	"N.d.A.",
	"N.d.E.",
	"N.d.T.",
	"O.d.G.",
	"S.A.R.",
	"S.M.A.R.T.",
	"S.p.A.",
	"Sig.",
	"U.S.",
	"U.S.A.",
	"a.C.",
	"ag.",
	"all.",
	"arch.",
	"avv.",
	"c.c.p.",
	"d.C.",
	"d.p.R.",
	"div.",
	"dott.",
	"dr.",
	"fig.",
	"int.",
	"mitt.",
	"on.",
	"p.",
	"p.i.",
	"pag.",
	"rag.",
	"sez.",
	"tab.",
	"tav.",
	"ver.",
	"vol.",
	NULL,

	/* portuguese */
	"A.C.",
	"A.M",
	"Alm.",
	"Av.",
	"D.C",
	"Dir.",
	"Dr.",
	"Dra.",
	"Dras.",
	"Drs.",
	"E.",
	"Est.",
	"Exma.",
	"Exmo.",
	"Fr.",
	"Ilma.",
	"Ilmo.",
	"Jr.",
	"Ltd.",
	"Ltda.",
	"Mar.",
	"N.Sra.",
	"N.T.",
	"P.M.",
	"Pe.",
	"Ph.D.",
	"R.",
	"S.",
	"S.A.",
	"Sta.",
	"Sto.",
	"V.T.",
	"W.C.",
	"a.C.",
	"a.m.",
	"abr.",
	"abrev.",
	"adm.",
	"aer.",
	"ago.",
	"agric.",
	"anat.",
	"ap.",
	"apart.",
	"apt.",
	"arit.",
	"arqueol.",
	"arquit.",
	"astron.",
	"autom.",
	"aux.",
	"biogr.",
	"bras.",
	"cap.",
	"caps.",
	"cat.",
	"cel.",
	"cf.",
	"col.",
	"com.",
	"comp.",
	"compl.",
	"cont.",
	"contab.",
	"cr\303\251d.",
	"cx.",
	"c\303\255rc.",
	"c\303\263d.",
	"d.C.",
	"des.",
	"desc.",
	"dez.",
	"dipl.",
	"dir.",
	"div.",
	"doc.",
	"d\303\251b.",
	"ed.",
	"educ.",
	"elem.",
	"eletr.",
	"eletr\303\264n.",
	"end.",
	"eng.",
	"esp.",
	"ex.",
	"f.",
	"fac.",
	"fasc.",
	"fem.",
	"fev.",
	"ff.",
	"fig.",
	"fil.",
	"filos.",
	"fisiol.",
	"fl.",
	"fot.",
	"fr.",
	"f\303\255s.",
	"geom.",
	"gram.",
	"g\303\252n.",
	"hist.",
	"ind.",
	"ingl.",
	"jan.",
	"jul.",
	"jun.",
	"jur.",
	"l.",
	"lat.",
	"lin.",
	"lit.",
	"liter.",
	"long.",
	"mai.",
	"mar.",
	"mat.",
	"matem.",
	"mov.",
	"m\303\241q.",
	"m\303\251d.",
	"m\303\272s.",
	"neol.",
	"nov.",
	"n\303\241ut.",
	"obs.",
	"odont.",
	"odontol.",
	"org.",
	"organiz.",
	"out.",
	"p.",
	"p. ex.",
	"p.m.",
	"pal.",
	"pol.",
	"port.",
	"pp.",
	"pq.",
	"prod.",
	"prof.",
	"profa.",
	"pron.",
	"pr\303\263x.",
	"psicol.",
	"p\303\241g.",
	"qu\303\255m.",
	"r.s.v.p.",
	"ref.",
	"rel.",
	"relat.",
	"rementente",
	"rep.",
	"res.",
	"rod.",
	"set.",
	"sociol.",
	"sup.",
	"s\303\251c.",
	"s\303\255mb.",
	"tec.",
	"tecnol.",
	"tel.",
	"trad.",
	"transp.",
	"univ.",
	"vol.",
	"vs.",
	"\303\241lg.",
	"\303\255nd.",
	NULL,

	/* russian */
	"\320\260\320\262\320\263.",
	"\320\260\320\277\321\200.",
	"\320\264\320\265\320\272.",
	"\320\264\320\276 \320\275. \321\215.",
	"\320\272\320\262.",
	"\320\275. \321\215.",
	"\320\275.\321\215.",
	"\320\275\320\276\321\217\320\261.",
	"\320\276\320\272\321\202.",
	"\320\276\321\202\320\264.",
	"\320\277\321\200\320\276\321\204.",
	"\321\200\321\203\320\261.",
	"\321\201\320\265\320\275\321\202.",
	"\321\202\320\265\320\273.",
	"\321\202\321\213\321\201.",
	"\321\203\320\273.",
	"\321\204\320\265\320\262\321\200.",
	"\321\217\320\275\320\262.",
	NULL,

	/* spanish */
	"A.C.",
	"AA.",
	"All.",
	"Ant.",
	"Av.",
	"Avda.",
	"Bien.",
	"C.",
	"C.P.",
	"C.S.",
	"C.V.",
	"CA.",
	"Col.",
	"Comm.",
	"Corp.",
	"C\303\255a.",
	"D.",
	"DC.",
	"Da.",
	"Desc.",
	"Desv.",
	"Dr.",
	"Dra.",
	"Drs.",
	"Dto.",
	"D\302\252.",
	"D\303\261a.",
	"Em.",
	"Emm.",
	"Exc.",
	"Excma.",
	"Excmas.",
	"Excmo.",
	"Excmos.",
	"Exma.",
	"Exmas.",
	"Exmo.",
	"Exmos.",
	"FF.CC.",
	"Fabric.",
	"Fr.",
	"H.P.",
	"Id.",
	"Ilma.",
	"Ilmas.",
	"Ilmo.",
	"Ilmos.",
	"Inc.",
	"JJ.OO.",
	"K.",
	"Kit.",
	"Korn.",
	"L.",
	"Lcda.",
	"Lcdo.",
	"Lda.",
	"Ldo.",
	"Lic.",
	"Ltd.",
	"Ltda.",
	"Ltdo.",
	"M.",
	"MM.",
	"Mons.",
	"Mr.",
	"Mrs.",
	"O.M.",
	"PP.",
	"R.D.",
	"R.U.",
	"RAM.",
	"RR.HH.",
	"Rdo.",
	"Rdos.",
	"Reg.",
	"Rev.",
	"Rol.",
	"Rvdmo.",
	"Rvdmos.",
	"Rvdo.",
	"Rvdos.",
	"SA.",
	"SS.AA.",
	"SS.MM.",
	"Sdad.",
	"Seg.",
	"Sol.",
	"Sr.",
	"Sra.",
	"Sras.",
	"Sres.",
	"Srta.",
	"Srtas.",
	"Sta.",
	"Sto.",
	"Trab.",
	"U.S.",
	"U.S.A.",
	"Var.",
	"Vda.",
	"a. C.",
	"a. e. c.",
	"abr.",
	"afma.",
	"afmas.",
	"afmo.",
	"afmos.",
	"ago.",
	"bco.",
	"bol.",
	"c/c.",
	"cap.",
	"cf.",
	"cfr.",
	"col.",
	"d. C.",
	"depto.",
	"deptos.",
	"dic.",
	"doc.",
	"dom.",
	"dpto.",
	"dptos.",
	"dtor.",
	"e. c.",
	"e.g.",
	"ed.",
	"ej.",
	"ene.",
	"feb.",
	"fig.",
	"figs.",
	"fund.",
	"hnos.",
	"jue.",
	"jul.",
	"jun.",
	"licda.",
	"licdo.",
	"lun.",
	"mar.",
	"may.",
	"mi\303\251.",
	"ms.",
	"mss.",
	"mtro.",
	"nov.",
	"ntra.",
	"ntro.",
	"oct.",
	"p.ej.",
	"prof.",
	"prov.",
	"sept.",
	"sras.",
	"sres.",
	"srs.",
	"ss.",
	"s\303\241b.",
	"trad.",
	"v.gr.",
	"vid.",
	"vie.",
	"vs.",
	NULL
};

static const char **sentsuppress_names(void)
{
	return (const char **)sentsuppress_list_names;
}

static const uint8_t **sentsuppress_list(const char *name, int *lenptr)
{
	const struct sentsuppress_list *ptr = sentsuppress_lists;

	while (ptr->name != NULL && strcmp(ptr->name, name) != 0) {
		ptr++;
	}

	if (ptr->name == NULL) {
		if(lenptr) {
			*lenptr = 0;
		}
		return NULL;
	}

	if(lenptr) {
		*lenptr = ptr->length;
	}
	return (const uint8_t **)(sentsuppress_strings + ptr->offset);
}

#endif /* SENTSUPPRESS_H */
