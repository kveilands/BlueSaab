/* Convert UTF8 Latin extended chars to plain ASCII
 *
 * Copyright (C) 2018 Girts Linde
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

//#include <string.h>
//#include <stdio.h>
//#include <assert.h>

/*
https://en.wikipedia.org/wiki/Latin_script_in_Unicode

UTF8   unicode
c3 80  00C0  ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏ
c3 90  00D0  ÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞß
c3 a0  00E0  àáâãäåæçèéêëìíîï
c3 b0  00F0  ðñòóôõö÷øùúûüýþÿ

c4 80  0100  ĀāĂăĄąĆćĈĉĊċČčĎď
c4 90  0110  ĐđĒēĔĕĖėĘęĚěĜĝĞğ
c4 a0  0120  ĠġĢģĤĥĦħĨĩĪīĬĭĮį
c4 b0  0130  İıĲĳĴĵĶķĸĹĺĻļĽľĿ

c5 80  0140  ŀŁłŃńŅņŇňŉŊŋŌōŎŏ
c5 90  0150  ŐőŒœŔŕŖŗŘřŚśŜŝŞş
c5 a0  0160  ŠšŢţŤťŦŧŨũŪūŬŭŮů
c5 b0  0170  ŰűŲųŴŵŶŷŸŹźŻżŽžſ

c6 80  0180  ƀƁƂƃƄƅƆƇƈƉƊƋƌƍƎƏ
c6 90  0190  ƐƑƒƓƔƕƖƗƘƙƚƛƜƝƞƟ
c6 a0  01A0  ƠơƢƣƤƥƦƧƨƩƪƫƬƭƮƯ
c6 b0  01B0  ưƱƲƳƴƵƶƷƸƹƺƻƼƽƾƿ

c7 80  01C0  ǀǁǂǃǄǅǆǇǈǉǊǋǌǍǎǏ
c7 80  01D0  ǐǑǒǓǔǕǖǗǘǙǚǛǜǝǞǟ
c7 80  01E0  ǠǡǢǣǤǥǦǧǨǩǪǫǬǭǮǯ
c7 80  01F0  ǰǱǲǳǴǵǶǷǸǹǺǻǼǽǾǿ

c8 80  0200  ȀȁȂȃȄȅȆȇȈȉȊȋȌȍȎȏ
c8 80  0210  ȐȑȒȓȔȕȖȗȘșȚțȜȝȞȟ
c8 80  0220  ȠȡȢȣȤȥȦȧȨȩȪȫȬȭȮȯ
c8 80  0230  ȰȱȲȳȴȵȶȷȸȹȺȻȼȽȾȿ
 */

const char *conversion[] = {
		"ÀÁÂÃÄÅÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖØÙÚÛÜÝàáâãäåçèéêëìíîïðñòóôõöøùúûüýÿ", // c3
		"AAAAAACEEEEIIIIDNOOOOOOUUUUYaaaaaaceeeeiiiidnoooooouuuuyy",
		"ĀāĂăĄąĆćĈĉĊċČčĎďĐđĒēĔĕĖėĘęĚěĜĝĞğĠġĢģĤĥĦħĨĩĪīĬĭĮįİıĴĵĶķĸĹĺĻļĽľĿ", // c4
		"AaAaAaCcCcCcCcDdDdEeEeEeEeEeGgGgGgGgHhHhIiIiIiIiIiJjKkkLlLlLlL",
		"ŀŁłŃńŅņŇňŉŊŋŌōŎŏŐőŔŕŖŗŘřŚśŜŝŞşŠšŢţŤťŦŧŨũŪūŬŭŮůŰűŲųŴŵŶŷŸŹźŻżŽž", // c5
		"lLlNnNnNnnNnOoOoOoRrRrRrSsSsSsSsTtTtTtUuUuUuUuUuUuWwYyYZzZzZz",
		"ƀƁƂƃƄƅƇƈƉƊƐƑƒƓƕƘƙƚƝƞƠơƤƥƦƫƬƭƮƯưƳƴƵƶ", // c6
		"bBBbbbCcDDEFfGhKklNnOoPpRtTtTUuYyZz",
		"ǍǎǏǐǑǒǓǔǕǖǗǘǙǚǛǜǞǟǠǡǤǥǦǧǨǩǪǫǬǭǴǵǸǹǺǻǾǿ", // c7
		"AaIiOoUuUuUuUuUuAaAaGgGgKkOoOoGgNnAaOo",
		"ȀȁȂȃȄȅȆȇȈȉȊȋȌȍȎȏȐȑȒȓȔȕȖȗȘșȚțȞȟȤȥȦȧȨȩȪȫȬȭȮȯȰȱȲȳȴȵȶȺȻȼȽȾȿ", // c8
		"AaAaEeEeIiIiOoOoRrRrUuUuSsTtHhZzAaEeOoOoOoOoYylntACcLTs",
		0
};

