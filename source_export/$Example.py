#!/usr/bin/env python

# http://mathhub.info/MitM/Models?Example


from mpdbase import *

class MPD_Example(MPDBase):
	def __init__(self, space, integration_surfaces=[]):
		MPDBase.__init__(self, 
			'Example',
			'http://mathhub.info/MitM/Models',
			space,
			integration_surfaces)

	def init_quantity_decls(self):
		self.quantity_decls['b'] = QuantityDecl(
			parent = 'http://mathhub.info/MitM/Models?Example',
			name = 'b',
			compute = lambda state: state['QEInteg'],
			is_constant = False,
			dimension = 'ElectricPotential',
			is_field = False 
		)

		self.quantity_decls['RR'] = QuantityDecl(
			parent = 'http://mathhub.info/MitM/Models?Example',
			name = 'RR',
			is_constant = False,
			dimension = 'DimNone',
			is_field = True 
		)

		self.quantity_decls['R'] = QuantityDecl(
			parent = 'http://mathhub.info/MitM/Models?Example',
			name = 'R',
			is_constant = False,
			dimension = 'ElectricalResistance',
			is_field = True 
		)

		self.quantity_decls['D'] = QuantityDecl(
			parent = 'http://mathhub.info/MitM/Models?Example',
			name = 'D',
			is_constant = False,
			dimension = '',
			is_field = True 
		)

		self.quantity_decls['I'] = QuantityDecl(
			parent = 'http://mathhub.info/MitM/Models?Example',
			name = 'I',
			is_constant = False,
			dimension = 'Current',
			is_field = True 
		)

		self.quantity_decls['U'] = QuantityDecl(
			parent = 'http://mathhub.info/MitM/Models?Example',
			name = 'U',
			is_constant = False,
			dimension = 'ElectricPotential',
			is_field = True 
		)

		self.quantity_decls['derivedQuantityEx'] = QuantityDecl(
			parent = 'http://mathhub.info/MitM/Models?Quantities2',
			name = 'derivedQuantityEx',
			compute = lambda state: (state['r'] * state['i']),
			is_constant = False,
			dimension = 'ElectricPotential',
			is_field = False 
		)

		self.quantity_decls['p'] = QuantityDecl(
			parent = 'http://mathhub.info/MitM/Models?Quantities2',
			name = 'p',
			is_constant = False,
			dimension = 'Power',
			is_field = False 
		)

		self.quantity_decls['c'] = QuantityDecl(
			parent = 'http://mathhub.info/MitM/Models?Quantities2',
			name = 'c',
			is_constant = False,
			dimension = 'ElectricalCapacitance',
			is_field = False 
		)

		self.quantity_decls['t'] = QuantityDecl(
			parent = 'http://mathhub.info/MitM/Models?Quantities2',
			name = 't',
			is_constant = False,
			dimension = 'Time',
			is_field = False 
		)

		self.quantity_decls['q'] = QuantityDecl(
			parent = 'http://mathhub.info/MitM/Models?Quantities2',
			name = 'q',
			is_constant = False,
			dimension = 'ElectricCharge',
			is_field = False 
		)

		self.quantity_decls['r'] = QuantityDecl(
			parent = 'http://mathhub.info/MitM/Models?Quantities2',
			name = 'r',
			is_constant = False,
			dimension = 'ElectricalResistance',
			is_field = False 
		)

		self.quantity_decls['i'] = QuantityDecl(
			parent = 'http://mathhub.info/MitM/Models?Quantities2',
			name = 'i',
			is_constant = False,
			dimension = 'Current',
			is_field = False 
		)

		self.quantity_decls['v'] = QuantityDecl(
			parent = 'http://mathhub.info/MitM/Models?Quantities2',
			name = 'v',
			is_constant = False,
			dimension = 'ElectricPotential',
			is_field = False 
		)

		self.quantity_decls['u'] = QuantityDecl(
			parent = 'http://mathhub.info/MitM/Models?Quantities2',
			name = 'u',
			is_constant = False,
			dimension = 'ElectricPotential',
			is_field = False 
		)

	def init_laws(self):
		self.laws['law5'] = Law(
			parent = 'http://mathhub.info/MitM/Models?Example',
			name = 'law5',
			solvables = ['p' ,'i' ,'v'],
			i = lambda state: (state['p'] / state['v']),
			v = lambda state: (state['p'] / state['i']),
			p = lambda state: (state['i'] * state['v']),
			law_test = lambda state: ((state['p']) - ((state['i'] * state['v']))),
			used_quantities = ['v' ,'i' ,'p'] 
		)

		self.laws['law4'] = Law(
			parent = 'http://mathhub.info/MitM/Models?Example',
			name = 'law4',
			u = lambda state: (state['p'] / state['i']),
			solvables = ['p' ,'i' ,'u'],
			i = lambda state: (state['p'] / state['u']),
			p = lambda state: (state['i'] * state['u']),
			law_test = lambda state: ((state['p']) - ((state['i'] * state['u']))),
			used_quantities = ['u' ,'i' ,'p'] 
		)

		self.laws['law3'] = Law(
			parent = 'http://mathhub.info/MitM/Models?Example',
			name = 'law3',
			u = lambda state: (state['q'] / state['c']),
			solvables = ['q' ,'u' ,'c'],
			q = lambda state: (state['c'] * state['u']),
			law_test = lambda state: ((state['q']) - ((state['c'] * state['u']))),
			c = lambda state: (state['q'] / state['u']),
			used_quantities = ['u' ,'c' ,'q'] 
		)

		self.laws['law2'] = Law(
			parent = 'http://mathhub.info/MitM/Models?Example',
			name = 'law2',
			t = lambda state: (state['q'] / state['i']),
			solvables = ['q' ,'i' ,'t'],
			i = lambda state: (state['q'] / state['t']),
			q = lambda state: (state['i'] * state['t']),
			law_test = lambda state: ((state['q']) - ((state['i'] * state['t']))),
			used_quantities = ['t' ,'i' ,'q'] 
		)

		self.laws['law_diff'] = Law(
			parent = 'http://mathhub.info/MitM/Models?Example',
			name = 'law_diff',
			solvables = ['D'],
			law_test = lambda state: ((state['D']) - ((derivative_on_space(state['U'], self.space))),
			used_quantities = ['U' ,'D'],
			D = lambda state: (derivative_on_space(state['U'], self.space) 
		)

		self.laws['law1'] = Law(
			parent = 'http://mathhub.info/MitM/Models?Example',
			name = 'law1',
			u = lambda state: (state['r'] * state['i']),
			solvables = ['u' ,'r' ,'i'],
			i = lambda state: (state['u'] / state['r']),
			law_test = lambda state: ((state['u']) - ((state['r'] * state['i']))),
			used_quantities = ['i' ,'r' ,'u'],
			r = lambda state: (state['u'] / state['i']) 
		)

		self.laws['law33'] = Law(
			parent = 'http://mathhub.info/MitM/Models?Example',
			name = 'law33',
			solvables = ['RR'],
			RR = lambda state: (state['RR'] ** state['RR']),
			law_test = lambda state: ((state['RR']) - ((state['RR'] ** state['RR']))),
			used_quantities = ['RR' ,'RR' ,'RR'] 
		)

		self.laws['law11'] = Law(
			parent = 'http://mathhub.info/MitM/Models?Example',
			name = 'law11',
			U = lambda state: (state['R'] * state['I']),
			solvables = ['U'],
			law_test = lambda state: ((state['U']) - ((state['R'] * state['I']))),
			used_quantities = ['I' ,'R' ,'U'] 
		)


