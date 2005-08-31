#include "utils.h"
#include "video.h"
#include "gui.h"
#include <sstream>

using namespace std;
using namespace hoa_video;
using hoa_utils::MakeWideString;
using hoa_utils::ustring;


namespace hoa_video
{


//-----------------------------------------------------------------------------
// TextBox
//-----------------------------------------------------------------------------

TextBox::TextBox()
: _x(0.0f),
  _y(0.0f),
  _width (0.0f),
  _height(0.0f),
  _xalign(-1),
  _yalign(-1),
  _finished(false),
  _currentTime(0),
  _mode(VIDEO_TEXT_INVALID),
  _displaySpeed(0.0f)
{
	_initialized = IsInitialized(_initializeErrors);
}


//-----------------------------------------------------------------------------
// ~TextBox
//-----------------------------------------------------------------------------

TextBox::~TextBox()
{
	// for now nothing to do since TextBox doesn't allocate any memory
}


//-----------------------------------------------------------------------------
// Update: increments the TextBox's timer for gradual text rendering
//         Returns false on unexpected failure
//-----------------------------------------------------------------------------

bool TextBox::Update(int32 frameTime)
{
	_currentTime += frameTime;
	return true;
}


string makestring(ustring text)
{
	int32 length = (int32) text.length();
	char *astring = new char[length+1];
	astring[length] = char('\0');
	
	for(int32 c = 0; c < length; ++c)
	{
		astring[c] = char(text[c]);
	}
	
	string str(astring);
	delete [] astring;
	
	return str;	
}

//-----------------------------------------------------------------------------
// Draw: actually draws the text (via the GameVideo interface)
//       Returns false on unexpected failure
//-----------------------------------------------------------------------------

bool TextBox::Draw()
{
	// quit if we have nothing to do
	if(_text.empty())
		return true;
		
	// determine the rectangle of the textbox based on the position, dimensions,
	// and draw flags in the video engine
	
	float left, right, top, bottom;
	GameVideo *video = GameVideo::GetReference();
	
	video->_PushContext();
	
	video->SetFont(_font);

	left   = _x;
	right  = _x;
	top    = _y;
	bottom = _y;
	
	CoordSys &cs = video->_coordSys;
	
	if(cs._upDir > 0.0f)
		top += _height;
	else
		bottom += _height;
		
	if(cs._rightDir > 0.0f)
		right += _width;
	else
		left += _width;
	
	float xoff, yoff;
	
	xoff = ((video->_xalign + 1) * _width)  * 0.5f * -cs._rightDir;
	yoff = ((video->_yalign + 1) * _height) * 0.5f * -cs._upDir;
	
	left   += xoff;
	right  += xoff;
	
	top    += yoff;
	bottom += yoff;	
	
	// figure out where the top of the rendered text is
	
	float textHeight = (float) CalculateTextHeight();
	float textTop;
	if(_yalign == 1)
	{
		// top alignment
		textTop = top;
	}
	else if(_yalign == 0)
	{
		textTop = top - cs._upDir * (_height - textHeight) * 0.5f;
	}
	else
	{
		// right alignment
		textTop = top - cs._upDir * (_height - textHeight);
	}
	
	
	int32 xalign;
	float textX;
	
	// figure out X alignment
	if(_xalign == -1)
	{
		// left align
		xalign = VIDEO_X_LEFT;
		textX = left;
	}
	else if(_xalign == 0)
	{
		// center align
		xalign = VIDEO_X_CENTER;
		textX = (left + right) * 0.5f;
	}
	else
	{
		// right align
		xalign = VIDEO_X_RIGHT;
		textX = right;
	}
	
	video->Move(textX, textTop);

	video->SetDrawFlags(xalign, VIDEO_Y_TOP, VIDEO_BLEND, 0);
	
	// draw the text line by line		

	for(int32 line = 0; line < (int32)_text.size(); ++line)
	{
		video->DrawText(_text[line]);
		video->MoveRelative(0.0f, _fontProperties.lineskip * -cs._upDir);
	}
		
	video->_PopContext();
	return true;
}


//-----------------------------------------------------------------------------
// SetPosition: sets the position of the textbox, based on the (0, 1024, 0, 768)
//              coordinate system. Note that the textbox IS affected by the 
//              video engine's alignment flags
//-----------------------------------------------------------------------------

void TextBox::SetPosition(float x, float y)
{
	_x = x;
	_y = y;
}


//-----------------------------------------------------------------------------
// GetPosition: returns the position of the textbox into x and y
//-----------------------------------------------------------------------------

void TextBox::GetPosition(float &x, float &y)
{
	x = _x;
	y = _y;
}


//-----------------------------------------------------------------------------
// SetDimensions: sets the dimensions of the textbox. Returns false if w and/or
//                w are negative or larger than 1024 and 768 respectively
//-----------------------------------------------------------------------------

bool TextBox::SetDimensions(float w, float h)
{
	if(w <= 0.0f || w > 1024.0f)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: TextBox::SetDimensions() failed, invalid width: " << w << endl;
		return false;
	}

