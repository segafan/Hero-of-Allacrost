///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

#include <sstream>

#include "utils.h"
#include "option.h"
#include "video.h"

using namespace std;

using namespace hoa_utils;
using namespace hoa_video;
using namespace hoa_video::private_video;

namespace hoa_video {


//-----------------------------------------------------------------------------
// OptionBox
//-----------------------------------------------------------------------------

OptionBox::OptionBox()
{
	_option_xalign = VIDEO_X_LEFT;
	_option_yalign = VIDEO_Y_TOP;
	_selectMode = VIDEO_SELECT_SINGLE;
	_switching = false;
	_hWrapMode = VIDEO_WRAP_MODE_NONE;
	_vWrapMode = VIDEO_WRAP_MODE_NONE;
	_cursorState = VIDEO_CURSOR_STATE_VISIBLE;
	_event = 0;
	_selection = -1;
	_switchSelection = -1;
	_firstSelection = -1;
	_numOptions = 0;
	_scrolling = 0;
	_scrollOffset = 0;
	_scrollStartOffset = 0;
	_scrollEndOffset = 0;
	_blinkTime = 0;
	_scrollTime = 0;
	_switchCursorX = -3;
	_switchCursorY = -3;
	_blink = false;
	_cursorX = 0.0f;
	_cursorY = 0.0f;
	_hSpacing = _vSpacing = 0.0f;
	_numRows = _numColumns = 0;
	_initialized = IsInitialized(_initialization_errors);
}


//-----------------------------------------------------------------------------
// SetFont: sets the font for this option box. Returns false if an invalid
//          font label is passed in.
//-----------------------------------------------------------------------------

void OptionBox::SetFont(const std::string &fontName)
{
	// try to get properties about the current font. Note we don't bother calling IsValidFont() to see
	// if this font has been loaded since GetFontProperties() implements that check
	if(VideoManager->GetFontProperties(fontName) == NULL)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: OptionBox::SetFont() failed because GameVideo::GetFontProperties() returned false for the font:\n" << fontName << endl;
		return;
	}

	_font = fontName;
	_initialized = IsInitialized(_initialization_errors);
}


//-----------------------------------------------------------------------------
// HandleLeftKey
//-----------------------------------------------------------------------------

void OptionBox::HandleLeftKey()
{
	// ignore input while scrolling, or if an event has already been logged this frame
	if(_scrolling || _event)
		return;

	if(!_ChangeSelection(-1, true))
		_event = VIDEO_OPTION_BOUNDS_LEFT;
}


//-----------------------------------------------------------------------------
// _ChangeSelection: helper function to change the current selection when
//                   the arrow keys are pressed. The offset parameter tells
//                   how much to change the current selection by.
//-----------------------------------------------------------------------------

