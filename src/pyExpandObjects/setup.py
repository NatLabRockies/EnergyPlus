from setuptools import setup

setup(
    name='pyExpandObjects',
    version='0.1',
    packages=['src'],
    url='https://github.com/john-grando/pyExpandObjects',
    license='',
    author='GARD Analytics and NREL for US DOE',
    author_email='',
    description='Tool for Expanding EpJSON Templates',
    entry_points={
        'console_scripts': ['expandobjects=src.main:main'],
    }
)
