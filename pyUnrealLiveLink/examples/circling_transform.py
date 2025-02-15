
# 
# Copyright (c) 2025 Patrick Palmer
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

import sys
import math
import time
import pyUnrealLiveLink as pyuell

# number of circles to perform 
CIRCLES = 30

# size in Unreal units of the circle on the X-Y plane 
CIRCLE_RADIUS = 200.0

# number of steps to perform the circles
STEP_COUNT = 10000

# Z axis location -- number of units above origin
HEIGHT = 100

angle = 0.0
world_time = 0.0
	
rc = pyuell.load();
if rc:
    print(f"error: unable to load Unreal Live Link shared object (error {rc})")
    sys.exit(1)

print("Starting...\n")

pyuell.set_provider_name("CirclingTransform")
pyuell.start_live_link()

# set up transform live link role
pyuell.set_transform_structure("circle", pyuell.Properties());
	
# initialize the per frame transform structure 
xform = pyuell.Transform()

# calculate the radian step
step = 2.0 * math.pi * float(CIRCLES) / float(STEP_COUNT)

# 24 fps
frame_time = 1.0 / 24.0

start_time = time.time()

# loop STEP_COUNT times sending data every 24 fps (not exact)
for i in range(STEP_COUNT):

    if (i % 96) == 0:
        print(f'frame {i}')

    x = math.sin(angle) * CIRCLE_RADIUS
    y = math.cos(angle) * CIRCLE_RADIUS
    z = HEIGHT
    xform.translation = (x, y, z)

    pyuell.update_transform_frame("circle", world_time, pyuell.Metadata(), pyuell.PropertyValues(), xform)
		
    angle += step
		
    # sleep 1 frame time
    time.sleep(frame_time)

    world_time += frame_time

end_time = time.time()
print(f"Done. Took {end_time - start_time} seconds.")

pyuell.unload()

sys.exit(0)

