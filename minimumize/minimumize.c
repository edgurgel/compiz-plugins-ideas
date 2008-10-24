/*
 * Compiz Fusion Minimumize plugin
 *
 * Copyright (c) 2008 Eduardo Gurgel Pinho <edgurgel@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Author(s):
 * Eduardo Gurgel Pinho <edgurgel@gmail.com>
 * Marco Diego Aurélio Mesquita <marcodiegomesquita@gmail.com>
 *
 * Description:
 *
 * Minimumize resizes a window 
 * TODO:
 *  - Check if the window is maximazed.
 *  - Minimumize with different factors. 
 */

#include <compiz-core.h>
#include "minimumize_options.h"


/* Give new window's size.
 */
static BOX
minimumizeUnextendBox (CompWindow *window,
			BOX	box,
			Bool	left,
			Bool	right,
			Bool	up,
			Bool	down)
{
    const int min_width = window->sizeHints.min_width;
    const int min_height = window->sizeHints.min_height;

    if(left) 
    {
	int width = box.x2 - box.x1;
	if(width/2 < min_width) {
	    box.x2 = box.x1 + min_width;	
	}
	else box.x2 = box.x2 - width/2;
    }
    if(right) 
    {
	    int width = box.x2 - box.x1;
	    if(width/2 < min_width) {
		    box.x1 = box.x2 - min_width;	
	    }
	    else box.x1 = box.x2 - width/2;	
    }
    if(up) 
    {
	    int height = box.y2 - box.y1;
	    if(height/2 < min_height) {
		    box.y2 = box.y1 + min_height;	
	    }
	    else box.y2 = box.y2 - height/2;
    }
    if(down) 
    {
	    int height = box.y2 - box.y1;
	    if(height/2 < min_height) {
		    box.y1 = box.y2 - min_height;	
	    }
	    else box.y1 = box.y2 - height/2;
    }
    return box;
}

/* Create a box for resizing.
 */
static BOX
minimumizeFindRect (CompWindow *w,
			Bool	left,
			Bool	right,
			Bool	up,
			Bool	down)
{
    BOX windowBox;
    windowBox.x1 = w->serverX;
    windowBox.x2 = w->serverX + w->serverWidth;
    windowBox.y1 = w->serverY;
    windowBox.y2 = w->serverY + w->serverHeight;
	
    return minimumizeUnextendBox (w, windowBox, left, right, up, down);
}

/* Calls out to compute the resize */
static unsigned int
minimumizeComputeResize(CompWindow     *w,
			XWindowChanges	*xwc,
			Bool	left,
			Bool	right,
			Bool	up,
			Bool	down)
{
    CompOutput   *output;
    unsigned int mask = 0;
    BOX          box;
    output = &w->screen->outputDev[outputDeviceForWindow (w)];
    box = minimumizeFindRect (w, left, right, up, down);

    if (box.x1 != w->serverX)
	mask |= CWX;

    if (box.y1 != w->serverY)
	mask |= CWY;

    if ((box.x2 - box.x1) != w->serverWidth)
	mask |= CWWidth;

    if ((box.y2 - box.y1) != w->serverHeight)
	mask |= CWHeight;

    xwc->x = box.x1;
    xwc->y = box.y1;
    xwc->width = box.x2 - box.x1; 
    xwc->height = box.y2 - box.y1;

    return mask;
}

/* 
 * Initially triggered keybinding.
 * Fetch the window, fetch the resize, constrain it.
 *
 */
