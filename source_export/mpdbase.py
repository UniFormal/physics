import numpy

class MPDBase(object):
	def __init__(self, name, parent, space, surface_integrators = []):
		self.name = ""
		self.parent = ""

                self.space = space
		self.surface_integrators = surface_integrators

		self.quantity_decls = {}
                self.init_quantity_decls()
                
                self.variable_quantity_decls_keys = filter(
                        lambda x: not self.quantity_decls[x].is_constant,
                        self.quantity_decls.keys())
		self.constant_quantity_decls_keys = filter(
                        lambda x: self.quantity_decls[x].is_constant,
                        self.quantity_decls.keys())
		self.field_quantity_decls_keys = filter(
                        lambda x: self.quantity_decls[x].is_field,
                        self.quantity_decls.keys())
		self.uniform_quantity_decls_keys = filter(
                        lambda x: not self.quantity_decls[x].is_field,
                        self.quantity_decls.keys())

		self.laws = {}
		self.init_laws()

		self.graph = []

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
		self.is_field = kwargs["is_field"]
		self.is_constant = kwargs["is_constant"]

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
        
class Law(Declaration):
	def __init__(self, **kwargs):
		Declaration.__init__(self, kwargs.pop("name"), kwargs.pop("parent"))
		self.solvables = kwargs.pop("solvables")
		self.used_quantities = kwargs.pop("used_quantities")
                self.law_test = kwargs.pop("law_test")
		self.rules = kwargs

	def __getitem__(self, rule_subject):
		return self.rules[rule_subject]

        def __getattr__(self, rule_subject):
                return self.rules[rule_subject]
                
	def __str__(self):
		s = "---Law---\n"
		s += Declaration.__str__(self) + '\n'
		s += "used quantities: " + ','.join(self.used_quantities)
		return s

        def test_law(self, state):
                return self.law_test(state)

class MPDState:
	def __init__(self, mpd):
		self.__dict__['mpd'] = mpd
                self.__dict__['state_values'] = {}
                for n in mpd.quantity_decls:
                        self.state_values[n] = None
                
	def update(self):
		for q in self.mpd.quantity_decls:
			if isinstance(q, DerivedQuantityDecl):
				q.update(self.state_values)

	def __str__(self):
		s = "----MPD State----\n"

		for sk in self.state_values:
			s += sk + ": " + str(self.state_values[sk]) + '\n'
		
		return s

	def __getitem__(self, quantity_name):
		return self.state_values[quantity_name]

        def __setitem__(self, quantity_name, val):
                self.state_values[quantity_name] = val

        def __getattr__(self, quantity_name):
                return self.state_values[quantity_name]

        def __setattr__(self, quantity_name, val):
                self.state_values[quantity_name] = val
        
        def test_law(self, law_name):
                return self.mpd.laws[law_name].test_law(self)
                        
        
	# interpolates values between this state and the other state by a factor of lambda [0, 1]  
	def interpolate01(self, other_state, lambda_):
		for sk in self.state_values:
                        self.state_values[sk] = self.state_values[sk]*(1.0-lambda_) + other_state[sk]*(lambda_)

	def interpolate(self, other_state, lambda_beg, lambda_end, lambda_):
		self.interpolate01(other_state, (lambda_-lambda_beg)/(lambda_end-lambda_beg))

	def copy_from(self, other_state):
		self.interpolate01(other_state, 1.0)

	def copy_to(self, other_state):
		other_state.copy_from(self)


def derivative_on_space(image, dom):
	deriv = numpy.diff(image)/numpy.diff(dom)
	return numpy.append(deriv, deriv[-1])

