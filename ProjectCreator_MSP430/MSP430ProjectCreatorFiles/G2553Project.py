import os
import sys
import shutil
from tkinter import *
from tkinter import filedialog
from tkinter import messagebox

def resource_path(relative_path):
    # Get absolute path to resource, works for dev and for PyInstaller 
    base_path = getattr(sys, '_MEIPASS', os.path.dirname(os.path.abspath(__file__)))
    return os.path.join(base_path, relative_path)

class CreateProjectG2553():

	def __init__(self, project_path):

		# project_path is the whole path of the project
		# i.e. C:\MSP430G2553\Lab01
	    # folder and files need to be created in folder Lab01/Lab01Project

	    # folder - Lab01Project 
	    # folder - .settings (create an empty folder in Lab01Project)
	    # folder - Debug (create an empty folder in Lab01Project)

	    # file   - .ccsproject (no modification needed in Lab01Project)
	    # file   - lnk_msp430g2553.cmd (no modification needed in Lab01Project)
	    # file   - MSP430G2553.ccxml (no modification needed in Lab01Project)
	    # file   - .cproject (replace string "Lab0" to "project_name" in Lab01Project)
	    # file   - .project (replace string "Lab0" to "project_name" in Lab01Project)

	    # file   - UART.h (no modification needed in Lab01)
	    # file   - UARTFuncs.c (no modification needed in Lab01)
	    # file   - user_newproject.c -> user_<project_name>.c in Lab01

		self.project_path = project_path + "\\"
		self.project_name = project_path.split("\\")[-1]

		# the path of project_nameProject folder
		self.projectNameProject_path = self.project_path + self.project_name + "Project\\" 

		# the path of .setting folder
		self.setting_path = self.projectNameProject_path + ".settings\\"

		# the path of Debug folder
		self.debug_path = self.projectNameProject_path + "Debug\\"

		self.project_path_exist = None

		# create project folder and the .setting and Debug folders
		if os.path.exists(self.project_path):
			# warning of similar project has been created!
			messagebox.showwarning("Invalide Project name", "Project " + self.project_name.upper() + " or " + \
				                   self.project_name.lower() +  " or similar has been used! " + \
				                  "Please choose another project name!")
			self.project_path_exist = True

			"""
			print("Project " + self.project_name.upper() + " or " + \
				  self.project_name.lower() +  
				  " or similar has been used! Please choose another project name!")
			"""
		else:
			os.makedirs(self.setting_path)
			os.makedirs(self.debug_path)
			#print(self.project_name + " project folders have been created successfully!")
			self.project_path_exist = False


	# copy no modificatin files
	def copy_project_files(self):

	    # file   - .ccsproject (no modification needed in Lab01Project)
	    # file   - lnk_msp430g2553.cmd (no modification needed in Lab01Project)
	    # file   - MSP430G2553.ccxml (no modification needed in Lab01Project)
	    # file   - UART.h (no modification needed in Lab01)
	    # file   - UARTFuncs.c (no modification needed in Lab01)

	    #src_dir = os.path.dirname(__file__) + '/' + "/G2553DefaultProject" 
	    #src_dir = os.path.abspath(src_dir)

	    ccsproject_path = resource_path(".\\G2553DefaultProject\\.ccsproject") 
	    lnk_msp430g2553_path = resource_path(".\\G2553DefaultProject\\lnk_msp430g2553.cmd")
	    MSP430G2553_path = resource_path(".\\G2553DefaultProject\\MSP430G2553.ccxml")
	    UART_path = resource_path(".\\G2553DefaultProject\\UART.h")
	    UARTFuncs_path = resource_path(".\\G2553DefaultProject\\UARTFuncs.c")

	    shutil.copy(ccsproject_path, self.projectNameProject_path)
	    shutil.copy(lnk_msp430g2553_path, self.projectNameProject_path)
	    shutil.copy(MSP430G2553_path, self.projectNameProject_path)
	    shutil.copy(UART_path, self.project_path)
	    shutil.copy(UARTFuncs_path, self.project_path)

	    #print(".ccsproject file is created successfully!")
	    #print("lnk_msp430g2553.cmd file is created successfully!")
	    #print("MSP430G2553.ccxml file is created successfully!")
	    #print("UART.h file is created successfully!")
	    #print("UARTFuncs.c file is created successfully!")


	def create_project_files(self):

	    # file   - .cproject (replace string "Lab0" to "project_name" in Lab01Project)
	    # file   - .project (replace string "Lab0" to "project_name" in Lab01Project)
	    # file   - user_newproject.c -> user_<project_name>.c in Lab01

	    user_src_c_path = resource_path(".\\G2553DefaultProject\\user_newproject.c") 
	    user_dst_c_path = self.project_path + "user_" + self.project_name + ".c"
	    shutil.copy(user_src_c_path, user_dst_c_path)
	    #print("user_" + self.project_name + ".c file is created successfully!")

	    # copy and modify .project file
	    project_file_path = resource_path(".\\G2553DefaultProject\\.project")
	    shutil.copy(project_file_path, self.projectNameProject_path)
	    with open(self.projectNameProject_path + "\\.project", "r") as proj_f:
	    	project_file_content = proj_f.read().replace('Lab0', self.project_name)
	    with open(self.projectNameProject_path + "\\.project", "w") as proj_f:
	    	proj_f.write(project_file_content)
	    #print(".project file is created successfully!")

	    # copy and modify .cproject file
	    cproject_path = resource_path(".\\G2553DefaultProject\\.cproject")
	    shutil.copy(cproject_path, self.projectNameProject_path)
	    with open(self.projectNameProject_path + "\\.cproject", "r") as cproject_f:
	    	cproject_content = cproject_f.read().replace('Lab0', self.project_name)
	    with open(self.projectNameProject_path + "\\.cproject", "w") as cproject_f:
	    	cproject_f.write(cproject_content)
	    #print(".cproject file is created successfully!")


	def create_project(self):
		if not self.project_path_exist:
			self.create_project_files()
			self.copy_project_files()
			messagebox.showinfo("MSP430G2553 Project", "Project " + self.project_name + " has been created successfully!")	
			#print("All done!\n")
		
