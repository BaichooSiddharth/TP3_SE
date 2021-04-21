import os
import subprocess


def run(cmd):
    try:
        program_output = subprocess.check_output(cmd, shell=True, universal_newlines=True, stderr=subprocess.STDOUT)
    except Exception as e:
        program_output = e.output
    print(program_output)
    return program_output.strip()

if os.path.exists("../tests/command_pt.in"):
    run("rm -rf ../src/command_pt.in")
    run("rm -rf ../src/command_tlb.in")
    run("cp ../tests/command_pt.in ../src/command_pt.in")
    run("cp ../tests/command_tlb.in ../src/command_tlb.in")
