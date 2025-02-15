
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

#
# Application showing the streaming of live link properties
# Use Unreal Live Link Debug UI to view the animated properties
#

import sys
import math
import time
import pyUnrealLiveLink as pyuell

# number of steps to perform the wave
STEP_COUNT = 10000

world_time = 0.0
	
rc = pyuell.load()
if rc:
    print(f"error: unable to load Unreal Live Link shared object (error {rc})")
    sys.exit(1)

print("Starting...\n")

pyuell.set_provider_name("WaveProperties")
pyuell.start_live_link()

# 24 fps
frame_time = 1.0 / 24.0

channel_count = 32

channel = pyuell.PropertyValues()
for i in range(channel_count):
    channel.append(0.0)

channel_offset = math.pi * 2.0 / 32.0 / 3.0

frame_number = 1

# set up live link role
prop = pyuell.Properties()
for i in range(channel_count):
    prop.append(f'Channel {i+1}')
pyuell.set_basic_structure("wave", prop)

metadata = pyuell.Metadata()

start_time = time.time()

# loop STEP_COUNT times sending data every 24 fps (not exact)
for i in range(STEP_COUNT):

    if (i % 96) == 0:
        print(f'frame {i}')

    for i in range(channel_count):

        f = float(frame_number + (channel_count - i)) * channel_offset

        channel[i] = (math.sin(f) + 1.0) / 2.0

    frame_number += 1

    pyuell.update_basic_frame("wave", world_time, metadata, channel)
		
    # sleep 1 frame time
    time.sleep(frame_time)

    world_time += frame_time

end_time = time.time()
print(f"Done. Took {end_time - start_time} seconds.")

pyuell.unload()

sys.exit(0)

