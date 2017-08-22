#!/usr/bin/env python

# http://mathhub.info/MitM/Models?Example


from mpdbase import *

class MPD_Example(MPDBase):
	def __init__(self, regions=1):
		MPDBase.__init__(self, 
			regions,
			'Example',
			'http://mathhub.info/MitM/Models')

	def init_quantity_decls(self):
		self.quantity_decls['derivedQuantityEx'] = QuantityDecl(
			name = 'derivedQuantityEx',
			parent = 'http://mathhub.info/MitM/Models?Cons1',
			dimension = 'ElectricPotential',
			compute = lambda state: (state['r'] * state['i']) 
		)

		self.quantity_decls['p'] = QuantityDecl(
			name = 'p',
			parent = 'http://mathhub.info/MitM/Models?Cons1',
			dimension = 'Power' 
		)

		self.quantity_decls['c'] = QuantityDecl(
			name = 'c',
			parent = 'http://mathhub.info/MitM/Models?Cons1',
			dimension = 'ElectricalCapacitance' 
		)

		self.quantity_decls['t'] = QuantityDecl(
			name = 't',
			parent = 'http://mathhub.info/MitM/Models?Cons1',
			dimension = 'Time' 
		)

		self.quantity_decls['q'] = QuantityDecl(
			name = 'q',
			parent = 'http://mathhub.info/MitM/Models?Cons1',
			dimension = 'ElectricCharge' 
		)

		self.quantity_decls['r'] = QuantityDecl(
			name = 'r',
			parent = 'http://mathhub.info/MitM/Models?Cons1',
			dimension = 'ElectricalResistance' 
		)

		self.quantity_decls['i'] = QuantityDecl(
			name = 'i',
			parent = 'http://mathhub.info/MitM/Models?Cons1',
			dimension = 'Current' 
		)

		self.quantity_decls['v'] = QuantityDecl(
			name = 'v',
			parent = 'http://mathhub.info/MitM/Models?Cons1',
			dimension = 'ElectricPotential' 
		)

		self.quantity_decls['u'] = QuantityDecl(
			name = 'u',
			parent = 'http://mathhub.info/MitM/Models?Cons1',
			dimension = 'ElectricPotential' 
		)

	def init_laws(self):
		self.laws['law5'] = Law(
			parent = 'http://mathhub.info/MitM/Models?Example',
			name = 'law5',
			variables = ['p' ,'i' ,'v'],
			i = lambda state: (state['p'] / state['v']),
			v = lambda state: (state['p'] / state['i']),
			p = lambda state: (state['i'] * state['v']),
			law_test = lambda state: ((state['p']) - ((state['i'] * state['v']))) 
		)

		self.laws['law4'] = Law(
			parent = 'http://mathhub.info/MitM/Models?Example',
			name = 'law4',
			u = lambda state: (state['p'] / state['i']),
			variables = ['p' ,'i' ,'u'],
			i = lambda state: (state['p'] / state['u']),
			p = lambda state: (state['i'] * state['u']),
			law_test = lambda state: ((state['p']) - ((state['i'] * state['u']))) 
		)

		self.laws['law3'] = Law(
			parent = 'http://mathhub.info/MitM/Models?Example',
			name = 'law3',
			u = lambda state: (state['q'] / state['c']),
			variables = ['q' ,'u' ,'c'],
			q = lambda state: (state['c'] * state['u']),
			law_test = lambda state: ((state['q']) - ((state['c'] * state['u']))),
			c = lambda state: (state['q'] / state['u']) 
		)

		self.laws['law2'] = Law(
			parent = 'http://mathhub.info/MitM/Models?Example',
			name = 'law2',
			t = lambda state: (state['q'] / state['i']),
			variables = ['q' ,'i' ,'t'],
			i = lambda state: (state['q'] / state['t']),
			q = lambda state: (state['i'] * state['t']),
			law_test = lambda state: ((state['q']) - ((state['i'] * state['t']))) 
		)

		self.laws['law1'] = Law(
			parent = 'http://mathhub.info/MitM/Models?Example',
			name = 'law1',
			u = lambda state: (state['r'] * state['i']),
			variables = ['u' ,'r' ,'i'],
			i = lambda state: (state['u'] / state['r']),
			law_test = lambda state: ((state['u']) - ((state['r'] * state['i']))),
			r = lambda state: (state['u'] / state['i']) 
		)

