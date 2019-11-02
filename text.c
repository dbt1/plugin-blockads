/*
 * $Id: text.c,v 1.0 2010/04/25 18:00:00 Exp $
 *
 * blockads - d-box2 linux project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
*/

#include "text.h"
#include "gfx.h"
#include "io.h"

/******************************************************************************
 * MyFaceRequester
 ******************************************************************************/

FT_Error MyFaceRequester(FTC_FaceID face_id, FT_Library _library, FT_Pointer request_data, FT_Face *aface)
{
	FT_Error result;
	(void)request_data;

	result = FT_New_Face(_library, face_id, 0, aface);

	if(!result) printf("<Font \"%s\" loaded>\n", (char*)face_id);
	else        printf("<Font \"%s\" failed>\n", (char*)face_id);

	return result;
}

int RenderChar(FT_ULong currentchar, int sx, int sy, int ex, int color)
{
	unsigned char pix[4]={bl[color],gn[color],rd[color],tr[color]};
	int row, pitch, bit, x = 0, y = 0;
	FT_UInt glyphindex;
	FT_Vector kerning;
	FT_Error error;

	currentchar=currentchar & 0xFF;

	//load char

	if(!(glyphindex = FT_Get_Char_Index(face, (int)currentchar)))
	{
		//printf("Blockads <FT_Get_Char_Index for Char \"%c\" failed\n", (int)currentchar);
		return 0;
	}


	if((error = FTC_SBitCache_Lookup(cache, &desc, glyphindex, &sbit, NULL)))
	{
		//printf("Blockads <FTC_SBitCache_Lookup for Char \"%c\" failed with Errorcode 0x%.2X>\n", (int)currentchar, error);
		return 0;
	}

	// no kerning used
	/*
	if(use_kerning)
	{
		FT_Get_Kerning(face, prev_glyphindex, glyphindex, ft_kerning_default, &kerning);

		prev_glyphindex = glyphindex;
		kerning.x >>= 6;
}
else
	*/
	kerning.x = 0;

//render char

if(color != -1) /* don't render char, return charwidth only */
{
	if(sx + sbit->xadvance >= ex) return -1; /* limit to maxwidth */

		for(row = 0; row < sbit->height; row++)
		{
			for(pitch = 0; pitch < sbit->pitch; pitch++)
			{
				for(bit = 7; bit >= 0; bit--)
				{
					if(pitch*8 + 7-bit >= sbit->width) break; /* render needed bits only */

						if((sbit->buffer[row * sbit->pitch + pitch]) & 1<<bit) memcpy(lbb + (startx + sx + sbit->left + kerning.x + x)*4 + fix_screeninfo.line_length*(starty + sy - sbit->top + y),pix,4);

						x++;
				}
			}

			x = 0;
			y++;
		}

}

//return charwidth

return sbit->xadvance + kerning.x;
}

/******************************************************************************
 * GetStringLen
 ******************************************************************************/

int GetStringLen(char *string)
{
	int stringlen = 0;

	//reset kerning

		prev_glyphindex = 0;

	//calc len

		while(*string)
		{
			stringlen += RenderChar(*string, -1, -1, -1, -1);
			string++;
		}

	return stringlen;
}

/******************************************************************************
 * RenderString
 ******************************************************************************/

void RenderString(char *string, int sx, int sy, int maxwidth, int layout, int size, int color)
{
	int stringlen, ex, charwidth;
	char rstr[256], *rptr=rstr;

	//set size
	
		if(strstr(string,"N/A")!=NULL)
		{
			sprintf(rstr,"---");
		}
		else
		{
			strcpy(rstr,string);
		}

		switch (size)
		{
#ifdef FT_NEW_CACHE_API
			case SMALL: desc.width = desc.height = 24; break;
			case MED:   desc.width = desc.height = 32; break;
			case BIG:   desc.width = desc.height = 40; break;
			default:    desc.width = desc.height = size; break;
#else
			case SMALL: desc.font.pix_width = desc.font.pix_height = 24; break;
			case MED:   desc.font.pix_width = desc.font.pix_height = 32; break;
			case BIG:   desc.font.pix_width = desc.font.pix_height = 40; break;
			default :   desc.font.pix_width = desc.font.pix_height = size; break;
#endif


		}
		
	//set alignment

		if(layout != LEFT)
		{
			stringlen = GetStringLen(rstr);

			switch(layout)
			{
				case CENTER:	if(stringlen < maxwidth) sx += (maxwidth - stringlen)/2;
						break;

				case RIGHT:	if(stringlen < maxwidth) sx += maxwidth - stringlen;
			}
		}

	//reset kerning

		prev_glyphindex = 0;

	//render string

		ex = sx + maxwidth;

		while(*rptr)
		{
			if((charwidth = RenderChar(*rptr, sx, sy, ex, color)) == -1) return; /* string > maxwidth */

			sx += charwidth;
			rptr++;
		}
}

/******************************************************************************
 * ShowMessage
 ******************************************************************************/

void ShowMessage(char *message, int wait)
{
	//layout

		RenderBox(105, 178, 514, 327, FILL, CMC);
		RenderBox(105, 178, 514, 327, GRID, CMCS);
		RenderBox(105, 220, 514, 327, GRID, CMCS);

	//message

		RenderString("Blockads Info", 107, 213, 406, CENTER, BIG, CMHT);

		RenderString(message, 107, 265, 406, CENTER, MED, CMCT);

		if(wait)
		{
			RenderBox(285, 286, 334, 310, FILL, CMCS);
			RenderString("OK", 287, 305, 46, CENTER, SMALL, CMCT);
		}
		memcpy(lfb, lbb, fix_screeninfo.line_length*var_screeninfo.yres);

		while(wait && (GetRCCode() != RC_OK));
}

