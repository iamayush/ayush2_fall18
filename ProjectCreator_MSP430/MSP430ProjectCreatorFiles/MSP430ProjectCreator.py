import os
import sys
# sys.path.append(os.path.abspath(os.path.dirname(__file__) + '/'))
# print(os.path.abspath(os.path.dirname(__file__) + '/'))
# sys.path.append(os.path.abspath(os.path.dirname(__file__) + '/' + '../..'))
import F2272Project # CreateProjectF2272
import G2553Project # CreateProjectG2553
import re
import webbrowser
from tkinter import *
from tkinter import filedialog
from tkinter import messagebox

def resource_path(relative_path):
    # Get absolute path to resource, works for dev and for PyInstaller 
    base_path = getattr(sys, '_MEIPASS', os.path.dirname(os.path.abspath(__file__)))
    return os.path.join(base_path, relative_path)

class ProjectCreatorGUI():

	def __init__(self, master):

		self.master = master
		self.master.title("CCS MSP430 Project Creator")
		self.master.geometry("404x262")
		self.master.resizable(width=False, height=False)

		self.MCU_OPTIONS = ["MSP430F2272", "MSP430G2553"]
		self.select_mcu = StringVar()
		self.select_mcu.set(self.MCU_OPTIONS[0])

		self.project_name = StringVar()
		self.project_dir = StringVar()
		self.project_dir_valid = False

		# COECSL Logo
		coecsl_logo_path = resource_path('COECSL.gif')
		self.coecsl_logo = PhotoImage(file=coecsl_logo_path)
		self.coscsl_label = Label(self.master, image=self.coecsl_logo)
		self.coscsl_label.grid(row=0, column=0, rowspan=3, padx=205, sticky=W)

		coecsl_website = Label(self.master, fg="blue", text="http://coecsl.ece.illinois.edu", 
			                   font=("Helvetica", 11), cursor="hand2")
		coecsl_website.grid(row=2, rowspan=2, column=0, padx=206, sticky=W)
		coecsl_website.bind("<Button-1>", self.open_coecsl_website)

		# msp430 label
		msp430_label = Label(self.master, text="Select MSP430 Controller: ", font=("Helvetica", 11))
		msp430_label.grid(row=0, padx=10, pady=10, sticky=W)

		self.msp430_option_menu = OptionMenu(self.master, self.select_mcu, *self.MCU_OPTIONS)
		self.msp430_option_menu.grid(row=1, padx=10, sticky=W)
		self.msp430_option_menu.config(width=16, font=("Helvetica", 11))

		# new project label
		new_project_label = Label(self.master, text="Enter New Project's Name: ", font=("Helvetica", 11))
		new_project_label.grid(row=2, padx=10, pady=10, sticky=W)

		self.project_name.trace("w", lambda name, index, mode: self.project_name_change_func())
		self.new_project_name = Entry(self.master, width=19, font=("Helvetica", 12), textvariable=self.project_name)
		self.new_project_name.grid(row=3, padx=10, sticky=W)

		# new project directory label
		new_project_dir_label = Label(self.master, text="Select New Project's Directory: ", font=("Helvetica", 11))
		new_project_dir_label.grid(row=4, padx=10, pady=10, sticky=W)

		self.new_project_dir = Entry(self.master, width=42, font=("Helvetica", 12), state='readonly',
			                         textvariable=self.project_dir)
		self.new_project_dir.grid(row=5, padx=10, sticky=W)

		# setup project directory browse
		self.browse_button = Button(text="Browse", width=13, font=("Helvetica", 10), command=self.choose_project_dir)
		self.browse_button.grid(row=6, padx=10, pady=15, sticky=W)
		# initially disable browse button
		# self.browse_button.config(state = self.browse_button_state)

		# create new project
		self.create_button = Button(text="Create", width=13, font=("Helvetica", 10), command=self.create_msp430_project, state="disable")
		self.create_button.grid(row=6, padx=144, pady=15, sticky=W)

		# exit program
		self.exit_button = Button(text="Exit", width=13, font=("Helvetica", 10), command=self.quit_program)
		self.exit_button.grid(row=6, padx=278, pady=15, sticky=W)

	def open_coecsl_website(self, event):
		webbrowser.open_new(r"http://coecsl.ece.illinois.edu")

	def quit_program(self):
		self.master.quit()

	def project_name_change_func(self):

		if not re.match("^[A-Za-z0-9_-]*$", self.project_name.get()):
			messagebox.showwarning("Invalid Project Name", "Project's name can only be created by " + \
				                    "letters, numbers, dash and underscore!")
			project_name_tmp = self.project_name.get()
			# delete the last invalid character
			self.project_name.set(project_name_tmp[:-1])

		if self.project_dir_valid:
			old_project_dir = self.project_dir.get()
			old_project_dir = old_project_dir.split("\\")[:-1]
			old_project_dir = "\\".join(old_project_dir)
			new_project_name = "\\" + self.project_name.get()
			self.project_dir.set(old_project_dir+new_project_name)

		if self.project_name.get() == "":
			self.create_button.config(state="disable")
		else:
			self.create_button.config(state="normal")

	def choose_project_dir(self):

		# self.project_name = self.new_project_name.get()

		if self.project_name.get() == "":
			messagebox.showwarning("Invalid Project Name", "Please input a project's name!")
		else:
			project_dir = filedialog.askdirectory(initialdir="C:\\")
			if not re.match("^[A-Za-z0-9/_:-]*$", project_dir):
				messagebox.showwarning("Invalid Project Path", "Project's path cannot contain " + \
					                   "special characters, like ~!@#$%^&*()+=|...")
			else:
				project_dir = project_dir.replace("/", "\\")
				project_dir = project_dir + "\\" + self.project_name.get()
				self.project_dir.set(project_dir)
				self.project_dir_valid = True
				# print(self.project_dir.get())

	def create_msp430_project(self):

		project_dir = self.project_dir.get()
		project_msp430 = self.select_mcu.get()

		if project_msp430 == "MSP430F2272":
			project_to_create = F2272Project.CreateProjectF2272(project_dir)
			project_to_create.create_project()
		elif project_msp430 == "MSP430G2553":
			project_to_create = G2553Project.CreateProjectG2553(project_dir)
			project_to_create.create_project()
		else:
			messagebox.showwarning("Impossible", "You are not supposed to be here!")

if __name__ == '__main__':
	root = Tk()
	new_project = ProjectCreatorGUI(root)
	root.mainloop()





