import VanRoosbrockEg
from mpdbase import *
import numpy as np

N = 10
EPSILON = 1e-3

model = VanRoosbrockEg.MPD_VanRoosbrockEg(np.linspace(0, 0.5, N))

state = MPDState(model)

state.gauss_seidel("cycle1", EPSILON)

print(state)


#state.electron_density = model['charge_density_law'].electron_density(state)

