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


deploy_config = GenericTemplate(j2_env.get_template(
	'config_deploiement.j2'))
deploy_config['buzzer_time_between_sequences'] = 4000
deploy_config['buzzer_cycle_duration'] = 500
deploy_config['altitude_filter_order'] = 3
deploy_config.to_file('test.out')
