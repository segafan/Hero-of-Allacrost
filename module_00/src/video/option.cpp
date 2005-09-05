#include "utils.h"
#include "video.h"
#include "gui.h"
#include <sstream>

using namespace std;
using namespace hoa_video;
using namespace hoa_video::private_video;
using namespace hoa_utils;


namespace hoa_video
{


//-----------------------------------------------------------------------------
// OptionBox
//-----------------------------------------------------------------------------

OptionBox::OptionBox()
: _initialized(false),
  _xalign(VIDEO_X_LEFT),
  _yalign(VIDEO_Y_TOP),
  _selectMode(VIDEO_SELECT_SINGLE),
  _switching(false),
  _wrapping(false),
  _cursorState(VIDEO_CURSOR_STATE_VISIBLE),
  _events(0),
  _selection(-1),
  _switchSelection(-1),
  _numOptions(0),
  _scrolling(0),
  _cursorX(0.0f),
  _cursorY(0.0f)
{
	_x = _y = 0.0f;
	_hSpacing = _vSpacing = 0.0f;
	_numRows = _numColumns = 0;
	_initialized = IsInitialized(_initializeErrors);
}


//-----------------------------------------------------------------------------
// SetFont: sets the font for this option box. Returns false if an invalid
//          font label is passed in.
//-----------------------------------------------------------------------------

bool OptionBox::SetFont(const std::string &fontName)
{
	// try to get pointer to video manager
	GameVideo *videoManager = GameVideo::GetReference();
	if(!videoManager)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: OptionBox::SetFont() failed, couldn't get pointer to GameVideo!" << endl;
		return false;
	}

	// try to get properties about the current font. Note we don't bother calling IsValidFont() to see
	// if this font has been loaded since GetFontProperties() implements that check
	if(!videoManager->GetFontProperties(fontName, _fontProperties))
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: OptionBox::SetFont() failed because GameVideo::GetFontProperties() returned false for the font:\n" << fontName << endl;
		return false;
	}

	_font = fontName;
	_initialized = IsInitialized(_initializeErrors);

	return true;
}


//-----------------------------------------------------------------------------
// HandleLeftKey
//-----------------------------------------------------------------------------

void OptionBox::HandleLeftKey()
{
	if(_numColumns > 1)
		_ChangeSelection(-1);
}


//-----------------------------------------------------------------------------
// _ChangeSelection: helper function to change the current selection when
//                   the arrow keys are pressed. The offset parameter tells
//                   how much to change the current selection by.
//-----------------------------------------------------------------------------

void OptionBox::_ChangeSelection(int32 offset)
{
	bool selectionChanged = true;

	_selection += offset;
	
	if(_wrapping)
	{
		if(_selection < 0)
			_selection += _numOptions;
		else if(_selection >= _numOptions)
			_selection -= _numOptions;			
	}	
	else if(_selection < 0 || _selection >= _numOptions)
	{
		_selection -= offset;
		selectionChanged = false;
	}

	if(selectionChanged)
	{
		_PlaySelectSound();
		_events |= VIDEO_OPTION_SELECTION_CHANGE;
	}
}


//-----------------------------------------------------------------------------
// HandleUpKey
//-----------------------------------------------------------------------------

void OptionBox::HandleUpKey()
{
	if(_numRows > 1)
		_ChangeSelection(-_numColumns);
}


//-----------------------------------------------------------------------------
// HandleDownKey
//-----------------------------------------------------------------------------

void OptionBox::HandleDownKey()
{
	if(_numRows > 1)
		_ChangeSelection(_numColumns);
}

//-----------------------------------------------------------------------------
// HandleRightKey
//-----------------------------------------------------------------------------

void OptionBox::HandleRightKey()
{
	if(_numColumns > 1)
		_ChangeSelection(1);
}


//-----------------------------------------------------------------------------
// SetCursorState: sets the cursor to shown, hidden, or blinking
//-----------------------------------------------------------------------------

bool OptionBox::SetCursorState(CursorState state)
{
	if(state <= VIDEO_CURSOR_STATE_INVALID || state >= VIDEO_CURSOR_STATE_TOTAL)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: Invalid cursor state passed to OptionBox::SetCursorState (" << state << ")" << endl;
		return false;
	}
	
	_cursorState = state;
	return true;
}


