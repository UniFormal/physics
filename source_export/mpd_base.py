
class MPDBase:
	def __init__(self, name, parent, regions):
		self.name = ""
		self.parent = ""
                self.regions = regions
        	self.laws = {}
		self.quantity_decls = {}
		self.init_quantity_decls()
		self.init_laws()

	def init_laws(self):
		pass

	def init_quantity_decls(self):
		pass

	def __getitem__(self, law_key):
		return self.laws[law_key]

	def __getattr__(self, quantity_key):
		return self.quantity_decls[quantity_key]

        def __str__(self):
		s = "----MPD----\n"
		s += "--name: " + self.name + '\n'
		s += "--parent: " + self.parent + '\n'
		s += "--regions: " + str(self.regions) + '\n'
		
		for qk in self.quantity_decls:
			s += str(self.quantity_decls[qk]) + '\n'
		for lk in self.laws:
			s += str(self.laws[lk]) + '\n'
		
		return s


class Declaration:
	def __init__(self, name, parent):
		self.name = name
		self.parent = parent
	
	def __str__(self):
		s = "name: " + self.name + '\n'
		s += "parent: " + self.parent
		return s

class QuantityDecl(Declaration):
	def __init__(self, **kwargs):
		Declaration.__init__(self, kwargs["name"], kwargs["parent"])
		self.init_value = 0.0
		self.dimension = kwargs["dimension"]

	def __str__(self):
		s = "---Quantity Decl---\n"
		s += Declaration.__str__(self) + '\n'
		s += "dimension: " + self.dimension
		return s


class DerivedQuantityDecl(QuantityDecl):
	def __init__(self, **kwargs):
		QuantityDecl.__init__(self, **kwargs)
		self.compute_lambda = kwargs["compute"]

	def compute(self, state):
		return self.compute_lambda(state)

	def update(self, state): # side effect: modifies state
		state[self.name] = self.compute(state)

	def __str__(self):
		s = "---Derived Quantity Decl---\n"
		s += Declaration.__str__(self) + '\n'
		s += "dimension: " + self.dimension
		return s

class FieldDecl(Declaration):
	def init(self, regions):
		self.init_value = [0.0 for i in range(regions)]

class Law(Declaration):
	def __init__(self, **kwargs):
		Declaration.__init__(self, kwargs.pop("name"), kwargs.pop("parent"))
		self.variables = kwargs.pop("variables")
		self.rules = kwargs

	def __getitem__(self, rule_subject):
		self.rules[rule_subject]

	def __str__(self):
		s = "---Law---\n"
		s += Declaration.__str__(self) + '\n'
		s += "variables: " + self.variables.join(',') 
		return s

class MPDState:
	def __init__(self, mpd):
		self.mpd = mpd
		self.state_values = []

	def update(self):
		for q in self.mpd.quantity_decls:
			if isinstance(q, DerivedQuantityDecl):
				q.update(self.state_values)

	def __str__(self):
		s = "----MPD State----\n"

		for sk in self.state_values:
			s += sk + ": " + str(self.state_values[sk]) + '\n'
		
		return s

	def __getitem__(self, state_value_key):
		return self.state_values[state_value_key]

        def _interpolate01_field(f1, f2, lambda_):
                for i in range(mpd.regions):
                        f1[i] = f1[i]*(1.0-lambda_) + f2[i]*lambda_
                        
        
	# interpolates values between this state and the other state by a factor of lambda [0, 1]  
	def interpolate01(self, other_state, lambda_):
		for sk in self.state_values:
                        if isinstance(mpd.quantity_decls[sk], FieldDecl):
                                _interpolate01_field(self.state_values[sk], other_state[sk], lambda_)
			else:
                                self.state_values[sk] = self.state_values[sk]*(1.0-lambda_) + other_state[sk]*(lambda_)

	def interpolate(self, other_state, lambda_beg, lambda_end, lambda_):
		self.interpolate01(other_state, (lambda_-lambda_beg)/(lambda_end-lambda_beg))

	def copy_from(self, other_state):
		self.interpolate01(other_state, 1.0)

	def copy_to(self, other_state):
		other_state.copy_from(self)

	
