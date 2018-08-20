"""Command line interface to generate the code."""


# general imports
import os
import sys
# module specific imports
from code_generator.deployment_generator import make_deploy_config


# default is posix systems
clear = lambda: os.system('clear')
# for windows systems
if os.name == 'nt':
	clear = lambda: os.system('cls')


class MenuSpecialExit(Exception):

	def  __init__(self, exit_num):
		self.exit_num = exit_num
		super().__init__()


class BaseMenu:
	# exit codes for menu calls
	EXIT_OK = 0
	RUN_MENU_AGAIN = 1
	HELP = 2
	EXIT = 3
	BACK = 4

	def __init__(self, description):
		self.description = description
		self.error_message = None

	def __str__(self):
		return f'BaseMenu({self.description})'

	def __call__(self):
		print('-' * 80)
		print(self.description)
		if self.error_message:
			print(self.error_message)
		print('-' * 80)

	def user_input(self):
		choice = input()
		if choice == 'exit':
			raise MenuSpecialExit(self.EXIT)
		elif choice == 'help':
			raise MenuSpecialExit(self.HELP)
		elif choice == 'back':
			raise MenuSpecialExit(self.BACK)
		else:
			return choice

	def set_error_message(self, msg):
		assert isinstance(msg, str)
		self.error_message = msg


class HelpMenu(BaseMenu):

	def __call__(self):
		super().__call__()
		print('Enter \'exit\' at any time to exit')
		print('Enter \'help\' at any time to display this message')
		print('Enter \'back\' at any time to return to the previous menu')
		self.user_input()


class MainMenu(BaseMenu):

	def __call__(self):
		super().__call__()
		print("Choose from the following options:")
		for i, menu in enumerate(self.menus):
			print(i, ' - ', menu.description)
		try:
			choice = self.user_input()
			if not choice: return
			next_menu = self.menus[int(choice)]
		except (IndexError, ValueError):
			print("Invalid choice, try again")
			return
		clear()
		next_menu()


class ExitMenu(BaseMenu):

	def __call__(self):
		sys.exit()


class FlightConfigMenu(BaseMenu):

	def __init__(self, deploy_config, *args):
		super().__init__(*args)
		self.deploy_config = deploy_config

	def __call__(self):
		super().__call__()
		self.list_params()
		choice = self.user_input()
		if not choice: return
		self.edit_param(int(choice))
		return self.RUN_MENU_AGAIN

	def list_params(self):
		print('Choose which parameter to edit:')
		print("i - parameter - current value")
		self.params = list(self.deploy_config.kwargs.items())
		for i, (k, v) in enumerate(self.params):
			print(i, '-', k, ':', v)

	def edit_param(self, index):
		param = self.params[index][0]
		print('Enter the new value for:', param)
		new_value = self.user_input()
		self.deploy_config[param] = new_value


class GenerateCodeMenu(BaseMenu):

	def __init__(self, deploy_config, *args):
		super().__init__(*args)
		self.deploy_config = deploy_config

	def __call__(self):
		super().__call__()
		print('The following files will be created:')
		print(self.deploy_config.name)
		print('Enter the path to the output file:')
		path_file = self.user_input()
		try:
			self.deploy_config.to_file(path_file)
		except FileNotFoundError:
			print('Invalid directory/path:', path_file)
			return self.RUN_MENU_AGAIN



def main():
	# make the flight config generic template
	deploy_config = make_deploy_config()
	# create menus
	main_menu = MainMenu('GAUL - Deployment code generator')
	edit_flight_conf = FlightConfigMenu(deploy_config,
		"Edit the flight's parameters")
	generate_code = GenerateCodeMenu(deploy_config, 'Generate the code')
	help_menu = HelpMenu('Help')
	exit_menu = ExitMenu('Exit')
	main_menu.menus = [edit_flight_conf, generate_code, help_menu, exit_menu]
	while True:
		clear()
		try:
			main_menu()
		except MenuSpecialExit as e:
			if e.exit_num == BaseMenu.HELP:
				clear()
				help_menu()
			elif e.exit_num == BaseMenu.EXIT:
				exit_menu()


if __name__ == '__main__':
	main()