bool OptionBox::_ChangeSelection(int32 offset, bool horizontal)
{
	// special case: if we have only one column, then the only way pressing
	//               left or right can cause the selection to change is
	//               by moving up or down (shifted).
	if(horizontal && _numColumns == 1 && _hWrapMode != VIDEO_WRAP_MODE_SHIFTED)
		return false;

	int32 row = _selection / _numColumns;
	int32 col = _selection % _numColumns;

	int32 totalRows = ((_numOptions-1+_numColumns) / _numColumns);

	// if scrolling is enabled (i.e. we have more rows we can possibly show)
	// then don't allow vertical wrapping
	if(totalRows > _numRows)
	{
		_vWrapMode = VIDEO_WRAP_MODE_NONE;
	}


	// **** case 1: horizontal change, wrapping is enabled ***************
	if(_numColumns > 1 && horizontal && _hWrapMode != VIDEO_WRAP_MODE_NONE)
	{
		if(offset == -1 && col == 0)                    // going too far to left
		{
			if(_hWrapMode == VIDEO_WRAP_MODE_STRAIGHT)
				offset += _numColumns;
			else if(_hWrapMode == VIDEO_WRAP_MODE_SHIFTED)
				if(row > 0 || _vWrapMode != VIDEO_WRAP_MODE_NONE)
					offset += _numOptions;
				else
					return false;
		}
		else if(offset == 1 && col == _numColumns - 1)  // going too far to right
		{
			if(_hWrapMode == VIDEO_WRAP_MODE_STRAIGHT)
				offset -= _numColumns;
			else if(_hWrapMode == VIDEO_WRAP_MODE_SHIFTED)
				if(row >= totalRows - 1 && _vWrapMode == VIDEO_WRAP_MODE_NONE)
					return false;
		}

		_selection = (_selection + offset) % _numOptions;

	}

	// **** case 2: vertical change, wrapping is enabled ***************
	else if(_numRows > 1 && !horizontal && _vWrapMode != VIDEO_WRAP_MODE_NONE)
	{
		// vertical wrapping

		if(offset < 0 && row == 0)                    // going too far up
		{
			if(_vWrapMode == VIDEO_WRAP_MODE_STRAIGHT)
				offset += _numOptions;
			else if(_vWrapMode == VIDEO_WRAP_MODE_SHIFTED)
			{
				offset += _numOptions;

				if(col == 0)
				{
					if(_hWrapMode != VIDEO_WRAP_MODE_NONE)
						offset += _numColumns - 1;
				}
				else
					--offset;
			}
		}
		else if(offset > 0 && row == _numRows - 1)  // going too far down
		{
			if(_vWrapMode == VIDEO_WRAP_MODE_SHIFTED)
			{
				if(col == _numColumns - 1)
				{
					if(_hWrapMode != VIDEO_WRAP_MODE_NONE)
						offset -= (_numColumns - 1);
				}
				else
					++offset;
			}
		}

		_selection = (_selection + offset) % _numOptions;
	}

	// **** case 3: selection out of bounds, no wrapping, don't do anything ***************
	else if( (horizontal && ((col == 0 && offset == -1) ||
	                        (col == _numColumns - 1 && offset == 1))) ||
	         (!horizontal && ((row == 0 && offset < 0) ||
	                        (row == totalRows - 1 && offset > 0))))
	{
		return false;
	}


	// **** case 4: no wrapping, but selection is within bounds ***************
	else
	{
		_selection += offset;
	}

	// if the new selection isn't currently being displayed, set scrolling mode
	int32 selRow = _selection / _numColumns;
	if(selRow < _scrollOffset || selRow > _scrollOffset + _numRows - 1)
	{
		_scrollTime = 0;

		if(selRow < _scrollOffset)
			_scrollDirection = -1;   // up
		else
			_scrollDirection = 1;    // down

		_scrollOffset += _scrollDirection;
		_scrolling = true;
	}

	_event = VIDEO_OPTION_SELECTION_CHANGE;

	return true;
}


//-----------------------------------------------------------------------------
// HandleUpKey
//-----------------------------------------------------------------------------

void OptionBox::HandleUpKey()
{
	// ignore input while scrolling, or if an event has already been logged this frame
	if(_scrolling || _event)
		return;

	if(!_ChangeSelection(-_numColumns, false))
		_event = VIDEO_OPTION_BOUNDS_UP;
}


//-----------------------------------------------------------------------------
// HandleDownKey
//-----------------------------------------------------------------------------

void OptionBox::HandleDownKey()
{
	// ignore input while scrolling, or if an event has already been logged this frame
	if(_scrolling || _event)
		return;

	if(!_ChangeSelection(_numColumns, false))
		_event = VIDEO_OPTION_BOUNDS_DOWN;
}

//-----------------------------------------------------------------------------
// HandleRightKey
//-----------------------------------------------------------------------------

void OptionBox::HandleRightKey()
{
	// ignore input while scrolling, or if an event has already been logged this frame
	if(_scrolling || _event)
		return;


	if(!_ChangeSelection(1, true))
		_event = VIDEO_OPTION_BOUNDS_RIGHT;
}


//-----------------------------------------------------------------------------
// SetCursorState: sets the cursor to shown, hidden, or blinking
//-----------------------------------------------------------------------------

void OptionBox::SetCursorState(CursorState state) {
	if(state <= VIDEO_CURSOR_STATE_INVALID || state >= VIDEO_CURSOR_STATE_TOTAL)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: Invalid cursor state passed to OptionBox::SetCursorState (" << state << ")" << endl;
		return;
	}

	_cursorState = state;
}


//-----------------------------------------------------------------------------
// SetCursorOffset: pixel offset to show cursor at. Usually this should be a
//                  little to the left of each text item
//-----------------------------------------------------------------------------

void OptionBox::SetCursorOffset(float x, float y)
{
	_cursorX = x;
	_cursorY = y;

}