//-----------------------------------------------------------------------------
// SetCursorOffset: pixel offset to show cursor at. Usually this should be a
//                  little to the left of each text item
//-----------------------------------------------------------------------------

bool OptionBox::SetCursorOffset(float x, float y)
{
	_cursorX = x;
	_cursorY = y;
	
	return true;
}


//-----------------------------------------------------------------------------
// HandleCancelKey
//-----------------------------------------------------------------------------

void OptionBox::HandleCancelKey()
{
	// if we're in switch mode and cancel key is hit, get out of switch mode
	// but don't send the cancel event since player still might want to select
	// something.
	
	if(_switchSelection >= 0)
		_switchSelection = -1;
	else
	{
		// send cancel event
		_events |= VIDEO_OPTION_CANCEL;
	}
	
	_PlaySelectSound();  // for now, cancel sounds the same as a selection
}


//-----------------------------------------------------------------------------
// HandleConfirmKey
//-----------------------------------------------------------------------------

void OptionBox::HandleConfirmKey()
{	
	// check that a valid option is selected	
	if(_selection < 0 || _selection >= _numOptions)
		return;		
	if(_options[_selection].disabled)
	{
		// play an annoying beep to tell player they clicked on a disabled option
		_PlayNoConfirmSound();
		return;
	}

	// case 1: switching 2 different elements
	if(_switchSelection >= 0 && _selection != _switchSelection)
	{
		// perform the actual switch
		_SwitchItems();
		
		// send a switch event
		_events |= VIDEO_OPTION_SWITCH;
		_PlaySwitchSound();
	}
	
	// case 2: partial confirm (confirming the first element in a double confirm)
	else if(_selectMode == VIDEO_SELECT_DOUBLE)
	{
		// mark the item that is getting switched	
		_switchSelection = _selection;
	}
	
	// case 3: confirm
	else
	{
		_events |= VIDEO_OPTION_CONFIRM;
		_PlayConfirmSound();
	}
	
	// get out of switch mode
	_switchSelection = -1;
}


//-----------------------------------------------------------------------------
// _SwitchItems: helper function to HandleConfirmKey, which does the dirty work
//               of switching two items.
//-----------------------------------------------------------------------------

void OptionBox::_SwitchItems()
{
	// switch the two options within the vector
	
	Option temp = _options[_selection];
	_options[_selection]       = _options[_switchSelection];
	_options[_switchSelection] = temp;
	
	// set _switchSelection to -1, so that we know we're not in switching mode
	// any more
	
	_switchSelection = -1;
}


//-----------------------------------------------------------------------------
// SetPosition: sets the position of the option box as a whole. Note that
//              the video engine's alignment flags will affect the positioning
//              at the point that OptionBox::Draw() is called, as will the
//              current coordinate system
//-----------------------------------------------------------------------------

void OptionBox::SetPosition(float x, float y)
{
	_x = x;
	_y = y;
}


//-----------------------------------------------------------------------------
// SetHorizontalSpacing: sets the pixel width of each "cell" in the option box
//-----------------------------------------------------------------------------

void OptionBox::SetHorizontalSpacing(float hSpacing)
{
	_hSpacing = hSpacing;
	_initialized = IsInitialized(_initializeErrors);
}


//-----------------------------------------------------------------------------
// SetVerticalSpacing: sets the pixel height of each "cell" in the option box
//-----------------------------------------------------------------------------

void OptionBox::SetVerticalSpacing(float vSpacing)
{
	_vSpacing = vSpacing;
	_initialized = IsInitialized(_initializeErrors);
}


//-----------------------------------------------------------------------------
// SetSize: sets the number of columns and rows in this option box
//-----------------------------------------------------------------------------

void OptionBox::SetSize(int32 columns, int32 rows)
{
	_numColumns = columns;
	_numRows    = rows;
	_initialized = IsInitialized(_initializeErrors);
}


