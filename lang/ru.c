/* lang/ru.c - Russian layout */

#include "qsk.h"

/* [shifted][row]; tokens are separated by spaces, "@xxx" is special */
const struct lang lang_ru = {
	"RU",
	{
		{
			"ё 1 2 3 4 5 6 7 8 9 0 - = @bksp",
			"й ц у к е н г ш щ з х ъ \\",
			"ф ы в а п р о л д ж э @enter",
			"@shift я ч с м и т ь б ю . @at @hk",
			"@lang @tab @space @left @right @quit",
		},
		{
			"Ё ! \" № ; % : ? * ( ) _ + @bksp",
			"Й Ц У К Е Н Г Ш Щ З Х Ъ /",
			"Ф Ы В А П Р О Л Д Ж Э @enter",
			"@shift Я Ч С М И Т Ь Б Ю , @at @hk",
			"@lang @tab @space @left @right @quit",
		},
	},
};
