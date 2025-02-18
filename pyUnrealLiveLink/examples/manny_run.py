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

import os
import sys
import json
import time
import pyUnrealLiveLink as pyuell

SCRIPT_DIRECTORY = os.path.dirname(os.path.abspath(__file__))

rc = pyuell.load();
if rc:
    print(f"error: unable to load Unreal Live Link shared object (error {rc})")
    sys.exit(1)

print("Starting...\n")

start_time = time.time()

with open(os.path.join(SCRIPT_DIRECTORY, "manny_run.json"), 'r') as f:
    data = json.load(f)

def bones_anim_structure(d):

    def trav(d, link, parent=None):
        for k,v in d.items():
            link[k] = parent
            if v:
                trav(v, link, k)
    
    links = {}
    trav(data['skeleton'], links)

    return links

anim_static = pyuell.AnimationStatic()
bones_hier = bones_anim_structure(data['skeleton'])
bones = list(bones_hier.keys())
print(bones)

for idx,b in enumerate(bones):
    if b == "root":
        parent_index = -1
    else:
        parent_index = bones.index(bones_hier[b])

    bone = pyuell.Bone()
    bone.name = b
    bone.parent_index = parent_index
    print("bone", idx, "name", b, "parent", bones_hier[b], "parent index", parent_index)
    anim_static.append(bone)

pyuell.set_provider_name("MannyRun")
pyuell.start_live_link()

pyuell.set_animation_structure("manny", pyuell.Properties(), anim_static)

anim = pyuell.Animation()
for b in bones:
    anim.append(pyuell.Transform())

world_time = 0.0

frame_count = len(data['animation']['root'])
frame_time = 1.0 / 24.0

for f in range(frame_count * 100):

    if (f % 96) == 0:
        print(f'frame {f}')

    frame = f % frame_count

    for idx, b in enumerate(bones):

        anim[idx].translation = data['animation'][b][frame]['t']
        anim[idx].rotation = data['animation'][b][frame]['r']

    pyuell.update_animation_frame("manny", world_time, pyuell.Metadata(), pyuell.PropertyValues(), anim)
 
    # sleep 1 frame time
    time.sleep(frame_time)

    world_time += frame_time


end_time = time.time()
print(f"Done. Took {end_time - start_time} seconds.")

pyuell.unload()

sys.exit(0)