//-----------------------------------------------------------------------------
// SetOptionAlignment: within each cell, how the text and cursor should be 
//                     aligned for option labels. Values can be:
//                     VIDEO_X_LEFT, VIDEO_X_CENTER, VIDEO_X_RIGHT
//                     VIDEO_Y_TOP, VIDEO_Y_CENTER, VIDEO_Y_BOTTOM
//-----------------------------------------------------------------------------

void OptionBox::SetOptionAlignment(int32 xalign, int32 yalign)
{
	_xalign = xalign;
	_yalign = yalign;
	_initialized = IsInitialized(_initializeErrors);
}


//-----------------------------------------------------------------------------
// SetSelectMode: can be VIDEO_SELECT_SINGLE or VIDEO_SELECT_DOUBLE. This
//                means that you can select an option by pressing confirm once
//                or by pressing it twice.
//-----------------------------------------------------------------------------

void OptionBox::SetSelectMode(SelectMode mode)
{
	_selectMode = mode;
	_initialized = IsInitialized(_initializeErrors);
}


//-----------------------------------------------------------------------------
// EnableSwitching: enable/disable clicking on an item and then clicking on another
//                  item to swap them. If double confirm selection mode is enabled,
//                  it is probably most intuitive to enable switching as well.
//-----------------------------------------------------------------------------

void OptionBox::EnableSwitching(bool enable)
{
	_switching = enable;
}


//-----------------------------------------------------------------------------
// EnableWrapping: if true is passed, then going past the end of the option box
//                 takes you back to the beginning, and vica versa.
//-----------------------------------------------------------------------------

void OptionBox::EnableWrapping(bool enable)
{
	_wrapping = enable;
}


//-----------------------------------------------------------------------------
// SetSelection: after you create a new option box, the default selection is 
//               -1 (i.e. nothing selected) but usually you want the first
//               option to be selected so after you call SetOptions(), call
//               SetSelection(0). Note that if you call SetSelection(0) before
//               calling SetOptions(), this will return false because there is
//               no element "0" yet.
//-----------------------------------------------------------------------------

bool OptionBox::SetSelection(int32 index)
{
	if(index < -1 || index >= _numOptions)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: OptionBox::SetSelection() was passed invalid index (" << index << ")" << endl;
		return false;
	}
	_selection = index;
	return true;
}


//-----------------------------------------------------------------------------
// SetOptions: this is the important function for setting the options to be
//             displayed for this option box. If you don't want labels next
//             to the options, just pass a vector of empty strings
//
//             If this function fails (perhaps due to a formatting error in
//             the text), it returns false and clears out the internal
//             _options vector
//-----------------------------------------------------------------------------

bool OptionBox::SetOptions(const vector<ustring> &formatText)
{
	_options.clear();
	_numOptions = 0;

	vector<ustring>::const_iterator iText = formatText.begin();
	vector<ustring>::const_iterator iEnd  = formatText.end();
	
	while(iText != iEnd)
	{
		const ustring &str = *iText;
		
		if(!_AddOption(str))
		{
			_options.clear();
			_numOptions = 0;
			if(VIDEO_DEBUG)
				cerr << "VIDEO ERROR: OptionBox::SetOptions() failed. Invalid format string." << endl;
			return false;
		}
		
		++_numOptions;		
		++iText;
	}
	
	return true;
}


//-----------------------------------------------------------------------------
// EnableOption: enables or disables the option with the given index
//-----------------------------------------------------------------------------

bool OptionBox::EnableOption(int32 index, bool enable)
{
	if(index < 0 || index >= _numOptions)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: Invalid index passed to OptionBox::EnableOption() (" << index << ")" << endl;
		return false;
	}

	_options[index].disabled = !enable;
	
	return true;
}


//-----------------------------------------------------------------------------
// Sort: sorts the list of options by the first word in the option text which
//       is not interrupted by a tag. For example, if the format string for
//       an option is "<20>Phoenix down<r>500 gold", then "Phoenix down" is the
//       string to sort by.
//-----------------------------------------------------------------------------

bool OptionBox::Sort()
{
	// STUB: I have not implemented this yet because apparently sorting
	//       unicode strings is extremely complicated. For example, in Portuguese
	//       maybe they use a similar alphabet, but the letters are in a different
	//       order :uhoh:
	return true;
}


