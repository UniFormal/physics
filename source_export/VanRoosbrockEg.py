#!/usr/bin/env python

# http://mathhub.info/MitM/Models?VanRoosbrockEg


from mpdbase import *

class MPD_VanRoosbrockEg(MPDBase):
	def __init__(self, space, integration_surfaces=[]):
		MPDBase.__init__(self, 'VanRoosbrockEg',
			'http://mathhub.info/MitM/Models',
			space,
			integration_surfaces)
		self.graph = [('current_density', 'electron_current'), ('electron_density', 'electron_density'), ('quasifermi_potential', 'electron_density'), ('electrostatic_potential', 'electron_density'), ('electric_field', 'electric_field_law'), ('displacement', 'displacement_law'), ('electric_field', 'displacement_law'), ('absolute_permittivity', 'displacement_law'), ('doping_profile', 'charge_density_law'), ('electron_density', 'charge_density_law'), ('charge_density', 'charge_density_law'), ('charge_density', 'gauss_law'), ('absolute_permittivity', 'permitivity_law'), ('relative_permittivity', 'permitivity_law')]

	def init_quantity_decls(self):
		self.quantity_decls['electron_mobility'] = QuantityDecl(
			parent = 'http://mathhub.info/MitM/Models?Quantities',
			name = 'electron_mobility',
			is_constant = False,
			dimension = '',
			is_field = False 
		)

		self.quantity_decls['doping_profile'] = QuantityDecl(
			parent = 'http://mathhub.info/MitM/Models?Quantities',
			name = 'doping_profile',
			is_constant = False,
			dimension = 'VolumeDensity',
			is_field = True 
		)

		self.quantity_decls['electron_density'] = QuantityDecl(
			parent = 'http://mathhub.info/MitM/Models?Quantities',
			name = 'electron_density',
			is_constant = False,
			dimension = 'VolumeDensity',
			is_field = True 
		)

		self.quantity_decls['charge_density'] = QuantityDecl(
			parent = 'http://mathhub.info/MitM/Models?Quantities',
			name = 'charge_density',
			is_constant = False,
			dimension = '',
			is_field = True 
		)

		self.quantity_decls['displacement'] = QuantityDecl(
			parent = 'http://mathhub.info/MitM/Models?Quantities',
			name = 'displacement',
			is_constant = False,
			dimension = '',
			is_field = True 
		)

		self.quantity_decls['quasifermi_potential'] = QuantityDecl(
			parent = 'http://mathhub.info/MitM/Models?Quantities',
			name = 'quasifermi_potential',
			is_constant = False,
			dimension = 'ElectricPotential',
			is_field = True 
		)

		self.quantity_decls['electrostatic_potential'] = QuantityDecl(
			parent = 'http://mathhub.info/MitM/Models?Quantities',
			name = 'electrostatic_potential',
			is_constant = False,
			dimension = 'ElectricPotential',
			is_field = True 
		)

		self.quantity_decls['electric_field'] = QuantityDecl(
			parent = 'http://mathhub.info/MitM/Models?Quantities',
			name = 'electric_field',
			is_constant = False,
			dimension = 'ElectricField',
			is_field = True 
		)

		self.quantity_decls['current_density'] = QuantityDecl(
			parent = 'http://mathhub.info/MitM/Models?Quantities',
			name = 'current_density',
			is_constant = False,
			dimension = 'ElectricCurrentDensity',
			is_field = True 
		)

		self.quantity_decls['absolute_permittivity'] = QuantityDecl(
			parent = 'http://mathhub.info/MitM/Models?Quantities',
			name = 'absolute_permittivity',
			is_constant = False,
			dimension = 'ElectricalPermittivity',
			is_field = False 
		)

		self.quantity_decls['relative_permittivity'] = QuantityDecl(
			parent = 'http://mathhub.info/MitM/Models?Quantities',
			name = 'relative_permittivity',
			is_constant = False,
			dimension = 'DimNone',
			is_field = False 
		)

		self.quantity_decls['band_energy'] = QuantityDecl(
			parent = 'http://mathhub.info/MitM/Models?Quantities',
			name = 'band_energy',
			is_constant = True,
			dimension = 'Energy',
			is_field = True 
		)

		self.quantity_decls['band_density'] = QuantityDecl(
			parent = 'http://mathhub.info/MitM/Models?Quantities',
			name = 'band_density',
			is_constant = True,
			dimension = 'VolumeDensity',
			is_field = True 
		)

		self.quantity_decls['temperature'] = QuantityDecl(
			parent = 'http://mathhub.info/MitM/Models?Quantities',
			name = 'temperature',
			is_constant = True,
			dimension = 'Temperature',
			is_field = False 
		)

		self.quantity_decls['boltzmann_constant'] = QuantityDecl(
			parent = 'http://mathhub.info/MitM/Models?Quantities',
			name = 'boltzmann_constant',
			is_constant = True,
			dimension = '',
			is_field = False 
		)

		self.quantity_decls['elementary_charge'] = QuantityDecl(
			parent = 'http://mathhub.info/MitM/Models?Quantities',
			name = 'elementary_charge',
			is_constant = True,
			dimension = 'ElectricCharge',
			is_field = False 
		)

		self.quantity_decls['electric_constant'] = QuantityDecl(
			parent = 'http://mathhub.info/MitM/Models?Quantities',
			name = 'electric_constant',
			is_constant = True,
			dimension = 'ElectricalPermittivity',
			is_field = False 
		)

	def init_laws(self):
		self.laws['electron_continuity_eqn'] = Law(
			parent = 'http://mathhub.info/MitM/Models?VanRoosbrockEg',
			name = 'electron_continuity_eqn',
			solvables = [],
			law_test = lambda state: (((derivative_on_space(state['current_density'], self.space))) - (0)),
			used_quantities = ['current_density'] 
		)

		self.laws['electron_current'] = Law(
			parent = 'http://mathhub.info/MitM/Models?VanRoosbrockEg',
			name = 'electron_current',
			solvables = ['current_density'],
			current_density = lambda state: (- (((state['elementary_charge'] * state['electron_mobility']) * state['electron_density']) * (derivative_on_space(state['quasifermi_potential'], self.space)))),
			law_test = lambda state: ((state['current_density']) - ((- (((state['elementary_charge'] * state['electron_mobility']) * state['electron_density']) * (derivative_on_space(state['quasifermi_potential'], self.space)))))),
			used_quantities = ['quasifermi_potential' ,'electron_density' ,'electron_mobility' ,'elementary_charge' ,'current_density'] 
		)

		self.laws['electron_density'] = Law(
			parent = 'http://mathhub.info/MitM/Models?VanRoosbrockEg',
			name = 'electron_density',
			electrostatic_potential = lambda state: ((((numpy.log((state['electron_density'] / state['band_density'])) * (state['boltzmann_constant'] * state['temperature'])) + state['band_energy']) / state['elementary_charge']) + state['quasifermi_potential']),
			solvables = ['electron_density' ,'electrostatic_potential' ,'quasifermi_potential'],
			electron_density = lambda state: (state['band_density'] * numpy.exp((((state['elementary_charge'] * (state['electrostatic_potential'] - state['quasifermi_potential'])) - state['band_energy']) / (state['boltzmann_constant'] * state['temperature'])))),
			quasifermi_potential = lambda state: (state['electrostatic_potential'] - (((numpy.log((state['electron_density'] / state['band_density'])) * (state['boltzmann_constant'] * state['temperature'])) + state['band_energy']) / state['elementary_charge'])),
			law_test = lambda state: ((state['electron_density']) - ((state['band_density'] * numpy.exp((((state['elementary_charge'] * (state['electrostatic_potential'] - state['quasifermi_potential'])) - state['band_energy']) / (state['boltzmann_constant'] * state['temperature'])))))),
			used_quantities = ['temperature' ,'boltzmann_constant' ,'band_energy' ,'quasifermi_potential' ,'electrostatic_potential' ,'elementary_charge' ,'band_density' ,'electron_density'] 
		)

		self.laws['electric_field_law'] = Law(
			parent = 'http://mathhub.info/MitM/Models?VanRoosbrockEg',
			name = 'electric_field_law',
			solvables = ['electric_field'],
			law_test = lambda state: ((state['electric_field']) - ((- (derivative_on_space(state['electrostatic_potential'], self.space))))),
			used_quantities = ['electrostatic_potential' ,'electric_field'],
			electric_field = lambda state: (- (derivative_on_space(state['electrostatic_potential'], self.space))) 
		)

		self.laws['displacement_law'] = Law(
			parent = 'http://mathhub.info/MitM/Models?VanRoosbrockEg',
			name = 'displacement_law',
			absolute_permittivity = lambda state: (state['displacement'] / state['electric_field']),
			solvables = ['displacement' ,'electric_field' ,'absolute_permittivity'],
			law_test = lambda state: ((state['displacement']) - ((state['absolute_permittivity'] * state['electric_field']))),
			used_quantities = ['electric_field' ,'absolute_permittivity' ,'displacement'],
			electric_field = lambda state: (state['displacement'] / state['absolute_permittivity']),
			displacement = lambda state: (state['absolute_permittivity'] * state['electric_field']) 
		)

		self.laws['charge_density_law'] = Law(
			parent = 'http://mathhub.info/MitM/Models?VanRoosbrockEg',
			name = 'charge_density_law',
			charge_density = lambda state: (state['elementary_charge'] * (state['doping_profile'] - state['electron_density'])),
			doping_profile = lambda state: ((state['charge_density'] / state['elementary_charge']) + state['electron_density']),
			solvables = ['charge_density' ,'doping_profile' ,'electron_density'],
			electron_density = lambda state: (state['doping_profile'] - (state['charge_density'] / state['elementary_charge'])),
			law_test = lambda state: ((state['charge_density']) - ((state['elementary_charge'] * (state['doping_profile'] - state['electron_density'])))),
			used_quantities = ['electron_density' ,'doping_profile' ,'elementary_charge' ,'charge_density'] 
		)

		self.laws['gauss_law'] = Law(
			parent = 'http://mathhub.info/MitM/Models?VanRoosbrockEg',
			name = 'gauss_law',
			charge_density = lambda state: (derivative_on_space(state['displacement'], self.space)),
			solvables = ['charge_density'],
			law_test = lambda state: ((state['charge_density']) - ((derivative_on_space(state['displacement'], self.space)))),
			used_quantities = ['displacement' ,'charge_density'] 
		)

		self.laws['permitivity_law'] = Law(
			parent = 'http://mathhub.info/MitM/Models?VanRoosbrockEg',
			name = 'permitivity_law',
			absolute_permittivity = lambda state: (state['electric_constant'] * state['relative_permittivity']),
			relative_permittivity = lambda state: (state['absolute_permittivity'] / state['electric_constant']),
			solvables = ['absolute_permittivity' ,'relative_permittivity'],
			law_test = lambda state: ((state['absolute_permittivity']) - ((state['electric_constant'] * state['relative_permittivity']))),
			used_quantities = ['relative_permittivity' ,'electric_constant' ,'absolute_permittivity'] 
		)