	if(h <= 0.0f || h > 768.0f)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: TextBox::SetDimensions() failed, invalid height: " << h << endl;
		return false;
	}
	
	_width = w;
	_height = h;

	_initialized = IsInitialized(_initializeErrors);
	
	return true;
}


//-----------------------------------------------------------------------------
// GetDimensions: return the width and height of the textbox into w and h
//-----------------------------------------------------------------------------

void TextBox::GetDimensions(float &w, float &h)
{
	w = _width;
	h = _height;
}


//-----------------------------------------------------------------------------
// SetAlignment: sets the alignment flags to be used for the text
//               returns false if any invalid alignment flag is passed
//-----------------------------------------------------------------------------

bool TextBox::SetAlignment(int xalign, int yalign)
{
	bool success = true;
	
	switch(xalign)
	{
		case VIDEO_X_LEFT:
			_xalign = -1;
			break;
		case VIDEO_X_CENTER:
			_xalign = 0;
			break;
		case VIDEO_X_RIGHT:
			_xalign = 1;
			break;
		default:
		{
			if(VIDEO_DEBUG)
				cerr << "VIDEO ERROR: TextBox::SetAlignment() failed, invalid xalign: " << xalign << endl;
			success = false;
		}
	};
	
	switch(yalign)
	{
		case VIDEO_Y_TOP:
			_yalign = 1;
			break;
		case VIDEO_Y_CENTER:
			_yalign = 0;
			break;
		case VIDEO_Y_BOTTOM:
			_yalign = -1;
			break;
		default:
		{
			if(VIDEO_DEBUG)
				cerr << "VIDEO ERROR: TextBox::SetAlignment() failed, invalid yalign: " << yalign << endl;
			success = false;
		}
	};
	
	_initialized = IsInitialized(_initializeErrors);
	
	return success;
}


//-----------------------------------------------------------------------------
// GetAlignment: returns the alignment flags into xalign and yalign
//-----------------------------------------------------------------------------

void TextBox::GetAlignment(int &xalign, int &yalign)
{
	xalign = _xalign;
	yalign = _yalign;
}


//-----------------------------------------------------------------------------
// SetFont: sets the font of this textbox. fontName must be the label of a valid
//          font loaded with LoadFont()
//-----------------------------------------------------------------------------

bool TextBox::SetFont(const string &fontName)
{	
	// try to get pointer to video manager
	GameVideo *videoManager = GameVideo::GetReference();
	if(!videoManager)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: TextBox::SetFont() failed, couldn't get pointer to GameVideo!" << endl;
		return false;
	}

	// try to get properties about the current font. Note we don't bother calling IsValidFont() to see
	// if this font has been loaded since GetFontProperties() implements that check
	if(!videoManager->GetFontProperties(fontName, _fontProperties))
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: TextBox::SetFont() failed because GameVideo::GetFontProperties() returned false for the font:\n" << fontName << endl;
		return false;
	}

	_font = fontName;
	_initialized = IsInitialized(_initializeErrors);
	
	return true;
}