//-----------------------------------------------------------------------------
// IsScrolling: returns true if the option box is in the middle of scrolling
//-----------------------------------------------------------------------------

bool OptionBox::IsScrolling()
{
	return _scrolling;
}


//-----------------------------------------------------------------------------
// GetEvents: returns bitfield of OptionBoxEvent's.
//-----------------------------------------------------------------------------

int32 OptionBox::GetEvents()
{
	int32 returnValue = _events;
	_events = 0;  // clear it out when it's checked
	
	return returnValue;
}


//-----------------------------------------------------------------------------
// GetSelection(): returns the current selection for the option box, with
//                 indices ranging from 0 to _numOptions - 1. If it is -1, then
//                 it means nothing is selected.
//-----------------------------------------------------------------------------

int32 OptionBox::GetSelection()
{
	return _selection;
}


//-----------------------------------------------------------------------------
// GetSwitchSelection: if the option box is a "double confirm" kind of box, and
//                     one item has been selected, this returns that item's
//                     index. Returns -1 if nothing is currently selected
//-----------------------------------------------------------------------------

int32 OptionBox::GetSwitchSelection()
{
	return _switchSelection;
}


//-----------------------------------------------------------------------------
// GetNumRows
//-----------------------------------------------------------------------------

int32 OptionBox::GetNumRows()
{
	return _numRows;
}


//-----------------------------------------------------------------------------
// GetNumColumns
//-----------------------------------------------------------------------------

int32 OptionBox::GetNumColumns()
{
	return _numColumns;
}


//-----------------------------------------------------------------------------
// GetNumOptions: returns number of options in the option box, based on
//                the size of the vector passed to SetOptions()
//-----------------------------------------------------------------------------

int32 OptionBox::GetNumOptions()
{
	return _numOptions;
}


//-----------------------------------------------------------------------------
// IsInitialized: validates all members to make sure the option box is completely
//                initialized and ready to show text. If anything is wrong,
//                we return false, and fill the "errors" string with a list of
//                errors so they can be printed to the console
//-----------------------------------------------------------------------------

bool OptionBox::IsInitialized(string &errors)
{
	bool success = true;
	
	errors.clear();
	ostringstream s;
	
	// check rows
	if(_numRows <= 0)
		s << "* Invalid number of rows (" << _numRows << ")" << endl;
	
	// check columns
	if(_numColumns <= 0)
		s << "* Invalid number of columns (" << _numColumns << ")" << endl;
	
	// check horizontal/vertical spacing
	if(_hSpacing <= 0.0f && _numColumns > 1)
		s << "* Invalid horizontal spacing (" << _hSpacing << ")" << endl;
	
	if(_vSpacing <= 0.0f && _numRows > 1)
		s << "* Invalid vertical spacing (" << _vSpacing << ")" << endl;

	// check alignment flags
	if(_xalign < VIDEO_X_LEFT || _xalign > VIDEO_X_RIGHT)
		s << "* Invalid x align (" << _xalign << ")" << endl;

	if(_yalign < VIDEO_Y_TOP || _yalign > VIDEO_Y_BOTTOM)
		s << "* Invalid y align (" << _yalign << ")" << endl;
	
	// check font
	if(_font.empty())
		s << "* Invalid font (none has been set)" << endl;
	
	// check selection mode
	if(_selectMode <= VIDEO_SELECT_INVALID || _selectMode >= VIDEO_SELECT_TOTAL)
		s << "* Invalid selection mode (" << _selectMode << ")" << endl;

	errors = s.str();
	
	if(!errors.empty())
		success = false;

	_initialized = success;	
	return success;
}


//-----------------------------------------------------------------------------
// _PlayConfirmSound: sound when user confirms an option
//-----------------------------------------------------------------------------

void OptionBox::_PlayConfirmSound()
{
}


//-----------------------------------------------------------------------------
// _PlayNoConfirmSound: sound when user tries to confirm on a disabled option
//-----------------------------------------------------------------------------

void OptionBox::_PlayNoConfirmSound()
{
}


//-----------------------------------------------------------------------------
// _PlaySelectSound: sound when selection changes
//-----------------------------------------------------------------------------

