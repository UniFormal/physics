import VanRoosbrockEg
from mpdbase import *
import numpy as np

model = VanRoosbrockEg.MPD_VanRoosbrockEg(np.linspace(0, 0.5, 10))
state = MPDState(model)
state.gauss_seidel("cycle1", epsilon=1e-3)
print(state)
