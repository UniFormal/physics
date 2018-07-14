import numpy
from networkx.algorithms import bipartite
import networkx as nx
import matplotlib.pyplot as plt


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
                        lambda x: not self.quantity_decls[x].is_uniform,
                        self.quantity_decls.keys())
		self.uniform_quantity_decls_keys = filter(
                        lambda x: self.quantity_decls[x].is_uniform,
                        self.quantity_decls.keys())

		self.laws = {}
		self.init_laws()

		self.computation_steps = {}
		self.init_computation_steps()

		self.graph = []

                self.cycles = None

	def init_laws(self):
		pass

	def init_quantity_decls(self):
		pass

	def init_computation_steps(self):
		pass

	def __getitem__(self, law_key):
		return self.laws[law_key]

        def __str__(self):
		s = "----MPD----\n"
		s += "--name: " + self.name + '\n'
		s += "--parent: " + self.parent + '\n'
		
		for qk in self.quantity_decls:
			s += str(self.quantity_decls[qk]) + '\n'
		for lk in self.laws:
			s += str(self.laws[lk]) + '\n'
		
		return s

        def pretty_mpd_graph(self):
                graph = filter(lambda x: not self.quantity_decls[x[0]].is_constant, self.graph)
                titlify = lambda s: s.replace("_", "\n").title()
                graph = map(lambda x: (titlify(x[0]), titlify(x[1])), graph)
                return graph

        def plot_mpd_graphic(self):
                edges = self.pretty_mpd_graph()
                quantity_nodes = map(lambda e: e[0], edges)
                law_nodes = map(lambda e: e[1], edges)

                G=nx.MultiGraph()

                G.add_nodes_from(quantity_nodes, bipartite=0)
                G.add_nodes_from(law_nodes, bipartite=1)
                G.add_edges_from(edges)
                print(nx.is_connected(G))
                pos=nx.spring_layout(G, iterations=1000, k=1.0/numpy.sqrt(len(G)), scale=2.0)
                #print(pos)
                g0, g1 = bipartite.sets(G)


                nx.draw_networkx_nodes(g0, pos, with_labels = True, node_size=300, scale = 300, node_shape="o", prog='dot', node_color=(1, 0.8, 0.8))

                nx.draw_networkx_nodes(g1, pos, with_labels = True, node_size=600, scale = 300, node_shape="s", prog='dot',node_color=(0.7, 0.8, 1.0))

                nx.draw_networkx_edges(G, pos, edge_color="grey", style="dashed")

                nx.draw_networkx_labels(G, pos, font_size=8)

                plt.show()
        
        def init_cycles(self):
                def traverse(hist):
                        start = hist[0]
                        for edge in self.graph:
                                # edge[0] is quantity node, edge[1] is law node
                                if edge[1] == hist[0][1]:
                                        continue
                                
                                if not self.laws[edge[1]].uses_quantity_of_name(hist[0][0]):
                                        continue

                                if edge not in hist:
                                        if edge[0] in map(lambda x: x[0], hist):
                                                # edge is partially in; kill it
                                                traverse(hist.append(edge))
                                        elif len(hist) > 2 and edge == hist[-1]:
                                                self.cycles.append(Cycle(hist).normalize())

                for edge in graph:
                        traverse([edge])

                # we cast to set to remove repeating elements
                self.cycles = list(set(self.cycles))

class ComputationStep:
	def __init__(self, **kwargs):
		self.name = kwargs["name"]
		self.parent = kwargs["parent"]
		self.used_quantity_names = kwargs["used_quantities"]
		self.law_quantity_name_pairs = kwargs["law_quantity_pairs"]
		self.compute_lambda = kwargs["compute"]
		self.substeps_lambdas = kwargs["substeps"]
		self.number_of_substeps = kwargs["number_of_substeps"]
		self.is_connected = kwargs["is_connected"]
		self.is_cyclic = kwargs["is_cyclic"]

	def __str__(self):
		s = "name: " + self.name + '\n'
		s += "parent: " + self.parent + '\n'
		s += "computes quantity: " + self.quantityDecl.name
		s += "from law: " + self.law.name 
		return s

	def compute(self, state):
		return self.compute_lambda(state)

	def compute_and_update(self, state):
		state[self.law_quantity_name_pairs[-1][1]] = self.compute_lambda(state)

        def compute_substeps_and_update(self, state):
                for pair in self.law_quantity_name_pairs:
                        state[pair[1]] = state.mpd[pair[0]][pair[1]](state)

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
                self.tensor_shape = kwargs["tensor_shape"]
                if "initial_value" in kwargs.keys():
                        self.initial_value = kwargs["initial_value"]
                else:
                        vec = numpy.array(0.0)
                        for x in self.tensor_shape:
                                vec = numpy.repeat(vec, x, axis=0)
                        self.initial_value = vec
                        
		self.dimension = kwargs["dimension"]
		self.is_uniform = kwargs["is_uniform"]
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

        def uses_quantity_of_name(self, name):
                return name in self.used_quantities
        
        def test_law(self, state):
                return self.law_test(state)

class MPDState:
	def __init__(self, mpd):
		self.__dict__['mpd'] = mpd
                self.__dict__['state_values'] = {}
                for n in mpd.quantity_decls:
                        i = mpd.quantity_decls[n].initial_value
                        shape = mpd.space.shape
                        if len(i.shape) == 0:
                                i = numpy.array([float(i)])
                        if len(shape) == 1:
                                self.state_values[n] = numpy.tile(i.flatten(), shape[0]).reshape(list(shape)+list(i.shape))
                        else:
                                allG = [numpy.repeat(i, shape[x+1], axis=0) for x in range(shape[0])]
                                self.state_values[n] = numpy.meshgrid(*allG)
                        
	def update(self):
		for q in self.mpd.quantity_decls:
			if isinstance(q, DerivedQuantityDecl):
				q.update(self.state_values)

	def __str__(self):
		s = "----MPD State----\n"

		for sk in self.state_values:
                        print(sk)
			s += sk + ": " + str(self.state_values[sk]) + '\n'
		
		return s

        def gauss_seidel(self, cycle_name, epsilon=1e-3):
                while True:
                        self.mpd.computation_steps[cycle_name].compute_substeps_and_update(self)
                        if (self.mpd[self.mpd.computation_steps[cycle_name].law_quantity_name_pairs[-1][0]].law_test(self) < epsilon).all():
                                break

        
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
                        if self.state_values[sk] is None or other_state.state_values[sk] is None:
                                continue
                        self.state_values[sk] = self.state_values[sk]*(1.0-lambda_) + other_state[sk]*(lambda_)

	def interpolate(self, other_state, lambda_beg, lambda_end, lambda_):
		self.interpolate01(other_state, (lambda_-lambda_beg)/(lambda_end-lambda_beg))

	def copy_from(self, other_state):
		self.interpolate01(other_state, 1.0)

	def copy_to(self, other_state):
		other_state.copy_from(self)

class TacticBase:
        def __init__():
                pass

def derivative_on_space(image, dom):
        deriv = numpy.diff(image)/numpy.diff(dom)
        return numpy.append(deriv, deriv[-1])

def gradient(scalar_field, dom_grids):
        #ds = numpy.diff(dom_grids[-1])[0][0]
        #print(ds)
        #print(scalar_field)
        return numpy.gradient(scalar_field.reshape([10]))

def divergence(vector_field, dom_grids):
        #ds = numpy.diff(dom_grids[-1])[0][0]
        div = numpy.sum(numpy.gradient(vector_field))
        #print("Div", vector_field, div)
        return div