void OptionBox::_PlaySelectSound() 
{
}


//-----------------------------------------------------------------------------
// _PlaySwitchSound: sound when two options are switched
//-----------------------------------------------------------------------------

void OptionBox::_PlaySwitchSound() 
{
}


//-----------------------------------------------------------------------------
// _AddOption: helper function to SetOptions. Adds a new option to _options vector
//-----------------------------------------------------------------------------

bool OptionBox::_AddOption(const hoa_utils::ustring &formatString)
{
	Option op;
	
	// by default, option isn't disabled
	op.disabled = false;

	// if we want to show a text label for this option, process the label plus
	// any formatting tags that it contains
	
	if(!formatString.empty())
	{
		// copy it into a temporary string that we can manipulate
		ustring tmp = formatString;
		
		uint16 openTag = static_cast<uint16>('<');
		uint16 endTag  = static_cast<uint16>('>');
		
		uint16 c = static_cast<uint16>('c');
		uint16 r = static_cast<uint16>('r');
		uint16 l = static_cast<uint16>('l');

		uint16 C = static_cast<uint16>('C');
		uint16 R = static_cast<uint16>('R');
		uint16 L = static_cast<uint16>('L');
		
		while(!tmp.empty())
		{
			OptionElement opElem;

			if(tmp[0] == openTag)
			{
				// process a tag
				
				size_t length = tmp.length();
				
				if(length < 3)
				{
					// all formatting tags are at least 3 characters long because you
					// need the opening (<) and close (>) plus stuff in the middle.
					
					if(VIDEO_DEBUG)
						cerr << "VIDEO ERROR: In OptionBox::_AddOption(), tag opening detected with string size less than 3." << endl;
					return false;
				}
				
				size_t endPos = tmp.find(endTag);
				
				if(endPos == ustring::npos)
				{
					// uh oh! We have a tag beginning but never ending. This is an error
					if(VIDEO_DEBUG)
						cerr << "VIDEO ERROR: In OptionBox::_AddOption(), format string had unclosed tag" << endl;
					return false;
				}
				
				// check if it's any of the alignment tags. Note, it's safe to directly
				// check tmp[1] and tmp[2] since we already ensured the length of the string
				// is at least 3
				
				if((tmp[1] == c || tmp[1] == C) && tmp[2] == endTag)
				{
					opElem.type = VIDEO_OPTION_ELEMENT_CENTER_ALIGN;
				}
				else if((tmp[1] == r || tmp[1] == R) && tmp[2] == endTag)
				{
					opElem.type = VIDEO_OPTION_ELEMENT_RIGHT_ALIGN;
				}
				else if((tmp[1] == l || tmp[1] == L) && tmp[2] == endTag)
				{
					opElem.type = VIDEO_OPTION_ELEMENT_LEFT_ALIGN;
				}
				else
				{
					// it's not a 1-letter tag, so we need to find the substring
					// between the opening and closing brackets
					
					string tagText = MakeByteString(tmp.substr(1, endPos - 1));
					
					if(IsNumber(tagText))
					{
						// this must be a positioning tag
						int32 position = atoi(tagText.c_str());
						opElem.type  = VIDEO_OPTION_ELEMENT_POSITION;
						opElem.value = position;
					}
					else
					{
						// this must be an image tag
						
						ImageDescriptor imd;
						imd.SetFilename(tagText);
						GameVideo *videoManager = GameVideo::GetReference();
						
						if(videoManager->LoadImage(imd))
						{
							opElem.type  = VIDEO_OPTION_ELEMENT_IMAGE;
							opElem.value = static_cast<int32> (op.images.size());
							op.images.push_back(imd);
						}
						else
						{
							if(VIDEO_DEBUG)
								cerr << "VIDEO ERROR: invalid tag <" << tagText << "> found in OptionBox::_AddOption()!" << endl;

							return false;
						}
					}					
				}
				
				// done processing tag, update tmp string
				if(endPos == length - 1)
				{
					// end of string
					tmp.clear();
				}
				else
				{
					tmp = tmp.substr(endPos + 1, length - endPos - 1);
				}
			}
			else
			{
				opElem.type  = VIDEO_OPTION_ELEMENT_TEXT;
				opElem.value = static_cast<int32>(op.text.size());

				// find the distance until the next tag
				size_t tagBegin = tmp.find(openTag);
				
				if(tagBegin == ustring::npos)
				{
					// no tag found, this is a "pure string" with no formatting, so just add it
					op.text.push_back(tmp);
					tmp.clear();					
				}
				else
				{
					// if there is a tag starting somewhere ahead, strip all the text
					// before it and add it
					
					op.text.push_back(tmp.substr(0, tagBegin));
					tmp = tmp.substr(tagBegin, tmp.length() - tagBegin);
				}
			}
			
			op.elements.push_back(opElem);			
		}
	}

	_options.push_back(op);
	
	return true;
}