//-----------------------------------------------------------------------------
// GetFont: returns the name of the font used for this textbox
//-----------------------------------------------------------------------------
string TextBox::GetFont()
{
	return _font;
}


//-----------------------------------------------------------------------------
// SetDisplayMode: sets up the display mode for this textbox, e.g. one char
//                 at a time, one line at a time, etc.
//                 This MUST be called before rendering any text since the
//                 default display mode is VIDEO_TEXT_INVALID
//-----------------------------------------------------------------------------

bool TextBox::SetDisplayMode(const TextDisplayMode &mode)
{
	if(mode <= VIDEO_TEXT_INVALID || mode > VIDEO_TEXT_TOTAL)
	{
		cerr << "VIDEO ERROR: TextBox::SetDisplayMode() failed because a mode with the value of " << mode << " was passed in!" << endl;
		return false;
	}
	
	_mode = mode;
	_initialized = IsInitialized(_initializeErrors);
	return true;
}


//-----------------------------------------------------------------------------
// GetDisplayMode: returns the current display mode
//-----------------------------------------------------------------------------

TextDisplayMode TextBox::GetDisplayMode()
{
	return _mode;
}


//-----------------------------------------------------------------------------
// SetDisplaySpeed: sets the current display speed for this textbox. The unit
//                  is characters per second. For display modes which are based
//                  on one line at a time, we assume 30 characters per line, so
//                  a display speed of 10 means 3 seconds per line. Although we
//                  could make it so that the display speed corresponds to characters
//                  per second for display modes which use characters, and lines
//                  per second for display modes which use lines, it's easier
//                  to have a consistent unit.
//
//                  Returns false if displaySpeed is negative or zero.
//-----------------------------------------------------------------------------

bool TextBox::SetDisplaySpeed(float displaySpeed)
{
	if(displaySpeed <= 0.0f)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: TextBox::SetDisplaySpeed() failed, tried to set a display speed of " << displaySpeed << endl;
		return false;
	}
	
	_displaySpeed = displaySpeed;
	_initialized = IsInitialized(_initializeErrors);
	return true;
}


//-----------------------------------------------------------------------------
// GetDisplaySpeed: return the current display speed
//-----------------------------------------------------------------------------

float TextBox::GetDisplaySpeed()
{
	return _displaySpeed;
}


//-----------------------------------------------------------------------------
// IsFinished: returns true if the textbox is done scrolling text and false
//             if we haven't finished the gradual text drawing that was started
//             when ShowText() was called. If SetText() hasn't been called yet, 
//             this is false by default.
//-----------------------------------------------------------------------------

bool TextBox::IsFinished()
{
	return _finished;
}


//-----------------------------------------------------------------------------
// ForceFinish: force the textbox to complete its current text scrolling. If
//              the textbox is empty (no text to display), this returns false
//-----------------------------------------------------------------------------

bool TextBox::ForceFinish()
{
	if(IsEmpty())
		return false;
		
	_finished = true;
	return true;
}


//-----------------------------------------------------------------------------
// ShowText: shows some text in the textbox, gradually scrolling it on to the
//           screen depending on the current text display mode.
//           Returns false if the textbox isn't properly initialized,
//           if the string passed is empty, or if the text doesn't fit in the box
//-----------------------------------------------------------------------------

