///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

#include "utils.h"
#include "video.h"
#include <sstream>
#include "menu_window.h"
#include "textbox.h"


using namespace std;
using namespace hoa_video;
using namespace hoa_video::private_video;
using hoa_utils::MakeUnicodeString;
using hoa_utils::ustring;


namespace hoa_video
{


//-----------------------------------------------------------------------------
// TextBox
//-----------------------------------------------------------------------------

TextBox::TextBox() 
{
	_finished = false;
	_currentTime = 0;
	_mode = VIDEO_TEXT_INVALID;
	_displaySpeed = 0.0f;
	_numChars = 0;
	_initialized = IsInitialized(_initialize_errors);
	_width = _height = 0.0f;
	_text_xalign = VIDEO_X_LEFT;
	_text_yalign = VIDEO_Y_BOTTOM;
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
	
	if(!_text.empty() && _currentTime > _endTime)
		_finished = true;
	
	return true;
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
		

	// fail if text box isn't initialized properly
	if(!_initialized)
	{
		if(VIDEO_DEBUG)
			cerr << "TextBox::Draw() failed because the textbox was not initialized:" << endl << _initialize_errors << endl;			
		return false;
	}
		
	// determine the rectangle of the textbox based on the position, dimensions,
	// and draw flags in the video engine
	
	float left, right, top, bottom;
	GameVideo *video = GameVideo::SingletonGetReference();
	
	video->_PushContext();
	
	video->SetDrawFlags(_xalign, _yalign, VIDEO_BLEND, 0);
	video->SetFont(_font);

	left   = 0.0f;
	right  = _width;
	bottom    = 0.0f;
	top    = _height;
	
	CalculateAlignedRect(left, right, bottom, top);
	
	int32 x, y, w, h;
	
	x = int32(left < right? left: right);
	y = int32(top < bottom? top : bottom);
	w = int32(right - left);
	if(w < 0) w = -w;
	h = int32(top - bottom);
	if(h < 0) h = -h;

	ScreenRect rect(x, y, w, h);

	if(_owner)
		rect.Intersect(_owner->GetScissorRect());
	rect.Intersect(video->GetScissorRect());
	video->EnableScissoring(_owner || video->IsScissoringEnabled());
	if(video->IsScissoringEnabled())
		video->SetScissorRect(rect);
	
	CoordSys &cs = video->_coord_sys;

	// figure out where the top of the rendered text is
	
	float textHeight = (float) _CalculateTextHeight();
	float textY;
	if(_yalign == VIDEO_Y_TOP)
	{
		// top alignment
		textY = top;
	}
	else if(_yalign == VIDEO_Y_CENTER)
	{
		textY = top - cs.GetVerticalDirection() * (_height - textHeight) * 0.5f;
	}
	else
	{
		// right alignment
		textY = top - cs.GetVerticalDirection() * (_height - textHeight);
	}
	
	
	float textX;
	
	// figure out X alignment
	if(_text_xalign == VIDEO_X_LEFT)
	{
		// left align
		textX = left;
	}
	else if(_text_xalign == VIDEO_X_CENTER)
	{
		// center align
		textX = (left + right) * 0.5f;
	}
	else
	{
		// right align
		textX = right;
	}
	
	video->Move(0.0f, textY);
	
	video->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, VIDEO_BLEND, 0);
	
	// draw the text line by line		

	_DrawTextLines(textX,textY, rect);
			
	video->_PopContext();
	return true;
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

