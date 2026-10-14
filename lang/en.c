/* lang/en.c - English layout */

#include "qsk.h"

/* [shifted][row]; tokens are separated by spaces, "@xxx" is special */
const struct lang lang_en = {
	"EN",
	{
		{
			"1 2 3 4 5 6 7 8 9 0 - = @bksp",
			"q w e r t y u i o p [ ] \\",
			"a s d f g h j k l ; ' @enter",
			"@shift z x c v b n m , . / @at @hk",
			"@lang @tab @space @left @right @quit",
		},
		{
			"! @ # $ % ^ & * ( ) _ + @bksp",
			"Q W E R T Y U I O P { } |",
			"A S D F G H J K L : \" @enter",
			"@shift Z X C V B N M < > ? @at @hk",
			"@lang @tab @space @left @right @quit",
		},
	},
};