bool TextBox::ShowText(const hoa_utils::ustring &text)
{
	// fail if empty string was passed
	if(text.empty())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: empty string passed to TextBox::ShowText()!" << endl;
		return false;
	}
	
	
	// fail if textbox isn't initialized properly
	if(!_initialized)
	{
		if(VIDEO_DEBUG)
			cerr << "TextBox::ShowText() failed because the textbox was not initialized:" << endl << _initializeErrors << endl;
			
		return false;		
	}
	
	
	bool success = true;
	
		
	// go through the text string, examining one line at a time and adding it
	// to the _text vector.
	
	size_t newlinePos;
	ustring tempStr = text;
	uint16 newline = static_cast<uint16>('\n');

	_text.clear();	
	
	do
	{
		newlinePos = tempStr.find(newline);
		
		// if there's no newline, just add the whole string
		// otherwise, add the part up to the newline and 
		if(newlinePos == ustring::npos)
		{
			AddLine(tempStr);
			break;
		}
		else
		{
			AddLine(tempStr.substr(0, newlinePos));
			tempStr = tempStr.substr(newlinePos+1, tempStr.length() - newlinePos);
		}		
	} while(newlinePos != ustring::npos);
			
	// calculate the height of the text and check it against the height of
	// the textbox. If we're out of room, spit out an error message and return
	// false, but still allow the font to render since it's not an unrecoverable
	// error.
	
	int32 textHeight = CalculateTextHeight();
	
	if(textHeight > _height)
	{
		if(VIDEO_DEBUG)
		{
			cerr << "VIDEO ERROR: Error in TextBox::ShowText()! Tried to display text of height (";
			cerr << textHeight << ")" << endl << "in a window of only height (" << _height << ")" << endl;
		}
			
		success = false;
	}
	
	// reset variables to indicate a new text display is in progress
	_currentTime = 0;
	
	// note, instant text display is basically always "finished" ;)
	if(_mode == VIDEO_TEXT_INSTANT)
		_finished = true;
	else
		_finished = false;

	return success;
}


//-----------------------------------------------------------------------------
// Clear: makes the textbox empty so it doesn't display any text.
//-----------------------------------------------------------------------------

bool TextBox::Clear()
{
	_finished = true;
	_text.clear();
	return true;
}


//-----------------------------------------------------------------------------
// IsEmpty: returns true if the textbox is currently blank
//-----------------------------------------------------------------------------

bool TextBox::IsEmpty()
{
	return _text.empty();
}


//-----------------------------------------------------------------------------
// IsInitialized: validates all members to make sure the textbox is completely
//                initialized and ready to show text. If anything is wrong,
//                we return false, and fill the "errors" string with a list of
//                errors so they can be printed to the console
//-----------------------------------------------------------------------------

bool TextBox::IsInitialized(string &errors)
{
	bool success = true;
	
	errors.clear();
	
	// check width
	if(_width <= 0.0f || _width > 1024.0f)
	{
		ostringstream s;
		s << "* Invalid width (" << _width << ")" << endl;
		errors += s.str();
		success = false;
	}
	
	// check height
	if(_height <= 0.0f || _height > 768.0f)
	{
		ostringstream s;
		s << "* Invalid height (" << _height << ")" << endl;
		errors += s.str();
		success = false;
	}
	
	// check display speed
	if(_displaySpeed <= 0.0f)
	{
		ostringstream s;
		s << "* Invalid display speed (" << _displaySpeed << ")" << endl;
		errors += s.str();
		success = false;
	}
	
	// check alignment flags
	if(_xalign < -1 || _xalign > 1)
	{
		ostringstream s;
		s << "* Invalid x align (" << _xalign << ")" << endl;
		errors += s.str();
		success = false;
	}
	
	if(_yalign < -1 || _yalign > 1)
	{
		ostringstream s;
		s << "* Invalid y align (" << _yalign << ")" << endl;
		errors += s.str();
		success = false;
	}
	
	
	// check font
	if(_font.empty())
	{
		ostringstream s;
		s << "* Invalid font (none has been set)" << endl;
		errors += s.str();
		success = false;		
	}

	
	// check display mode
	if(_mode <= VIDEO_TEXT_INVALID || _mode > VIDEO_TEXT_TOTAL)
	{
		ostringstream s;
		s << "* Invalid display mode (" << _mode << ")" << endl;
		errors += s.str();
		success = false;
	}

	_initialized = success;	
	return success;
}


