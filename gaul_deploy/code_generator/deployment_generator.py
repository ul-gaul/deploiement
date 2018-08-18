"""Generate all the deployment system's code."""


# misc imports
from jinja2 import Environment, FileSystemLoader


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
# sampling period is in micro-seconds
deploy_config['data_sampling_period'] = 100000
# speeds are in m/s but must be converted to m/sample in the config file
# for example, if the sampling period is 100000 us (10 Hz), then the converted
# speed value will be 3 (3 m/0.1s = 30 m/s)
deploy_config['breakpoint_speed_to_burnout'] = 30
deploy_config['breakpoint_speed_to_pre_drogue'] = 30
deploy_config['breakpoint_speed_to_idle'] = 0.1
# altitudes are in meters
deploy_config['breakpoint_altitude_to_burnout'] = 6
deploy_config['breakpoint_altitude_to_drift'] = 460
# number of samples (at sample rate) between apogee and drogue deployment
deploy_config['breakpoint_delta_time_apogee'] = 4
# expected altitude range in meters
deploy_config['flight_maximal_altitude'] = 0
deploy_config['flight_minimal_alitude'] = 4000
deploy_config.to_file('test.out')
