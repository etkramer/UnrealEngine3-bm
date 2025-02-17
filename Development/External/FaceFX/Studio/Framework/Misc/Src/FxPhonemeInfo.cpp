//------------------------------------------------------------------------------
// Extended information about the phonemes.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxArray.h"
#include "FxPhonemeInfo.h"

namespace OC3Ent
{

namespace Face
{

#define MAKE_PHONEINFO( phon, cls, tb, sampa, ipa, sample, cfc ) \
	FxPhonExtendedInfo phon##Info((phon), (cls), (tb), (sampa), (ipa), (sample), (cfc));\
	phonInfo.PushBack( phon##Info );

// Gets information about a phoneme
const FxPhonExtendedInfo& FxGetPhonemeInfo( const FxPhoneme& phon )
{
	FxAssert( PHON_FIRST <= phon && phon <= PHON_LAST );
	static FxArray<FxPhonExtendedInfo> phonInfo;
	if( phonInfo.Length() == 0 )
	{
		MAKE_PHONEINFO( PHON_SIL, PHONCLASS_SILENCE, wxT("SIL"), wxT("sil"), wxT("sil"), wxT("(silence)"), wxT("silence") );
		MAKE_PHONEINFO( PHON_P, PHONCLASS_PLOSIVE, wxT("P"), wxT("p"), wxT("p"), wxT("/p/ay"), wxT("voiceless bilabial plosive") );
		MAKE_PHONEINFO( PHON_B, PHONCLASS_PLOSIVE, wxT("B"), wxT("b"), wxT("b"), wxT("/b/it"), wxT("voiced bilabial plosive") );
		MAKE_PHONEINFO( PHON_T, PHONCLASS_PLOSIVE, wxT("T"), wxT("t"), wxT("t"), wxT("/t/ie"), wxT("voiceless alveolar plosive") );
		MAKE_PHONEINFO( PHON_D, PHONCLASS_PLOSIVE, wxT("D"), wxT("d"), wxT("d"), wxT("/d/og"), wxT("voiced alveolar plosive") );
		MAKE_PHONEINFO( PHON_K, PHONCLASS_PLOSIVE, wxT("K"), wxT("k"), wxT("k"), wxT("/c/at"), wxT("voiceless velar plosive") );
		MAKE_PHONEINFO( PHON_G, PHONCLASS_PLOSIVE, wxT("G"), wxT("g"),  wxString((wxChar)609), wxT("/g/um"), wxT("voiced velar plosive") );
		MAKE_PHONEINFO( PHON_M, PHONCLASS_NASAL, wxT("M"), wxT("m"), wxT("m"), wxT("/m/an"), wxT("bilabial nasal") );
		MAKE_PHONEINFO( PHON_N, PHONCLASS_NASAL, wxT("N"), wxT("n"), wxT("n"), wxT("/n/o"), wxT("alveolar nasal") );
		MAKE_PHONEINFO( PHON_NG, PHONCLASS_NASAL, wxT("NG"), wxT("N"),  wxString((wxChar)331), wxT("si/ng/"), wxT("velar nasal") );
		MAKE_PHONEINFO( PHON_RA, PHONCLASS_TRILL, wxT("RA"), wxT("r"), wxT("r"), wxT("Spanish pe/rr/o"), wxT("alveolar trill") );
		MAKE_PHONEINFO( PHON_RU, PHONCLASS_TRILL, wxT("RU"), wxT("R\\"),  wxString((wxChar)640), wxT("French /r/ue"), wxT("uvular trill") );
		MAKE_PHONEINFO( PHON_FLAP, PHONCLASS_PLOSIVE, wxT("FLAP"), wxT("4"), wxString((wxChar)638), wxT("wa/t/er"), wxT("alveolar flap") );
		MAKE_PHONEINFO( PHON_PH, PHONCLASS_FRICATIVE, wxT("PH"), wxT("p\\"),  wxString((wxChar)632), wxT("Japanese /f/uhai"), wxT("voiceless bilabial fricative") );
		MAKE_PHONEINFO( PHON_F, PHONCLASS_FRICATIVE, wxT("F"), wxT("f"), wxT("f"), wxT("/f/un"), wxT("voiceless labiodental fricative") );
		MAKE_PHONEINFO( PHON_V, PHONCLASS_FRICATIVE, wxT("V"), wxT("v"), wxT("v"), wxT("/v/an"), wxT("voiced labiodental fricative") );
		MAKE_PHONEINFO( PHON_TH, PHONCLASS_FRICATIVE, wxT("TH"), wxT("T"),  wxString((wxChar)952), wxT("/th/ing"), wxT("voiceless dental fricative") );
		MAKE_PHONEINFO( PHON_DH, PHONCLASS_FRICATIVE, wxT("DH"), wxT("D"),  wxString((wxChar)240), wxT("/th/is"), wxT("voiced dental fricative") );
		MAKE_PHONEINFO( PHON_S, PHONCLASS_FRICATIVE, wxT("S"), wxT("s"), wxT("s"), wxT("/s/it"), wxT("voiceless alveolar fricative") );
		MAKE_PHONEINFO( PHON_Z, PHONCLASS_FRICATIVE, wxT("Z"), wxT("z"), wxT("z"), wxT("/z/oo"), wxT("voiced alveolar fricative") );
		MAKE_PHONEINFO( PHON_SH, PHONCLASS_FRICATIVE, wxT("SH"), wxT("S"),  wxString((wxChar)643), wxT("/sh/oe"), wxT("voiceless postalveolar fricative") );
		MAKE_PHONEINFO( PHON_ZH, PHONCLASS_FRICATIVE, wxT("ZH"), wxT("Z"),  wxString((wxChar)658), wxT("trea/s/ure"), wxT("voiced postalveolar fricative") );
		MAKE_PHONEINFO( PHON_CX, PHONCLASS_FRICATIVE, wxT("CX"), wxT("C"),  wxString((wxChar)231), wxT("German di/ch/t"), wxT("voiceless palatal fricative") );
		MAKE_PHONEINFO( PHON_X, PHONCLASS_FRICATIVE, wxT("X"), wxT("x"), wxT("x"), wxT("German da/ch/"), wxT("voiceless velar fricative") );
		MAKE_PHONEINFO( PHON_GH, PHONCLASS_FRICATIVE, wxT("GH"), wxT("G"),  wxString((wxChar)611), wxT("Japanese ha/g/e"), wxT("voiced velar fricative") );
		MAKE_PHONEINFO( PHON_HH, PHONCLASS_FRICATIVE, wxT("HH"), wxT("h"), wxT("h"), wxT("/h/appy"), wxT("voiceless glottal fricative") );
		MAKE_PHONEINFO( PHON_R, PHONCLASS_APPROXIMATE, wxT("R"), wxT("r\\"),  wxString((wxChar)633), wxT("/r/ed"), wxT("alveolar approximate") );
		MAKE_PHONEINFO( PHON_Y, PHONCLASS_APPROXIMATE, wxT("Y"), wxT("j"), wxT("j"), wxT("/y/es"), wxT("palatal approximate") );
		MAKE_PHONEINFO( PHON_L, PHONCLASS_APPROXIMATE, wxT("L"), wxT("l"), wxT("l"), wxT("/l/ap"), wxT("alveolar lateral approximate") );
		MAKE_PHONEINFO( PHON_W, PHONCLASS_APPROXIMATE, wxT("W"), wxT("w"), wxT("w"), wxT("/w/in"), wxT("labial-velar approximate") );
		MAKE_PHONEINFO( PHON_H, PHONCLASS_APPROXIMATE, wxT("H"), wxT("H"),  wxString((wxChar)613), wxT("French /hu/it"), wxT("labial-palatal approximate") );
		MAKE_PHONEINFO( PHON_TS, PHONCLASS_FRICATIVE, wxT("TS"), wxT("ts"), wxString((wxChar)678), wxT("pi/zz/a"), wxT("voiceless alveolar affricate") );
		MAKE_PHONEINFO( PHON_CH, PHONCLASS_FRICATIVE, wxT("CH"), wxT("tS"), wxString((wxChar)679), wxT("/ch/ip"), wxT("voiceless postalveolar affricate") );
		MAKE_PHONEINFO( PHON_JH, PHONCLASS_FRICATIVE, wxT("JH"), wxT("dZ"), wxString((wxChar)676), wxT("/j/ump"), wxT("voiced postalveolar affricate") );
		MAKE_PHONEINFO( PHON_IY, PHONCLASS_VOWEL, wxT("IY"), wxT("i"), wxT("i"), wxT("b/ee/t"), wxT("close front unrounded") );
		MAKE_PHONEINFO( PHON_E, PHONCLASS_VOWEL, wxT("E"), wxT("e"), wxT("e"), wxT("pl/ay/ (non-diphthong)"), wxT("close-mid front unrounded") );
		MAKE_PHONEINFO( PHON_EN, PHONCLASS_VOWEL, wxT("EN"), wxT("e~"), wxString((wxChar)101)+wxString((wxChar)771), wxT("French /ain/si"), wxT("nasalized close-mid front unrounded") );
		MAKE_PHONEINFO( PHON_EH, PHONCLASS_VOWEL, wxT("EH"), wxT("E"), wxString((wxChar)603), wxT("b/e/d"), wxT("open-mid front unrounded") );
		MAKE_PHONEINFO( PHON_A, PHONCLASS_VOWEL, wxT("A"), wxT("a"), wxT("a"), wxT("German r/a/tte"), wxT("open front unrounded") );
		MAKE_PHONEINFO( PHON_AA, PHONCLASS_VOWEL, wxT("AA"), wxT("A"), wxString((wxChar)593), wxT("h/o/t"), wxT("open back unrounded") );
		MAKE_PHONEINFO( PHON_AAN, PHONCLASS_VOWEL, wxT("AAN"), wxT("A~"), wxString((wxChar)593)+wxString((wxChar)771), wxT("French /un/"), wxT("nasalized open back unrounded") );
		MAKE_PHONEINFO( PHON_AO, PHONCLASS_VOWEL, wxT("AO"), wxT("O"), wxString((wxChar)596), wxT("French s/o/rt"), wxT("open-mid back rounded") );
		MAKE_PHONEINFO( PHON_AON, PHONCLASS_VOWEL, wxT("AON"), wxT("O~"), wxString((wxChar)596)+wxString((wxChar)771), wxT("French s/on/"), wxT("nasalized open-mid back rounded") );
		MAKE_PHONEINFO( PHON_O, PHONCLASS_VOWEL, wxT("O"), wxT("o"), wxT("o"), wxT("r/ow/ (non-diphthong)"), wxT("close-mid back rounded") );
		MAKE_PHONEINFO( PHON_ON, PHONCLASS_VOWEL, wxT("ON"), wxT("o~"), wxString((wxChar)111)+wxString((wxChar)771), wxT("French m/an/teau"), wxT("nasalized close-mid back rounded") );
		MAKE_PHONEINFO( PHON_UW, PHONCLASS_VOWEL, wxT("UW"), wxT("u"), wxT("u"), wxT("s/oo/n"), wxT("close back rounded") );
		MAKE_PHONEINFO( PHON_UY, PHONCLASS_VOWEL, wxT("UY"), wxT("y"), wxT("y"), wxT("French ch/u/te"), wxT("close front rounded") );
		MAKE_PHONEINFO( PHON_EU, PHONCLASS_VOWEL, wxT("EU"), wxT("2"), wxString((wxChar)248), wxT("French p/eu/"), wxT("close-mid front rounded") );
		MAKE_PHONEINFO( PHON_OE, PHONCLASS_VOWEL, wxT("OE"), wxT("9"), wxString((wxChar)339), wxT("French j/eu/ne"), wxT("open-mid front rounded") );
		MAKE_PHONEINFO( PHON_OEN, PHONCLASS_VOWEL, wxT("OEN"), wxT("9~"), wxString((wxChar)339)+wxString((wxChar)771), wxT("French br/un/"), wxT("nasalized open-mid front rounded") );
		MAKE_PHONEINFO( PHON_AH, PHONCLASS_VOWEL, wxT("AH"), wxT("V"), wxString((wxChar)652), wxT("r/u/n"), wxT("open-mid back unrounded") );
		MAKE_PHONEINFO( PHON_IH, PHONCLASS_VOWEL, wxT("IH"), wxT("I"),wxString((wxChar)618), wxT("b/i/t"), wxT("near-close near-front unrounded") );
		MAKE_PHONEINFO( PHON_UU, PHONCLASS_VOWEL, wxT("UU"), wxT("Y"), wxString((wxChar)655), wxT("German sch/ü/tzen"), wxT("near-close near-front rounded") );
		MAKE_PHONEINFO( PHON_UH, PHONCLASS_VOWEL, wxT("UH"), wxT("U"), wxString((wxChar)650), wxT("h/oo/k"), wxT("near-close near-back rounded") );
		MAKE_PHONEINFO( PHON_AX, PHONCLASS_VOWEL, wxT("AX"), wxT("@"), wxString((wxChar)601), wxT("/a/bout"), wxT("mid central") );
		MAKE_PHONEINFO( PHON_UX, PHONCLASS_VOWEL, wxT("UX"), wxT("6"), wxString((wxChar)592), wxT("German de/r/"), wxT("near-open central") );
		MAKE_PHONEINFO( PHON_AE, PHONCLASS_VOWEL, wxT("AE"), wxT("{"), wxString((wxChar)230), wxT("f/a/t"), wxT("near-open front unrounded") );
		MAKE_PHONEINFO( PHON_ER, PHONCLASS_VOWEL, wxT("ER"), wxT("3"),  wxString((wxChar)604), wxT("p/er/fect"), wxT("open-mid central unrounded") );
		MAKE_PHONEINFO( PHON_AXR, PHONCLASS_VOWEL, wxT("AXR"), wxT("@`"), wxString((wxChar)602), wxT("butt/er/"), wxT("rhotacized mid central") );
		MAKE_PHONEINFO( PHON_EXR, PHONCLASS_VOWEL, wxT("EXR"), wxT("3`"), wxString((wxChar)605), wxT("f/ur/"), wxT("rhotacized open-mid central") );
		MAKE_PHONEINFO( PHON_EY, PHONCLASS_DIPHTHONG, wxT("EY"), wxT("EI"), wxString((wxChar)603)+wxString((wxChar)618), wxT("pl/ay/"), wxT("diphthong @todo") );
		MAKE_PHONEINFO( PHON_AW, PHONCLASS_DIPHTHONG, wxT("AW"), wxT("aU"), wxString(wxT('a'))+wxString((wxChar)650), wxT("ab/ou/t"), wxT("diphthong @todo") );
		MAKE_PHONEINFO( PHON_AY, PHONCLASS_DIPHTHONG, wxT("AY"), wxT("aI"), wxString(wxT('a'))+wxString((wxChar)618), wxT("b/i/te"), wxT("diphthong @todo") );
		MAKE_PHONEINFO( PHON_OY, PHONCLASS_DIPHTHONG, wxT("OY"), wxT("OI"), wxString((wxChar)596)+wxString((wxChar)618), wxT("b/oy/"), wxT("diphthong @todo") );
		MAKE_PHONEINFO( PHON_OW, PHONCLASS_DIPHTHONG, wxT("OW"), wxT("@U"), wxString((wxChar)601)+wxString((wxChar)650), wxT("b/oa/t"), wxT("diphthong @todo") );
	}
	return phonInfo[phon];
}

const FxPhonExtendedInfo& FxGetPhonemeInfo( FxSize phonIndex )
{
	return FxGetPhonemeInfo( static_cast<FxPhoneme>(phonIndex) );
}

#define MAKE_PHONECLASSINFO( phoneclass, nameStr, descStr ) \
	FxPhonClassExtendedInfo phoneclass##Info((phoneclass), (nameStr), (descStr));\
	phonClassInfo.PushBack( phoneclass##Info );

const FxPhonClassExtendedInfo& FxGetPhonemeClassInfo( const FxPhonemeClass& phonClass )
{
	FxAssert( PHONCLASS_FIRST <= phonClass && phonClass <= PHONCLASS_LAST );
	static FxArray<FxPhonClassExtendedInfo> phonClassInfo;
	if( phonClassInfo.Length() == 0 )
	{
		MAKE_PHONECLASSINFO( PHONCLASS_VOWEL,	    wxT("vowel"),       wxT("@todo") );
		MAKE_PHONECLASSINFO( PHONCLASS_DIPHTHONG,   wxT("diphthong"),   wxT("@todo") );
		MAKE_PHONECLASSINFO( PHONCLASS_FRICATIVE,   wxT("fricative"),   wxT("@todo") );
		MAKE_PHONECLASSINFO( PHONCLASS_NASAL,	    wxT("nasal"),       wxT("@todo") );
		MAKE_PHONECLASSINFO( PHONCLASS_APPROXIMATE, wxT("approximate"), wxT("@todo") );
		MAKE_PHONECLASSINFO( PHONCLASS_PLOSIVE,		wxT("plosive"),     wxT("@todo") );
		MAKE_PHONECLASSINFO( PHONCLASS_TRILL,	    wxT("trill"),       wxT("@todo") );
		MAKE_PHONECLASSINFO( PHONCLASS_SILENCE,	    wxT("silence"),     wxT("@todo") );
	}
	return phonClassInfo[phonClass - PHONCLASS_FIRST]; // offset from the beginning of the valid phonclasses.
}

const FxPhonClassExtendedInfo& FxGetPhonemeClassInfo( FxSize phonClassIndex )
{
	return FxGetPhonemeClassInfo( static_cast<FxPhonemeClass>(phonClassIndex) );
}

} // namespace Face

} // namespace OC3Ent
