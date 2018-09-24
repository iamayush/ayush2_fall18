# -*- mode: python -*-

block_cipher = None


a = Analysis(['MSP430ProjectCreator.py'],
             pathex=['N:\\msp430\\me461\\Fall18\\MSP430ProjectCreatorFiles'],
             binaries=[],
             datas=[],
             hiddenimports=[],
             hookspath=[],
             runtime_hooks=[],
             excludes=[],
             win_no_prefer_redirects=False,
             win_private_assemblies=False,
             cipher=block_cipher)

a.datas += [('COECSL.gif', 'N:\\msp430\\me461\\Fall18\\MSP430ProjectCreatorFiles\\COECSL.gif', 'DATA')]
a.datas += [('CCS.ico', 'N:\\msp430\\me461\\Fall18\\MSP430ProjectCreatorFiles\\CCS.ico', 'DATA')]
a.datas += Tree('./F2272DefaultProject', prefix='F2272DefaultProject')
a.datas += Tree('./G2553DefaultProject', prefix='G2553DefaultProject')     

pyz = PYZ(a.pure, a.zipped_data,
             cipher=block_cipher)

exe = EXE(pyz,
          a.scripts,
          a.binaries,
          a.zipfiles,
          a.datas,
          name='MSP430ProjectCreator',
          debug=False,
          strip=False,
          upx=True,
          runtime_tmpdir=None,
          # console=False, program run without console in Windows
          console=False,
          # Add an icon to the program
          icon='N:\\msp430\\me461\\Fall18\\MSP430ProjectCreatorFiles\\CCS.ico')