//-----------------------------------------------------------------------------
// CalculateTextHeight: calculates the height of the text as it would be rendered
//                      with the set font
//
// Note: this is a pretty low level function so it doesn't do any checking
//       to see if the current font is actually valid
//-----------------------------------------------------------------------------

int32 TextBox::CalculateTextHeight()
{
	if(_text.empty())
		return 0;
	else
		return _fontProperties.height + _fontProperties.lineskip * ((int32)_text.size()-1);
}


//-----------------------------------------------------------------------------
// AddLine: adds new line to the _text vector. If the line is too long, it
//          word wraps and creates new lines
//-----------------------------------------------------------------------------

void TextBox::AddLine(const hoa_utils::ustring &line)
{
	// perform word wrapping in a loop until all the text is added
	
	ustring tempLine = line;
	
	GameVideo *videoManager = GameVideo::GetReference();
	
	while(!tempLine.empty())
	{
		int32 textWidth = videoManager->CalculateTextWidth(_font, line);
		
		// check if the text can fit. If so, push the whole thing and return
		if(textWidth < _width)
		{
			_text.push_back(tempLine);
			return;
		}
		
		// the text didn't fit, so find the maximum number of words which CAN
		// fit and create a line out of them. Note that to distinguish between
		// word boundaries we can't simply search for a space character (' '), 
		// because perhaps some languages use a different unicode character for
		// spaces, or perhaps some languages don't even have spaces! (who knows)
		// so we use a function that tells us where it's allowable to create a line break
		
		ustring wrappedLine;		
		int32 numWrappedChars = 0;
		int32 lastBreakableIndex = -1;
		int32 lineLength = (int32)tempLine.length();
		
		while(numWrappedChars < lineLength)
		{
			wrappedLine += tempLine[numWrappedChars];
			
			if(IsBreakableChar(tempLine[numWrappedChars]))
			{
				int32 textWidth = videoManager->CalculateTextWidth(_font, wrappedLine);
				
				if(textWidth < _width)
				{
					// if we haven't gone past the breaking point, then mark this as a
					// possible breaking point
					lastBreakableIndex = numWrappedChars;
				}
				else
				{
					// we went too far, so if there was a previous breaking point then
					// break off the string at that point. If not, it means we ran into
					// a really long word, so just break it off at this point
					
					if(lastBreakableIndex != -1)
						numWrappedChars = lastBreakableIndex;
					
					break;
				}
			}			

			++numWrappedChars;
		}
		
		textWidth = videoManager->CalculateTextWidth(_font, wrappedLine);		
		if(textWidth >= _width && lastBreakableIndex != -1)
		{
			numWrappedChars = lastBreakableIndex;
		}
		wrappedLine = tempLine.substr(0, numWrappedChars);
		
		// at this point, numWrappedChars tells us how many characters are going to be added
		// as a line. (or we could just do wrappedLine.length()). So, add this to the vector,
		// and truncate tempLine
		
		_text.push_back(wrappedLine);
		if(numWrappedChars == lineLength)
			return;
		
		tempLine = tempLine.substr(numWrappedChars + 1, lineLength - numWrappedChars);
	}
}


//-----------------------------------------------------------------------------
// IsBreakableChar: returns true if the given character can be broken upon for
//                  a line break. (for example in English, 0x20 (space) is
//                  okay to break on. It might vary in other languages)
//-----------------------------------------------------------------------------

bool TextBox::IsBreakableChar(uint16 character)
{
	if(character == 0x20)
		return true;
	
	return false;
}


//-----------------------------------------------------------------------------
// ShowText: non-unicode version
//-----------------------------------------------------------------------------

bool TextBox::ShowText(const std::string &text)
{
	ustring wstr = MakeWideString(text);	
	return ShowText(wstr);
}


}  // namespace hoa_video