//-----------------------------------------------------------------------------
// ~OptionBox
//-----------------------------------------------------------------------------

OptionBox::~OptionBox()
{
	_ClearOptions();
}


//-----------------------------------------------------------------------------
// _ClearOptions: clear the options vector
//-----------------------------------------------------------------------------

void OptionBox::_ClearOptions()
{
	// firstly, go through each option and unreference any images it is
	// holding on to
	
	GameVideo *videoManager = GameVideo::GetReference();
	for(int32 j = 0; j < static_cast<int32>(_options.size()); ++j)
	{
		for(int32 i = 0; i < static_cast<int32>(_options[j].images.size()); ++i)
		{
			videoManager->DeleteImage(_options[j].images[i]);
		}
	}
	
	// now clear the vector
	
	_options.clear();
}


//-----------------------------------------------------------------------------
// Draw: draws the option box, taking the current x/y align flags in the
//       video engine into account, as well as the current coordinate system
//-----------------------------------------------------------------------------

bool OptionBox::Draw()
{
	// fail if option box isn't initialized properly
	if(!_initialized)
	{
		if(VIDEO_DEBUG)
			cerr << "OptionBox::Draw() failed because the option box was not initialized:" << endl << _initializeErrors << endl;			
		return false;		
	}

	float left, right, bottom, top;

	// calculate width and height of option box
	
	left   = 0.0f;
	bottom = 0.0f;
	right  = _numColumns * _hSpacing;
	top    = _numRows    * _vSpacing;

	_CalculateScreenRect(left, right, bottom, top);
	
	GameVideo *video = GameVideo::GetReference();
	CoordSys &cs = video->_coordSys;
	
	video->_PushContext();	
	video->SetFont(_font);
	
	video->SetDrawFlags(_xalign, _yalign, VIDEO_X_NOFLIP, VIDEO_Y_NOFLIP, VIDEO_NO_BLEND, 0);

	OptionCellBounds bounds;

	bounds.cellYTop    = top;
	bounds.cellYCenter = bounds.cellYTop - 0.5f * _vSpacing * cs._upDir;
	bounds.cellYBottom = bounds.cellYCenter * 2.0f - bounds.cellYTop;

	float yoff = -_vSpacing * cs._upDir;
	float xoff = _hSpacing * cs._rightDir;
	
	bool finished = false;
	
	// go through all the "cells" and draw them	
	for(int32 row = 0; row < _numRows; ++row)
	{
		bounds.cellXLeft   = left;
		bounds.cellXCenter = bounds.cellXLeft + 0.5f * _hSpacing * cs._rightDir;
		bounds.cellXRight  = bounds.cellXCenter * 2.0f - bounds.cellXLeft;
	
		for(int32 col = 0; col < _numColumns; ++col)
		{
			int32 index = row * _numColumns + col;
			
			if(index >= _numOptions)
			{
				finished = true;
				break;
			}			
			
			float leftEdge = 999999.0f;  // x offset to where the text actually begins			
			float x, y;
			
			int32 xalign = _xalign;
			int32 yalign = _yalign;
			
			_SetupAlignment(xalign, yalign, bounds, x, y);
					
			Option &op = _options[index];
			
			if(op.disabled)
				video->SetTextColor(Color::gray);
			else
				video->SetTextColor(Color::white);
				
			for(int32 opElem = 0; opElem < (int32) op.elements.size(); ++opElem)
			{				
				switch(op.elements[opElem].type)
				{
					case VIDEO_OPTION_ELEMENT_LEFT_ALIGN:
					{
						xalign = VIDEO_X_LEFT;
						_SetupAlignment(xalign, _yalign, bounds, x, y);
						break;
					}

					case VIDEO_OPTION_ELEMENT_CENTER_ALIGN:
					{
						xalign = VIDEO_X_CENTER;
						_SetupAlignment(xalign, _yalign, bounds, x, y);
						break;
					}

					case VIDEO_OPTION_ELEMENT_RIGHT_ALIGN:
					{
						xalign = VIDEO_X_RIGHT;
						_SetupAlignment(xalign, _yalign, bounds, x, y);
						break;
					}

					case VIDEO_OPTION_ELEMENT_IMAGE:
					{
						int32 imageIndex = op.elements[opElem].value;
						
						if(imageIndex >= 0 && imageIndex < (int32)op.images.size())
						{
							video->DrawImage(op.images[imageIndex]);

							float edge = x - bounds.cellXLeft;
							float width = op.images[imageIndex].GetWidth();
							if(xalign == VIDEO_X_CENTER)
								edge -= width * 0.5f * cs._rightDir;
							else if(xalign == VIDEO_X_RIGHT)
								edge -= width * cs._rightDir;
							
							if(edge < leftEdge)
								leftEdge = edge;
							

						}
						break;
					}

					case VIDEO_OPTION_ELEMENT_POSITION:
					{
						x = bounds.cellXLeft + op.elements[opElem].value * cs._rightDir;
						video->Move(x, y);
						break;
					}
					
					case VIDEO_OPTION_ELEMENT_TEXT:
					{
						int32 textIndex = op.elements[opElem].value;
												
						if(textIndex >= 0 && textIndex < (int32)op.text.size())
						{						
							const ustring &text = op.text[textIndex];
							float width = video->CalculateTextWidth(_font, text);
							float edge = x - bounds.cellXLeft;
							
							if(xalign == VIDEO_X_CENTER)
								edge -= width * 0.5f * cs._rightDir;
							else if(xalign == VIDEO_X_RIGHT)
								edge -= width * cs._rightDir;
							
							if(edge < leftEdge)
								leftEdge = edge;
							
							video->DrawText(text);
						}
						
						break;
					}
				};
			}


			// if this is the index where we are supposed to show the cursor, show it			
			if(index == _selection)
			{
				_SetupAlignment(VIDEO_X_LEFT, _yalign, bounds, x, y);
				video->SetDrawFlags(VIDEO_BLEND, 0);
				video->MoveRelative(_cursorX + leftEdge, _cursorY);
				ImageDescriptor *defaultCursor = video->GetDefaultCursor();
				
				if(defaultCursor)			
					video->DrawImage(*defaultCursor);				
			}
		
			bounds.cellXLeft   += xoff;
			bounds.cellXCenter += xoff;
			bounds.cellXRight  += xoff;	
		}
		
		if(finished)
			break;

		bounds.cellYTop    += yoff;
		bounds.cellYCenter += yoff;
		bounds.cellYBottom += yoff;		
	}
	
	video->_PopContext();
	return true;
}


void OptionBox::_SetupAlignment(int32 xalign, int32 yalign, const OptionCellBounds &bounds, float &x, float &y)
{	
	GameVideo *video = GameVideo::GetReference();
	video->SetDrawFlags(xalign, yalign, 0);
	
	switch(xalign)
	{
		case VIDEO_X_LEFT:
			x = bounds.cellXLeft;
			break;
		case VIDEO_X_CENTER:
			x = bounds.cellXCenter;
			break;			
		default:
			x = bounds.cellXRight;
			break;		
	};
	
	switch(yalign)
	{
		case VIDEO_Y_TOP:
			y = bounds.cellYTop;
			break;
		case VIDEO_Y_CENTER:
			y = bounds.cellYCenter;
			break;
		default:
			y = bounds.cellYBottom;
			break;
	};
	
	video->Move(x, y);
}


bool OptionBox::Update(int32 frameTime)
{
	return true;
} 

} // namespace hoa_video