//-----------------------------------------------------------------------------
// HandleCancelKey
//-----------------------------------------------------------------------------

void OptionBox::HandleCancelKey()
{
	// ignore input while scrolling, or if an event has already been logged this frame
	if(_scrolling || _event)
		return;

	// if we're in switch mode and cancel key is hit, get out of switch mode
	// but don't send the cancel event since player still might want to select
	// something.

	if(_firstSelection >= 0)
		_firstSelection = -1;
	else
	{
		// send cancel event
		_event = VIDEO_OPTION_CANCEL;
	}

}


//-----------------------------------------------------------------------------
// HandleConfirmKey
//-----------------------------------------------------------------------------

void OptionBox::HandleConfirmKey()
{
	// ignore input while scrolling, or if an event has already been logged this frame
	if(_scrolling || _event)
		return;

	// check that a valid option is selected
	if(_selection < 0 || _selection >= _numOptions)
		return;

	// case 1: switching 2 different elements
	if(_firstSelection >= 0 && _selection != _firstSelection)
	{
		if(_switching)
		{
			_switchSelection = _firstSelection;

			// perform the actual switch
			_SwitchItems();

			// send a switch event
			_event = VIDEO_OPTION_SWITCH;
		}
	}

	// case 2: partial confirm (confirming the first element in a double confirm)
	else if(_selectMode == VIDEO_SELECT_DOUBLE && _firstSelection == -1)
	{
		// mark the item that is getting switched
		_firstSelection = _selection;
	}

	// case 3: confirm
	else
	{
		if(_options[_selection].disabled)
		{
			// play an annoying beep to tell player they clicked on a disabled option
			return;
		}

		_event = VIDEO_OPTION_CONFIRM;

		// get out of switch mode
		_firstSelection = -1;
	}
}


//-----------------------------------------------------------------------------
// _SwitchItems: helper function to HandleConfirmKey, which does the dirty work
//               of switching two items.
//-----------------------------------------------------------------------------

void OptionBox::_SwitchItems()
{
	// switch the two options within the vector

	Option temp = _options[_selection];
	_options[_selection]       = _options[_firstSelection];
	_options[_firstSelection] = temp;

	// set _firstSelection to -1, so that we know we're not in switching mode
	// any more

	_firstSelection = -1;
}


//-----------------------------------------------------------------------------
// SetCellSize: sets the pixel width/height of each "cell" in the option box
//-----------------------------------------------------------------------------

void OptionBox::SetCellSize(float hSpacing, float vSpacing)
{
	_hSpacing = hSpacing;
	_vSpacing = vSpacing;
	_initialized = IsInitialized(_initialization_errors);
}


//-----------------------------------------------------------------------------
// SetSize: sets the number of columns and rows in this option box
//-----------------------------------------------------------------------------

void OptionBox::SetSize(int32 columns, int32 rows)
{
	_numColumns = columns;
	_numRows    = rows;
	_initialized = IsInitialized(_initialization_errors);
}


//-----------------------------------------------------------------------------
// SetOptionAlignment: within each cell, how the text and cursor should be
//                     aligned for option labels. Values can be:
//                     VIDEO_X_LEFT, VIDEO_X_CENTER, VIDEO_X_RIGHT
//                     VIDEO_Y_TOP, VIDEO_Y_CENTER, VIDEO_Y_BOTTOM
//-----------------------------------------------------------------------------

void OptionBox::SetOptionAlignment(int32 xalign, int32 yalign)
{
	_option_xalign = xalign;
	_option_yalign = yalign;
	_initialized = IsInitialized(_initialization_errors);
}


//-----------------------------------------------------------------------------
// SetSelectMode: can be VIDEO_SELECT_SINGLE or VIDEO_SELECT_DOUBLE. This
//                means that you can select an option by pressing confirm once
//                or by pressing it twice.
//-----------------------------------------------------------------------------