static Bool
minimumizeTrigger(CompDisplay     *d,
		 CompAction      *action,
		 CompActionState state,
		 CompOption      *option,
		 int             nOption)
{
    Window     xid;
    CompWindow *w;

    xid = getIntOptionNamed (option, nOption, "window", 0);
    w   = findWindowAtDisplay (d, xid);
    if (w)
    {
	int            width, height;
	unsigned int   mask;
	XWindowChanges xwc;

	Bool left, right, up, down;
	left = minimumizeGetMinimumizeLeft(w->screen->display);
	right = minimumizeGetMinimumizeRight(w->screen->display);
	up = minimumizeGetMinimumizeUp(w->screen->display);
	down = minimumizeGetMinimumizeDown(w->screen->display);
	
	mask = minimumizeComputeResize (w, &xwc, left, right, up, down);
	if (mask)
	{
	    if (constrainNewWindowSize (w, xwc.width, xwc.height,
					&width, &height))
	    {
		mask |= CWWidth | CWHeight;
		xwc.width  = width;
		xwc.height = height;
	    }

	    if (w->mapNum && (mask & (CWWidth | CWHeight)))
		sendSyncRequest (w);

	    configureXWindow (w, mask, &xwc);
	}
    }

    return TRUE;
}

/* 
 * Minimumazing on specified direction.
 */
static Bool
minimumizeTriggerDirection(CompDisplay	*d,
		 CompAction *action,
		 CompActionState    state,
		 CompOption *option,
		 Bool	left,
		 Bool	right,
		 Bool	up,
		 Bool	down,
		 int	nOption)
{
    Window     xid;
    CompWindow *w;

    xid = getIntOptionNamed (option, nOption, "window", 0);
    w   = findWindowAtDisplay (d, xid);
    if (w)
    {
	int            width, height;
	unsigned int   mask;
	XWindowChanges xwc;

	mask = minimumizeComputeResize (w, &xwc, left,right,up,down);
	if (mask)
	{
	    if (constrainNewWindowSize (w, xwc.width, xwc.height,
					&width, &height))
	    {
		mask |= CWWidth | CWHeight;
		xwc.width  = width;
		xwc.height = height;
	    }

	    if (w->mapNum && (mask & (CWWidth | CWHeight)))
		sendSyncRequest (w);

	    configureXWindow (w, mask, &xwc);
	}
    }

    return TRUE;
}

/* 
 * Minimumazing to left region limit.
 */
static Bool
minimumizeTriggerLeft(CompDisplay     *d,
		 CompAction      *action,
		 CompActionState state,
		 CompOption      *option,
		 int             nOption)
{
    return minimumizeTriggerDirection(d,action,state,option,
//				left	right	up  down
				TRUE, FALSE, FALSE, FALSE,nOption);

}

/* 
 * Minimumazing to right region limit.
 *
 */
static Bool
minimumizeTriggerRight(CompDisplay     *d,
		 CompAction      *action,
		 CompActionState state,
		 CompOption      *option,
		 int             nOption)
{
	return minimumizeTriggerDirection(d,action,state,option,
//				left	right		up		down
					FALSE, TRUE, FALSE, FALSE,nOption);
}

/* 
 * Minimumazing to upper region limit.
 *
 */
static Bool
minimumizeTriggerUp(CompDisplay     *d,
		 CompAction      *action,
		 CompActionState state,
		 CompOption      *option,
		 int             nOption)
{
    return minimumizeTriggerDirection(d,action,state,option,
//				left	right	up  down
				FALSE, FALSE, TRUE, FALSE,nOption);

}

/* 
 * Minimumazing to bottom region limit.
 *
 */
static Bool
minimumizeTriggerDown(CompDisplay     *d,
		 CompAction      *action,
		 CompActionState state,
		 CompOption      *option,
		 int             nOption)
{
    return minimumizeTriggerDirection(d,action,state,option,
//				left	right	up	down
				FALSE, FALSE, FALSE, TRUE,nOption);

}

/* 
 * Minimumazing horizontally.
 *
 */
static Bool
minimumizeTriggerHorizontally(CompDisplay     *d,
		 CompAction      *action,
		 CompActionState state,
		 CompOption      *option,
		 int             nOption)
{
	return minimumizeTriggerDirection(d,action,state,option,
//				left	right	up  down
				TRUE, TRUE, FALSE, FALSE,nOption);

}

/* 
  * Minimumazing vertically.
 *
 */
static Bool
minimumizeTriggerVertically(CompDisplay     *d,
		 CompAction      *action,
		 CompActionState state,
		 CompOption      *option,
		 int             nOption)
{
    return minimumizeTriggerDirection(d,action,state,option,
//				left	right		up		down
				FALSE, FALSE, TRUE, TRUE,nOption);

}

