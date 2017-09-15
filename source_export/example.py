import VanRoosbrockEg
from mpdbase import *
import numpy as np

N = 10
EPSILON = 1e-3

model = VanRoosbrockEg.MPD_VanRoosbrockEg(np.linspace(0, 0.5, N))

state = MPDState(model)

names = (model.constant_quantity_decls_keys)
for s in names:
    print(model.quantity_decls[s])
    print(model.quantity_decls[s].is_field)

# constants
state.temperature = 200.0
state.elementary_charge = -1.60217662e-19
state.electric_constant = 8.85418782e-12
state.band_energy = np.repeat(state.elementary_charge * 5.5, N) #5 e.v
state.band_density = np.repeat(10e25 * state.elementary_charge * 1e6, N)
state.boltzmann_constant = 1.38064852e-23

# initial conditions
state.doping_profile = np.repeat(1e-21, N)
state.relative_permittivity = 1.0
state.absolute_permittivity = model['permitivity_law'].absolute_permittivity(state)
state.electron_mobility = 1e-4 # [m^2 / Vs]

#doesn't make sense to intialize this, but for sake of example:
state.quasifermi_potential = np.repeat(state.elementary_charge * 0.5, N)

#we let charge density be the pivot of the cycle and initiate it to anything:
state.charge_density = np.repeat(-2.0, N)

state.electron_density = model['charge_density_law'].electron_density(state)
while True:
    state.electrostatic_potential = model['electron_density'].electrostatic_potential(state)
    state.electric_field = model['electric_field_law'].electric_field(state)
    state.displacement = model['displacement_law'].displacement(state)
    state.charge_density = model['gauss_law'].charge_density(state)
    state.electron_density = model['charge_density_law'].electron_density(state)
    if all(model['charge_density_law'].law_test(state) < EPSILON):
        break


print(model)

print(state)


# state2 2

state2 = MPDState(model)

# constants
state2.temperature = 120.0
state2.elementary_charge = -1.60217662e-19
state2.electric_constant = 8.85418782e-12
state2.band_energy = np.repeat(state2.elementary_charge * 1.5, N) #5 e.v
state2.band_density = np.repeat(3e25 * state2.elementary_charge * 1e6, N)
state2.boltzmann_constant = 1.38064852e-23

# initial conditions
state2.doping_profile = np.repeat(1.2e-21, N)
state2.relative_permittivity = 1.2
state2.absolute_permittivity = model['permitivity_law'].absolute_permittivity(state2)
state2.electron_mobility = 1.3e-4 # [m^2 / Vs]

#doesn't make sense to intialize this, but for sake of example:
state2.quasifermi_potential = np.repeat(state2.elementary_charge * 0.5, N)

#we let charge density be the pivot of the cycle and initiate it to anything:
state2.charge_density = np.repeat(-1.3, N)

state2.electron_density = model['charge_density_law'].electron_density(state2)
while True:
    state2.electrostatic_potential = model['electron_density'].electrostatic_potential(state2)
    state2.electric_field = model['electric_field_law'].electric_field(state2)
    state2.displacement = model['displacement_law'].displacement(state2)
    state2.charge_density = model['gauss_law'].charge_density(state2)
    state2.electron_density = model['charge_density_law'].electron_density(state2)
    if all(model['charge_density_law'].law_test(state2) < EPSILON):
        break


print(state2)
    
# interpolate state2 and state1 with lambda = 0.5
state2.interpolate01(state, 0.5)
print("interpolated state")
print(state2)
