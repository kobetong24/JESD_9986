import os
import sys
import shutil
import platform
import argparse
import string
import re

root_path = os.path.dirname(os.path.abspath(__file__))
UsageDescription = "Run eval coverage test with specified generic"

# generate html
def generate_html(name, mode, use_7044, use_ce): 
    app_path = os.path.join(root_path, 'debug')
    os.chdir(app_path) 
    if os.path.exists('%s_app'%name):
        os.system('./%s_app %s %s %s'%(name, mode, use_7044, use_ce))
        os.system('gcov %s_app.c'%name)
        os.system('../../../../../lcov-1.13/bin/lcov -d . -t %s_test -o %s_eval_test.info -b . -c'%(name, name))
        os.system('../../../../../lcov-1.13/bin/genhtml -o %s_eval_ads9_gcov_html %s_eval_test.info'%(name, name))
    os.chdir(root_path) 
    
def main():
    parser = argparse.ArgumentParser(description = UsageDescription)
    parser.add_argument("-g", default = "ad9986")
    parser.add_argument("-m", default = "")
    parser.add_argument("-use_7044", default = "0", help = "1 | 0")
    parser.add_argument("-use_ce", default = "0", help = "1 | 0")
    args = parser.parse_args()    
    generic = args.g.lower()
    mode = args.m.lower()
    use_7044 = args.use_7044
    use_ce = args.use_ce

    os.chdir(root_path)        
    generate_html(generic, mode, use_7044, use_ce)

if __name__ == '__main__':
    main()