/* 
 * Minimumazing to upper left region limit.
 *
 */
static Bool
minimumizeTriggerUpLeft(CompDisplay     *d,
		 CompAction      *action,
		 CompActionState state,
		 CompOption      *option,
		 int             nOption)
{

    return minimumizeTriggerDirection(d,action,state,option,
//				left	right		up		down
				TRUE, FALSE, TRUE, FALSE,nOption);

}

/* 
 * Minimumazing to upper right region limit.
 *
 */
static Bool
minimumizeTriggerUpRight(CompDisplay     *d,
		 CompAction      *action,
		 CompActionState state,
		 CompOption      *option,
		 int             nOption)
{
    return minimumizeTriggerDirection(d,action,state,option,
//				left	right		up		down
				FALSE, TRUE, TRUE, FALSE,nOption);

}

/* 
 * Minimumazing to bottom left region limit.
 *
 */
static Bool
minimumizeTriggerDownLeft(CompDisplay     *d,
		 CompAction      *action,
		 CompActionState state,
		 CompOption      *option,
		 int             nOption)
{
    return minimumizeTriggerDirection(d,action,state,option,
				      TRUE, FALSE, FALSE, TRUE,nOption);
}

/*
 * Minimumazing to bottom right region limit.
 *
 */

static Bool
minimumizeTriggerDownRight(CompDisplay     *d,
		 CompAction      *action,
		 CompActionState state,
		 CompOption      *option,
		 int             nOption)
{
    return minimumizeTriggerDirection(d,action,state,option,
//				left	right		up		down
				FALSE, TRUE, FALSE, TRUE,nOption);
}


/* Configuration, initialization, boring stuff. --------------------- */
static Bool
minimumizeInitDisplay (CompPlugin  *p,
		      CompDisplay *d)
{
    if (!checkPluginABI ("core", CORE_ABIVERSION))
	return FALSE;

    minimumizeSetTriggerKeyInitiate (d, minimumizeTrigger);
	
    minimumizeSetTriggerLeftInitiate (d, minimumizeTriggerLeft);
    minimumizeSetTriggerRightInitiate (d, minimumizeTriggerRight);
    minimumizeSetTriggerUpInitiate (d, minimumizeTriggerUp);
    minimumizeSetTriggerDownInitiate (d, minimumizeTriggerDown);
    minimumizeSetTriggerHorizontallyInitiate (d, minimumizeTriggerHorizontally);
    minimumizeSetTriggerVerticallyInitiate (d, minimumizeTriggerVertically);
    minimumizeSetTriggerUpLeftInitiate (d, minimumizeTriggerUpLeft);
    minimumizeSetTriggerUpRightInitiate (d, minimumizeTriggerUpRight);
    minimumizeSetTriggerDownLeftInitiate (d, minimumizeTriggerDownLeft);
    minimumizeSetTriggerDownRightInitiate (d, minimumizeTriggerDownRight);

    return TRUE;
}

static CompBool
minimumizeInitObject (CompPlugin *p,
		     CompObject *o)
{
    static InitPluginObjectProc dispTab[] = {
	(InitPluginObjectProc) 0, /* InitCore */
	(InitPluginObjectProc) minimumizeInitDisplay,
	0, 
	0 
    };

    RETURN_DISPATCH (o, dispTab, ARRAY_SIZE (dispTab), TRUE, (p, o));
}

static void
minimumizeFiniObject (CompPlugin *p,
		     CompObject *o)
{
    static FiniPluginObjectProc dispTab[] = {
	(FiniPluginObjectProc) 0, /* FiniCore */
	0, 
	0, 
	0 
    };

    DISPATCH (o, dispTab, ARRAY_SIZE (dispTab), (p, o));
}

CompPluginVTable minimumizeVTable = {
    "minimumize",
    0,
    0,
    0,
    minimumizeInitObject,
    minimumizeFiniObject,
    0,
    0
};

CompPluginVTable*
getCompPluginInfo (void)
{
    return &minimumizeVTable;
}

