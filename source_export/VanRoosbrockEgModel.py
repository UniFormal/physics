#!/usr/bin/env python

# http://mathhub.info/MitM/Modelss?VanRoosbrockEg


from mpdbase import *

class MPD_VanRoosbrockEg(MPDBase):
	def __init__(self, space, integration_surfaces=[]):
		MPDBase.__init__(self, 
			'VanRoosbrockEg',
			'http://mathhub.info/MitM/Modelss',
			space,
			integration_surfaces)
		self.graph = [('electron_mobility', 'electron_current'), ('electron_density', 'electron_current'), ('current_density', 'electron_current'), ('elementary_charge', 'electron_current'), ('electron_density', 'electron_density_law'), ('quasifermi_potential', 'electron_density_law'), ('electrostatic_potential', 'electron_density_law'), ('band_energy', 'electron_density_law'), ('band_density', 'electron_density_law'), ('temperature', 'electron_density_law'), ('boltzmann_constant', 'electron_density_law'), ('elementary_charge', 'electron_density_law'), ('electric_field', 'electric_field_law'), ('displacement', 'displacement_relation'), ('electric_field', 'displacement_relation'), ('absolute_permittivity', 'displacement_relation'), ('doping_profile', 'charge_density_law'), ('electron_density', 'charge_density_law'), ('charge_density', 'charge_density_law'), ('elementary_charge', 'charge_density_law'), ('charge_density', 'gauss_law'), ('absolute_permittivity', 'permitivity_law'), ('relative_permittivity', 'permitivity_law'), ('electric_constant', 'permitivity_law')]

	def init_quantity_decls(self):
		self.quantity_decls['electron_mobility'] = QuantityDecl(
			parent = 'http://mathhub.info/MitM/Modelss?Quantities',
			name = 'electron_mobility',
			is_uniform = False,
			is_constant = False,
			dimension = '' 
		)

		self.quantity_decls['doping_profile'] = QuantityDecl(
			parent = 'http://mathhub.info/MitM/Modelss?Quantities',
			name = 'doping_profile',
			is_uniform = False,
			is_constant = False,
			dimension = 'VolumeDensity' 
		)

		self.quantity_decls['electron_density'] = QuantityDecl(
			parent = 'http://mathhub.info/MitM/Modelss?Quantities',
			name = 'electron_density',
			is_uniform = False,
			is_constant = False,
			dimension = 'VolumeDensity' 
		)

		self.quantity_decls['charge_density'] = QuantityDecl(
			parent = 'http://mathhub.info/MitM/Modelss?Quantities',
			name = 'charge_density',
			is_uniform = False,
			is_constant = False,
			dimension = '' 
		)

		self.quantity_decls['displacement'] = QuantityDecl(
			parent = 'http://mathhub.info/MitM/Modelss?Quantities',
			name = 'displacement',
			is_uniform = False,
			is_constant = False,
			dimension = '' 
		)

		self.quantity_decls['quasifermi_potential'] = QuantityDecl(
			parent = 'http://mathhub.info/MitM/Modelss?Quantities',
			name = 'quasifermi_potential',
			is_uniform = False,
			is_constant = False,
			dimension = 'ElectricPotential' 
		)

		self.quantity_decls['electrostatic_potential'] = QuantityDecl(
			parent = 'http://mathhub.info/MitM/Modelss?Quantities',
			name = 'electrostatic_potential',
			is_uniform = False,
			is_constant = False,
			dimension = 'ElectricPotential' 
		)

		self.quantity_decls['electric_field'] = QuantityDecl(
			parent = 'http://mathhub.info/MitM/Modelss?Quantities',
			name = 'electric_field',
			is_uniform = False,
			is_constant = False,
			dimension = 'ElectricField' 
		)

		self.quantity_decls['current_density'] = QuantityDecl(
			parent = 'http://mathhub.info/MitM/Modelss?Quantities',
			name = 'current_density',
			is_uniform = False,
			is_constant = False,
			dimension = 'ElectricCurrentDensity' 
		)

		self.quantity_decls['absolute_permittivity'] = QuantityDecl(
			parent = 'http://mathhub.info/MitM/Modelss?Quantities',
			name = 'absolute_permittivity',
			is_uniform = True,
			is_constant = False,
			dimension = 'ElectricalPermittivity' 
		)

		self.quantity_decls['relative_permittivity'] = QuantityDecl(
			parent = 'http://mathhub.info/MitM/Modelss?Quantities',
			name = 'relative_permittivity',
			is_uniform = True,
			is_constant = False,
			dimension = 'DimNone' 
		)

		self.quantity_decls['band_energy'] = QuantityDecl(
			parent = 'http://mathhub.info/MitM/Modelss?Quantities',
			name = 'band_energy',
			is_uniform = False,
			is_constant = True,
			dimension = 'Energy' 
		)

		self.quantity_decls['band_density'] = QuantityDecl(
			parent = 'http://mathhub.info/MitM/Modelss?Quantities',
			name = 'band_density',
			is_uniform = False,
			is_constant = True,
			dimension = 'VolumeDensity' 
		)

		self.quantity_decls['temperature'] = QuantityDecl(
			parent = 'http://mathhub.info/MitM/Modelss?Quantities',
			name = 'temperature',
			is_uniform = True,
			is_constant = True,
			dimension = 'Temperature' 
		)

		self.quantity_decls['boltzmann_constant'] = QuantityDecl(
			parent = 'http://mathhub.info/MitM/Modelss?Quantities',
			name = 'boltzmann_constant',
			is_uniform = True,
			is_constant = True,
			dimension = '' 
		)

		self.quantity_decls['elementary_charge'] = QuantityDecl(
			parent = 'http://mathhub.info/MitM/Modelss?Quantities',
			name = 'elementary_charge',
			is_uniform = True,
			is_constant = True,
			dimension = 'ElectricCharge' 
		)

		self.quantity_decls['electric_constant'] = QuantityDecl(
			parent = 'http://mathhub.info/MitM/Modelss?Quantities',
			name = 'electric_constant',
			is_uniform = True,
			is_constant = True,
			dimension = 'ElectricalPermittivity' 
		)

		self.quantity_decls['test_quantity2'] = QuantityDecl(
			parent = 'http://mathhub.info/MitM/Modelss?Quantities',
			name = 'test_quantity2',
			is_uniform = True,
			initial_value = numpy.array([[1.1 ,2.2 ,3.3] ,[4.4 ,5.5 ,6.6] ,[7.7 ,8.8 ,9.9]]),
			is_constant = True,
			dimension = 'ElectricalPermittivity' 
		)

		self.quantity_decls['test_quantity'] = QuantityDecl(
			parent = 'http://mathhub.info/MitM/Modelss?Quantities',
			name = 'test_quantity',
			is_uniform = True,
			initial_value = numpy.array([1.1 ,2.2 ,3.3]),
			is_constant = True,
			dimension = 'ElectricalPermittivity' 
		)

	def init_laws(self):
		self.laws['electron_continuity_eqn'] = Law(
			parent = 'http://mathhub.info/MitM/Modelss?VanRoosbrockEg',
			name = 'electron_continuity_eqn',
			solvables = [],
			law_test = lambda state: ((numpy.array(0.0)) - (divergence(state['current_density'], self.space))),
			used_quantities = ['current_density'] 
		)

		self.laws['electron_current'] = Law(
			parent = 'http://mathhub.info/MitM/Modelss?VanRoosbrockEg',
			name = 'electron_current',
			elementary_charge = lambda state: (((state['current_density'] / gradient(state['quasifermi_potential'], self.space)) / state['electron_density']) / state['electron_mobility']),
			solvables = ['current_density' ,'quasifermi_potential' ,'electron_density' ,'electron_mobility' ,'elementary_charge'],
			electron_density = lambda state: ((state['current_density'] / gradient(state['quasifermi_potential'], self.space)) / (state['elementary_charge'] * state['electron_mobility'])),
			current_density = lambda state: (((state['elementary_charge'] * state['electron_mobility']) * state['electron_density']) * gradient(state['quasifermi_potential'], self.space)),
			law_test = lambda state: (((((state['elementary_charge'] * state['electron_mobility']) * state['electron_density']) * gradient(state['quasifermi_potential'], self.space))) - (state['current_density'])),
			electron_mobility = lambda state: (((state['current_density'] / gradient(state['quasifermi_potential'], self.space)) / state['electron_density']) / state['elementary_charge']),
			used_quantities = ['current_density' ,'quasifermi_potential' ,'electron_density' ,'electron_mobility' ,'elementary_charge'] 
		)

		self.laws['electron_density_law'] = Law(
			parent = 'http://mathhub.info/MitM/Modelss?VanRoosbrockEg',
			name = 'electron_density_law',
			electrostatic_potential = lambda state: ((((numpy.log((state['electron_density'] / state['band_density'])) * (state['boltzmann_constant'] * state['temperature'])) + state['band_energy']) / state['elementary_charge']) + state['quasifermi_potential']),
			elementary_charge = lambda state: (((numpy.log((state['electron_density'] / state['band_density'])) * (state['boltzmann_constant'] * state['temperature'])) + state['band_energy']) / (state['electrostatic_potential'] - state['quasifermi_potential'])),
			solvables = ['electron_density' ,'temperature' ,'boltzmann_constant' ,'band_energy' ,'quasifermi_potential' ,'electrostatic_potential' ,'elementary_charge' ,'band_density'],
			temperature = lambda state: ((numpy.log((state['electron_density'] / state['band_density'])) * ((state['elementary_charge'] * (state['electrostatic_potential'] - state['quasifermi_potential'])) - state['band_energy'])) / state['boltzmann_constant']),
			electron_density = lambda state: (state['band_density'] * numpy.exp((((state['elementary_charge'] * (state['electrostatic_potential'] - state['quasifermi_potential'])) - state['band_energy']) / (state['boltzmann_constant'] * state['temperature'])))),
			quasifermi_potential = lambda state: ((((numpy.log((state['electron_density'] / state['band_density'])) * (state['boltzmann_constant'] * state['temperature'])) + state['band_energy']) / state['elementary_charge']) - state['electrostatic_potential']),
			boltzmann_constant = lambda state: ((numpy.log((state['electron_density'] / state['band_density'])) * ((state['elementary_charge'] * (state['electrostatic_potential'] - state['quasifermi_potential'])) - state['band_energy'])) / state['temperature']),
			law_test = lambda state: (((state['band_density'] * numpy.exp((((state['elementary_charge'] * (state['electrostatic_potential'] - state['quasifermi_potential'])) - state['band_energy']) / (state['boltzmann_constant'] * state['temperature']))))) - (state['electron_density'])),
			band_density = lambda state: (state['electron_density'] / numpy.exp((((state['elementary_charge'] * (state['electrostatic_potential'] - state['quasifermi_potential'])) - state['band_energy']) / (state['boltzmann_constant'] * state['temperature'])))),
			used_quantities = ['electron_density' ,'temperature' ,'boltzmann_constant' ,'band_energy' ,'quasifermi_potential' ,'electrostatic_potential' ,'elementary_charge' ,'band_density'],
			band_energy = lambda state: ((numpy.log((state['electron_density'] / state['band_density'])) * (state['boltzmann_constant'] * state['temperature'])) - (state['elementary_charge'] * (state['electrostatic_potential'] - state['quasifermi_potential']))) 
		)

		self.laws['electric_field_law'] = Law(
			parent = 'http://mathhub.info/MitM/Modelss?VanRoosbrockEg',
			name = 'electric_field_law',
			solvables = ['electric_field' ,'electrostatic_potential'],
			law_test = lambda state: (((- gradient(state['electrostatic_potential'], self.space))) - (state['electric_field'])),
			used_quantities = ['electric_field' ,'electrostatic_potential'],
			electric_field = lambda state: (- gradient(state['electrostatic_potential'], self.space)) 
		)

		self.laws['displacement_relation'] = Law(
			parent = 'http://mathhub.info/MitM/Modelss?VanRoosbrockEg',
			name = 'displacement_relation',
			absolute_permittivity = lambda state: (state['displacement'] / state['electric_field']),
			solvables = ['displacement' ,'electric_field' ,'absolute_permittivity'],
			law_test = lambda state: (((state['absolute_permittivity'] * state['electric_field'])) - (state['displacement'])),
			used_quantities = ['displacement' ,'electric_field' ,'absolute_permittivity'],
			electric_field = lambda state: (state['displacement'] / state['absolute_permittivity']),
			displacement = lambda state: (state['absolute_permittivity'] * state['electric_field']) 
		)

		self.laws['charge_density_law'] = Law(
			parent = 'http://mathhub.info/MitM/Modelss?VanRoosbrockEg',
			name = 'charge_density_law',
			elementary_charge = lambda state: (state['charge_density'] / (state['doping_profile'] - state['electron_density'])),
			charge_density = lambda state: (state['elementary_charge'] * (state['doping_profile'] - state['electron_density'])),
			doping_profile = lambda state: ((state['charge_density'] / state['elementary_charge']) + state['electron_density']),
			solvables = ['charge_density' ,'electron_density' ,'doping_profile' ,'elementary_charge'],
			electron_density = lambda state: ((state['charge_density'] / state['elementary_charge']) - state['doping_profile']),
			law_test = lambda state: (((state['elementary_charge'] * (state['doping_profile'] - state['electron_density']))) - (state['charge_density'])),
			used_quantities = ['charge_density' ,'electron_density' ,'doping_profile' ,'elementary_charge'] 
		)

		self.laws['gauss_law'] = Law(
			parent = 'http://mathhub.info/MitM/Modelss?VanRoosbrockEg',
			name = 'gauss_law',
			charge_density = lambda state: divergence(state['displacement'], self.space),
			solvables = ['charge_density' ,'displacement'],
			law_test = lambda state: ((divergence(state['displacement'], self.space)) - (state['charge_density'])),
			used_quantities = ['charge_density' ,'displacement'] 
		)

		self.laws['permitivity_law'] = Law(
			parent = 'http://mathhub.info/MitM/Modelss?VanRoosbrockEg',
			name = 'permitivity_law',
			absolute_permittivity = lambda state: (state['electric_constant'] * state['relative_permittivity']),
			relative_permittivity = lambda state: (state['absolute_permittivity'] / state['electric_constant']),
			solvables = ['absolute_permittivity' ,'relative_permittivity' ,'electric_constant'],
			electric_constant = lambda state: (state['absolute_permittivity'] / state['relative_permittivity']),
			law_test = lambda state: (((state['electric_constant'] * state['relative_permittivity'])) - (state['absolute_permittivity'])),
			used_quantities = ['absolute_permittivity' ,'relative_permittivity' ,'electric_constant'] 
		)

	def init_computation_steps(self):
		self.computation_steps['cycle1'] = ComputationStep(
			parent = 'http://mathhub.info/MitM/Modelss?VanRoosbrockEg?cycle1',
			name = 'cycle1',
			is_connected = True,
			number_of_substeps = 5,
			compute = lambda state: ((state['absolute_permittivity'] * (- gradient(((((numpy.log((((divergence(state['displacement'], self.space) / state['elementary_charge']) - state['doping_profile']) / state['band_density'])) * (state['boltzmann_constant'] * state['temperature'])) + state['band_energy']) / state['elementary_charge']) + state['quasifermi_potential']), self.space)))),
			substeps = [lambda state: (divergence(state['displacement'], self.space)),lambda state: (((state['charge_density'] / state['elementary_charge']) - state['doping_profile'])),lambda state: (((((numpy.log((state['electron_density'] / state['band_density'])) * (state['boltzmann_constant'] * state['temperature'])) + state['band_energy']) / state['elementary_charge']) + state['quasifermi_potential'])),lambda state: ((- gradient(state['electrostatic_potential'], self.space))),lambda state: ((state['absolute_permittivity'] * state['electric_field']))],
			law_quantity_pairs = [('gauss_law', 'charge_density'),('charge_density_law', 'electron_density'),('electron_density_law', 'electrostatic_potential'),('electric_field_law', 'electric_field'),('displacement_relation', 'displacement')],
			used_quantities = ['quasifermi_potential' ,'elementary_charge' ,'band_energy' ,'temperature' ,'boltzmann_constant' ,'band_density' ,'doping_profile' ,'displacement' ,'absolute_permittivity'],
			is_cyclic = True 
		)

