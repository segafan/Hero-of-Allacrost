#ifndef __VIDEO_VIDEO_STATE_HEADER__
#define __VIDEO_VIDEO_STATE_HEADER__

/*******************************************************************************
* interface for all video states, you cant instantiate it, and you shouldnt
* directly call any of its functions, Video::LoadVideoState does that, so i 
* guess you should just ignore it
*******************************************************************************/
class IVideoState
{
public:
	virtual void Select()=0;   //called when loaded
	virtual void Deselect()=0; //called when unloaded
};

#endif
