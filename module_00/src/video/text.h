#ifndef __TEXT_HEADER__
#define __TEXT_HEADER__


#include "utils.h"


namespace hoa_video
{

/*!***************************************************************************
 *  \brief Styles for text shadows
 *****************************************************************************/

enum TextShadowStyle
{
	VIDEO_TEXT_SHADOW_INVALID = -1,
	
	VIDEO_TEXT_SHADOW_NONE,       //! no text shadow, even if shadows are enabled
	VIDEO_TEXT_SHADOW_DARK,       //! shadowed area is darkened (default)
	VIDEO_TEXT_SHADOW_LIGHT,      //! shadowed area is lightened
	VIDEO_TEXT_SHADOW_BLACK,      //! shadowed area is completely black
	VIDEO_TEXT_SHADOW_COLOR,      //! shadowed area is the color of the text, but less alpha
	VIDEO_TEXT_SHADOW_INVCOLOR,   //! shadowed area is the inverse of the text's color (e.g. white text, black shadow)
	
	VIDEO_TEXT_SHADOW_TOTAL
	
};


/*!***************************************************************************
 *  \brief this structure holds properties about fonts
 *****************************************************************************/

class FontProperties
{
public:
	int32 height;   //! maximum height of all glyphs
	int32 lineskip; //! SDL_ttf's recommended amount of spacing between lines
	int32 ascent;   //! height above baseline of font
	int32 descent;  //! height below baseline of font
	int32 shadowX;  //! x offset of text shadow
	int32 shadowY;  //! y offset of text shadow
	
	TextShadowStyle shadowStyle;  //! style of text shadow
};

}  // namespace hoa_video


#endif
