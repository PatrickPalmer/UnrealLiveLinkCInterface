/** 
 * Copyright (c) 2020 Patrick Palmer, The Jim Henson Company.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "UnrealLiveLinkCInterfaceAPI.h"
#ifdef WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <time.h>
#include <math.h>

/* number of circles to perform */
#define CIRCLES 30

/* size in Unreal units of the circle on the X-Y plane */
#define CIRCLE_RADIUS 200

/* number of steps to perform the circles */
#define STEP_COUNT 10000

/* Z axis location -- number of units above origin */
#define HEIGHT 100

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


int main()
{
	int i;
	int rc;
	time_t t;
	double angle = 0.0;
	double worldTime = 0.0;
	
#ifdef WIN32
	const char * sharedObj = "UnrealLiveLinkCInterface.dll";
#else
	const char * sharedObj = "libUnrealLiveLinkCInterface.so";
	struct timespec ts;

	ts.tv_sec = 0;
	ts.tv_nsec = 16 * 1000000;
#endif

	rc = UnrealLiveLink_Load(sharedObj);
	if (rc != UNREAL_LIVE_LINK_OK)
	{
		printf("error: unable to load %s (error %d)\n", sharedObj, rc);
		return 1;
	}

	UnrealLiveLink_SetProviderName("CirclingTransform");
	rc = UnrealLiveLink_StartLiveLink();
	if (rc != UNREAL_LIVE_LINK_OK) 
	{
		printf("error: unable to start live link (error %d)\n", rc);
		return 1;
	}

	printf("Starting...\n");
	t = time(NULL);

	/* set up transform live link role */
	UnrealLiveLink_SetTransformStructure("circle", 0);
	
	/* initialize the per frame transform structure */
	struct UnrealLiveLink_Transform xform;
	UnrealLiveLink_InitTransform(&xform);
	xform.translation[2] = HEIGHT;

	/* calculate the radian step */
	const double step = 2.0 * M_PI * CIRCLES / STEP_COUNT;

	/* loop STEP_COUNT times sending data every 16ms (not exact) */
	for (i = 0; i < STEP_COUNT; i++)
	{
		xform.translation[0] = (float)sin(angle) * CIRCLE_RADIUS;
		xform.translation[1] = (float)cos(angle) * CIRCLE_RADIUS;

		UnrealLiveLink_UpdateTransformFrame("circle", worldTime, 0, 0, &xform);
		
		angle += step;
		
		/* sleep 16ms */
#ifdef WIN32
		Sleep(16);
#else
		nanosleep(&ts, &ts);
#endif

		worldTime += 16.0;
	}	

	printf("Done. Took %lld seconds.\n", time(NULL) - t);

	UnrealLiveLink_Unload();

	return 0;
}

