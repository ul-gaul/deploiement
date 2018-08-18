"""Generate all the deployment system's code."""


# misc imports
from jinja2 import Environment, FileSystemLoader
# template generators imports
from deployment_configuration import DeploymentConfiguration


j2_env = Environment(loader=FileSystemLoader('templates'), trim_blocks=True)


class GenericTemplate:

	def __init__(self, template):
		self.template = template
		self.kwargs = {}

	def render(self):
		return self.template.render(**self.kwargs) + '\n'

	def to_file(self, output_path):
		with open(output_path, 'w') as f:
			f.write(self.render())

	def __getitem__(self, key):
		return self.kwargs[key]

	def __setitem__(self, key, value):
		self.kwargs[key] = value


deployment_configuration = GenericTemplate(j2_env.get_template('test.j2'))
deployment_configuration['a'] = 'allo'
deployment_configuration['b'] = 'asldfjasl;djkf asdf;asjldfkljasd'
deployment_configuration.to_file('test.out')