	_initialized = IsInitialized(_initialize_errors);
	
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
// SetTextAlignment: sets the alignment flags to be used for the text
//                   returns false if any invalid alignment flag is passed
//-----------------------------------------------------------------------------

bool TextBox::SetTextAlignment(int32 xalign, int32 yalign)
{
	bool success = true;
	_text_xalign = xalign;
	_text_yalign = yalign;	
	
	_initialized = IsInitialized(_initialize_errors);
	
	return success;
}


//-----------------------------------------------------------------------------
// GetTextAlignment: returns the alignment flags into xalign and yalign
//-----------------------------------------------------------------------------

void TextBox::GetTextAlignment(int32 &xalign, int32 &yalign)
{
	xalign = _text_xalign;
	yalign = _text_yalign;
}


//-----------------------------------------------------------------------------
// SetFont: sets the font of this textbox. fontName must be the label of a valid
//          font loaded with LoadFont()
//-----------------------------------------------------------------------------

bool TextBox::SetFont(const string &fontName)
{	
	// try to get pointer to video manager
	GameVideo *videoManager = GameVideo::SingletonGetReference();
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
	_initialized = IsInitialized(_initialize_errors);
	
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
	if(mode <= VIDEO_TEXT_INVALID || mode >= VIDEO_TEXT_TOTAL)
	{
		cerr << "VIDEO ERROR: TextBox::SetDisplayMode() failed because a mode with the value of " << mode << " was passed in!" << endl;
		return false;
	}
	
	_mode = mode;
	_initialized = IsInitialized(_initialize_errors);
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
//                  on one line at a time, we assume VIDEO_CHARS_PER_LINE 
//                  characters per line, so say VIDEO_CHARS_PER_LINE is 30, and
//                  we have a display speed of 10. Then that means 3 seconds per line. 
//                  Although we could make it so that the display speed corresponds 
//                  to characters per second for display modes which use characters, 
//                  and lines per second for display modes which use lines, it's easier
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
	_initialized = IsInitialized(_initialize_errors);
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
//             when SetDisplayText() was called. If SetText() hasn't been called yet, 
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
// SetDisplayText: shows some text in the textbox, gradually scrolling it on to the
//           screen depending on the current text display mode.
//           Returns false if the textbox isn't properly initialized,
//           if the string passed is empty, or if the text doesn't fit in the box
//-----------------------------------------------------------------------------

bool TextBox::SetDisplayText(const hoa_utils::ustring &text)
{
	// fail if empty string was passed
	if(text.empty())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: empty string passed to TextBox::SetDisplayText()!" << endl;
		return false;
	}
	
	
	// fail if textbox isn't initialized properly
	if(!_initialized)
	{
		if(VIDEO_DEBUG)
			cerr << "TextBox::SetDisplayText() failed because the textbox was not initialized:" << endl << _initialize_errors << endl;
			
		return false;		
	}
	
	
	bool success = true;
	
		
	// go through the text string, examining one line at a time and adding it
	// to the _text vector.
	
	size_t newlinePos;
	ustring tempStr = text;
	uint16 newline = static_cast<uint16>('\n');

	_text.clear();	
	_numChars = 0;
	
	do
	{
		newlinePos = tempStr.find(newline);
		
		// if there's no newline, just add the whole string
		// otherwise, add the part up to the newline and 
		if(newlinePos == ustring::npos)
		{
			_AddLine(tempStr);
			break;
		}
		else
		{
			_AddLine(tempStr.substr(0, newlinePos));
			tempStr = tempStr.substr(newlinePos+1, tempStr.length() - newlinePos);
		}		
	} while(newlinePos != ustring::npos);
			
	// calculate the height of the text and check it against the height of
	// the textbox. If we're out of room, spit out an error message and return
	// false, but still allow the font to render since it's not an unrecoverable
	// error.
	
	int32 textHeight = _CalculateTextHeight();
	
	if(textHeight > _height)
	{
		if(VIDEO_DEBUG)
		{
			cerr << "VIDEO ERROR: Error in TextBox::SetDisplayText()! Tried to display text of height (";
			cerr << textHeight << ")" << endl << "in a window of only height (" << _height << ")" << endl;
		}
			
		success = false;
	}
	
	// reset variables to indicate a new text display is in progress
	_currentTime = 0;
	
	// figure out how much time the text will take to display
	
	switch(_mode)
	{
		case VIDEO_TEXT_INSTANT:    // instant
		{
			_endTime = 0;
			break;
		}		
		
		case VIDEO_TEXT_CHAR:       // One character at a time
		case VIDEO_TEXT_FADECHAR:
		case VIDEO_TEXT_REVEAL:
		{		
			// we want milliseconds per string
			// displaySpeed is characters per second
			// numChars is characters per string
			// 1000 is milliseconds per second
			
			_endTime = int32(1000.0f * _numChars / _displaySpeed);
			break;
		}

		case VIDEO_TEXT_FADELINE:   // One line at a time
		{
			// same calculation as one character at a time, except instead of _numChars,
			// we use number of lines, times VIDEO_CHARS_PER_LINE
			
			_endTime = int32(1000.0f * (_text.size() * VIDEO_CHARS_PER_LINE) / _displaySpeed);			
			break;
		}
		default:
		{
			_endTime = 0;
			if(VIDEO_DEBUG)		
				cerr << "VIDEO ERROR: undetected display mode in TextBox::SetDisplayText()!" << endl;
			break;
		}	
	};
	
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
	_numChars = 0;
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
	ostringstream s;
	
	// check width
	if(_width <= 0.0f || _width > 1024.0f)
		s << "* Invalid width (" << _width << ")" << endl;
	
	// check height
	if(_height <= 0.0f || _height > 768.0f)
		s << "* Invalid height (" << _height << ")" << endl;
	
	// check display speed
	if(_displaySpeed <= 0.0f)
		s << "* Invalid display speed (" << _displaySpeed << ")" << endl;
	
	// check alignment flags
	if(_text_xalign < VIDEO_X_LEFT || _text_xalign > VIDEO_X_RIGHT)
		s << "* Invalid x align (" << _text_xalign << ")" << endl;
	
	if(_text_yalign < VIDEO_Y_TOP || _text_yalign > VIDEO_Y_BOTTOM)
		s << "* Invalid y align (" << _text_yalign << ")" << endl;
	
	// check font
	if(_font.empty())
		s << "* Invalid font (none has been set)" << endl;
	
	// check display mode
	if(_mode <= VIDEO_TEXT_INVALID || _mode >= VIDEO_TEXT_TOTAL)
		s << "* Invalid display mode (" << _mode << ")" << endl;

	_initialized = success;	
	return success;
}


//-----------------------------------------------------------------------------
// _CalculateTextHeight: calculates the height of the text as it would be rendered
//                       with the set font
//
// Note: this is a pretty low level function so it doesn't do any checking
//       to see if the current font is actually valid
//-----------------------------------------------------------------------------
int32 TextBox::_CalculateTextHeight()
{
	if(_text.empty())
		return 0;
	else
		return _fontProperties.height + _fontProperties.lineskip * ((int32)_text.size()-1);
}


//-----------------------------------------------------------------------------
// _AddLine: adds new line to the _text vector. If the line is too long, it
//           word wraps and creates new lines
//-----------------------------------------------------------------------------
void TextBox::_AddLine(const hoa_utils::ustring &line)
{
	// perform word wrapping in a loop until all the text is added
	
	ustring tempLine = line;
	
	GameVideo *videoManager = GameVideo::SingletonGetReference();
	
	while(!tempLine.empty())
	{
		int32 textWidth = videoManager->CalculateTextWidth(_font, line);
		
		// check if the text can fit. If so, push the whole thing and return
		if(textWidth < _width)
		{
			_text.push_back(tempLine);
			_numChars += (int32) tempLine.size();
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
			
			if(_IsBreakableChar(tempLine[numWrappedChars]))
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
		_numChars += (int32) wrappedLine.size();
		if(numWrappedChars == lineLength)
			return;
		
		tempLine = tempLine.substr(numWrappedChars + 1, lineLength - numWrappedChars);
	}
}


//-----------------------------------------------------------------------------
// _IsBreakableChar: returns true if the given character can be broken upon for
//                   a line break. (for example in English, 0x20 (space) is
//                   okay to break on. It might vary in other languages)
//-----------------------------------------------------------------------------

bool TextBox::_IsBreakableChar(uint16 character)
{
	if(character == 0x20)
		return true;
	
	return false;
}


//-----------------------------------------------------------------------------
// SetDisplayText: non-unicode version
//-----------------------------------------------------------------------------

bool TextBox::SetDisplayText(const std::string &text)
{
	ustring wstr = MakeUnicodeString(text);
	return SetDisplayText(wstr);
}


//-----------------------------------------------------------------------------
// _DrawTextLines: does the dirtywork of actually drawing text, taking the
//                 display mode into consideration
//-----------------------------------------------------------------------------

void TextBox::_DrawTextLines(float textX, float textY, ScreenRect scissorRect)
{
	GameVideo *video = GameVideo::SingletonGetReference();
	CoordSys  &cs    = video->_coord_sys;
	
	int32 numCharsDrawn = 0;	
	
	TextDisplayMode mode = _mode;
	
	if(_finished)
		mode = VIDEO_TEXT_INSTANT;	
	
	// calculate the fraction of the text to display
	float percentComplete; 
	if(_finished)
		percentComplete = 1.0f;
	else
		percentComplete = (float)_currentTime / (float)_endTime;
	
	for(int32 line = 0; line < (int32)_text.size(); ++line)
	{	
		float xOffset;
		
		// calculate the xOffset for this line
		float lineWidth = (float) video->CalculateTextWidth(_font, _text[line]);
	
		int32 xAlign = video->_ConvertXAlign(_text_xalign);
				
		xOffset = textX + ((xAlign + 1) * lineWidth) * 0.5f * -cs.GetHorizontalDirection();
		video->MoveRelative(xOffset, 0.0f);	
	
		int32 lineSize = (int32) _text[line].size();
		
		switch(mode)
		{
			case VIDEO_TEXT_INSTANT:
			{
				video->DrawText(_text[line]);
				break;
			}
			case VIDEO_TEXT_CHAR:
			{
				// figure out which character is currently being rendered
				int32 curChar = int32(percentComplete * _numChars);
				
				if(numCharsDrawn + lineSize < curChar)
				{
					// if this line is before the current character, just render the whole thing
					video->DrawText(_text[line]);
				}
				else
				{
					// if this line contains the current character, then we need to figure out
					// how many characters it can draw until reaching curChar, and then only
					// draw that many characters
					
					int32 numCompletedChars = curChar - numCharsDrawn;
					
					if(numCompletedChars > 0)
					{					
						ustring substring = _text[line].substr(0, numCompletedChars);					
						video->DrawText(substring);
					}
				}				
				
				break;
			}
			case VIDEO_TEXT_FADECHAR:
			{
				// figure out which character is currently being rendered
				float fCurChar = percentComplete * _numChars;
				int32 curChar = int32(fCurChar);
				float curPct  = fCurChar - curChar;
				
				if(numCharsDrawn + lineSize <= curChar)
				{
					// if this line is before the current character, just render the whole thing
					video->DrawText(_text[line]);
				}
				else
				{
					// if this line contains the current character, then we need to draw all
					// the characters before curChar at full alpha, and then draw curChar
					// based on the time	
					
					int32 numCompletedChars = curChar - numCharsDrawn;
					
					// check if this line is even visible at all yet
					if(numCompletedChars >= 0)
					{
						ustring substring;
						
						// if we have already drawn some characters on this line, draw them
						if(numCompletedChars > 0)
						{
							substring = _text[line].substr(0, numCompletedChars);
							video->DrawText(substring);
						}
						
						// now draw the current character from this line (faded)
						Color oldColor = video->GetTextColor();
						Color newColor = oldColor;
						newColor[3] *= curPct;
						
						video->SetTextColor(newColor);
						video->MoveRelative((float)video->CalculateTextWidth(_font, substring), 0.0f);
						video->DrawText(_text[line].substr(numCompletedChars, 1));
						video->SetTextColor(oldColor);
					}
				}				
				
				break;
			}

			case VIDEO_TEXT_FADELINE:
			{
				// figure out which character is currently being rendered
				float fLines = percentComplete * _text.size();
				int32 lines  = int32(fLines);
				float curPct = fLines - lines;
				
				if(line < lines)
				{
					// if this line is before the current character, just render the whole thing
					video->DrawText(_text[line]);
				}
				else if(line == lines)
				{
					Color oldColor = video->GetTextColor();
					Color newColor = oldColor;
					newColor[3] *= curPct;
					
					video->SetTextColor(newColor);
					//video->MoveRelative(video->CalculateTextWidth(_font, _text[line]), 0.0f);
					video->DrawText(_text[line]);						
					video->SetTextColor(oldColor);
				}				
				
				break;
			}
			
			case VIDEO_TEXT_REVEAL:
			{			
				// figure out which character is currently being rendered
				float fCurChar = percentComplete * _numChars;
				int32 curChar = int32(fCurChar);
				float curPct  = fCurChar - curChar;
				
				if(numCharsDrawn + lineSize <= curChar)
				{
					// if this line is before the current character, just render the whole thing
					video->DrawText(_text[line]);
				}
				else
				{
					// if this line contains the current character, then we need to draw all
					// the characters before curChar at full alpha, and then draw curChar
					// based on the time	
					
					int32 numCompletedChars = curChar - numCharsDrawn;
					
					// check if this line is even visible at all yet
					if(numCompletedChars >= 0)
					{
						ustring substring;
						
						// if we have already drawn some characters on this line, draw them
						if(numCompletedChars > 0)
						{
							substring = _text[line].substr(0, numCompletedChars);
							video->DrawText(substring);
						}
						
						// now draw the current character from this line (scissored)
						
						ustring curCharString = _text[line].substr(numCompletedChars, 1);						

						// rectangle of the current character, in window coordinates
						int32 charX, charY, charW, charH;
						charX = int32(xOffset + cs.GetHorizontalDirection() * video->CalculateTextWidth(_font, substring));
						charY = int32(textY - cs.GetVerticalDirection() * (_fontProperties.height + _fontProperties.descent));
						
						if(cs.GetHorizontalDirection() < 0.0f)
							charY = int32(cs.GetBottom()) - charY;

						if(cs.GetVerticalDirection() < 0.0f)
							charX = int32(cs.GetLeft()) - charX;
							
						charW = video->CalculateTextWidth(_font, curCharString);
						charH = _fontProperties.height;
						
						// multiply width by percentage done
						charW = int32(curPct * charW);						
						video->MoveRelative(cs.GetHorizontalDirection() * video->CalculateTextWidth(_font, substring), 0.0f);
						
						video->PushState();
						ScreenRect charScissorRect(charX, charY, charW, charH);
						scissorRect.Intersect(charScissorRect);
						video->EnableScissoring(true);
						video->SetScissorRect(scissorRect);
						video->DrawText(curCharString);
						video->PopState();
					}
				}				
				
				break;
			}
			
			default:
			{
				video->DrawText(_text[line]);
				
				if(VIDEO_DEBUG)
				{
					cerr << "VIDEO ERROR: Unsupported text display mode (" << mode << ") found in TextBox::_DrawTextLines!" << endl;
				}
				break;
			}
		};


		numCharsDrawn += lineSize;
		//video->MoveRelative(-xOffset, _fontProperties.lineskip * -cs._upDir);
		
		textY += _fontProperties.lineskip * -cs.GetVerticalDirection();
		video->Move(0.0f, textY);
	}
}


}  // namespace hoa_video
