#!/usr/bin/env python3
import os
import subprocess
import sys
import time

def execute_command(command):
    process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
    output, error = process.communicate()
    return process.returncode, output.decode().strip()

def process_c_files(directory, headercmd):
    c_files = [filename for filename in os.listdir(directory) if filename.endswith('.c')]
    failed = 0
    passed = 0
    compile_failed = 0
    
    green_color = '\033[92m'
    red_color = '\033[91m'
    blue_color = '\033[94m'
    reset_color = '\033[0m'
    for c_file in c_files:
        c_file_path = os.path.join(directory, c_file)
        compile_command = f'{headercmd} {c_file_path}'
        returncode, output = execute_command(compile_command)
        if returncode != 0:
            print(f'[{red_color}  FAILED  {reset_color}] {c_file_path}')
            print(f'Compilation failed for {c_file_path}')
            compile_command = f'clang {c_file_path}'
            returncode2, output_aout = execute_command(compile_command)
            if returncode2 != 0:
                print('err come from src')
            else:
                print('err come from mclang')
            compile_failed = compile_failed + 1
        else:
            # execute_command('./a.out')
            print(f'[{blue_color} RUN      {reset_color}] {c_file_path}')
            start_time = time.perf_counter()
            returncode_aout, output_aout = execute_command('./a.out')
            end_time = time.perf_counter()
            # ms
            elapsed_time = (end_time - start_time) * 1000
            compile_command = f'clang {c_file_path}'
            execute_command(compile_command)
            returncode_aout2, output_aout = execute_command('./a.out')

            if returncode_aout == returncode_aout2:
                print(f'[{green_color}       OK {reset_color}] {c_file_path} ({elapsed_time:.2f} ms)')
                passed = passed + 1
            else:
                print(f'[{red_color}  FAILED  {reset_color}] {c_file_path}')
                print(f'Expected: {returncode_aout2}')
                print(f'  Actual: {returncode_aout}')
                failed = failed + 1
    print()
    print(f'[  ======  ]')
    print(f'[{blue_color}  Passed  {reset_color}] {directory}: {passed} / {passed + failed + compile_failed}')
    if failed + compile_failed != 0:
        print (f'Failed test {failed + compile_failed} / {passed + failed + compile_failed}')
    if failed != 0:
        print (f'Run Failed test {failed} / {passed + failed + compile_failed}')
    if compile_failed != 0:
        print (f'Compile Failed test {compile_failed} / {passed + failed + compile_failed}')

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print('Usage: python script.py <directory>')
        sys.exit(1)
    directory = sys.argv[1]
    if not os.path.isdir(directory):
        print('Invalid directory')
        sys.exit(1)
    headercmd = "./build/mclang"
    process_c_files(directory, headercmd)
    print("\n \033[94m===== Open OPT =====\033[0m \n")
    headercmd = "./build/mclang -O3"
    process_c_files(directory, headercmd)