void OptionBox::SetSelectMode(SelectMode mode)
{
	_selectMode = mode;
	_initialized = IsInitialized(_initialization_errors);
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
// SetVerticalWrapMode: controls how the cursor wraps when it goes beyond the
//                      top or bottom row of the option box.
//-----------------------------------------------------------------------------

void OptionBox::SetVerticalWrapMode(WrapMode mode)
{
	_vWrapMode = mode;
}


void OptionBox::SetHorizontalWrapMode(WrapMode mode)
{
	_hWrapMode = mode;
}


//-----------------------------------------------------------------------------
// SetSelection: after you create a new option box, the default selection is
//               -1 (i.e. nothing selected) but usually you want the first
//               option to be selected so after you call SetOptions(), call
//               SetSelection(0). Note that if you call SetSelection(0) before
//               calling SetOptions(), this will return false because there is
//               no element "0" yet.
//-----------------------------------------------------------------------------

void OptionBox::SetSelection(int32 index)
{
	if(index < -1 || index >= _numOptions)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: OptionBox::SetSelection() was passed invalid index (" << index << ")" << endl;
		return;
	}
	_selection = index;

	int32 _selRow = _selection / _numColumns;

	// if the new selection isn't currently being displayed, instantly scroll to it
	if(_selRow < _scrollOffset || _selRow > _scrollOffset + _numRows - 1)
	{
		_scrollOffset = _selRow - _numRows + 1;

		int32 totalNumRows = (_numOptions + _numColumns - 1) / _numColumns;

		if(_scrollOffset + _numRows >= totalNumRows)
		{
			_scrollOffset = totalNumRows - _numRows;
		}
	}
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

		Option option;

		if(!_ParseOption(str, option))
		{
			_options.clear();
			_numOptions = 0;
			if(VIDEO_DEBUG)
				cerr << "VIDEO ERROR: OptionBox::SetOptions() failed. Invalid format string." << endl;
			return false;
		}

		_options.push_back(option);

		++_numOptions;
		++iText;
	}

	return true;
}


//-----------------------------------------------------------------------------
// SetOptionText: changes the text for a given option
//-----------------------------------------------------------------------------
bool OptionBox::SetOptionText(int32 index, const hoa_utils::ustring &text)
{
	// check for bad index

	if(index < 0)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: called OptionBox::SetOptionText() with index less than zero (" << index << ")" << endl;
		return false;
	}


	if(index >= _numOptions)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: called OptionBox::SetOptionText() with an index that was too large (" << index << ")" << endl;
		return false;
	}


	// if index is fine, go ahead and change the option text

	_ParseOption(text, _options[index]);

	return true;
}


//-----------------------------------------------------------------------------
// AddOption: Adds a new option to the OptionBox
//-----------------------------------------------------------------------------

