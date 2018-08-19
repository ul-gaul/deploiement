"""Command line interface to generate the code."""


# general imports
import sys
# module specific imports
from code_generator.deployment_generator import make_deploy_config


class BaseMenu:

	def __init__(self, description):
		self.description = description

	def __str__(self):
		return f'BaseMenu({self.description})'

	def __call__(self):
		print(self.description)

	def display_help(self):
		print('Enter \'exit\' at any time to exit')
		print('Enter \'help\' at any time to display this message')

	def make_choice(self):
		choice = input()
		if choice == 'exit':
			sys.exit()
		elif choice == 'help':
			self.display_help()
		else:
			return choice


class MainMenu(BaseMenu):

	def __call__(self):
		super().__call__()
		print("Choose from the following options:")
		for i, menu in enumerate(self.menus):
			print(i, ' - ', menu.description)
		try:
			choice = self.make_choice()
			if not choice: return
			next_menu = self.menus[int(choice)]
		except (IndexError, ValueError):
			print("Invalid choice, try again")
			return
		next_menu()


class ExitMenu(BaseMenu):

	def __call__(self):
		super().__call__()
		sys.exit()


class FlightConfigMenu(BaseMenu):

	def __init__(self, *args):
		super().__init__(*args)
		self.deploy_config = make_deploy_config()

	def __call__(self):
		super().__call__()
		self.list_params()
		choice = self.make_choice()
		if not choice: return
		self.edit_param(int(choice))

	def list_params(self):
		print('Choose which parameter to edit:')
		print("i - parameter - current value")
		self.params = list(self.deploy_config.kwargs.items())
		print(self.params)
		for i, (k, v) in enumerate(self.params):
			print(i, '-', k, ':', v)

	def edit_param(self, index):
		param = self.params[index][0]
		print('Enter the new value for:', param)
		new_value = input()
		self.deploy_config[param] = new_value


class GenerateCodeMenu(BaseMenu):

	def __call__(self):
		super().__call__()
		input('generate code!')


def main():
	main_menu = MainMenu('GAUL - Deployment code generator')
	edit_flight_conf = FlightConfigMenu("Edit the flight's parameters")
	generate_code = GenerateCodeMenu('Generate the code')
	exit_menu = ExitMenu('Exit')
	main_menu.menus = [edit_flight_conf, generate_code, exit_menu]
	while True:
		main_menu()

if __name__ == '__main__':
	main()