static char find2(const char *cp, const char *from, const char *to) {
	// Assuming the first byte of all chars is the same, so compare only the second byte
	char c = cp[1];
	while (*from) {
		from++;
		if (*from == c) {
			return *to;
		}
		from++;
		to++;
	}
	return 0;
}

void utf_convert(const char *from, char *to, int size) {
	char *to_last = to + size - 1;
	while (*from && to < to_last) {
		if ((*from & 0b11100000) == 0b11000000) { // this will be 2 bytes of UTF8 encoded data
			if (from[1] == 0) // string ends, no second byte
				break;
			char c = 0;
			const char **p = conversion;
			while (*p) {
				const char *convert_from = p[0];
				if (*from == *convert_from) { // check the first byte - if this is the string to search
					const char *convert_to = p[1];
					c = find2(from, convert_from, convert_to);
					break;
				}
				p += 2;
			}
			if (c) { // match found
				*to++ = c;
			}
			from+=2;
		} else {
			*to++ = *from++;
		}
	}
	*to = 0; // terminate the string
}

/*
void test_convert() {
	printf("start\n");

	char buf[100];

	utf_convert("qwertyuiop[]\asdfghjkl;'zxcvbnm,.//", buf, sizeof(buf));
	assert(strcmp(buf, "qwertyuiop[]\asdfghjkl;'zxcvbnm,.//") == 0);

	utf_convert("QWERTYUIOP{}|ASDFGHJKL:\"ZXCVBNM<>?", buf, sizeof(buf));
	assert(strcmp(buf, "QWERTYUIOP{}|ASDFGHJKL:\"ZXCVBNM<>?") == 0);

	utf_convert("`1234567890-=~!@#$%^&*()_+", buf, sizeof(buf));
	assert(strcmp(buf, "`1234567890-=~!@#$%^&*()_+") == 0);

	utf_convert("aaaaaa", buf, 3);
	assert(strcmp(buf, "aa") == 0); // fills the size exactly, including the terminator

	utf_convert("ÄÅÇÈ", buf, sizeof(buf));
	assert(strcmp(buf, "AACE") == 0);

	utf_convert("ĚěĜĝ", buf, sizeof(buf));
	assert(strcmp(buf, "EeGg") == 0);

	utf_convert("ŘřŚś", buf, sizeof(buf));
	assert(strcmp(buf, "RrSs") == 0);

	utf_convert("ƝƞƠơƤƥ", buf, sizeof(buf));
	assert(strcmp(buf, "NnOoPp") == 0);

	utf_convert("ǍǎǏǸǹǺǻǾǿ", buf, sizeof(buf));
	assert(strcmp(buf, "AaINnAaOo") == 0);

	utf_convert("ȰȱȲȳȽȾȿ", buf, sizeof(buf));
	assert(strcmp(buf, "OoYyLTs") == 0);

	utf_convert("Glāžšķūņu rūķīši", buf, sizeof(buf));
	assert(strcmp(buf, "Glazskunu rukisi") == 0);

	// drop unrecognized 2-byte UTF8 chars
	utf_convert("0ȸ1ʖ2", buf, sizeof(buf));
	assert(strcmp(buf, "012") == 0);

	// string ends after 1st byte of 2-byte UTF8 char
	utf_convert("zz\xc6", buf, sizeof(buf));
	assert(strcmp(buf, "zz") == 0);

	printf("end\n");
}
*/