bool OptionBox::AddOption(const hoa_utils::ustring &text)
{
	Option option;

	// Parse the option string
	if (!_ParseOption(text, option))
	{
		if(VIDEO_DEBUG)
		{
			cerr << "VIDEO ERROR: OptionBox::AddOption() failed. Invalid format string." << endl;
		}
		return false;
	}

	// And add it!
	_options.push_back(option);
	++_numOptions;

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
// IsScrolling: returns true if the option box is in the middle of scrolling
//-----------------------------------------------------------------------------

bool OptionBox::IsScrolling() const
{
	return _scrolling;
}


//-----------------------------------------------------------------------------
// GetEvents: returns bitfield of OptionBoxEvent's.
//-----------------------------------------------------------------------------

int32 OptionBox::GetEvent()
{
	int32 returnValue = _event;
	_event = 0;  // clear it out when it's checked

	return returnValue;
}


//-----------------------------------------------------------------------------
// GetSelection(): returns the current selection for the option box, with
//                 indices ranging from 0 to _numOptions - 1. If it is -1, then
//                 it means nothing is selected.
//-----------------------------------------------------------------------------

int32 OptionBox::GetSelection() const
{
	return _selection;
}


//-----------------------------------------------------------------------------
// GetSwitchSelection: if the option box is a "double confirm" kind of box, and
//                     one item has been selected, this returns that item's
//                     index. Returns -1 if nothing is currently selected
//-----------------------------------------------------------------------------

int32 OptionBox::GetSwitchSelection() const
{
	return _switchSelection;
}


//-----------------------------------------------------------------------------
// GetNumRows
//-----------------------------------------------------------------------------

int32 OptionBox::GetNumRows() const
{
	return _numRows;
}


//-----------------------------------------------------------------------------
// GetNumColumns
//-----------------------------------------------------------------------------

int32 OptionBox::GetNumColumns() const
{
	return _numColumns;
}


//-----------------------------------------------------------------------------
// GetNumOptions: returns number of options in the option box, based on
//                the size of the vector passed to SetOptions()
//-----------------------------------------------------------------------------

int32 OptionBox::GetNumOptions() const
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
	if(_option_xalign < VIDEO_X_LEFT || _option_xalign > VIDEO_X_RIGHT)
		s << "* Invalid x align (" << _option_xalign << ")" << endl;

	if(_option_yalign < VIDEO_Y_TOP || _option_yalign > VIDEO_Y_BOTTOM)
		s << "* Invalid y align (" << _option_yalign << ")" << endl;

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
// returns true if the given option is enabled
//-----------------------------------------------------------------------------
bool OptionBox::IsEnabled(int32 index) const
{
	return !_options[index].disabled;
}



//-----------------------------------------------------------------------------
// _ParseOption: helper function to SetOptions. Reads in option format string,
//               and spits out an Option structure
//-----------------------------------------------------------------------------
bool OptionBox::_ParseOption(const hoa_utils::ustring &formatString, Option &op)
{
	// by default, option isn't disabled
	op.disabled = false;

	// clear all vectors, since you can't guarantee that the Option passed in is
	// empty
	op.elements.clear();
	op.text.clear();
	op.images.clear();

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
						cerr << "VIDEO ERROR: In OptionBox::_ParseOption(), tag opening detected with string size less than 3." << endl;
					return false;
				}

				size_t endPos = tmp.find(endTag);

				if(endPos == ustring::npos)
				{
					// uh oh! We have a tag beginning but never ending. This is an error
					if(VIDEO_DEBUG)
						cerr << "VIDEO ERROR: In OptionBox::_ParseOption(), format string had unclosed tag" << endl;
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

					string tagText = MakeStandardString(tmp.substr(1, endPos - 1));

					if(IsStringNumeric(tagText))
					{
						// this must be a positioning tag
						int32 position = atoi(tagText.c_str());
						opElem.type  = VIDEO_OPTION_ELEMENT_POSITION;
						opElem.value = position;
					}
					else
					{
						// this must be an image tag

						StillImage imd;
						imd.SetFilename(tagText);

						if(VideoManager->LoadImage(imd))
						{
							opElem.type  = VIDEO_OPTION_ELEMENT_IMAGE;
							opElem.value = static_cast<int32> (op.images.size());
							op.images.push_back(imd);
						}
						else
						{
							if(VIDEO_DEBUG)
								cerr << "VIDEO ERROR: invalid tag <" << tagText << "> found in OptionBox::_ParseOption()!" << endl;

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

	for(int32 j = 0; j < static_cast<int32>(_options.size()); ++j)
	{
		for(int32 i = 0; i < static_cast<int32>(_options[j].images.size()); ++i)
		{
			VideoManager->DeleteImage(_options[j].images[i]);
		}
	}

	// now clear the vector

	_options.clear();
}


//-----------------------------------------------------------------------------
// Draw: draws the option box, taking the current x/y align flags in the
//       video engine into account, as well as the current coordinate system
//-----------------------------------------------------------------------------

void OptionBox::Draw()
{
	// fail if option box isn't initialized properly
	if (!_initialized) {
		return;
	}

	VideoManager->_PushContext();
	VideoManager->SetDrawFlags(_xalign, _yalign, VIDEO_BLEND, 0);

	// calculate width and height of option box

	float left, right, bottom, top;
	left   = 0.0f;
	bottom = 0.0f;
	right  = _numColumns * _hSpacing;
	top    = _numRows    * _vSpacing;

	CalculateAlignedRect(left, right, bottom, top);

	int32 x, y, w, h;

	x = int32(left < right? left: right);
	y = int32(top < bottom? top : bottom);
	w = int32(right - left);
	if(w < 0) w = -w;
	h = int32(top - bottom);
	if(h < 0) h = -h;

	ScreenRect rect(x, y, w, h);

	int32 cursorMargin = static_cast<int32>(VideoManager->GetDefaultCursor()->GetWidth() + 1 - this->_cursorX);
	rect.left -= cursorMargin;
	rect.width += cursorMargin;

	if(_owner)
		rect.Intersect(_owner->GetScissorRect());
	rect.Intersect(VideoManager->GetScissorRect());
	VideoManager->EnableScissoring(_owner && VideoManager->IsScissoringEnabled());
	if(VideoManager->IsScissoringEnabled())
		VideoManager->SetScissorRect(rect);

	CoordSys &cs = VideoManager->_coord_sys;

	VideoManager->SetFont(_font);

	VideoManager->SetDrawFlags(_option_xalign, _option_yalign, VIDEO_X_NOFLIP, VIDEO_Y_NOFLIP, VIDEO_BLEND, 0);


	int32 rowMin, rowMax;
	float cellOffset = 0.0f;


	if(!_scrolling)
	{
		rowMin = _scrollOffset;
		rowMax = _scrollOffset + _numRows;
	}
	else if(_scrollDirection == -1)
	{
		rowMin = _scrollOffset;
		rowMax = _scrollOffset + _numRows + 1;

		cellOffset = cs.GetVerticalDirection() * (1.0f - (_scrollTime / (VIDEO_OPTION_SCROLL_TIME))) * _vSpacing;
	}
	else  // scrolling down
	{
		rowMin = _scrollOffset - 1;
		rowMax = _scrollOffset + _numRows;

		cellOffset = cs.GetVerticalDirection() * ((_scrollTime / (VIDEO_OPTION_SCROLL_TIME))) * _vSpacing;
	}


	OptionCellBounds bounds;

	bounds.y_top    = top + cellOffset;
	bounds.y_center = bounds.y_top - 0.5f * _vSpacing * cs.GetVerticalDirection();
	bounds.y_bottom = bounds.y_center * 2.0f - bounds.y_top;

	float yoff = -_vSpacing * cs.GetVerticalDirection();
	float xoff = _hSpacing * cs.GetHorizontalDirection();

	bool finished = false;

	// go through all the "cells" and draw them
	for(int32 row = rowMin; row < rowMax; ++row)
	{
		bounds.x_left   = left;
		bounds.x_center = bounds.x_left + 0.5f * _hSpacing * cs.GetHorizontalDirection();
		bounds.x_right  = bounds.x_center * 2.0f - bounds.x_left;

		for(int32 col = 0; col < _numColumns; ++col)
		{
			int32 index = row * _numColumns + col;

			if(index >= _numOptions || index < 0)
			{
				finished = true;
				break;
			}

			float leftEdge = 999999.0f;  // x offset to where the text actually begins
			float x, y;

			int32 xalign = _option_xalign;
			int32 yalign = _option_yalign;

			_SetupAlignment(xalign, yalign, bounds, x, y);

			Option &op = _options.at(index);

			if(op.disabled)
				VideoManager->SetTextColor(Color::gray);
			else
				VideoManager->SetTextColor(Color::white);

			for(int32 opElem = 0; opElem < (int32) op.elements.size(); ++opElem)
			{
				switch(op.elements[opElem].type)
				{
					case VIDEO_OPTION_ELEMENT_LEFT_ALIGN:
					{
						xalign = VIDEO_X_LEFT;
						_SetupAlignment(xalign, _option_yalign, bounds, x, y);
						break;
					}

					case VIDEO_OPTION_ELEMENT_CENTER_ALIGN:
					{
						xalign = VIDEO_X_CENTER;
						_SetupAlignment(xalign, _option_yalign, bounds, x, y);
						break;
					}

					case VIDEO_OPTION_ELEMENT_RIGHT_ALIGN:
					{
						xalign = VIDEO_X_RIGHT;
						_SetupAlignment(xalign, _option_yalign, bounds, x, y);
						break;
					}

					case VIDEO_OPTION_ELEMENT_IMAGE:
					{
						int32 imageIndex = op.elements[opElem].value;

						if(imageIndex >= 0 && imageIndex < (int32)op.images.size())
						{
							if(op.disabled)
								VideoManager->DrawImage(op.images[imageIndex], Color::gray);
							else
								VideoManager->DrawImage(op.images[imageIndex], Color::white);

							float edge = x - bounds.x_left;
							float width = op.images[imageIndex].GetWidth();
							if(xalign == VIDEO_X_CENTER)
								edge -= width * 0.5f * cs.GetHorizontalDirection();
							else if(xalign == VIDEO_X_RIGHT)
								edge -= width * cs.GetHorizontalDirection();

							if(edge < leftEdge)
								leftEdge = edge;

						}
						break;
					}

					case VIDEO_OPTION_ELEMENT_POSITION:
					{
						x = bounds.x_left + op.elements[opElem].value * cs.GetHorizontalDirection();
						VideoManager->Move(x, y);
						break;
					}

					case VIDEO_OPTION_ELEMENT_TEXT:
					{
						int32 textIndex = op.elements[opElem].value;

						if(textIndex >= 0 && textIndex < (int32)op.text.size())
						{
							const ustring &text = op.text[textIndex];
							float width = static_cast<float>(VideoManager->CalculateTextWidth(_font, text));
							float edge = x - bounds.x_left;

							if(xalign == VIDEO_X_CENTER)
								edge -= width * 0.5f * cs.GetHorizontalDirection();
							else if(xalign == VIDEO_X_RIGHT)
								edge -= width * cs.GetHorizontalDirection();

							if(edge < leftEdge)
								leftEdge = edge;

							VideoManager->DrawText(text);
						}

						break;
					}

					// Added by Roots: These cases weren't accounted for and were generating warnings.
					// plese fix this up when you get a chance
					case VIDEO_OPTION_ELEMENT_INVALID:
					case VIDEO_OPTION_ELEMENT_TOTAL:
					default:
						break;
				};
			}

			float cursorOffset = 0;
			if(_scrolling)
			{
				if(_scrollDirection == -1)
				{
					cursorOffset = -cellOffset;
				}
				else
					cursorOffset = -cellOffset + cs.GetVerticalDirection() * _vSpacing;
			}

			// if this is the index where we are supposed to show the switch cursor, show it
			if(index == _firstSelection && !_blink && _cursorState != VIDEO_CURSOR_STATE_HIDDEN)
			{
				_SetupAlignment(VIDEO_X_LEFT, _option_yalign, bounds, x, y);
				VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
				VideoManager->MoveRelative(_cursorX + leftEdge + _switchCursorX, cursorOffset + _cursorY + _switchCursorY);
				StillImage *defaultCursor = VideoManager->GetDefaultCursor();

				if(defaultCursor)
					VideoManager->DrawImage(*defaultCursor, Color::white);
			}

			// if this is the index where we are supposed to show the cursor, show it
			if(index == _selection && !(_blink && _cursorState == VIDEO_CURSOR_STATE_BLINKING) && _cursorState != VIDEO_CURSOR_STATE_HIDDEN)
			{
				_SetupAlignment(VIDEO_X_LEFT, _option_yalign, bounds, x, y);
				VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
				VideoManager->MoveRelative(_cursorX + leftEdge, cursorOffset + _cursorY);
				StillImage *defaultCursor = VideoManager->GetDefaultCursor();

				if(defaultCursor)
					VideoManager->DrawImage(*defaultCursor, Color::white);
			}

			bounds.x_left   += xoff;
			bounds.x_center += xoff;
			bounds.x_right  += xoff;
		}

		if(finished)
			break;

		bounds.y_top    += yoff;
		bounds.y_center += yoff;
		bounds.y_bottom += yoff;
	}

	VideoManager->_PopContext();
}


//-----------------------------------------------------------------------------
// _SetupAlignment: does the dirty work of figuring out alignment for each
//                  option cell
//-----------------------------------------------------------------------------

void OptionBox::_SetupAlignment(int32 xalign, int32 yalign, const OptionCellBounds &bounds, float &x, float &y) {
	VideoManager->SetDrawFlags(xalign, yalign, 0);

	switch(xalign)
	{
		case VIDEO_X_LEFT:
			x = bounds.x_left;
			break;
		case VIDEO_X_CENTER:
			x = bounds.x_center;
			break;
		default:
			x = bounds.x_right;
			break;
	};

	switch(yalign)
	{
		case VIDEO_Y_TOP:
			y = bounds.y_top;
			break;
		case VIDEO_Y_CENTER:
			y = bounds.y_center;
			break;
		default:
			y = bounds.y_bottom;
			break;
	};

	VideoManager->Move(x, y);
}


//-----------------------------------------------------------------------------
// Update: update any blinking or scrolling effects for the option box
//-----------------------------------------------------------------------------

void OptionBox::Update(uint32 frameTime)
{
	_blink = ((_blinkTime / VIDEO_CURSOR_BLINK_RATE) % 2) == 1;
	_blinkTime += frameTime;

	if(_scrolling)
	{
		_scrollTime += frameTime;

		if(_scrollTime > VIDEO_OPTION_SCROLL_TIME)
		{
			_scrollTime = 0;
			_scrolling = false;
		}
	}
}

} // namespace hoa_video
